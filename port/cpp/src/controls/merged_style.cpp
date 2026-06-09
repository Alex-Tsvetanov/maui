// maui::controls::merged_style — implicit + class style resolution/application (merged_style.hpp).
// Ported from MergedStyle.cs (the implicit/class style layering; ApplyToDerivedTypes deferred).
#include "maui/controls/merged_style.hpp"

#include <any>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "maui/controls/element.hpp"
#include "maui/controls/resource_dictionary.hpp"
#include "maui/controls/style.hpp"
#include "maui/core/setter_specificity.hpp"
#include "maui/core/type_tag.hpp"

namespace maui::controls
{
    namespace
    {
        // A based-on-by-key resolver bound to an element's resource chain: look up the key, return it as a
        // style if it is one. Shared by every style applied through the merged style so a style's
        // base_resource_key resolves against the same chain.
        style::resource_resolver chain_resolver(element& owner)
        {
            return [&owner](std::string_view key) -> std::shared_ptr<style> {
                if (const std::any* value = owner.try_get_resource(key))
                {
                    if (const auto* found = std::any_cast<std::shared_ptr<style>>(value))
                    {
                        return *found;
                    }
                }
                return nullptr;
            };
        }
    } // namespace

    std::shared_ptr<style> merged_style::resolve_implicit() const
    {
        if (!target_type_)
        {
            return nullptr;
        }
        const std::any* value = owner_->try_get_resource(implicit_style_key(*target_type_));
        if (value == nullptr)
        {
            return nullptr;
        }
        const auto* found = std::any_cast<std::shared_ptr<style>>(value);
        return found != nullptr ? *found : nullptr;
    }

    std::vector<std::shared_ptr<style>> merged_style::resolve_classes() const
    {
        std::vector<std::shared_ptr<style>> resolved;
        for (const std::string& class_name : style_classes_)
        {
            const std::string key = std::string{k_style_class_prefix} + class_name;
            const std::any* value = owner_->try_get_resource(key);
            if (value == nullptr)
            {
                continue;
            }
            // A class key holds a vector<shared_ptr<style>> (classes accumulate); apply the first that targets
            // this element's type. (C# OnClassStyleChanged picks FirstOrDefault(s => s.CanBeAppliedTo(type)).)
            if (const auto* styles = std::any_cast<std::vector<std::shared_ptr<style>>>(value))
            {
                for (const std::shared_ptr<style>& candidate : *styles)
                {
                    if (candidate && (!target_type_ || candidate->target_type() == *target_type_))
                    {
                        resolved.push_back(candidate);
                        break;
                    }
                }
            }
        }
        return resolved;
    }

    void merged_style::set_style_classes(std::vector<std::string> classes)
    {
        style_classes_ = std::move(classes);
        refresh();
    }

    void merged_style::refresh()
    {
        const std::shared_ptr<style> implicit = resolve_implicit();
        std::vector<std::shared_ptr<style>> classes = resolve_classes();
        const style::resource_resolver resolve = chain_resolver(*owner_);

        // Re-apply only what changed (MergedStyle.SetStyle's shouldReApply… diffing), so an unrelated
        // resource change doesn't churn the precedence list. Order matches C#: implicit first, then classes.
        const bool implicit_changed = implicit != applied_implicit_;
        const bool classes_changed = classes != applied_classes_;

        if (implicit_changed && applied_implicit_)
        {
            applied_implicit_->unapply(*owner_, maui::core::setter_specificity::style_implicit, resolve);
        }
        if (classes_changed)
        {
            for (const std::shared_ptr<style>& applied : applied_classes_)
            {
                applied->unapply(*owner_, maui::core::setter_specificity::style_class, resolve);
            }
        }

        applied_implicit_ = implicit;
        applied_classes_ = std::move(classes);

        if (implicit_changed && applied_implicit_)
        {
            applied_implicit_->apply(*owner_, maui::core::setter_specificity::style_implicit, resolve);
        }
        if (classes_changed)
        {
            for (const std::shared_ptr<style>& applied : applied_classes_)
            {
                applied->apply(*owner_, maui::core::setter_specificity::style_class, resolve);
            }
        }
    }

    void merged_style::unapply()
    {
        const style::resource_resolver resolve = chain_resolver(*owner_);
        if (applied_implicit_)
        {
            applied_implicit_->unapply(*owner_, maui::core::setter_specificity::style_implicit, resolve);
            applied_implicit_ = nullptr;
        }
        for (const std::shared_ptr<style>& applied : applied_classes_)
        {
            applied->unapply(*owner_, maui::core::setter_specificity::style_class, resolve);
        }
        applied_classes_.clear();
    }
} // namespace maui::controls
