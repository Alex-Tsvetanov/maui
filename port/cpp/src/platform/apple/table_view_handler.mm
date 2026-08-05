// table_view_handler — macOS (AppKit) platform recipe: a real view-based NSTableView inside an
// NSScrollView, driven by an NSTableViewDataSource + NSTableViewDelegate. The AppKit analog of
// TableViewRenderer (macOS has no native grouped table, so the port flattens header+rows per section into
// one column: a non-selectable GROUP ROW per non-empty section, then its cells):
//   - numberOfRowsInTableView returns the header-aware flat count,
//   - isGroupRow: / shouldSelectRow: render the section headers as macOS group rows (bold, not selectable),
//   - viewForTableColumn:row: for a header builds a bold title label; for a cell DEQUEUES an
//     NSTableCellView by the cell's reuse identifier (a fresh make records a `realized`, a reuse hit a
//     `reused`) and builds the PER-CELL-TYPE native content — a primary text label plus an embedded
//     NSSwitch (switch_cell), NSTextField (entry_cell), or NSImageView (image_cell) — rebinding it from
//     the cross-platform describe_cell fields,
//   - tableViewSelectionDidChange: records the selection (cells only) + routes through the model's
//     RowSelected (taps the cell), the NSTableView selection analog of HandleRowSelected.
// The platform mirror (events / realized incl. the per-content fields / selected_path / section_headers)
// is populated alongside the real NSTableView so the apple suite asserts the same realize/reuse/selection
// + cell-content + section oracle as headless. Obj-C++ with ARC.
//
// Coverage note (documented): switch / entry / image cells + section group rows are rendered here; the
// image view is populated with a placeholder when the cell has a resolved ImageSource (the full async
// thumbnail decode rides the image-service seam, as in W3-31 — the row only proves the image view is
// built + bound). view_cell hosts its arbitrary content View: the View's native subtree is mounted on
// demand (mount_element_tree) into a MauiTableCellView that re-arranges it in -layout (frame-based, the
// AppKit twin of the iOS MauiTableViewCell / MauiCollectionViewItemContainerView), and the row self-sizes
// to the measured content via heightOfRow.

#import <AppKit/AppKit.h>
#import <objc/runtime.h>

#include <cmath>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "maui/controls/cells/view_cell.hpp"
#include "maui/controls/element.hpp"
#include "maui/controls/i_table_view.hpp"
#include "maui/controls/table_model.hpp"
#include "maui/controls/table_view_handler.hpp"
#include "maui/core/handler_registry.hpp"
#include "maui/core/i_maui_context.hpp"
#include "maui/core/i_view.hpp"
#include "maui/core/i_view_handler.hpp"
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
    // One entry in the AppKit flat layout: macOS NSTableView has no native grouped sections, so the port
    // flattens [ header(s0), cell(s0,0), …, header(s1), cell(s1,0), … ] into one column. A header entry is
    // a non-selectable section group row; a cell entry carries its [section,row] path.
    struct flat_entry
    {
        bool is_header = false;
        int section = 0;
        int row = 0; // valid only when !is_header
    };

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
        // The header-aware flat layout: a group-row header before each non-empty section, then its rows.
        static std::vector<flat_entry> flatten(table_model& model)
        {
            std::vector<flat_entry> entries;
            const int sections = model.get_section_count();
            for (int s = 0; s < sections; ++s)
            {
                const int rows = model.get_row_count(s);
                if (rows <= 0)
                {
                    continue; // empty sections contribute no header + no rows
                }
                entries.push_back({.is_header = true, .section = s, .row = 0});
                for (int r = 0; r < rows; ++r)
                {
                    entries.push_back({.is_header = false, .section = s, .row = r});
                }
            }
            return entries;
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

    // On-demand mount of an element's whole native subtree (the AppKit twin of the iOS handler's
    // mount_element_tree / the collection_view handler's ensure_mounted). A ViewCell's content View is not a
    // logical child of any page tree the generic mount walked, so it arrives here UNMOUNTED. Depth-first
    // POST-ORDER: attach each element's registered handler by handler_type_tag (SetMauiContext before
    // SetVirtualView, the C# order), then re-host via mount_into_handler. Idempotent (skips already-mounted).
    void mount_element_tree(maui::core::i_maui_context* context, maui::controls::element& root)
    {
        if (context == nullptr)
        {
            return;
        }
        root.visit_logical_children([context](maui::controls::element& child) { mount_element_tree(context, child); });

        auto* element_face = dynamic_cast<maui::core::i_element*>(&root);
        if (element_face == nullptr)
        {
            return;
        }
        if (!element_face->handler())
        {
            if (const std::optional<maui::core::type_tag> tag = root.handler_type_tag(); tag.has_value())
            {
                if (std::shared_ptr<maui::core::i_element_handler> handler = context->handlers().create_handler(*tag))
                {
                    handler->set_maui_context(context);
                    element_face->set_handler(std::move(handler));
                }
            }
        }
        root.mount_into_handler();
    }

    // The ViewCell's hosted content as an i_view (or null), its native subtree mounted on demand.
    maui::core::i_view* view_cell_content(maui::core::i_maui_context* context, const cell& source)
    {
        const auto* vc = dynamic_cast<const maui::controls::view_cell*>(&source);
        if (vc == nullptr || vc->view() == nullptr)
        {
            return nullptr;
        }
        if (auto* content_element = dynamic_cast<maui::controls::element*>(vc->view().get()))
        {
            mount_element_tree(context, *content_element);
        }
        return dynamic_cast<maui::core::i_view*>(vc->view().get());
    }
} // namespace

