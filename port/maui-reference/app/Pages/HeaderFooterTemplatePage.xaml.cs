// Hand-written code-behind (the GENERATED marker is intentionally absent so e2e.py gen leaves this file
// alone) — assigns the per-row PhotoItem source the ItemTemplate binds {Binding Image}/{Binding Caption},
// mirroring the original ExampleTemplates.PhotoTemplate() (each row shows its own image). The port twin is
// gallery_xaml/Views/header_footer_template.xaml.cpp + ViewModels/photo_items.hpp.
namespace MauiReference.Pages;

public partial class HeaderFooterTemplatePage : ContentPage
{
    public HeaderFooterTemplatePage()
    {
        InitializeComponent();

        var images = new[] { "cover1.jpg", "oasis.jpg", "photo.jpg" };
        var items = new List<PhotoItem>();
        for (int n = 0; n < images.Length; n++)
        {
            items.Add(new PhotoItem { Image = images[n], Caption = $"{images[n]}, {n}" });
        }
        ItemsCV.ItemsSource = items;
    }

    public sealed class PhotoItem
    {
        public string Image { get; set; } = "";
        public string Caption { get; set; } = "";
    }
}
