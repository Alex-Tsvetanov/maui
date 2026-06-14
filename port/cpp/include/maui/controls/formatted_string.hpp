#pragma once
// maui::controls::formatted_string  <=  Microsoft.Maui.Controls.FormattedString
//
// A text built from a sequence of attributed spans. In C# FormattedString : Element owns an
// ObservableCollection<Span> (the ContentProperty), re-raises OnPropertyChanged(nameof(Spans)) whenever
// the collection OR any span's property changes (SpansCollectionChanged + OnItemPropertyChanged), parents
// each span as a logical child, and flows its own BindingContext into each span
// (OnBindingContextChanged → SetInheritedBindingContext). ToString() concatenates the spans' text.
//
// The port's element base supplies the bindable change-notification surface (property_changed by name),
// the logical-parent/inherited-BindingContext propagation, and SetBinding. So a formatted_string IS-A
// element; this type adds the observable spans collection on top.
//
// Ownership (PROFILE §8): the collection OWNS its spans (shared_ptr<span>), exactly as C#'s collection
// holds the only strong reference. Each span's property_changed is subscribed with a RAII token that is
// dropped (disconnected) BEFORE the span leaves the collection (or before the formatted_string dies), so
// a span's change-notification never reaches a dead subscriber (§8 teardown doctrine).
//
// Surface deviations (documented, narrow): the explicit/implicit string conversion operators are spelled
// as the named factory from_string + the to_string aggregator (C++ implicit conversions on a heap-only,
// non-copyable element would be a footgun); the FormattedStringConverter (XAML) is out of scope here.

#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "maui/controls/element.hpp"
#include "maui/controls/span.hpp"
#include "maui/core/event.hpp"

namespace maui::controls
{
    class formatted_string : public element
    {
    public:
        formatted_string();

        // C# FormattedString.SpansCollectionChanged + OnItemPropertyChanged collapse to ONE signal the
        // owner (label) re-raises on: any span-collection mutation OR any span property change. (The label
        // subscribes this to re-build its attributed text; it also fires property_changed("spans").)
        maui::core::event<> changed;

        // ---- Spans (the ContentProperty) — the observable collection ----
        // Append a span (FormattedString.Spans.Add). Throws std::runtime_error on a null span
        // (SpanCollection.InsertItem's ArgumentNullException, the port's stand-in).
        void add_span(std::shared_ptr<span> value);
        // Replace the span at `index` (SpanCollection.SetItem; null throws like Add).
        void set_span(std::size_t index, std::shared_ptr<span> value);
        // Remove `value`; returns true iff it was present (ObservableCollection.Remove).
        bool remove_span(const std::shared_ptr<span>& value);
        // Remove every span (SpanCollection.ClearItems — raises one Remove for the whole list so each span
        // is unsubscribed + unparented).
        void clear_spans();

        [[nodiscard]] std::size_t span_count() const
        {
            return spans_.size();
        }
        [[nodiscard]] const std::shared_ptr<span>& span_at(std::size_t index) const
        {
            return spans_[index];
        }
        [[nodiscard]] const std::vector<std::shared_ptr<span>>& spans() const
        {
            return spans_;
        }

        // C# FormattedString.ToString() — string.Concat(Spans.Select(s => s.Text)).
        [[nodiscard]] std::string to_string() const;

        // C# implicit operator FormattedString(string): a single span carrying `text`. A heap-only element
        // can't be value-converted, so this is the named factory the conversion stands for.
        [[nodiscard]] static std::shared_ptr<formatted_string> from_string(std::string text);

    protected:
        // FormattedString.OnBindingContextChanged: flow this element's context into each span (in addition
        // to the base element propagation, which already does exactly this for logical children — so the
        // base default suffices; spans are registered as logical children below).
        void for_each_logical_child(const std::function<void(element&)>& visit) const override;

    private:
        // Parent the span (flows BindingContext down) + subscribe its property_changed (OnItemPropertyChanged)
        // and RETURN the RAII token, which the caller stores index-parallel to spans_.
        [[nodiscard]] maui::core::scoped_connection attach_span(span& value);
        // Unparent the span (the caller has already dropped its parallel token — §8 ordering).
        static void detach_span(span& value);
        // The collection or a span changed: re-raise property_changed("spans") + the `changed` signal
        // (OnCollectionChanged → OnPropertyChanged(nameof(Spans)) / OnItemPropertyChanged).
        void raise_spans_changed();

        std::vector<std::shared_ptr<span>> spans_;
        // Per-span property_changed subscription tokens, parallel to spans_ (dropped on detach — §8).
        std::vector<maui::core::scoped_connection> span_tokens_;
    };
} // namespace maui::controls