// An NSTableCellView that hosts a ViewCell's arbitrary MAUI content View, re-arranging it against the cell
// bounds on every layout pass (a plain NSView never wires -layout to anything). The AppKit twin of iOS's
// MauiTableViewCell and the direct analog of MauiCollectionViewItemContainerView's -layout override:
// frame-based measure+arrange (compute_frame), which autoresizing alone cannot express for a composite view.
@interface MauiTableCellView : NSTableCellView
- (void)hostContent:(NSView*)native forView:(maui::core::i_view*)view;
@end

@implementation MauiTableCellView
{
    NSView* _hosted;                 // the hosted content's native view (a subview of self)
    maui::core::i_view* _hostedView; // NON-owning: the model's view_cell co-owns the content View
}

- (void)hostContent:(NSView*)native forView:(maui::core::i_view*)view
{
    if (_hosted != native)
    {
        [_hosted removeFromSuperview];
        _hosted = native;
        if (native != nil)
        {
            native.autoresizingMask = NSViewNotSizable; // measure/arrange owns the frame
            [self addSubview:native];
        }
    }
    _hostedView = view;
    self.needsLayout = YES;
}

- (void)layout
{
    [super layout];
    if (_hostedView == nullptr)
    {
        return;
    }
    const NSRect bounds = self.bounds;
    _hostedView->measure(bounds.size.width, bounds.size.height);
    _hostedView->arrange(maui::graphics::rect{0.0, 0.0, bounds.size.width, bounds.size.height});
}
@end

// The datasource + delegate, holding a non-owning back-pointer to the C++ handler. It caches the
// header-aware flat layout (rebuilt on each reload) so numberOfRows / viewForTableColumn / selection all
// agree on which flat rows are section headers vs cells.
@interface MauiTableViewSource : NSObject <NSTableViewDataSource, NSTableViewDelegate>
@property(nonatomic, assign) table_view_handler* handler;
- (void)rebuildLayout;
@end

@implementation MauiTableViewSource
{
    std::vector<maui::controls::flat_entry> _layout;
}

- (void)rebuildLayout
{
    _layout.clear();
    if (self.handler != nullptr)
    {
        if (auto* model = maui::controls::table_view_source_bridge::model_for(*self.handler))
        {
            _layout = maui::controls::table_view_source_bridge::flatten(*model);
        }
    }
}

