// table_view_handler — macOS (AppKit) platform recipe: a real view-based NSTableView inside an
// NSScrollView, driven by an NSTableViewDataSource + NSTableViewDelegate. The AppKit analog of
// TableViewRenderer (macOS has no native grouped table, so the port flattens sections+rows into one
// column and keeps a row→[section,row] map; section titles ride as the row's accessory, deferred):
//   - numberOfRowsInTableView returns the flattened cell count,
//   - viewForTableColumn:row: DEQUEUES an NSTextField row view by the cell's reuse identifier (its type
//     name) via makeViewWithIdentifier: — a fresh make records a `realized`, a reuse hit records a
//     `reused` — and fills its stringValue from the cell (the GetCell text),
//   - tableViewSelectionDidChange: records the selection + routes through the model's RowSelected (taps
//     the cell), the NSTableView selection analog of HandleRowSelected.
// The platform mirror (events / realized / selected_path) is populated alongside the real NSTableView so
// the apple suite asserts the same realize/reuse/selection oracle as headless. Obj-C++ with ARC.
//
// Coverage note (documented): the row hosts the cell's primary text; live per-cell native editors
// (an embedded NSButton switch / NSTextField inside the row) + native section group rows are deferred —
// the cross-platform cell properties + the realize/reuse/selection seam are covered here.

#import <AppKit/AppKit.h>
#import <objc/runtime.h>

#include <memory>
#include <string>
#include <utility>
#include <vector>

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

namespace maui::controls
{
    // Bridges the Obj-C datasource/delegate to the C++ handler's mirror (the handler owns the records).
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
        // The flattened [section,row] path of the Nth visible cell (skips empty sections).
        static table_row_path path_for_flat_row(table_model& model, int flat)
        {
            int seen = 0;
            const int sections = model.get_section_count();
            for (int s = 0; s < sections; ++s)
            {
                const int rows = model.get_row_count(s);
                if (flat < seen + rows)
                {
                    return {s, flat - seen};
                }
                seen += rows;
            }
            return {0, 0};
        }
        static int flat_row_count(table_model& model)
        {
            int total = 0;
            const int sections = model.get_section_count();
            for (int s = 0; s < sections; ++s)
            {
                total += model.get_row_count(s);
            }
            return total;
        }
    };
} // namespace maui::controls

namespace
{
    NSScrollView* as_scroll(void* native)
    {
        return (__bridge NSScrollView*)native;
    }
    NSTableView* as_table(void* native)
    {
        NSScrollView* const scroll = as_scroll(native);
        return scroll != nil ? (NSTableView*)scroll.documentView : nil;
    }
    char g_source_key = 0;
} // namespace

// The datasource + delegate, holding a non-owning back-pointer to the C++ handler.
@interface MauiTableViewSource : NSObject <NSTableViewDataSource, NSTableViewDelegate>
@property(nonatomic, assign) table_view_handler* handler;
@end

@implementation MauiTableViewSource

- (NSInteger)numberOfRowsInTableView:(NSTableView*)tableView
{
    (void)tableView;
    table_model* const model =
        self.handler != nullptr ? maui::controls::table_view_source_bridge::model_for(*self.handler) : nullptr;
    return model != nullptr ? maui::controls::table_view_source_bridge::flat_row_count(*model) : 0;
}

