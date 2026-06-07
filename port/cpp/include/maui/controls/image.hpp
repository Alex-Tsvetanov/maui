#pragma once
// maui::controls::image  <=  Microsoft.Maui.Controls.Image (aspect + file source)
//
// A view that displays an image. Ported from Image.cs. Same API shape as the other controls: a bare-noun
// interface getter + method accessors backed by a private property<T> whose change flows through
// view::on_property_changed to the handler. This cut exposes the scaling mode (Aspect) and a file Source.
//
// source: the control OWNS the source as a property<std::shared_ptr<i_image_source>> (so a source change
// flows through the same value engine + on_property_changed → handler->update_value as every other
// property); i_image::source() returns the raw .get() (a borrow). Setting a different source instance
// (distinct shared_ptr) fires a change and re-runs map_source — the synchronous (re)load.
//
// OUT OF SCOPE this cut (deferred async source subsystem): IsLoading / IsAnimationPlaying / IsOpaque, the
// non-file source kinds (uri/stream/font), and async loading + caching.

#include <memory>
#include <utility>

#include "maui/controls/view.hpp"
#include "maui/core/aspect.hpp"
#include "maui/core/bindable_property.hpp"
#include "maui/core/i_image.hpp"
#include "maui/core/i_image_source.hpp"
#include "maui/core/property.hpp"

namespace maui::controls
{
    class image : public view<maui::core::i_image>
    {
    public:
        // Shared bindable-property descriptors (one instance per type, like Image.AspectProperty/.SourceProperty).
        static const maui::core::bindable_property<maui::core::aspect>& aspect_property();
        static const maui::core::bindable_property<std::shared_ptr<maui::core::i_image_source>>& source_property();

        // ---- i_image ----
        [[nodiscard]] maui::core::aspect aspect() const override
        {
            return aspect_.get();
        }
        // Raw borrow into the owned shared_ptr (null when unset). The control retains ownership.
        [[nodiscard]] maui::core::i_image_source* source() const override
        {
            return source_.get().get();
        }

        // ---- public setters (drive the handler via on_property_changed → update_value) ----
        void set_aspect(maui::core::aspect value)
        {
            aspect_.set(value);
        }
        // The control takes ownership of the source. Passing a distinct instance fires the change.
        void set_source(std::shared_ptr<maui::core::i_image_source> value)
        {
            source_.set(std::move(value));
        }

    private:
        maui::core::property<maui::core::aspect> aspect_{*this, aspect_property()};
        maui::core::property<std::shared_ptr<maui::core::i_image_source>> source_{*this, source_property()};
    };
} // namespace maui::controls
