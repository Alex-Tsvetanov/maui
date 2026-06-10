#pragma once
// maui::hosting::add_maui_controls_handlers  <=  Microsoft.Maui.Controls.Hosting.AppHostBuilderExtensions
//   .AddControlsHandlers / AddMauiControlsHandlers (the v1 control set the port has)
//
// Registers the default control → handler table — the explicit, reflection-free analog of
// AddControlsHandlers, re-listing the same pairs the controls' MAUI_REGISTER_HANDLER registrars publish
// to the process-wide default registry. Re-listed explicitly because every built maui_app owns its OWN
// registry (build isolation: a test can replace a pair without cross-build cross-talk), and because the
// explicit list is the §6-preferred primitive (no OBJECT-library/whole-archive caveat).

namespace maui::hosting
{
    class i_maui_handlers_collection;

    i_maui_handlers_collection& add_maui_controls_handlers(i_maui_handlers_collection& handlers);
} // namespace maui::hosting