- (NSInteger)numberOfRowsInTableView:(NSTableView*)tableView
{
    (void)tableView;
    return static_cast<NSInteger>(_layout.size());
}

// AppKit group rows render the section header style (bold, full-width) — the macOS section-group-row look.
- (BOOL)tableView:(NSTableView*)tableView isGroupRow:(NSInteger)row
{
    (void)tableView;
    return row >= 0 && static_cast<std::size_t>(row) < _layout.size() &&
           _layout[static_cast<std::size_t>(row)].is_header;
}

// Headers are not selectable (they are group rows, like the C# section headers).
- (BOOL)tableView:(NSTableView*)tableView shouldSelectRow:(NSInteger)row
{
    (void)tableView;
    return !(row >= 0 && static_cast<std::size_t>(row) < _layout.size() &&
             _layout[static_cast<std::size_t>(row)].is_header);
}

// Build the row's native content: a bold header label for a group row, or — for a cell — the per-cell-type
// native content (a text label, plus a switch / text field / image view embedded in a container row view).
- (NSView*)tableView:(NSTableView*)tableView viewForTableColumn:(NSTableColumn*)tableColumn row:(NSInteger)row
{
    (void)tableColumn;
    table_view_handler* const handler = self.handler;
    if (handler == nullptr || row < 0 || static_cast<std::size_t>(row) >= _layout.size())
    {
        return nil;
    }
    table_model* const model = maui::controls::table_view_source_bridge::model_for(*handler);
    table_view_platform* const platform = maui::controls::table_view_source_bridge::platform_for(*handler);
    if (model == nullptr)
    {
        return nil;
    }
    const maui::controls::flat_entry entry = _layout[static_cast<std::size_t>(row)];

    if (entry.is_header)
    {
        // The section group row: a bold title label (NSTableViewRenderer's section header). Recorded in the
        // section_headers mirror by reload(); here we just build the native view.
        const std::string title = model->get_section_title(entry.section);
        NSTextField* header = [tableView makeViewWithIdentifier:@"maui_section_header" owner:self];
        if (header == nil)
        {
            header = [NSTextField labelWithString:@""];
            header.identifier = @"maui_section_header";
            header.font = [NSFont boldSystemFontOfSize:NSFont.smallSystemFontSize];
        }
        header.stringValue = [NSString stringWithUTF8String:title.c_str()];
        return header;
    }

    const maui::controls::table_row_path path{.section = entry.section, .row = entry.row};
    std::shared_ptr<cell> source = model->get_cell(path.section, path.row);
    const std::string reuse_id = source != nullptr ? table_view_handler::reuse_identifier(*source) : std::string{};
    NSString* const identifier = [NSString stringWithUTF8String:reuse_id.c_str()];

    // Dequeue a reusable container row view by the cell's reuse id (a fresh make = realize, a hit = reuse).
    NSTableCellView* rowView = [tableView makeViewWithIdentifier:identifier owner:self];
    const bool reused = rowView != nil;

    maui::controls::realized_row realized;
    realized.path = path;
    realized.reuse_id = reuse_id;
    if (source != nullptr)
    {
        realized.text = table_view_handler::display_text(*source);
        realized.source = source;
        table_view_handler::describe_cell(realized, *source); // the per-cell-type content fields
    }

    const bool is_view_cell =
        source != nullptr && table_view_handler::classify_cell(*source) == maui::controls::cell_content_kind::view;
    if (!reused)
    {
        // A view cell hosts an arbitrary content View → a MauiTableCellView that re-arranges the hosted
        // native subtree in -layout. The reuse id is TYPE-keyed, so view cells get a dedicated bucket and
        // never share a native row with the text/switch/entry cells (or vice-versa).
        rowView = is_view_cell ? [[MauiTableCellView alloc] initWithFrame:NSMakeRect(0, 0, 400, 24)]
                               : [[NSTableCellView alloc] initWithFrame:NSMakeRect(0, 0, 400, 24)];
        rowView.identifier = identifier;
        NSTextField* const label = [NSTextField labelWithString:@""];
        label.tag = 1001; // the primary text label (kept addressable across reuse)
        [rowView addSubview:label];
        rowView.textField = label;
        // The per-cell-type accessory (built once per realized view; rebound below for both make + reuse):
        switch (realized.content)
        {
            case maui::controls::cell_content_kind::toggle: {
                NSSwitch* const sw = [[NSSwitch alloc] initWithFrame:NSMakeRect(300, 0, 50, 24)];
                sw.tag = 1002;
                [rowView addSubview:sw];
                break;
            }
            case maui::controls::cell_content_kind::entry: {
                NSTextField* const tf = [[NSTextField alloc] initWithFrame:NSMakeRect(150, 0, 200, 24)];
                tf.tag = 1003;
                [rowView addSubview:tf];
                break;
            }
            case maui::controls::cell_content_kind::image: {
                NSImageView* const iv = [[NSImageView alloc] initWithFrame:NSMakeRect(0, 0, 24, 24)];
                iv.tag = 1004;
                [rowView addSubview:iv];
                break;
            }
            case maui::controls::cell_content_kind::text:
            case maui::controls::cell_content_kind::view:
            case maui::controls::cell_content_kind::none:
                break;
        }
    }

    // Bind the primary text + the per-cell-type content to the (possibly reused) native sub-controls.
    if (NSTextField* const label = [rowView viewWithTag:1001])
    {
        label.stringValue = [NSString stringWithUTF8String:realized.text.c_str()];
    }
    if (realized.content == maui::controls::cell_content_kind::toggle)
    {
        if (auto* sw = (NSSwitch*)[rowView viewWithTag:1002])
        {
            sw.state = realized.toggle_on ? NSControlStateValueOn : NSControlStateValueOff;
        }
    }
    else if (realized.content == maui::controls::cell_content_kind::entry)
    {
        if (auto* tf = (NSTextField*)[rowView viewWithTag:1003])
        {
            tf.stringValue = [NSString stringWithUTF8String:realized.entry_text.c_str()];
            tf.placeholderString = [NSString stringWithUTF8String:realized.entry_placeholder.c_str()];
        }
    }
    else if (realized.content == maui::controls::cell_content_kind::image)
    {
        if (auto* iv = (NSImageView*)[rowView viewWithTag:1004])
        {
            // A resolved ImageSource → a placeholder image so the view is populated (the real decode rides
            // the same image-service seam the image handler uses; the row only needs a non-nil image to
            // prove the image view was built + bound). No source → cleared.
            iv.image = realized.has_image ? [[NSImage alloc] initWithSize:NSMakeSize(24, 24)] : nil;
        }
    }
    else if (is_view_cell)
    {
        // ViewCellRenderer: host the cell's content View. Mount its native subtree on demand (it is not a
        // logical child of any page tree the generic mount walked) and hand the native NSView + its i_view to
        // the MauiTableCellView, which arranges it against the cell bounds in -layout.
        if (auto* view_shell = [rowView isKindOfClass:[MauiTableCellView class]] ? (MauiTableCellView*)rowView : nil)
        {
            NSView* native = nil;
            maui::core::i_view* content =
                source != nullptr ? view_cell_content(handler->maui_context(), *source) : nullptr;
            if (content != nullptr)
            {
                if (auto* content_handler = dynamic_cast<maui::core::i_view_handler*>(content->handler().get()))
                {
                    native = (__bridge NSView*)content_handler->native_view();
                }
            }
            [view_shell hostContent:native forView:native != nil ? content : nullptr];
        }
    }

    if (platform != nullptr)
    {
        platform->realized.push_back(realized);
        platform->events.push_back({.kind = reused ? table_row_event_kind::reused : table_row_event_kind::realized,
                                    .path = path,
                                    .reuse_id = reuse_id});
    }
    return rowView;
}

