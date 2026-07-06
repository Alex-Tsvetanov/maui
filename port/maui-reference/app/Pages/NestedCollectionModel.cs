// Shared data model for the nested_collection twin. Faithfully reproduces the shape of
// src/Controls/samples/Controls.Sample/Pages/Controls/CollectionViewGalleries/NestedGalleries/
// NestedCollectionViewGallery.xaml.cs's NestedItemSource/NestedCollectionViewModel (20 sources titled
// "Source 0".."Source 19", each holding a list of image/caption items), with two deliberate,
// documented reductions to satisfy docs/AUTHORING.md rule 8 (determinism — no wall-clock/randomness):
//   - the original's per-source item count is `new Random().Next(6, 15)` (non-deterministic); here it's
//     a fixed count per source (varying by source index only, so the page still shows different-length
//     inner lists, just reproducibly).
//   - the original's DemoFilteredItemSource stamps each item's Date with DateTime.Now.AddDays(n); the
//     Caption text ("{image}, {n}") is reproduced verbatim, Date is omitted (unused by any template).
namespace MauiReference.Pages;

public class NestedGalleryItem
{
    public NestedGalleryItem(string caption, string image, int index)
    {
        Caption = caption;
        Image = image;
        Index = index;
    }

    public string Caption { get; set; }
    public string Image { get; set; }
    public int Index { get; set; }

    public override string ToString() => $"Item: {Index}";
}

public class NestedItemSource
{
    static readonly string[] Images =
    {
        "cover1.jpg",
        "oasis.jpg",
        "photo.jpg",
        "Vegetables.jpg",
        "Fruits.jpg",
        "FlowerBuds.jpg",
        "Legumes.jpg",
    };

    public List<NestedGalleryItem> Items { get; set; }
    public string Title { get; set; }

    public NestedItemSource(string title, int count)
    {
        Items = new List<NestedGalleryItem>();
        for (int n = 0; n < count; n++)
        {
            var image = Images[n % Images.Length];
            Items.Add(new NestedGalleryItem($"{image}, {n}", image, n));
        }
        Title = title;
    }
}

public class NestedCollectionViewModel
{
    public List<NestedItemSource> Items { get; set; }

    public NestedCollectionViewModel()
    {
        Items = new List<NestedItemSource>();
        for (int n = 0; n < 20; n++)
        {
            // Deterministic stand-in for the original's Random.Next(6, 15): cycle 6..14 by source index.
            var count = 6 + (n % 9);
            Items.Add(new NestedItemSource($"Source {n}", count));
        }
    }
}
