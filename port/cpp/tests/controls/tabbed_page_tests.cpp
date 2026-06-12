// Tests for multi_page<TPage> + tabbed_page — ported from MultiPageTests.cs (instantiated for
// TabbedPage, exactly like the C# `TabbedPageTests : MultiPageTests<Page>` fixture) + the
// TabbedPageTests.cs extras (TestConstructor / LogicalAndInternalChildrenMaintainOrder), plus the
// headless handler seam (the tabbed_page_platform mirrors + the i_tabbed_view native-selection sync).
//
// Port notes (the deviations the control header documents):
//   - C#'s ChildAdded/ChildRemoved counting is observed through the logical tree (logical_parent set /
//     cleared) — the port's element has no child_added event.
//   - C#'s `Assert.Same(CurrentPage.BindingContext, SelectedItem)` becomes value equality on the typed
//     context (`*binding_context<std::string>() == ...`) — the port boxes a copy of the item.
//   - ObservableList<T> range cases use maui::core::observable_collection's range ops.

#include "maui/controls/tabbed_page.hpp"

#include <any>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "maui/controls/content_page.hpp"
#include "maui/controls/label.hpp"
#include "maui/controls/templates/data_template.hpp"
#include "maui/core/i_tabbed_view.hpp"
#include "maui/core/observable_collection.hpp"
#include "maui/core/tabbed_page_handler.hpp"
#include "maui/graphics/colors.hpp"
#include <gtest/gtest.h>

namespace
{
    using maui::controls::content_page;
    using maui::controls::data_template;
    using maui::controls::tabbed_page;
    using maui::core::collection_changed_action;
    using maui::core::collection_changed_args;
    using maui::core::observable_collection;
    using maui::core::tabbed_page_handler;

    // The C# test template: `new ContentPage { Content = new Label bound to "." }` — a page owning a
    // label whose text binds to the page's BindingContext (the templated item).
    class bound_label_page : public content_page
    {
    public:
        bound_label_page()
        {
            set_content(text_label_);
            text_label_.set_binding("text", ".");
        }

        [[nodiscard]] const maui::controls::label& text_label() const
        {
            return text_label_;
        }

    private:
        maui::controls::label text_label_;
    };

    [[nodiscard]] std::shared_ptr<data_template> make_label_template()
    {
        return std::make_shared<data_template>(
            []() -> std::shared_ptr<maui::core::bindable_object> { return std::make_shared<bound_label_page>(); });
    }

    // The C# assertPage closure: the child at `index` is a template-created page whose bound label
    // shows `text`, and its internal Index matches its position (GetIndex(page) == index).
    void expect_template_page(const tabbed_page& tabs, std::size_t index, std::string_view text)
    {
        ASSERT_LT(index, tabs.children().size());
        content_page* const child = tabs.children()[index];
        EXPECT_EQ(tabs.get_index(*child), static_cast<int>(index));
        const auto* page = dynamic_cast<const bound_label_page*>(child);
        ASSERT_NE(page, nullptr);
        EXPECT_EQ(page->text_label().text(), text);
    }

    // ---- TabbedPageTests.cs ----

    TEST(tabbed_page, constructor_starts_with_no_children)
    {
        tabbed_page tabs;
        EXPECT_TRUE(tabs.children().empty());
        EXPECT_EQ(tabs.current_page(), nullptr);
    }

    TEST(tabbed_page, logical_and_internal_children_maintain_order)
    {
        tabbed_page tabs;
        content_page page1;
        content_page page2;

        tabs.add(page2);
        tabs.insert(0, page1);
        tabs.remove(page1);
        tabs.insert(0, page1);

        ASSERT_EQ(tabs.children().size(), 2U);
        EXPECT_EQ(tabs.children()[0], &page1);
        EXPECT_EQ(tabs.children()[1], &page2);
    }

    // ---- MultiPageTests.cs (the Children path) ----

