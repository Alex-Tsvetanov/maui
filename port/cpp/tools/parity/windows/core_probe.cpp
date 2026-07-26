// maui windows CORE PROBE — a console .exe that proves the port's cross-platform core WORKS on Windows,
// not merely that it compiles.
//
// Lane 2 (docs/WINDOWS_TOOLCHAIN.md): built by tools/parity/windows/build_core_check.sh with mingw-w64
// against the port's platform-independent half plus the headless handler mirrors. It renders nothing and
// is NOT a parity artifact — the headless mirrors have no widgets. What it does exercise is everything
// underneath a backend: the property/bindable system, control construction, text measurement, the
// handler mapper dispatch, the generic mount, and a full measure+arrange pass.
//
// It SELF-CHECKS rather than just printing: every step asserts a concrete expectation and the process
// exits non-zero on the first failure, so it is usable as a guest-side smoke test (and by CI later)
// instead of something a human has to read. Each line is `ok  <name>: <detail>` / `FAIL <name>: <why>`.
//
// Deliberately no <print>: mingw's libstdc++ does not link std::print (missing __open_terminal /
// __write_to_terminal), so this uses std::format + fputs. See the doc's mingw findings.

#include <cstdio>
#include <format>
#include <limits>
#include <memory>
#include <string>

#include "maui/controls/application.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/window.hpp"
#include "maui/core/i_window.hpp"
#include "maui/graphics/color.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"
#include "maui/hosting/app_host.hpp"
#include "maui/hosting/maui_app.hpp"
#include "maui/hosting/maui_app_builder.hpp"
#include "maui/ui.hpp"

namespace ui = maui::ui;

namespace
{
    int g_failures = 0;

    void ok(const std::string& name, const std::string& detail)
    {
        std::fputs(std::format("  ok  {}: {}\n", name, detail).c_str(), stdout);
    }

    void fail(const std::string& name, const std::string& why)
    {
        ++g_failures;
        std::fputs(std::format("  FAIL {}: {}\n", name, why).c_str(), stdout);
    }

    void check(const std::string& name, bool cond, const std::string& detail)
    {
        if (cond)
        {
            ok(name, detail);
        }
        else
        {
            fail(name, detail);
        }
    }

    // ---- 1. graphics + property system, no app at all -------------------------------------------
    void probe_graphics()
    {
        // A colour round-trip through the port's own accessors: cheap, but it is the layer
        // everything else sits on, so a broken float/endianness assumption shows up here first.
        // color's channels are public float members (not accessors) — see graphics/color.hpp.
        const maui::graphics::color c = maui::graphics::color::from_rgba(0.25F, 0.5F, 0.75F, 1.0F);
        const bool colour_ok = c.red > 0.24F && c.red < 0.26F && c.blue > 0.74F && c.blue < 0.76F;
        check("graphics.color", colour_ok,
              std::format("rgba({:.2f},{:.2f},{:.2f},{:.2f})", c.red, c.green, c.blue, c.alpha));

        const maui::graphics::rect r{10, 20, 100, 50};
        check("graphics.rect", r.width == 100 && r.height == 50 && r.x == 10 && r.y == 20,
              std::format("{{{},{},{},{}}}", r.x, r.y, r.width, r.height));
    }

    // ---- 2. a control's text measurement (the headless font-metric path) ------------------------
    void probe_label_measure()
    {
        maui::controls::label l;
        l.set_text("Hello, MAUI C++ on Windows");
        constexpr double inf = std::numeric_limits<double>::infinity();
        const maui::graphics::size d = l.measure(inf, inf);
        // A non-degenerate desired size is the real assertion: a zero here means the text-measurement
        // path silently returned nothing, which would make every layout on Windows collapse.
        check("label.measure", d.width > 0 && d.height > 0,
              std::format("desired {:.1f}x{:.1f} for {} chars", d.width, d.height, l.text().size()));

        maui::controls::label empty;
        const maui::graphics::size de = empty.measure(inf, inf);
        check("label.measure(empty)", de.width >= 0 && de.height >= 0,
              std::format("desired {:.1f}x{:.1f}", de.width, de.height));
    }

    // ---- 3. the whole hosting path: builder -> app -> window -> mount -> layout ------------------
    class probe_app : public ui::app
    {
    public:
        probe_app()
        {
            set_content(ui::page(ui::label("core probe")));
            set_title("maui core probe");
        }
    };

    void probe_hosting()
    {
        // Mirrors src/platform/headless/host_run.cpp's run_app body (the sequence every backend shares),
        // but keeps the pieces so the geometry can be asserted instead of discarded.
        maui::hosting::maui_app_builder builder = maui::hosting::maui_app::create_builder();
        builder.use_maui_app<probe_app>();
        const std::unique_ptr<maui::hosting::maui_app> app = builder.build();
        if (app == nullptr)
        {
            fail("hosting.build", "maui_app_builder::build() returned null");
            return;
        }
        ok("hosting.build", "maui_app built");

        const std::shared_ptr<maui::controls::application>& application = app->application();
        if (application == nullptr)
        {
            fail("hosting.application", "app->application() is null");
            return;
        }
        auto* window = dynamic_cast<maui::controls::window*>(application->create_window());
        if (window == nullptr)
        {
            fail("hosting.window", "create_window() did not yield a controls::window");
            return;
        }
        ok("hosting.window", "window created");

        maui::hosting::mount_window(*app, *window);
        ok("hosting.mount", "mount_window completed (handlers attached across the tree)");

        constexpr double w = 1024;
        constexpr double h = 800;
        const maui::graphics::size settled = maui::hosting::drive_layout(*window, w, h);
        // The layout must actually consume the offered viewport. A zero/NaN here is the failure mode that
        // matters: it means measure+arrange ran but produced nothing, and every capture downstream would
        // be an empty window that still looks like a legitimate screenshot.
        const bool sane = settled.width > 0 && settled.height > 0 && settled.width <= w + 1 && settled.height <= h + 1;
        check("hosting.layout", sane,
              std::format("settled {:.1f}x{:.1f} for offered {:.0f}x{:.0f}", settled.width, settled.height, w, h));

        // A second pass must be stable — a differing result means layout is not idempotent, which on a
        // real backend shows up as content shifting between two captures of the same page.
        const maui::graphics::size again = maui::hosting::drive_layout(*window, w, h);
        check("hosting.layout(stable)", again.width == settled.width && again.height == settled.height,
              std::format("second pass {:.1f}x{:.1f}", again.width, again.height));
    }
} // namespace

int main()
{
    std::fputs("maui core probe (windows, mingw cross-build, headless mirrors)\n", stdout);
    probe_graphics();
    probe_label_measure();
    probe_hosting();
    if (g_failures != 0)
    {
        std::fputs(std::format("{} FAILURE(S)\n", g_failures).c_str(), stdout);
        return 1;
    }
    std::fputs("PASS - the cross-platform core runs on Windows\n", stdout);
    return 0;
}
