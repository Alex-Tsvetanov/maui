#pragma once
// maui::samples::effects_page — ports EffectsPage.xaml (Maui.Controls.Sample.Pages.EffectsPage).
//
// The C# page shows two entries, each carrying a focus effect via Entry.Effects: a FocusRoutingEffect
// (a RoutingEffect — the platform-independent wrapper that resolves an inner platform effect by id) and
// a FocusPlatformEffect (the platform effect directly). The effect's job on the real platforms is to
// recolor the entry's background on focus; the page itself has no code-behind beyond InitializeComponent.
//
// This port reproduces the G3 effect-attach machinery code-first and surfaces it through a readout (the
// gallery convention): each entry gets an effect added to element.effects(); the readout reports the
// attach state, and buttons detach / re-attach the routing effect so the attach/detach lifecycle is
// visible. The native focus-recolor is a deferred platform concern (headless has no native view to
// recolor) — the demonstrated, testable behavior is the effect lifecycle (is_attached / inner-resolution
// / element-property-changed forwarding), which is fully exercised here.
//
// The page OWNS its whole element tree (the sample_app pattern). It is backend-agnostic — a sample main
// hosts page() in a window; the headless test tree exercises the same wiring deterministically.
//
// Interactions demonstrated:
//   - the first entry carries a routing_effect (a focus_routing_effect, our FocusRoutingEffect analog),
//     whose inner() resolves through the explicit effect registry to a focus_platform_effect,
//   - the second entry carries a focus_platform_effect directly (the FocusPlatformEffect analog),
//   - "Detach routing effect" removes it from the entry's effects collection (send_detached fires;
//     is_attached flips false); "Re-attach" adds it back (send_attached fires) — the readout tracks both,
//   - editing either entry routes a property change through the element to the attached effects
//     (send_on_element_property_changed), the C# OnElementPropertyChanged seam.
//
// note: C#'s FocusRoutingEffect / FocusPlatformEffect recolor the native control on focus; that native
//       behavior is per-backend and deferred. The port's focus_platform_effect records its lifecycle
//       (attached/detached + property-changed count) so the readout can show the effect is live without a
//       native view — fidelity is in the lifecycle, not the (absent) headless pixel.

#include <memory>
#include <string>

#include "maui/controls/button.hpp"
#include "maui/controls/content_page.hpp"
#include "maui/controls/effect.hpp"
#include "maui/controls/effect_registry.hpp"
#include "maui/controls/entry.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/platform_effect.hpp"
#include "maui/controls/routing_effect.hpp"
#include "maui/controls/vertical_stack_layout.hpp"

namespace maui::samples
{
    // FocusPlatformEffect analog: a platform_effect that records its lifecycle. On a real backend its
    // OnAttached would recolor the native control on focus; headless has no native view, so it records the
    // attach/detach + element-property-changed so the readout can report a live effect (see header note).
    class focus_platform_effect : public maui::controls::platform_effect<void, void>
    {
    public:
        [[nodiscard]] bool attached() const
        {
            return attached_;
        }
        [[nodiscard]] int property_changed_count() const
        {
            return property_changed_count_;
        }

    protected:
        void on_attached() override
        {
            attached_ = true;
        }
        void on_detached() override
        {
            attached_ = false;
        }
        void on_element_property_changed(std::string_view /*name*/) override
        {
            ++property_changed_count_;
        }

    private:
        bool attached_ = false;
        int property_changed_count_ = 0;
    };

    // FocusRoutingEffect analog: a RoutingEffect resolving an inner platform effect by id (C# resolves an
    // [ExportEffect] type; the port resolves an explicitly-registered factory — registered in the ctor).
    // The resolution id mirrors the "ResolutionGroupName.ExportEffect" string a developer passes.
    inline constexpr const char* k_focus_effect_id = "MauiSample.FocusEffect";

    class focus_routing_effect : public maui::controls::routing_effect
    {
    public:
        focus_routing_effect() : routing_effect(k_focus_effect_id)
        {
        }
    };

