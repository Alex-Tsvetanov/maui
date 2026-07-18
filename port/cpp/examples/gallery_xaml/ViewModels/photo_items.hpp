#pragma once
// photo_items.hpp — bindable item data for the gallery_xaml code-behind of header_footer_template.xaml.
//
// The original C# ExampleTemplates.PhotoTemplate() binds each row's {Binding Image} and {Binding Caption}
// off a CollectionViewGalleryTestItem. The shared markup sets NO ItemsSource; the hand-written code-behind
// (header_footer_template.xaml.cpp) assigns it, so — like super_teams.hpp — the item objects must be
// bindable_objects that REGISTER the bound property names (boxed_item::of stores a shared_ptr<bindable_object>
// item as bindable, so the ItemTemplate's binding context resolves {Binding Image}/{Binding Caption} against
// it). Each photo_item carries its OWN image (cover1.jpg / oasis.jpg / photo.jpg), so the item cell renders
// the real per-row image, matching original MAUI (not a single fixed source).
//
// Image is a std::shared_ptr<i_image_source>, NOT a std::string: the loader registers image.Source as a
// binding-only bindable_property<shared_ptr<i_image_source>> with NO string→ImageSource text converter (see
// register_xaml_extra_converters.cpp), so a string {Binding Image} would never resolve to an image and the
// cell would show a caption with no thumbnail. Binding an actual image_source::from_file(...) makes the
// std::any value match image::source_property()'s type exactly, so try_set_value applies it directly.

#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "maui/controls/file_image_source.hpp"
#include "maui/core/bindable_object.hpp"
#include "maui/core/i_image_source.hpp"
#include "maui/core/observable.hpp"

namespace examples::ViewModels
{
    // One demo row — the ItemTemplate binds {Binding Image} (the per-row photo file) and {Binding Caption}.
    class photo_item : public maui::core::bindable_object
    {
    public:
        photo_item(std::string image_file, std::string caption)
        {
            Image.set(maui::controls::image_source::from_file(std::move(image_file)));
            Caption.set(std::move(caption));
        }
        maui::core::observable<std::shared_ptr<maui::core::i_image_source>> Image{*this, "Image"};
        maui::core::observable<std::string> Caption{*this, "Caption"};
    };

    // DemoFilteredItemSource(3): three rows captioned "<image>, <n>", each carrying its own image file.
    [[nodiscard]] inline std::vector<std::shared_ptr<photo_item>> photo_items()
    {
        static const std::vector<std::string> images{"cover1.jpg", "oasis.jpg", "photo.jpg"};
        std::vector<std::shared_ptr<photo_item>> rows;
        rows.reserve(images.size());
        for (std::size_t n = 0; n < images.size(); ++n)
        {
            rows.push_back(std::make_shared<photo_item>(images[n], images[n] + ", " + std::to_string(n)));
        }
        return rows;
    }
} // namespace examples::ViewModels