- (CGFloat)tableView:(NSTableView*)tableView heightOfRow:(NSInteger)row
{
    // A view cell self-sizes to its MEASURED content (a frame-based hosted View has no Auto Layout
    // constraints to size against); every other row (headers, text/switch/entry/image cells) keeps the
    // table's fixed rowHeight, preserving the prior behavior exactly.
    table_view_handler* const handler = self.handler;
    const CGFloat fixed = tableView.rowHeight > 0 ? tableView.rowHeight : 24.0;
    if (handler == nullptr || row < 0 || static_cast<std::size_t>(row) >= _layout.size())
    {
        return fixed;
    }
    const maui::controls::flat_entry entry = _layout[static_cast<std::size_t>(row)];
    table_model* const model = maui::controls::table_view_source_bridge::model_for(*handler);
    if (entry.is_header || model == nullptr)
    {
        return fixed;
    }
    std::shared_ptr<cell> source = model->get_cell(entry.section, entry.row);
    if (source == nullptr || table_view_handler::classify_cell(*source) != maui::controls::cell_content_kind::view)
    {
        return fixed;
    }
    if (auto* content = view_cell_content(handler->maui_context(), *source))
    {
        const maui::graphics::size measured = content->measure(tableView.bounds.size.width, INFINITY);
        return std::max(static_cast<CGFloat>(std::ceil(measured.height)), fixed);
    }
    return fixed;
}

