// table_view_handler — iOS (UIKit) platform recipe: a real UITableView driven by a
// UITableViewDataSource + UITableViewDelegate, ported from the behavior of
// Microsoft.Maui.Controls.Handlers.Compatibility.TableViewRenderer (TableViewModelRenderer):
//   - numberOfSectionsInTableView / numberOfRowsInSection / titleForHeaderInSection read the table_model,
//   - cellForRowAtIndexPath DEQUEUES a UITableViewCell by the cell's reuse identifier (its type name) —
//     a fresh alloc records a `realized`, a dequeue hit records a `reused` — and fills textLabel from the
//     cell (TableViewModelRenderer.GetCell),
//   - didSelectRowAtIndexPath records the selection + routes through the model's RowSelected (which taps
//     the cell), matching TableViewModelRenderer.HandleRowSelected.
// The platform mirror (events / realized / selected_path) is populated alongside the real UITableView so
// the on-simulator suite asserts the same realize/reuse/selection oracle as headless. Compiled as
// Objective-C++ with ARC only for the `ios` backend.
//
// Coverage note (documented): the row hosts the cell's primary text (text_cell.Text / switch_cell.Text /
// entry_cell.Label); the live per-cell native editors (an embedded UISwitch / UITextField inside the
// row) are not wired this unit — the cross-platform cell properties + the realize/reuse/selection seam
// are. view_cell content hosting beyond the text mirror is likewise deferred.

#import <UIKit/UIKit.h>
#import <objc/runtime.h>

#include <algorithm>
#include <memory>
#include <string>
#include <utility>

#include "maui/controls/i_table_view.hpp"
#include "maui/controls/table_model.hpp"
#include "maui/controls/table_view_handler.hpp"
#include "maui/graphics/rect.hpp"
#include "maui/graphics/size.hpp"

using maui::controls::cell;
using maui::controls::table_model;
using maui::controls::table_row_event_kind;
using maui::controls::table_row_path;
using maui::controls::table_view_handler;
using maui::controls::table_view_platform;

// The datasource + delegate, holding a non-owning back-pointer to the C++ handler.
@interface MauiTableViewSource : NSObject <UITableViewDataSource, UITableViewDelegate>
@property(nonatomic, assign) table_view_handler* handler;
@end

namespace
{
    UITableView* as_table(void* native)
    {
        return (__bridge UITableView*)native;
    }
    // A unique address used as the associated-object key for the retained datasource/delegate.
    char g_source_key = 0;
} // namespace

namespace maui::controls
{
    // Bridges the Obj-C datasource callbacks to the C++ handler's mirror (the handler owns the records).
    struct table_view_source_bridge
    {
        static table_model* model_for(table_view_handler& handler)
        {
            auto* view = handler.virtual_view();
            return view != nullptr ? view->model() : nullptr;
        }
        static table_view_platform* platform_for(table_view_handler& handler)
        {
            return handler.typed_platform_view();
        }
    };
} // namespace maui::controls

@implementation MauiTableViewSource

- (NSInteger)numberOfSectionsInTableView:(UITableView*)tableView
{
    (void)tableView;
    table_model* const model =
        self.handler != nullptr ? maui::controls::table_view_source_bridge::model_for(*self.handler) : nullptr;
    return model != nullptr ? model->get_section_count() : 0;
}

- (NSInteger)tableView:(UITableView*)tableView numberOfRowsInSection:(NSInteger)section
{
    (void)tableView;
    table_model* const model =
        self.handler != nullptr ? maui::controls::table_view_source_bridge::model_for(*self.handler) : nullptr;
    return model != nullptr ? model->get_row_count(static_cast<int>(section)) : 0;
}

- (NSString*)tableView:(UITableView*)tableView titleForHeaderInSection:(NSInteger)section
{
    (void)tableView;
    table_model* const model =
        self.handler != nullptr ? maui::controls::table_view_source_bridge::model_for(*self.handler) : nullptr;
    if (model == nullptr)
    {
        return nil;
    }
    const std::string title = model->get_section_title(static_cast<int>(section));
    return title.empty() ? nil : [NSString stringWithUTF8String:title.c_str()];
}