    TEST(tabbed_page, set_children_fires_pages_added_and_attaches)
    {
        tabbed_page tabs;
        int pages_added = 0;
        tabs.pages_changed.connect([&pages_added](const collection_changed_args& args) {
            if (args.action == collection_changed_action::add)
            {
                ++pages_added;
            }
        });

        content_page first;
        content_page second;
        tabs.add(first);
        tabs.add(second);

        EXPECT_EQ(pages_added, 2);
        EXPECT_EQ(tabs.children().size(), 2U);
        // The C# ChildAdded/LogicalChildren assertions: each child joined the logical tree.
        EXPECT_EQ(first.logical_parent(), &tabs);
        EXPECT_EQ(second.logical_parent(), &tabs);
    }

    TEST(tabbed_page, overwrite_children_detaches_then_reattaches)
    {
        tabbed_page tabs;
        content_page first;
        content_page second;
        tabs.add(first);
        tabs.add(second);

        int removed = 0;
        int added = 0;
        tabs.pages_changed.connect([&](const collection_changed_args& args) {
            if (args.action == collection_changed_action::remove)
            {
                ++removed;
            }
            else if (args.action == collection_changed_action::add)
            {
                ++added;
            }
        });

        tabs.remove(first);
        tabs.remove(second);
        EXPECT_EQ(first.logical_parent(), nullptr); // ChildRemoved: left the logical tree

        content_page third;
        content_page fourth;
        tabs.add(third);
        tabs.add(fourth);

        EXPECT_EQ(removed, 2);
        EXPECT_EQ(added, 2);
        EXPECT_EQ(tabs.children().size(), 2U);
    }

    TEST(tabbed_page, current_page_set_after_add)
    {
        tabbed_page tabs;
        EXPECT_EQ(tabs.current_page(), nullptr);

        bool property = false;
        tabs.property_changed.connect([&property](std::string_view name) {
            if (name == "current_page")
            {
                property = true;
            }
        });

        content_page child;
        tabs.add(child);

        EXPECT_EQ(tabs.current_page(), &child);
        EXPECT_TRUE(property) << "CurrentPage property change did not fire";
    }

    TEST(tabbed_page, current_page_changed_after_remove)
    {
        tabbed_page tabs;
        content_page child;
        content_page child2;
        tabs.add(child);
        tabs.add(child2);

        bool property = false;
        tabs.property_changed.connect([&property](std::string_view name) {
            if (name == "current_page")
            {
                property = true;
            }
        });

        tabs.remove(child);

        EXPECT_EQ(tabs.current_page(), &child2);
        EXPECT_TRUE(property) << "CurrentPage property change did not fire";
    }

    TEST(tabbed_page, current_page_null_after_remove)
    {
        tabbed_page tabs;
        content_page child;
        tabs.add(child);

        bool property = false;
        tabs.property_changed.connect([&property](std::string_view name) {
            if (name == "current_page")
            {
                property = true;
            }
        });

        tabs.remove(child);

        EXPECT_EQ(tabs.current_page(), nullptr);
        EXPECT_TRUE(property) << "CurrentPage property change did not fire";
    }

    TEST(tabbed_page, current_page_changed_event)
    {
        tabbed_page tabs;
        content_page first;
        content_page second;
        tabs.add(first);
        tabs.add(second);

        bool raised = false;
        tabs.current_page_changed.connect([&raised] { raised = true; });

        tabs.set_current_page(tabs.children()[0]); // already current -> no event
        EXPECT_FALSE(raised);

        tabs.set_current_page(tabs.children()[1]);
        EXPECT_TRUE(raised);
    }

    // ---- MultiPageTests.cs (the ItemsSource / ItemTemplate path) ----

    TEST(tabbed_page, templated_page)
    {
        tabbed_page tabs;
        tabs.set_item_template(make_label_template());

        tabs.set_items_source(std::vector<std::string>{"Foo", "Bar"});

        ASSERT_EQ(tabs.children().size(), 2U);
        expect_template_page(tabs, 0, "Foo");
        expect_template_page(tabs, 1, "Bar");
    }

