// Shared data model for pages/gap_carousel_position_binding.xaml: exposes a fixed CurrentPosition so
// real MAUI's {Binding CurrentPosition} resolves to a concrete starting slide (deterministic per
// docs/AUTHORING.md rule 8).
namespace MauiReference.Pages;

public class GapCarouselPositionBindingViewModel
{
    public int CurrentPosition { get; set; } = 1;
}