- (void)tableViewSelectionDidChange:(NSNotification*)notification
{
    NSTableView* const tableView = (NSTableView*)notification.object;
    const NSInteger row = tableView.selectedRow;
    if (row < 0 || self.handler == nullptr || static_cast<std::size_t>(row) >= _layout.size())
    {
        return;
    }
    const maui::controls::flat_entry entry = _layout[static_cast<std::size_t>(row)];
    if (entry.is_header)
    {
        return; // group rows are not selectable
    }
    self.handler->simulate_select(entry.section, entry.row);
}
@end

namespace maui::controls
{
    table_view_platform::table_view_platform() = default;

    // The teardown that must run whether the handler is DISCONNECTED or merely DESTROYED. The native
    // view outlives the handler in any real app (a superview retains it) and the trampolines it keeps
    // in its associated objects carry RAW handler pointers; nothing calls disconnect_handler() when a
    // handler is destroyed (there is no ~view_handler doing it), so the platform dtor has to run this
    // too or the next native callback dereferences freed memory. Idempotent: disconnect_handler()
    // destroys the platform right after calling it, so both paths run on the same object.
    namespace
    {
        void detach_trampolines(table_view_platform& platform)
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
            platform.section_headers.clear();
        }
    } // namespace

    table_view_platform::~table_view_platform()
    {
        detach_trampolines(*this); // before any CFRelease: the void* slot holds the last retain
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
        detach_trampolines(platform);
    }

    void table_view_handler::reload()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->realized.clear(); // re-filled by viewForTableColumn during the layout pass
        platform->section_headers.clear();
        // Record the section headers/group rows for each non-empty section (the mirror the suites assert;
        // the native group rows are built by viewForTableColumn below).
        if (auto* model = virtual_view() != nullptr ? virtual_view()->model() : nullptr)
        {
            const int sections = model->get_section_count();
            for (int s = 0; s < sections; ++s)
            {
                if (model->get_row_count(s) > 0)
                {
                    platform->section_headers.push_back({.section = s, .title = model->get_section_title(s)});
                }
            }
        }
        NSTableView* const native = as_table(platform->native);
        // Rebuild the header-aware flat layout the datasource walks before reloadData reads its counts.
        if (auto* source = (MauiTableViewSource*)objc_getAssociatedObject(native, &g_source_key))
        {
            [source rebuildLayout];
        }
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