    TEST(tabbed_page, selected_item_set_after_add)
    {
        tabbed_page tabs;
        EXPECT_EQ(tabs.current_page(), nullptr);

        auto items = std::make_shared<observable_collection<std::string>>();
        tabs.set_items_source(items);

        bool selected = false;
        bool current = false;
        tabs.property_changed.connect([&](std::string_view name) {
            if (name == "current_page")
            {
                current = true;
            }
            else if (name == "selected_item")
            {
                selected = true;
            }
        });

        items->add("foo");

        EXPECT_EQ(tabs.selected_item<std::string>(), "foo");
        ASSERT_NE(tabs.current_page(), nullptr);
        const auto context = tabs.current_page()->binding_context<std::string>();
        ASSERT_NE(context, nullptr);
        EXPECT_EQ(*context, "foo"); // C# Assert.Same(CurrentPage.BindingContext, SelectedItem)
        EXPECT_TRUE(current) << "CurrentPage property change did not fire";
        EXPECT_TRUE(selected) << "SelectedItem property change did not fire";
    }

    TEST(tabbed_page, selected_item_null_after_remove)
    {
        tabbed_page tabs;
        auto items = std::make_shared<observable_collection<std::string>>(std::vector<std::string>{"foo"});
        tabs.set_items_source(items);

        bool selected = false;
        bool current = false;
        tabs.property_changed.connect([&](std::string_view name) {
            if (name == "current_page")
            {
                current = true;
            }
            else if (name == "selected_item")
            {
                selected = true;
            }
        });

        items->remove("foo");

        EXPECT_FALSE(tabs.has_selected_item());
        EXPECT_EQ(tabs.current_page(), nullptr);
        EXPECT_TRUE(current) << "CurrentPage property change did not fire";
        EXPECT_TRUE(selected) << "SelectedItem property change did not fire";
    }

    // "When ItemsSource is set with items, the first item should automatically be selected."
    TEST(tabbed_page, selected_item_set_after_items_source_set)
    {
        tabbed_page tabs;

        bool selected = false;
        bool current = false;
        tabs.property_changed.connect([&](std::string_view name) {
            if (name == "current_page")
            {
                current = true;
            }
            else if (name == "selected_item")
            {
                selected = true;
            }
        });

        tabs.set_items_source(std::vector<std::string>{"foo"});

        EXPECT_EQ(tabs.selected_item<std::string>(), "foo");
        ASSERT_NE(tabs.current_page(), nullptr);
        const auto context = tabs.current_page()->binding_context<std::string>();
        ASSERT_NE(context, nullptr);
        EXPECT_EQ(*context, "foo");
        EXPECT_TRUE(current) << "CurrentPage property change did not fire";
        EXPECT_TRUE(selected) << "SelectedItem property change did not fire";
    }

    TEST(tabbed_page, selected_item_no_longer_present)
    {
        tabbed_page tabs;
        tabs.set_items_source(std::vector<std::string>{"foo", "bar"});
        tabs.set_selected_item(std::string{"bar"});

        tabs.set_items_source(std::vector<std::string>{"fad", "baz"});

        EXPECT_EQ(tabs.selected_item<std::string>(), "fad");
    }

    TEST(tabbed_page, selected_item_after_move)
    {
        tabbed_page tabs;
        auto items = std::make_shared<observable_collection<std::string>>(std::vector<std::string>{"foo", "bar"});
        tabs.set_items_source(items);

        EXPECT_EQ(tabs.selected_item<std::string>(), "foo");
        ASSERT_NE(tabs.current_page(), nullptr);
        EXPECT_EQ(*tabs.current_page()->binding_context<std::string>(), "foo");

        tabs.set_selected_item(std::string{"bar"});
        EXPECT_EQ(*tabs.current_page()->binding_context<std::string>(), "bar");

        items->move(1, 0);

        EXPECT_EQ(tabs.selected_item<std::string>(), items->at(0));
        ASSERT_NE(tabs.current_page(), nullptr);
        EXPECT_EQ(*tabs.current_page()->binding_context<std::string>(), items->at(0));
    }