- (NSView*)tableView:(NSTableView*)tableView viewForTableColumn:(NSTableColumn*)tableColumn row:(NSInteger)row
{
    (void)tableColumn;
    table_view_handler* const handler = self.handler;
    if (handler == nullptr)
    {
        return nil;
    }
    table_model* const model = maui::controls::table_view_source_bridge::model_for(*handler);
    table_view_platform* const platform = maui::controls::table_view_source_bridge::platform_for(*handler);
    if (model == nullptr)
    {
        return nil;
    }
    const table_row_path path =
        maui::controls::table_view_source_bridge::path_for_flat_row(*model, static_cast<int>(row));
    std::shared_ptr<cell> source = model->get_cell(path.section, path.row);
    const std::string reuse_id = source != nullptr ? table_view_handler::reuse_identifier(*source) : std::string{};
    NSString* const identifier = [NSString stringWithUTF8String:reuse_id.c_str()];

    NSTextField* field = [tableView makeViewWithIdentifier:identifier owner:self];
    const bool reused = field != nil;
    if (!reused)
    {
        field = [NSTextField labelWithString:@""];
        field.identifier = identifier;
    }
    const std::string text = source != nullptr ? table_view_handler::display_text(*source) : std::string{};
    field.stringValue = [NSString stringWithUTF8String:text.c_str()];

    if (platform != nullptr)
    {
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
    return field;
}

- (void)tableViewSelectionDidChange:(NSNotification*)notification
{
    NSTableView* const tableView = (NSTableView*)notification.object;
    const NSInteger row = tableView.selectedRow;
    if (row < 0 || self.handler == nullptr)
    {
        return;
    }
    table_model* const model = maui::controls::table_view_source_bridge::model_for(*self.handler);
    if (model == nullptr)
    {
        return;
    }
    const table_row_path path =
        maui::controls::table_view_source_bridge::path_for_flat_row(*model, static_cast<int>(row));
    self.handler->simulate_select(path.section, path.row);
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
        NSScrollView* const scroll = [[NSScrollView alloc] initWithFrame:NSZeroRect];
        NSTableView* const native = [[NSTableView alloc] initWithFrame:NSZeroRect];
        NSTableColumn* const column = [[NSTableColumn alloc] initWithIdentifier:@"maui_column"];
        column.width = 400;
        [native addTableColumn:column];
        native.headerView = nil;
        scroll.documentView = native;
        scroll.hasVerticalScroller = YES;
        platform->native = (__bridge_retained void*)scroll;
        return platform;
    }

    void table_view_handler::on_connect_handler(table_view_platform& platform)
    {
        NSTableView* const native = as_table(platform.native);
        MauiTableViewSource* const source = [[MauiTableViewSource alloc] init];
        source.handler = this;
        objc_setAssociatedObject(native, &g_source_key, source, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
        native.dataSource = source;
        native.delegate = source;
        reload();
    }

    void table_view_handler::on_disconnect_handler(table_view_platform& platform)
    {
        NSTableView* const native = as_table(platform.native);
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
        platform->realized.clear(); // re-filled by viewForTableColumn during the layout pass
        NSTableView* const native = as_table(platform->native);
        as_scroll(platform->native).frame = NSMakeRect(0, 0, 400, 5000);
        native.frame = NSMakeRect(0, 0, 400, 5000);
        [native reloadData];
        [native layoutSubtreeIfNeeded];
        // Force row views to realize for every row (NSTableView is lazy; ask for each).
        const NSInteger rows = native.numberOfRows;
        for (NSInteger r = 0; r < rows; ++r)
        {
            (void)[native rowViewAtRow:r makeIfNecessary:YES];
            (void)[native viewAtColumn:0 row:r makeIfNecessary:YES];
        }
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
        model->row_selected(section, row); // NSTableView selection → TableModel.RowSelected (taps cell)
    }

    void table_view_handler::map_row_height(table_view_handler& handler, i_table_view& view)
    {
        auto* platform = handler.typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->row_height = view.row_height();
        if (view.row_height() > 0)
        {
            as_table(platform->native).rowHeight = static_cast<CGFloat>(view.row_height());
        }
    }

    void table_view_handler::map_has_uneven_rows(table_view_handler& handler, i_table_view& view)
    {
        if (auto* platform = handler.typed_platform_view())
        {
            platform->has_uneven_rows = view.has_uneven_rows();
        }
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
        [as_scroll(platform->native) setFrame:NSMakeRect(frame.x, frame.y, frame.width, frame.height)];
    }
} // namespace maui::controls
