#pragma once
// maui::controls::image  <=  Microsoft.Maui.Controls.Image
//
// A view that displays an image. Ported from Image.cs. Same API shape as the other controls: a bare-noun
// interface getter + method accessors backed by a private property<T> whose change flows through
// view::on_property_changed to the handler. Exposes the scaling mode (Aspect), the Source (file / uri /
// stream / font), IsOpaque, IsAnimationPlaying, and the read-only IsLoading.
//
// source: the control OWNS the source as a property<std::shared_ptr<i_image_source>> (so a source change
// flows through the same value engine + on_property_changed → handler->update_value as every other
// property); i_image::source() returns the raw .get() (a borrow). Setting a distinct source instance fires
// a change and re-runs map_source (file = synchronous, uri/stream/font = async via the loader).
//
// is_opaque / is_animation_playing: bindable + mapped, defaulting to false (C# ImageElement defaults).
// is_loading: a READ-ONLY state pushed by the handler/loader via update_is_loading (C# Image.IsLoading is a
// read-only bindable set through IImageSourcePart.UpdateIsLoading). is_loading() reads it; there is no
// public setter — the load lifecycle owns it. The write fires a property change so an observer can react.
//
// DEVIATION: the native GIF multi-frame animation behind IsAnimationPlaying is not implemented (see
// i_image.hpp / image_handler.hpp). Async loading + caching + font sources are now supported.

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
        // Declare the style TargetType so an implicit / class style targeting `image` matches this control.
        image()
        {
            this->set_style_target_type<image>();
        }

        // Shared bindable-property descriptors (one instance per type, like Image.AspectProperty/.SourceProperty).
        static const maui::core::bindable_property<maui::core::aspect>& aspect_property();
        static const maui::core::bindable_property<std::shared_ptr<maui::core::i_image_source>>& source_property();
        static const maui::core::bindable_property<bool>& is_opaque_property();
        static const maui::core::bindable_property<bool>& is_animation_playing_property();
        static const maui::core::bindable_property<bool>& is_loading_property();

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
        [[nodiscard]] bool is_opaque() const override
        {
            return is_opaque_.get();
        }
        [[nodiscard]] bool is_animation_playing() const override
        {
            return is_animation_playing_.get();
        }
        // The loader pushes its in-flight state here (C# IImageSourcePart.UpdateIsLoading) → drives the
        // read-only IsLoading. Written at manual specificity (the control's own internal state).
        void update_is_loading(bool is_loading) override
        {
            is_loading_.set(is_loading);
        }

        // ---- read-only loading state (C# Image.IsLoading) ----
        [[nodiscard]] bool is_loading() const
        {
            return is_loading_.get();
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
        void set_is_opaque(bool value)
        {
            is_opaque_.set(value);
        }
        void set_is_animation_playing(bool value)
        {
            is_animation_playing_.set(value);
        }

    private:
        maui::core::property<maui::core::aspect> aspect_{*this, aspect_property()};
        maui::core::property<std::shared_ptr<maui::core::i_image_source>> source_{*this, source_property()};
        maui::core::property<bool> is_opaque_{*this, is_opaque_property()};
        maui::core::property<bool> is_animation_playing_{*this, is_animation_playing_property()};
        maui::core::property<bool> is_loading_{*this, is_loading_property()};
    };
} // namespace maui::controls