    TEST(tabbed_page, untemplated_items_source_page)
    {
        tabbed_page tabs;
        tabs.set_items_source(std::vector<std::string>{"Foo", "Bar"});

        ASSERT_EQ(tabs.children().size(), 2U);
        // CreateDefault: a plain page titled with the item's text.
        EXPECT_EQ(tabs.children()[0]->title(), "Foo");
        EXPECT_EQ(tabs.children()[1]->title(), "Bar");
    }

    TEST(tabbed_page, children_read_only_while_items_source_set)
    {
        tabbed_page tabs;
        tabs.set_items_source(std::vector<std::string>{"Foo"});

        content_page outsider;
        tabs.add(outsider); // C# throws NotSupportedException; the port no-ops (documented)
        EXPECT_EQ(tabs.children().size(), 1U);
        EXPECT_EQ(outsider.logical_parent(), nullptr);
    }

    TEST(tabbed_page, template_pages_added)
    {
        tabbed_page tabs;
        tabs.set_item_template(make_label_template());
        auto items = std::make_shared<observable_collection<std::string>>(std::vector<std::string>{"Foo", "Bar"});
        tabs.set_items_source(items);

        items->add("Baz");

        ASSERT_EQ(tabs.children().size(), 3U);
        expect_template_page(tabs, 0, "Foo");
        expect_template_page(tabs, 1, "Bar");
        expect_template_page(tabs, 2, "Baz");
    }

    TEST(tabbed_page, template_pages_range_added)
    {
        tabbed_page tabs;
        tabs.set_item_template(make_label_template());
        auto items = std::make_shared<observable_collection<std::string>>(std::vector<std::string>{"Foo", "Bar"});
        tabs.set_items_source(items);

        int added_count = 0;
        tabs.pages_changed.connect([&added_count](const collection_changed_args& args) {
            if (args.action != collection_changed_action::add)
            {
                return;
            }
            ++added_count;
            EXPECT_EQ(args.new_count, 2U); // ONE range notification carrying both pages
        });

        items->add_range({"Baz", "Bam"});

        EXPECT_EQ(added_count, 1);
        ASSERT_EQ(tabs.children().size(), 4U);
        expect_template_page(tabs, 0, "Foo");
        expect_template_page(tabs, 1, "Bar");
        expect_template_page(tabs, 2, "Baz");
        expect_template_page(tabs, 3, "Bam");
    }

    TEST(tabbed_page, template_pages_inserted)
    {
        tabbed_page tabs;
        tabs.set_item_template(make_label_template());
        auto items = std::make_shared<observable_collection<std::string>>(std::vector<std::string>{"Foo", "Bar"});
        tabs.set_items_source(items);

        items->insert(1, "Baz");

        ASSERT_EQ(tabs.children().size(), 3U);
        expect_template_page(tabs, 0, "Foo");
        expect_template_page(tabs, 1, "Baz");
        expect_template_page(tabs, 2, "Bar");
    }

    TEST(tabbed_page, template_pages_range_inserted)
    {
        tabbed_page tabs;
        tabs.set_item_template(make_label_template());
        auto items = std::make_shared<observable_collection<std::string>>(std::vector<std::string>{"Foo", "Bar"});
        tabs.set_items_source(items);

        items->insert_range(1, {"Baz", "Bam"});

        ASSERT_EQ(tabs.children().size(), 4U);
        expect_template_page(tabs, 0, "Foo");
        expect_template_page(tabs, 1, "Baz");
        expect_template_page(tabs, 2, "Bam");
        expect_template_page(tabs, 3, "Bar");
    }

    TEST(tabbed_page, template_pages_removed)
    {
        tabbed_page tabs;
        tabs.set_item_template(make_label_template());
        auto items = std::make_shared<observable_collection<std::string>>(std::vector<std::string>{"Foo", "Bar"});
        tabs.set_items_source(items);

        items->remove("Foo");

        ASSERT_EQ(tabs.children().size(), 1U);
        expect_template_page(tabs, 0, "Bar");
    }

