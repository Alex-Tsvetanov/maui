// maui::controls::formatted_string — out-of-line definitions: the observable spans collection (parenting
// + per-span subscription with §8-safe teardown), ToString aggregation, and the from_string factory.

#include "maui/controls/formatted_string.hpp"

#include <algorithm>
#include <cstddef>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#include "maui/controls/element.hpp"
#include "maui/controls/span.hpp"
#include "maui/core/event.hpp"

namespace maui::controls
{
    formatted_string::formatted_string() = default;

    void formatted_string::add_span(std::shared_ptr<span> value)
    {
        if (!value)
        {
            // SpanCollection.InsertItem: item ?? throw new ArgumentNullException — the port's stand-in.
            throw std::runtime_error("a formatted_string span must not be null");
        }
        span_tokens_.push_back(attach_span(*value)); // parallel to spans_; same index after the push below
        spans_.push_back(std::move(value));
        raise_spans_changed();
    }

    void formatted_string::set_span(std::size_t index, std::shared_ptr<span> value)
    {
        if (!value)
        {
            throw std::runtime_error("a formatted_string span must not be null");
        }
        // §8: unparent the OLD span, then subscribe the new one IN PLACE — the move-assign drops the OLD
        // token (disconnecting the old span's property_changed) while the old span is still owned, before
        // it is replaced. The token vector stays index-parallel to spans_.
        detach_span(*spans_[index]);
        span_tokens_[index] = attach_span(*value);
        spans_[index] = std::move(value);
        raise_spans_changed();
    }

    bool formatted_string::remove_span(const std::shared_ptr<span>& value)
    {
        const auto it = std::ranges::find(spans_, value);
        if (it == spans_.end())
        {
            return false;
        }
        const auto removed_index = static_cast<std::ptrdiff_t>(it - spans_.begin());
        const std::shared_ptr<span> removed = std::move(*it); // move out: keep the span alive through teardown
        detach_span(*removed);                                // §8: unparent; `removed` keeps the span alive below
        spans_.erase(spans_.begin() + removed_index);
        span_tokens_.erase(span_tokens_.begin() + removed_index); // drops the token while `removed` is live
        raise_spans_changed();
        return true;
    }

    void formatted_string::clear_spans()
    {
        if (spans_.empty())
        {
            return;
        }
        // §8: disconnect every span's subscription (drop the tokens) and unparent it before the collection
        // drops it. Tokens go first so a span change mid-teardown can't reach this subscriber.
        span_tokens_.clear();
        for (const auto& span_ptr : spans_)
        {
            element::detach_logical_child(*span_ptr);
        }
        spans_.clear();
        raise_spans_changed();
    }

    std::string formatted_string::to_string() const
    {
        std::string result;
        for (const auto& span_ptr : spans_)
        {
            if (span_ptr)
            {
                result.append(span_ptr->text());
            }
        }
        return result;
    }

    std::shared_ptr<formatted_string> formatted_string::from_string(std::string text)
    {
        auto fs = std::make_shared<formatted_string>();
        auto only = std::make_shared<span>();
        only->set_text(std::move(text));
        fs->add_span(std::move(only));
        return fs;
    }

    void formatted_string::for_each_logical_child(const std::function<void(element&)>& visit) const
    {
        for (const auto& span_ptr : spans_)
        {
            if (span_ptr)
            {
                visit(*span_ptr);
            }
        }
    }

    maui::core::scoped_connection formatted_string::attach_span(span& value)
    {
        // C# OnCollectionChanged add-branch: AddLogicalChild(span) + subscribe its PropertyChanged. The
        // logical-child attach flows THIS formatted_string's BindingContext into the span
        // (SetInheritedBindingContext) and parents it. The subscription re-raises Spans on ANY span change;
        // the returned token is stored index-parallel to spans_ and dropped on detach (§8).
        attach_logical_child(value);
        return maui::core::connect_scoped(value.property_changed, [this](std::string_view) { raise_spans_changed(); });
    }

    void formatted_string::detach_span(span& value)
    {
        // §8: the caller has already dropped (or is about to drop) this span's parallel token; here we only
        // unparent it (clear its window + logical parent). Disconnecting the token BEFORE this guarantees a
        // span change mid-teardown can never reach the now-detaching subscriber.
        element::detach_logical_child(value);
    }

    void formatted_string::raise_spans_changed()
    {
        // C# OnCollectionChanged / OnItemPropertyChanged both end at OnPropertyChanged(nameof(Spans)); the
        // single `changed` signal is the owner-facing re-raise (Label.OnFormattedTextChanged).
        this->on_property_changed("spans");
        changed.raise();
    }
} // namespace maui::controls
