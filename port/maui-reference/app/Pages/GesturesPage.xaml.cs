// Hand-written partial for pages/gestures.xaml — the GENERATED marker is deliberately ABSENT so
// `e2e.py gen` leaves this file alone (see docs/AUTHORING.md; the generated stub it replaced said so).
//
// WHY THIS EXISTS. The twin declares five GestureRecognizers but wired no handlers, while the code-first
// builder page (port/cpp/examples/gallery/pages/gestures_page.hpp) updates a readout on every gesture
// event. At rest both columns read "Last gesture: (none)", so the still comparison never saw the gap —
// but the moment a scenario DRIVES the page, only the port's column would change and the board would
// report MOTION MISMATCH, blaming the port for this file's omission.
//
// WIRED VIA x:Name, NOT VIA XAML EVENT ATTRIBUTES, and that is a hard constraint rather than a style
// choice: pages/gestures.xaml is the SAME file the C++ loader hydrates for the cpp_xaml column, and that
// loader refuses event attributes loudly (xaml_loader.event_wiring_is_a_loud_deferral). A
// `Tapped="OnTapped"` here would break that column outright.
//
// THE STRINGS ARE A CONTRACT, not a description. They must match gestures_page.hpp byte for byte or the
// two columns diverge on a driven frame — which is the very failure this file exists to prevent. Note
// the two format strings, because C#'s default ToString() matches NEITHER:
//   "Pan {0:F0},{1:F0}"  — snprintf "%.0f", so NO decimal point at all (Pan 12,-3)
//   "Pinch x{0:F2}"      — snprintf "%.2f", so exactly two (Pinch x1.25)
// and "Pan canceled" is the US spelling with one 'l', matching the C++ side.
namespace MauiReference.Pages;

public partial class GesturesPage : ContentPage
{
    public GesturesPage()
    {
        InitializeComponent();

        // Reached through the collection rather than by naming each recognizer, so the XAML needs only
        // the one x:Name on the target — and the port's loader is never asked to put a name on a
        // non-view element.
        foreach (var recognizer in GestureTarget.GestureRecognizers)
        {
            switch (recognizer)
            {
                case TapGestureRecognizer tap:
                    tap.Tapped += (_, _) => SetReadout("Tapped");
                    break;
                case PanGestureRecognizer pan:
                    pan.PanUpdated += (_, e) => SetReadout(e.StatusType switch
                    {
                        GestureStatus.Started => "Pan started",
                        GestureStatus.Running => $"Pan {e.TotalX:F0},{e.TotalY:F0}",
                        GestureStatus.Completed => "Pan completed",
                        _ => "Pan canceled",
                    });
                    break;
                case PinchGestureRecognizer pinch:
                    pinch.PinchUpdated += (_, e) =>
                    {
                        if (e.Status == GestureStatus.Running)
                        {
                            SetReadout($"Pinch x{e.Scale:F2}");
                        }
                    };
                    break;
                case SwipeGestureRecognizer swipe:
                    swipe.Swiped += (_, e) => SetReadout($"Swiped {e.Direction}");
                    break;
                case PointerGestureRecognizer pointer:
                    pointer.PointerEntered += (_, _) => SetReadout("Pointer entered");
                    pointer.PointerMoved += (_, _) => SetReadout("Pointer moved");
                    pointer.PointerPressed += (_, _) => SetReadout("Pointer pressed");
                    pointer.PointerReleased += (_, _) => SetReadout("Pointer released");
                    pointer.PointerExited += (_, _) => SetReadout("Pointer exited");
                    break;
            }
        }
    }

    // gestures_page.hpp:208-213 — the label is always the prefix plus the gesture name, never the
    // gesture name alone.
    void SetReadout(string gesture) => Readout.Text = "Last gesture: " + gesture;
}