    TEST(tabbed_page, template_pages_range_removed)
    {
        tabbed_page tabs;
        tabs.set_item_template(make_label_template());
        auto items = std::make_shared<observable_collection<std::string>>(
            std::vector<std::string>{"Foo", "Bar", "Baz", "Bam", "Who"});
        tabs.set_items_source(items);

        items->remove_at(1, 2);

        ASSERT_EQ(tabs.children().size(), 3U);
        expect_template_page(tabs, 0, "Foo");
        expect_template_page(tabs, 1, "Bam");
        expect_template_page(tabs, 2, "Who");
    }

    TEST(tabbed_page, template_pages_reordered)
    {
        tabbed_page tabs;
        tabs.set_item_template(make_label_template());
        auto items = std::make_shared<observable_collection<std::string>>(std::vector<std::string>{"Foo", "Bar"});
        tabs.set_items_source(items);

        items->move(0, 1);

        ASSERT_EQ(tabs.children().size(), 2U);
        expect_template_page(tabs, 0, "Bar");
        expect_template_page(tabs, 1, "Foo");
    }

    TEST(tabbed_page, template_pages_range_reordered_forward)
    {
        tabbed_page tabs;
        tabs.set_item_template(make_label_template());
        auto items = std::make_shared<observable_collection<std::string>>(
            std::vector<std::string>{"Foo", "Bar", "Baz", "Bam", "Who", "Where"});
        tabs.set_items_source(items);

        items->move(1, 4, 2);

        ASSERT_EQ(tabs.children().size(), 6U);
        expect_template_page(tabs, 0, "Foo");
        expect_template_page(tabs, 1, "Bam");
        expect_template_page(tabs, 2, "Who");
        expect_template_page(tabs, 3, "Bar");
        expect_template_page(tabs, 4, "Baz");
        expect_template_page(tabs, 5, "Where");
    }

    TEST(tabbed_page, template_pages_range_reordered_backward)
    {
        tabbed_page tabs;
        tabs.set_item_template(make_label_template());
        auto items = std::make_shared<observable_collection<std::string>>(
            std::vector<std::string>{"Foo", "Bar", "Baz", "Bam", "Who", "Where", "When"});
        tabs.set_items_source(items);

        items->move(4, 1, 2);

        ASSERT_EQ(tabs.children().size(), 7U);
        expect_template_page(tabs, 0, "Foo");
        expect_template_page(tabs, 1, "Who");
        expect_template_page(tabs, 2, "Where");
        expect_template_page(tabs, 3, "Bar");
        expect_template_page(tabs, 4, "Baz");
        expect_template_page(tabs, 5, "Bam");
        expect_template_page(tabs, 6, "When");
    }

    TEST(tabbed_page, template_pages_replaced)
    {
        tabbed_page tabs;
        tabs.set_item_template(make_label_template());
        auto items = std::make_shared<observable_collection<std::string>>(std::vector<std::string>{"Foo", "Bar"});
        tabs.set_items_source(items);

        items->set(0, "Baz");

        ASSERT_EQ(tabs.children().size(), 2U);
        expect_template_page(tabs, 0, "Baz");
        expect_template_page(tabs, 1, "Bar");
    }

    TEST(tabbed_page, templated_pages_source_replaced)
    {
        tabbed_page tabs;
        tabs.set_item_template(make_label_template());
        tabs.set_items_source(
            std::make_shared<observable_collection<std::string>>(std::vector<std::string>{"Foo", "Bar"}));

        tabs.set_items_source(
            std::make_shared<observable_collection<std::string>>(std::vector<std::string>{"Baz", "Bar"}));

        ASSERT_EQ(tabs.children().size(), 2U);
        expect_template_page(tabs, 0, "Baz");
        expect_template_page(tabs, 1, "Bar");
    }