- (UITableViewCell*)tableView:(UITableView*)tableView cellForRowAtIndexPath:(NSIndexPath*)indexPath
{
    table_view_handler* const handler = self.handler;
    if (handler == nullptr)
    {
        return [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:@"empty"];
    }
    table_model* const model = maui::controls::table_view_source_bridge::model_for(*handler);
    table_view_platform* const platform = maui::controls::table_view_source_bridge::platform_for(*handler);
    const int section = static_cast<int>(indexPath.section);
    const int row = static_cast<int>(indexPath.row);
    std::shared_ptr<cell> source = model != nullptr ? model->get_cell(section, row) : nullptr;

    const std::string reuse_id = source != nullptr ? table_view_handler::reuse_identifier(*source) : std::string{};
    NSString* const identifier = [NSString stringWithUTF8String:reuse_id.c_str()];
    UITableViewCell* dequeued = [tableView dequeueReusableCellWithIdentifier:identifier];
    const bool reused = dequeued != nil;
    if (!reused)
    {
        dequeued = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleSubtitle reuseIdentifier:identifier];
    }
    const std::string text = source != nullptr ? table_view_handler::display_text(*source) : std::string{};
    dequeued.textLabel.text = [NSString stringWithUTF8String:text.c_str()];

    if (platform != nullptr)
    {
        const maui::controls::table_row_path path{.section = section, .row = row};
        maui::controls::realized_row realized;
        realized.path = path;
        realized.reuse_id = reuse_id;
        realized.text = text;
        realized.source = source;
        platform->realized.push_back(std::move(realized));
        platform->events.push_back({.kind = reused ? table_row_event_kind::reused : table_row_event_kind::realized,
                                    .path = path,
                                    .reuse_id = reuse_id});
    }
    return dequeued;
}

- (void)tableView:(UITableView*)tableView didSelectRowAtIndexPath:(NSIndexPath*)indexPath
{
    [tableView deselectRowAtIndexPath:indexPath animated:NO];
    if (self.handler != nullptr)
    {
        self.handler->simulate_select(static_cast<int>(indexPath.section), static_cast<int>(indexPath.row));
    }
}
@end

namespace maui::controls
{
    table_view_platform::table_view_platform() = default;

    table_view_platform::~table_view_platform()
    {
        if (native != nullptr)
        {
            CFRelease(native); // balances the __bridge_retained in create_platform_view
            native = nullptr;
        }
    }

    std::unique_ptr<table_view_platform> table_view_handler::create_platform_view()
    {
        auto platform = std::make_unique<table_view_platform>();
        UITableView* const native = [[UITableView alloc] initWithFrame:CGRectZero style:UITableViewStyleGrouped];
        platform->native = (__bridge_retained void*)native;
        return platform;
    }

    void table_view_handler::on_connect_handler(table_view_platform& platform)
    {
        UITableView* const native = as_table(platform.native);
        MauiTableViewSource* const source = [[MauiTableViewSource alloc] init];
        source.handler = this;
        // Retain the source alongside the table view (associated object) so it outlives this scope.
        objc_setAssociatedObject(native, &g_source_key, source, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
        native.dataSource = source;
        native.delegate = source;
        reload();
    }

    void table_view_handler::on_disconnect_handler(table_view_platform& platform)
    {
        UITableView* const native = as_table(platform.native);
        if (native != nil)
        {
            native.dataSource = nil;
            native.delegate = nil;
            objc_setAssociatedObject(native, &g_source_key, nil, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
        }
        platform.realized.clear();
        platform.recycle_pool.clear();
        platform.selected_path.reset();
    }

    void table_view_handler::reload()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->realized.clear(); // re-filled by cellForRow during the layout pass
        UITableView* const native = as_table(platform->native);
        // A generous frame so every row is in the viewport and cellForRow fires for all of them.
        native.frame = CGRectMake(0, 0, 400, 5000);
        [native reloadData];
        [native layoutIfNeeded];
    }

    void table_view_handler::simulate_select(int section, int row)
    {
        auto* platform = typed_platform_view();
        auto* view = virtual_view();
        if (platform == nullptr || view == nullptr)
        {
            return;
        }
        table_model* const model = view->model();
        if (model == nullptr)
        {
            return;
        }
        const table_row_path path{.section = section, .row = row};
        platform->selected_path = path;
        platform->events.push_back({.kind = table_row_event_kind::selected, .path = path, .reuse_id = {}});
        model->row_selected(section, row); // UITableView didSelectRow → TableModel.RowSelected (taps cell)
    }

    void table_view_handler::map_row_height(table_view_handler& handler, i_table_view& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->row_height = view.row_height();
        UITableView* const native = as_table(platform->native);
        native.rowHeight =
            view.row_height() > 0 ? static_cast<CGFloat>(view.row_height()) : UITableViewAutomaticDimension;
    }

    void table_view_handler::map_has_uneven_rows(table_view_handler& handler, i_table_view& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->has_uneven_rows = view.has_uneven_rows();
        UITableView* const native = as_table(platform->native);
        native.estimatedRowHeight = view.has_uneven_rows() ? 44.0 : 0.0;
    }

    maui::graphics::size table_view_handler::get_desired_size(double width_constraint,
                                                              double /*height_constraint*/) const
    {
        const double width = (width_constraint > 0 && width_constraint < 40.0) ? 40.0 : width_constraint;
        return {width > 0 ? width : 40.0, 40.0};
    }

    void table_view_handler::platform_arrange(const maui::graphics::rect& frame)
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        [as_table(platform->native) setFrame:CGRectMake(frame.x, frame.y, frame.width, frame.height)];
    }
} // namespace maui::controls
