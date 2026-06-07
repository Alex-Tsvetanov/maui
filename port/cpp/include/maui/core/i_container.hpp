#pragma once
// maui::core::i_container  <=  Microsoft.Maui.IContainer (IContainer : IList<IView>)
//
// A view that holds child views. Ported from src/Core/src/Core/IContainer.cs. The C# contract is the
// full IList<IView>; this is the subset the layout managers + controls use: count + indexed access
// (read) and the basic mutators to populate it. Children are referenced, not owned (ownership of the
// element tree is the parent control's, per PROFILE §8).

namespace maui::core
{
    class i_view;

    class i_container
    {
    public:
        virtual ~i_container() = default;

        [[nodiscard]] virtual int count() const = 0;
        // The child at `index`. Const because the container is logically const here even though layout
        // mutates the children (measure/arrange) through the returned reference.
        [[nodiscard]] virtual i_view& at(int index) const = 0;

        virtual void add(i_view& child) = 0;
        virtual void insert(int index, i_view& child) = 0;
        virtual void remove_at(int index) = 0;
        virtual void clear() = 0;
        [[nodiscard]] virtual int index_of(const i_view& child) const = 0;

    protected:
        i_container() = default;
        i_container(const i_container&) = default;
        i_container(i_container&&) = default;
        i_container& operator=(const i_container&) = default;
        i_container& operator=(i_container&&) = default;
    };
} // namespace maui::core
