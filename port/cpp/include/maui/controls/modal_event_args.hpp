#pragma once
// maui::controls::modal_event_args         <=  Microsoft.Maui.Controls.ModalEventArgs
// maui::controls::modal_pushing_event_args <=  Microsoft.Maui.Controls.ModalPushingEventArgs
// maui::controls::modal_pushed_event_args  <=  Microsoft.Maui.Controls.ModalPushedEventArgs
// maui::controls::modal_popping_event_args <=  Microsoft.Maui.Controls.ModalPoppingEventArgs
// maui::controls::modal_popped_event_args  <=  Microsoft.Maui.Controls.ModalPoppedEventArgs
//
// The argument family of the Window modal-navigation events, ported from
// src/Controls/src/Core/ModalEventArgs.cs + ModalPushingEventArgs.cs + ModalPushedEventArgs.cs +
// ModalPoppingEventArgs.cs + ModalPoppedEventArgs.cs (one header for the small hierarchy — they are
// a single cluster). C#'s `Page Modal` carries the page whose navigation triggered the event; the
// port's modal pages are content_pages, so `modal` is a NON-owning content_page* (the navigation
// subsystem owns the page's lifetime, PROFILE §8).
//
// modal_popping_event_args is MUTABLE shared state across handlers — the window raises it as
// event<modal_popping_event_args&> so a subscriber can set `cancel` (C# ModalPoppingEventArgs.Cancel)
// and the pop is aborted (Window.OnModalPopping returns args.Cancel; the modal navigation then fires
// pop_canceled and leaves the modal in place).

namespace maui::controls
{
    class content_page;

    // C# ModalEventArgs (abstract base): carries the modal page. Non-owning (the navigation
    // subsystem owns it).
    struct modal_event_args
    {
        content_page* modal = nullptr;
    };

    // C# ModalPushingEventArgs — the modal is about to be pushed.
    struct modal_pushing_event_args : modal_event_args
    {
    };

    // C# ModalPushedEventArgs — the modal was just pushed.
    struct modal_pushed_event_args : modal_event_args
    {
    };

    // C# ModalPoppingEventArgs — the modal is about to be popped; a subscriber may cancel it.
    struct modal_popping_event_args : modal_event_args
    {
        bool cancel = false;
    };

    // C# ModalPoppedEventArgs — the modal was just popped.
    struct modal_popped_event_args : modal_event_args
    {
    };
} // namespace maui::controls