    // "Setting CurrentPage (usually from renderers) should update SelectedItem properly."
    TEST(tabbed_page, setting_current_page_with_templates_updates_selected_item)
    {
        tabbed_page tabs;
        tabs.set_items_source(std::vector<std::string>{"Foo", "Bar"});

        // If these aren't correct, the rest of the test is invalid.
        ASSERT_EQ(tabs.current_page(), tabs.children()[0]);
        ASSERT_EQ(tabs.selected_item<std::string>(), "Foo");

        tabs.set_current_page(tabs.children()[1]);

        EXPECT_EQ(tabs.selected_item<std::string>(), "Bar");
    }

    TEST(tabbed_page, pages_changed_on_items_source_change)
    {
        tabbed_page tabs;
        tabs.set_items_source(std::vector<std::string>{"Baz", "Bam"});

        int fail = 0;
        int reset = 0;
        tabs.pages_changed.connect([&](const collection_changed_args& args) {
            if (args.action == collection_changed_action::reset)
            {
                ++reset;
            }
            else
            {
                ++fail;
            }
        });

        tabs.set_items_source(std::vector<std::string>{"Foo", "Bar"});

        EXPECT_EQ(reset, 1) << "PagesChanged wasn't raised or was raised too many times for Reset";
        EXPECT_EQ(fail, 0) << "PagesChanged was raised with an unexpected action";
    }

    TEST(tabbed_page, pages_changed_on_template_change)
    {
        tabbed_page tabs;
        tabs.set_items_source(std::vector<std::string>{"Foo", "Bar"});

        int fail = 0;
        int reset = 0;
        tabs.pages_changed.connect([&](const collection_changed_args& args) {
            if (args.action == collection_changed_action::reset)
            {
                ++reset;
            }
            else
            {
                ++fail;
            }
        });

        tabs.set_item_template(make_label_template());

        EXPECT_EQ(reset, 1) << "PagesChanged wasn't raised or was raised too many times for Reset";
        EXPECT_EQ(fail, 0) << "PagesChanged was raised with an unexpected action";
    }

    TEST(tabbed_page, selected_item_set_before_template)
    {
        tabbed_page tabs;
        tabs.set_items_source(std::vector<std::string>{"foo", "bar"});
        tabs.set_selected_item(std::string{"bar"});

        tabs.set_item_template(make_label_template());

        EXPECT_EQ(tabs.selected_item<std::string>(), "bar");
    }

    TEST(tabbed_page, current_page_updated_with_template)
    {
        tabbed_page tabs;
        tabs.set_items_source(std::vector<std::string>{"foo", "bar"});

        content_page* const untemplated = tabs.current_page();

        bool raised = false;
        tabs.property_changed.connect([&raised](std::string_view name) {
            if (name == "current_page")
            {
                raised = true;
            }
        });

        tabs.set_item_template(make_label_template());

        EXPECT_TRUE(raised) << "CurrentPage did not change with the template";
        EXPECT_NE(tabs.current_page(), untemplated);
    }

    // ---- the headless handler seam (tabbed_page_platform mirrors + the native-selection sync) ----

    TEST(tabbed_page_handler_seam, mirrors_pages_titles_current_and_selection)
    {
        tabbed_page tabs;
        content_page first;
        first.set_title("First");
        content_page second;
        second.set_title("Second");
        tabs.add(first);
        tabs.add(second);

        auto handler = std::make_shared<tabbed_page_handler>();
        tabs.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);

        ASSERT_EQ(platform->hosted_pages.size(), 2U);
        EXPECT_EQ(platform->hosted_pages[0], &first);
        EXPECT_EQ(platform->tab_titles, (std::vector<std::string>{"First", "Second"}));
        EXPECT_EQ(platform->hosted_current, &first);
        EXPECT_EQ(platform->selected_index, 0);

