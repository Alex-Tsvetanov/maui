// maui windows PIPELINE SMOKE app — a deployable .exe that exercises the deploy → launch → present →
// capture → score chain WITHOUT needing the WinUI 3 backend to exist yet.
//
// WHAT THIS IS NOT: this is NOT a parity backend and its Win32 painting is NOT the port's Windows
// render. MAUI's Windows backend is WinUI 3 (`Microsoft.UI.Xaml.Controls.Button`, `TextBlock` — see
// docs/WINDOWS_TOOLCHAIN.md), so a Win32/GDI window can never be visually compared against MAUI. Do
// not grow this into a backend; the fidelity lane is `MAUI_BACKEND=windows` built with MSVC on the
// guest. This file exists so the *plumbing* (vm_agent_windows.py + the host orchestrator) can be
// validated and debugged end-to-end before any of that lands.
//
// WHY IT IS STILL WORTH HAVING: it is cross-compilable from the macOS dev machine with mingw-w64
// (tools/parity/windows/build_smoke.sh), so the whole VM pipeline is testable the moment a Windows VM
// exists — no MSVC, no Windows App SDK, no guest build. It deliberately mirrors the real hosts'
// contract so the runner treats it like any other column:
//
//   - MAUI_SAMPLE_PAGE  selects what to draw (the real hosts mount that gallery page)
//   - MAUI_APPEARANCE   light|dark, driving the surface colour like the Android apphost does
//   - a FIXED 1024x800 client area, so `present --w 1024 --h 800` is a no-op resize on success
//   - deterministic, page-derived content, so two captures of the same page are byte-identical and a
//     mismatch means the pipeline moved something (the property a smoke test needs)
//
// Deterministic by construction: no time, no random, no animation, no network. Every pixel is a pure
// function of (page, appearance), which is what makes it a usable oracle for "did the capture path
// photograph the right window at the right size".

#include <windows.h>

#include <array>
#include <cstdint>
#include <string>
#include <string_view>

namespace
{
    constexpr int k_client_w = 1024; // must match the runner's `present --w/--h`
    constexpr int k_client_h = 800;

    struct theme
    {
        COLORREF surface; // page background
        COLORREF ink;     // primary text
        COLORREF muted;   // secondary text
        COLORREF accent;  // the swatch band
    };

    // Light/dark surfaces chosen to match what the other hosts use for an unstyled page, so a human
    // eyeballing the three columns sees the same field: white / #121212 (the Android apphost's pair).
    constexpr theme k_light{RGB(255, 255, 255), RGB(0, 0, 0), RGB(102, 102, 102), RGB(0, 120, 212)};
    constexpr theme k_dark{RGB(18, 18, 18), RGB(255, 255, 255), RGB(170, 170, 170), RGB(96, 205, 255)};

    std::string env_or(const char* name, std::string fallback)
    {
        // GetEnvironmentVariableA over getenv: the runner passes vars through CreateProcess, and this
        // reads the live block rather than a snapshot taken at CRT start.
        char buf[512];
        const DWORD n = GetEnvironmentVariableA(name, buf, static_cast<DWORD>(sizeof(buf)));
        if (n == 0 || n >= sizeof(buf))
        {
            return fallback;
        }
        return std::string(buf, n);
    }

    // FNV-1a over the page key — a stable per-page value so each page gets its own swatch row. Pure
    // and deterministic (no hashing library, no seeding), which keeps two runs byte-identical.
    std::uint32_t stable_hash(std::string_view s)
    {
        std::uint32_t h = 2166136261U;
        for (const unsigned char c : s)
        {
            h ^= c;
            h *= 16777619U;
        }
        return h;
    }

    struct app_state
    {
        std::string page;
        bool dark = false;
    };

