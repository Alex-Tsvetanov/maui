#pragma once
// maui::core::i_item_delegate<T>  <=  Microsoft.Maui.IItemDelegate<T>
//
// The minimal "give me your items" contract a native item host pulls through (the UIPickerView model
// reads row count + row titles from it). Ported from src/Core/src/Primitives/IItemDelegate.cs;
// i_picker derives i_item_delegate<std::string>, exactly like IPicker : IItemDelegate<string>.

namespace maui::core
{
    template <class T> class i_item_delegate
    {
    public:
        virtual ~i_item_delegate() = default;

        [[nodiscard]] virtual int get_count() const = 0;
        [[nodiscard]] virtual T get_item(int index) const = 0;

    protected:
        i_item_delegate() = default;
        i_item_delegate(const i_item_delegate&) = default;
        i_item_delegate(i_item_delegate&&) = default;
        i_item_delegate& operator=(const i_item_delegate&) = default;
        i_item_delegate& operator=(i_item_delegate&&) = default;
    };
} // namespace maui::core
