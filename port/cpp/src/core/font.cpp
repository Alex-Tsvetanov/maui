// maui::core::font (font.hpp).
#include "maui/core/font.hpp"

#include <cmath>
#include <string>
#include <string_view>
#include <utility>

namespace maui::core
{
    font::font(std::string family, double size, font_slant slant, font_weight weight, bool enable_scaling)
        : family_(std::move(family)), size_(size), slant_(slant), weight_(weight), disable_scaling_(!enable_scaling)
    {
    }

    const std::string& font::family() const
    {
        return family_;
    }
    double font::size() const
    {
        return size_;
    }
    font_slant font::slant() const
    {
        return slant_;
    }
    font_weight font::weight() const
    {
        return weight_;
    }
    bool font::auto_scaling_enabled() const
    {
        return !disable_scaling_;
    }
    bool font::is_default() const
    {
        return family_.empty() && (size_ <= 0.0 || std::isnan(size_)) && slant_ == font_slant::normal &&
               weight() == font_weight::regular;
    }

    font font::with_size(double new_size) const
    {
        return {family_, new_size, slant_, weight(), auto_scaling_enabled()};
    }
    font font::with_slant(font_slant new_slant) const
    {
        return {family_, size_, new_slant, weight(), auto_scaling_enabled()};
    }
    font font::with_weight(font_weight new_weight) const
    {
        return {family_, size_, slant_, new_weight, auto_scaling_enabled()};
    }
    font font::with_weight(font_weight new_weight, font_slant new_slant) const
    {
        return {family_, size_, new_slant, new_weight, auto_scaling_enabled()};
    }
    font font::with_auto_scaling(bool enabled) const
    {
        return {family_, size_, slant_, weight(), enabled};
    }

    font font::of_size(std::string_view name, double size, font_weight weight, font_slant slant, bool enable_scaling)
    {
        return {std::string(name), size, slant, weight, enable_scaling};
    }
    font font::system_font_of_size(double size, font_weight weight, font_slant slant, bool enable_scaling)
    {
        return {std::string(), size, slant, weight, enable_scaling};
    }
    font font::system_font_of_weight(font_weight weight, font_slant slant, bool enable_scaling)
    {
        return {std::string(), 0.0, slant, weight, enable_scaling};
    }
    font font::default_font()
    {
        return font().with_weight(font_weight::regular);
    }

    std::string font::to_string() const
    {
        return "Family: " + family_ + ", Size: " + std::to_string(size_) +
               ", Weight: " + std::to_string(static_cast<int>(weight())) +
               ", Slant: " + std::to_string(static_cast<int>(slant_)) +
               ", AutoScalingEnabled: " + (auto_scaling_enabled() ? "true" : "false");
    }

    bool operator==(const font& a, const font& b)
    {
        return a.family() == b.family() && a.size() == b.size() && a.weight() == b.weight() && a.slant() == b.slant() &&
               a.auto_scaling_enabled() == b.auto_scaling_enabled();
    }
    bool operator!=(const font& a, const font& b)
    {
        return !(a == b);
    }
} // namespace maui::core
