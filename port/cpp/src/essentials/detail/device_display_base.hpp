#pragma once
// maui::devices::detail::device_display_base  <=
// Microsoft.Maui.Devices.DeviceDisplayImplementationBase (DeviceDisplay.shared.cs, internal): the
// abstract base C# itself uses for this feature. The first MainDisplayInfoChanged subscriber
// primes the metrics cache (SetCurrent(MainDisplayInfo)) and starts the platform listeners; the
// last removal stops them. on_main_display_info_changed() re-reads the metrics and raises only
// when they differ from the cache (DisplayInfo equality - refresh rate excluded).

#include "maui/core/event.hpp"
#include "maui/essentials/device_display.hpp"

namespace maui::devices::detail
{
    class device_display_base : public i_device_display
    {
    public:
        [[nodiscard]] display_info main_display_info() const override
        {
            return platform_get_main_display_info();
        }

        [[nodiscard]] bool keep_screen_on() const override
        {
            return platform_get_keep_screen_on();
        }

        void set_keep_screen_on(bool value) override
        {
            platform_set_keep_screen_on(value);
        }

        maui::core::connection_token add_main_display_info_changed(
            maui::core::move_only_function<void(const display_info&)> handler) override
        {
            if (subscribers_ == 0)
            {
                current_metrics_ = main_display_info();
                platform_start_screen_metrics_listeners();
            }
            ++subscribers_;
            return main_display_info_changed_.connect(std::move(handler));
        }

        bool remove_main_display_info_changed(maui::core::connection_token token) override
        {
            if (!main_display_info_changed_.disconnect(token))
            {
                return false;
            }
            if (--subscribers_ == 0)
            {
                platform_stop_screen_metrics_listeners();
            }
            return true;
        }

    protected:
        device_display_base() = default;

        // GetMainDisplayInfo / Get/SetKeepScreenOn / Start/StopScreenMetricsListeners.
        [[nodiscard]] virtual display_info platform_get_main_display_info() const = 0;
        [[nodiscard]] virtual bool platform_get_keep_screen_on() const = 0;
        virtual void platform_set_keep_screen_on(bool value) = 0;
        virtual void platform_start_screen_metrics_listeners() = 0;
        virtual void platform_stop_screen_metrics_listeners() = 0;

        // OnMainDisplayInfoChanged(): re-read, dedupe, raise.
        void on_main_display_info_changed()
        {
            const display_info metrics = main_display_info();
            if (!(metrics == current_metrics_))
            {
                current_metrics_ = metrics;
                main_display_info_changed_.raise(metrics);
            }
        }

    private:
        maui::core::event<display_info> main_display_info_changed_;
        int subscribers_ = 0;
        display_info current_metrics_;
    };
} // namespace maui::devices::detail