        tabs.set_current_page(&second);
        EXPECT_EQ(platform->hosted_current, &second);
        EXPECT_EQ(platform->selected_index, 1);
    }

    TEST(tabbed_page_handler_seam, pages_change_and_title_change_refresh_the_tabs)
    {
        tabbed_page tabs;
        content_page first;
        first.set_title("First");
        tabs.add(first);

        auto handler = std::make_shared<tabbed_page_handler>();
        tabs.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);

        content_page second;
        second.set_title("Second");
        tabs.add(second); // PagesChanged -> Handler.UpdateValue(ItemsSource)
        EXPECT_EQ(platform->hosted_pages.size(), 2U);
        EXPECT_EQ(platform->tab_titles, (std::vector<std::string>{"First", "Second"}));

        second.set_title("Renamed"); // the OnPagePropertyChanged Title wiring
        EXPECT_EQ(platform->tab_titles, (std::vector<std::string>{"First", "Renamed"}));
    }

    TEST(tabbed_page_handler_seam, native_tab_selection_drives_current_page)
    {
        tabbed_page tabs;
        content_page first;
        content_page second;
        tabs.add(first);
        tabs.add(second);

        auto handler = std::make_shared<tabbed_page_handler>();
        tabs.set_handler(handler);

        // The native chrome reports a user tab tap through the i_tabbed_view seam.
        auto& seam = static_cast<maui::core::i_tabbed_view&>(tabs);
        seam.on_tab_selected(1);

        EXPECT_EQ(tabs.current_page(), &second);
        EXPECT_EQ(handler->typed_platform_view()->selected_index, 1);

        seam.on_tab_selected(99); // out of range -> ignored
        EXPECT_EQ(tabs.current_page(), &second);
    }

    TEST(tabbed_page_handler_seam, bar_colors_mirror_with_unset_as_nullopt)
    {
        tabbed_page tabs;
        content_page first;
        tabs.add(first);

        auto handler = std::make_shared<tabbed_page_handler>();
        tabs.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);

        // Never set -> system defaults (C# null Color).
        EXPECT_FALSE(platform->bar_background_color.has_value());
        EXPECT_FALSE(platform->bar_text_color.has_value());
        EXPECT_FALSE(platform->selected_tab_color.has_value());
        EXPECT_FALSE(platform->unselected_tab_color.has_value());

        tabs.set_bar_background_color(maui::graphics::colors::red);
        tabs.set_bar_text_color(maui::graphics::colors::white);
        tabs.set_selected_tab_color(maui::graphics::colors::blue);
        tabs.set_unselected_tab_color(maui::graphics::colors::gray);

        ASSERT_TRUE(platform->bar_background_color.has_value());
        EXPECT_EQ(*platform->bar_background_color, maui::graphics::colors::red);
        ASSERT_TRUE(platform->bar_text_color.has_value());
        EXPECT_EQ(*platform->bar_text_color, maui::graphics::colors::white);
        ASSERT_TRUE(platform->selected_tab_color.has_value());
        EXPECT_EQ(*platform->selected_tab_color, maui::graphics::colors::blue);
        ASSERT_TRUE(platform->unselected_tab_color.has_value());
        EXPECT_EQ(*platform->unselected_tab_color, maui::graphics::colors::gray);
    }

    TEST(tabbed_page_handler_seam, items_source_path_hosts_template_pages)
    {
        tabbed_page tabs;
        auto handler = std::make_shared<tabbed_page_handler>();
        tabs.set_handler(handler);
        auto* platform = handler->typed_platform_view();
        ASSERT_NE(platform, nullptr);

        auto items = std::make_shared<observable_collection<std::string>>(std::vector<std::string>{"One", "Two"});
        tabs.set_items_source(items);

        EXPECT_EQ(platform->hosted_pages.size(), 2U);
        EXPECT_EQ(platform->tab_titles, (std::vector<std::string>{"One", "Two"})); // CreateDefault titles
        EXPECT_EQ(platform->hosted_current, tabs.children()[0]);

        items->add("Three");
        EXPECT_EQ(platform->hosted_pages.size(), 3U);
        EXPECT_EQ(platform->selected_index, 0);
    }
} // namespace