    class effects_page
    {
    public:
        effects_page()
        {
            // Register the inner platform effect the routing effect resolves (the [ExportEffect] analog).
            // Idempotent: re-running the sample / tests just replaces the factory (registry is last-wins).
            maui::controls::register_effect(k_focus_effect_id, [] {
                return std::shared_ptr<maui::controls::effect>(std::make_shared<focus_platform_effect>());
            });

            page_.set_title("Effects");
            stack_.set_spacing(8);
            readout_.set_text("Ready");

            // ---- entry 1: a routing effect (FocusRoutingEffect) ----------------------------------------
            routing_label_.set_text("Entry With Focus Routing Effect");
            entry1_.set_text("Alert Simple");
            entry1_.set_placeholder("Routing effect entry");
            entry1_.text_changed.connect([this](const std::string& /*old_text*/, const std::string& /*new_text*/) {
                update_readout("entry1 changed");
            });

            // ---- entry 2: a platform effect directly (FocusPlatformEffect) -----------------------------
            platform_label_.set_text("Entry With Focus Platform Effect");
            entry2_.set_text("Alert Simple");
            entry2_.set_placeholder("Platform effect entry");
            entry2_.text_changed.connect([this](const std::string& /*old_text*/, const std::string& /*new_text*/) {
                update_readout("entry2 changed");
            });

            // ---- the lifecycle buttons (detach / re-attach the routing effect) -------------------------
            detach_btn_.set_text("Detach routing effect");
            detach_btn_.clicked.connect([this] { detach_routing_effect(); });
            attach_btn_.set_text("Re-attach routing effect");
            attach_btn_.clicked.connect([this] { attach_routing_effect(); });

            stack_.add(routing_label_);
            stack_.add(entry1_);
            stack_.add(platform_label_);
            stack_.add(entry2_);
            stack_.add(detach_btn_);
            stack_.add(attach_btn_);
            stack_.add(readout_);
            page_.set_content(stack_);

            // Attach both effects to their entries via the Effects collection (Entry.Effects in XAML). The
            // routing effect is held in routing_effect_ so detach/re-attach can find it by pointer.
            attach_routing_effect();
            platform_effect_ = std::make_shared<focus_platform_effect>();
            entry2_.effects().add(platform_effect_);
        }

        [[nodiscard]] maui::controls::content_page& page()
        {
            return page_;
        }

        // The owned views/effects, exposed for the hosting main + headless tests.
        [[nodiscard]] maui::controls::label& readout()
        {
            return readout_;
        }
        [[nodiscard]] maui::controls::entry& routing_entry()
        {
            return entry1_;
        }
        [[nodiscard]] maui::controls::entry& platform_entry()
        {
            return entry2_;
        }
        [[nodiscard]] maui::controls::effect* routing_effect_ptr() const
        {
            return routing_effect_.get();
        }

    private:
        void attach_routing_effect()
        {
            if (routing_effect_ && entry1_.effects().contains(routing_effect_.get()))
            {
                return; // already attached
            }
            routing_effect_ = std::make_shared<focus_routing_effect>();
            entry1_.effects().add(routing_effect_);
            update_readout("routing effect attached");
        }

        void detach_routing_effect()
        {
            if (routing_effect_ && entry1_.effects().remove(routing_effect_.get()))
            {
                update_readout("routing effect detached");
            }
            else
            {
                update_readout("routing effect already detached");
            }
        }

        void update_readout(const std::string& what)
        {
            const bool routing_on = routing_effect_ && entry1_.effects().contains(routing_effect_.get());
            readout_.set_text(what + " — routing attached: " + (routing_on ? "yes" : "no"));
        }

        maui::controls::content_page page_;
        maui::controls::vertical_stack_layout stack_;
        maui::controls::label routing_label_;
        maui::controls::entry entry1_;
        maui::controls::label platform_label_;
        maui::controls::entry entry2_;
        maui::controls::button detach_btn_;
        maui::controls::button attach_btn_;
        maui::controls::label readout_;

        // The effects (subscribers): owned here as shared_ptr so detach/re-attach can re-add the same
        // object; the entries' effects collections share ownership while attached (§8).
        std::shared_ptr<maui::controls::effect> routing_effect_;
        std::shared_ptr<maui::controls::effect> platform_effect_;
    };
} // namespace maui::samples