    void draw(HDC dc, const app_state& st)
    {
        const theme& th = st.dark ? k_dark : k_light;
        RECT client{0, 0, k_client_w, k_client_h};

        HBRUSH bg = CreateSolidBrush(th.surface);
        FillRect(dc, &client, bg);
        DeleteObject(bg);

        SetBkMode(dc, TRANSPARENT);

        // A 1px frame at the very edge of the client area: if the capture is off by a pixel, cropped,
        // or includes window chrome, the frame is visibly clipped — a cheap geometry assertion that a
        // flat colour field could not give us.
        HPEN pen = CreatePen(PS_SOLID, 1, th.muted);
        HGDIOBJ old_pen = SelectObject(dc, pen);
        HGDIOBJ old_brush = SelectObject(dc, GetStockObject(NULL_BRUSH));
        Rectangle(dc, 0, 0, k_client_w, k_client_h);
        SelectObject(dc, old_brush);
        SelectObject(dc, old_pen);
        DeleteObject(pen);

        HFONT title = CreateFontW(-40, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_TT_PRECIS,
                                  CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Segoe UI");
        HFONT body = CreateFontW(-20, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_TT_PRECIS,
                                 CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Segoe UI");

        const std::wstring wpage(st.page.begin(), st.page.end());
        SelectObject(dc, title);
        SetTextColor(dc, th.ink);
        RECT r_title{40, 48, k_client_w - 40, 120};
        DrawTextW(dc, wpage.c_str(), -1, &r_title, DT_LEFT | DT_SINGLELINE);

        SelectObject(dc, body);
        SetTextColor(dc, th.muted);
        const std::wstring sub = std::wstring(L"maui windows pipeline smoke  ·  ") + (st.dark ? L"dark" : L"light") +
                                 L"  ·  1024x800 client";
        RECT r_sub{40, 122, k_client_w - 40, 170};
        DrawTextW(dc, sub.c_str(), -1, &r_sub, DT_LEFT | DT_SINGLELINE);

        // Per-page swatch row: 8 cells whose shades derive from the page hash. Identical for the same
        // page, different across pages — so a capture attributed to the wrong page is obvious, which
        // is precisely the bug that put one column's window into another's on macOS.
        const std::uint32_t h = stable_hash(st.page);
        for (int i = 0; i < 8; ++i)
        {
            const int cell_w = (k_client_w - 80) / 8;
            const int x = 40 + i * cell_w;
            const auto shade = static_cast<BYTE>(40 + ((h >> (i * 3)) & 0x7F));
            HBRUSH b = CreateSolidBrush(RGB(shade, GetGValue(th.accent) / (i + 1) + shade / 3, GetBValue(th.accent)));
            RECT cell{x, 200, x + cell_w - 8, 320};
            FillRect(dc, &cell, b);
            DeleteObject(b);
        }

        // A crosshair grid every 100px, so the runner's absolute click coordinates can be eyeballed
        // against the capture when a scenario misses its target.
        HPEN grid = CreatePen(PS_DOT, 1, th.muted);
        old_pen = SelectObject(dc, grid);
        for (int x = 100; x < k_client_w; x += 100)
        {
            MoveToEx(dc, x, 360, nullptr);
            LineTo(dc, x, k_client_h - 40);
        }
        for (int y = 400; y < k_client_h - 40; y += 100)
        {
            MoveToEx(dc, 40, y, nullptr);
            LineTo(dc, k_client_w - 40, y);
        }
        SelectObject(dc, old_pen);
        DeleteObject(grid);

        SelectObject(dc, body);
        SetTextColor(dc, th.ink);
        RECT r_foot{40, k_client_h - 76, k_client_w - 40, k_client_h - 40};
        DrawTextW(dc, L"- end of page -", -1, &r_foot, DT_LEFT | DT_SINGLELINE);

        DeleteObject(title);
        DeleteObject(body);
    }

    LRESULT CALLBACK wnd_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
    {
        auto* st = reinterpret_cast<app_state*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
        switch (msg)
        {
            case WM_PAINT: {
                PAINTSTRUCT ps;
                HDC dc = BeginPaint(hwnd, &ps);
                if (st != nullptr)
                {
                    draw(dc, *st);
                }
                EndPaint(hwnd, &ps);
                return 0;
            }
            case WM_ERASEBKGND:
                return 1; // we paint every pixel in WM_PAINT; skipping this avoids a white flash
            case WM_CLOSE:
                DestroyWindow(hwnd);
                return 0;
            case WM_DESTROY:
                PostQuitMessage(0);
                return 0;
            default:
                return DefWindowProcW(hwnd, msg, wp, lp);
        }
    }
} // namespace

int WINAPI wWinMain(HINSTANCE inst, HINSTANCE, PWSTR, int)
{
    // PER_MONITOR_AWARE_V2 so the client area is 1024x800 in PHYSICAL pixels and matches what
    // vm_agent_windows.py captures. Without this the guest's display scaling silently shrinks the
    // window's real pixel size and every capture is the wrong dimensions.
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    app_state st;
    st.page = env_or("MAUI_SAMPLE_PAGE", "smoke");
    st.dark = env_or("MAUI_APPEARANCE", "light") == "dark";

    // Zero-init THEN set cbSize: `WNDCLASSEXW wc{sizeof(wc)}` initializes only the first member and
    // trips -Wmissing-field-initializers (the port builds warning-free).
    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = wnd_proc;
    wc.hInstance = inst;
    wc.lpszClassName = L"MauiWindowsSmoke";
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    RegisterClassExW(&wc);

    // AdjustWindowRect so the CLIENT area is exactly 1024x800 (CreateWindow's size includes chrome).
    // The runner presents to 1024x800 outer, and PrintWindow captures the whole window, so what
    // matters is that the client is a known fixed size on every column.
    RECT want{0, 0, k_client_w, k_client_h};
    constexpr DWORD style = WS_OVERLAPPEDWINDOW & ~(WS_THICKFRAME | WS_MAXIMIZEBOX);
    AdjustWindowRect(&want, style, FALSE);

    const std::wstring wpage(st.page.begin(), st.page.end());
    const std::wstring caption = L"maui-smoke: " + wpage;
    HWND hwnd = CreateWindowExW(0, wc.lpszClassName, caption.c_str(), style, CW_USEDEFAULT, CW_USEDEFAULT,
                                want.right - want.left, want.bottom - want.top, nullptr, nullptr, inst, nullptr);
    if (hwnd == nullptr)
    {
        return 1;
    }
    SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(&st));
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return 0;
}
