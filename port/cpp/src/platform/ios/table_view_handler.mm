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
// Coverage note (documented): cellForRow builds the PER-CELL-TYPE native content (the C# per-cell
// renderers' GetCell) — a UISwitch accessory bound to switch_cell.On, a UITextField accessory bound to
// entry_cell.Text/Placeholder, the cell's imageView populated for image_cell — plus text/detail labels.
// Section headers render natively via titleForHeaderInSection (recorded in the section_headers mirror).
// The image is a placeholder when the cell has a resolved ImageSource (the full async thumbnail decode
// rides the image-service seam, W3-31). view_cell hosts its arbitrary content View: the View's whole
// native subtree is mounted ON DEMAND (mount_element_tree, the analog of the collection_view handler's
// ensure_mounted / the C# ViewCellRenderer's ToPlatform), hosted in a MauiTableViewCell that re-arranges
// it in -layoutSubviews (frame-based, mirroring MauiCollectionViewItem.layoutTemplatedContent), and the
// row self-sizes to the measured content via heightForRowAtIndexPath.

#import <UIKit/UIKit.h>
#import <objc/runtime.h>

#include <algorithm>
#include <cmath>
#include <memory>
#include <optional>
#include <string>
#include <utility>

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

    // On-demand mount of an element's whole native subtree, mirroring the collection_view handler's
    // ensure_mounted (and the C# ToPlatform-on-demand path). A ViewCell's content View is NOT a logical
    // child of any page tree the generic mount walks, so it arrives here UNMOUNTED — its handler/native
    // view unbuilt. Depth-first POST-ORDER (children first, so each child's native view exists before its
    // parent hosts it): attach each element's registered handler by its runtime handler_type_tag
    // (SetMauiContext before SetVirtualView, the C# order), then re-fire the container host command
    // (mount_into_handler) so the now-attached children's native views are hosted. Idempotent — an element
    // that already carries a handler is skipped (re-attaching would rebuild + orphan the old native view).
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
        if (!element_face->handler()) // skip an already-mounted element (idempotent re-mount guard)
        {
            if (const std::optional<maui::core::type_tag> tag = root.handler_type_tag(); tag.has_value())
            {
                if (std::shared_ptr<maui::core::i_element_handler> handler = context->handlers().create_handler(*tag))
                {
                    handler->set_maui_context(context);            // SetMauiContext precedes SetVirtualView (C#)
                    element_face->set_handler(std::move(handler)); // the view owns its handler (PROFILE §11)
                }
            }
        }
        root.mount_into_handler(); // re-host the (now-attached) children's native views
    }

    // The ViewCell's hosted content as an i_view (or null when the cell hosts nothing / a non-view element),
    // its native subtree mounted on demand. The view_cell (held by the table_model) co-owns the content, so
    // the returned native view outlives this call as long as the model holds the cell.
    maui::core::i_view* view_cell_content(maui::core::i_maui_context* context, const cell& source)
    {
        const auto* vc = dynamic_cast<const maui::controls::view_cell*>(&source);
        if (vc == nullptr || vc->view() == nullptr)
        {
            return nullptr;
        }
        auto* content_element = dynamic_cast<maui::controls::element*>(vc->view().get());
        if (content_element != nullptr)
        {
            mount_element_tree(context, *content_element);
        }
        return dynamic_cast<maui::core::i_view*>(vc->view().get());
    }
} // namespace

// A UITableViewCell that hosts a ViewCell's arbitrary MAUI content View. The C# ViewCellRenderer hosts the
// View in the cell's contentView; here the View's native subtree (built by mount_element_tree) is added as
// a contentView subview and RE-ARRANGED on every layout pass against the cell's real bounds — frame-based,
// the direct analog of MauiCollectionViewItem.layoutTemplatedContent (autoresizing alone cannot arrange a
// composite View's internal children). A dedicated reuse bucket (the view_cell typeid) means only view
// cells ever dequeue this class.
@interface MauiTableViewCell : UITableViewCell
// Host `native` (the mounted content's UIView) for `view` (its cross-platform i_view, arranged in layout).
- (void)hostContent:(UIView*)native forView:(maui::core::i_view*)view;
@end

@implementation MauiTableViewCell
{
    UIView* _hosted;                 // the hosted content's native view (a contentView subview)
    maui::core::i_view* _hostedView; // NON-owning: the model's view_cell co-owns the content View
}

- (void)hostContent:(UIView*)native forView:(maui::core::i_view*)view
{
    if (_hosted != native)
    {
        [_hosted removeFromSuperview];
        _hosted = native;
        if (native != nil)
        {
            native.autoresizingMask = UIViewAutoresizingNone; // measure/arrange owns the frame, not autoresize
            [self.contentView addSubview:native];
        }
    }
    _hostedView = view;
    [self setNeedsLayout];
}

- (void)prepareForReuse
{
    [super prepareForReuse];
    [_hosted removeFromSuperview];
    _hosted = nil;
    _hostedView = nullptr;
}

- (void)layoutSubviews
{
    [super layoutSubviews];
    if (_hostedView == nullptr)
    {
        return;
    }
    // Measure the content against the cell, then arrange it across the cell bounds — arrange resolves the
    // aligned frame (compute_frame) and pushes native frames via platform_arrange, recursing into children.
    const CGRect bounds = self.contentView.bounds;
    _hostedView->measure(bounds.size.width, bounds.size.height);
    _hostedView->arrange(maui::graphics::rect{0.0, 0.0, bounds.size.width, bounds.size.height});
}
@end

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
    const bool is_view_cell =
        source != nullptr && table_view_handler::classify_cell(*source) == maui::controls::cell_content_kind::view;
    if (!reused)
    {
        // A view cell hosts an arbitrary content View → its own MauiTableViewCell (re-arranges the hosted
        // native subtree in -layoutSubviews). The reuse id is TYPE-keyed, so view cells get a dedicated
        // bucket and never dequeue a subtitle cell (or vice-versa). Every other cell uses Subtitle style so
        // the text/detail labels exist; ImageCell uses the cell's imageView.
        dequeued = is_view_cell ? [[MauiTableViewCell alloc] initWithStyle:UITableViewCellStyleDefault
                                                           reuseIdentifier:identifier]
                                : [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleSubtitle
                                                         reuseIdentifier:identifier];
    }

    const maui::controls::table_row_path path{.section = section, .row = row};
    maui::controls::realized_row realized;
    realized.path = path;
    realized.reuse_id = reuse_id;
    if (source != nullptr)
    {
        realized.text = table_view_handler::display_text(*source);
        realized.source = source;
        table_view_handler::describe_cell(realized, *source); // the per-cell-type content fields
    }

    // The primary text + the per-cell-type native content (the C# per-cell renderers' GetCell). The reuse
    // id is TYPE-KEYED, so a dequeued cell already carries the matching accessory type — BUILD the accessory
    // only on the !reused branch (tagged, mirroring the AppKit twin's 1002/1003/1004), then on reuse just
    // rebind on/text/placeholder/image via viewWithTag:. Rebuilding on every reuse (the bug) churned the
    // accessory object each time, defeating cell recycling and dropping in-progress edits.
    dequeued.textLabel.text = [NSString stringWithUTF8String:realized.text.c_str()];
    dequeued.detailTextLabel.text = [NSString stringWithUTF8String:realized.detail.c_str()];

    if (!reused)
    {
        // First realization for this reuse id: build the per-cell-type accessory once. (image rides the
        // cell's built-in imageView, which is recycled with the cell — no separate accessory needed.)
        switch (realized.content)
        {
            case maui::controls::cell_content_kind::toggle: {
                // SwitchCellRenderer: a UISwitch accessory bound to switch_cell.On.
                UISwitch* const sw = [[UISwitch alloc] initWithFrame:CGRectZero];
                sw.tag = 1002;
                dequeued.accessoryView = sw;
                break;
            }
            case maui::controls::cell_content_kind::entry: {
                // EntryCellRenderer: a UITextField accessory bound to entry_cell.Text/Placeholder (the label
                // rides textLabel via display_text → entry_cell.Label).
                UITextField* const tf = [[UITextField alloc] initWithFrame:CGRectMake(0, 0, 200, 30)];
                tf.tag = 1003;
                dequeued.accessoryView = tf;
                break;
            }
            case maui::controls::cell_content_kind::image:
            case maui::controls::cell_content_kind::text:
            case maui::controls::cell_content_kind::view:
            case maui::controls::cell_content_kind::none:
                dequeued.accessoryView = nil;
                break;
        }
    }

    // Rebind the (possibly reused) accessory's value — for BOTH first realize and reuse.
    switch (realized.content)
    {
        case maui::controls::cell_content_kind::toggle: {
            if (auto* sw = (UISwitch*)[dequeued.accessoryView viewWithTag:1002])
            {
                sw.on = realized.toggle_on;
            }
            break;
        }
        case maui::controls::cell_content_kind::entry: {
            if (auto* tf = (UITextField*)[dequeued.accessoryView viewWithTag:1003])
            {
                tf.text = [NSString stringWithUTF8String:realized.entry_text.c_str()];
                tf.placeholder = [NSString stringWithUTF8String:realized.entry_placeholder.c_str()];
            }
            break;
        }
        case maui::controls::cell_content_kind::image: {
            // ImageCellRenderer: imageView.Image from the resolved ImageSource (a placeholder proves the
            // image view is built + bound; the full async decode rides the image-service seam, W3-31).
            if (realized.has_image)
            {
                UIGraphicsImageRendererFormat* const format = [[UIGraphicsImageRendererFormat alloc] init];
                format.scale = 1;
                UIGraphicsImageRenderer* const renderer =
                    [[UIGraphicsImageRenderer alloc] initWithSize:CGSizeMake(24, 24) format:format];
                dequeued.imageView.image = [renderer imageWithActions:^(UIGraphicsImageRendererContext* context) {
                  [[UIColor grayColor] setFill];
                  [context fillRect:CGRectMake(0, 0, 24, 24)];
                }];
            }
            else
            {
                dequeued.imageView.image = nil;
            }
            break;
        }
        case maui::controls::cell_content_kind::view: {
            // ViewCellRenderer: host the cell's content View. Mount its native subtree on demand (it is not
            // a logical child of any page tree the generic mount walked, so it arrives unbuilt) and hand the
            // native view + its i_view to the MauiTableViewCell, which arranges it against the cell bounds in
            // -layoutSubviews. A view cell always dequeues a MauiTableViewCell (dedicated type bucket).
            if (auto* view_cell_shell =
                    [dequeued isKindOfClass:[MauiTableViewCell class]] ? (MauiTableViewCell*)dequeued : nil)
            {
                UIView* native = nil;
                if (source != nullptr)
                {
                    if (auto* content = view_cell_content(handler->maui_context(), *source))
                    {
                        if (auto* content_handler = dynamic_cast<maui::core::i_view_handler*>(content->handler().get()))
                        {
                            native = (__bridge UIView*)content_handler->native_view();
                        }
                        [view_cell_shell hostContent:native forView:native != nil ? content : nullptr];
                        break;
                    }
                }
                [view_cell_shell hostContent:nil forView:nullptr]; // no content / no native view
            }
            break;
        }
        case maui::controls::cell_content_kind::text:
        case maui::controls::cell_content_kind::none:
            break;
    }

    if (platform != nullptr)
    {
        platform->realized.push_back(realized);
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

- (CGFloat)tableView:(UITableView*)tableView heightForRowAtIndexPath:(NSIndexPath*)indexPath
{
    // A view cell self-sizes to its MEASURED content — a frame-based hosted View has no Auto Layout
    // constraints for UITableViewAutomaticDimension to size against, so measure it directly and return a
    // concrete height (floored at the standard 44pt minimum). Every other row keeps the mapped rowHeight
    // (>0) else UITableViewAutomaticDimension (the built-in subtitle/switch/entry cells self-size).
    table_view_handler* const handler = self.handler;
    table_model* const model =
        handler != nullptr ? maui::controls::table_view_source_bridge::model_for(*handler) : nullptr;
    const CGFloat mapped = tableView.rowHeight;
    if (handler == nullptr || model == nullptr)
    {
        return mapped > 0 ? mapped : UITableViewAutomaticDimension;
    }
    std::shared_ptr<cell> source =
        model->get_cell(static_cast<int>(indexPath.section), static_cast<int>(indexPath.row));
    if (source == nullptr || table_view_handler::classify_cell(*source) != maui::controls::cell_content_kind::view)
    {
        return mapped > 0 ? mapped : UITableViewAutomaticDimension;
    }
    if (auto* content = view_cell_content(handler->maui_context(), *source))
    {
        const maui::graphics::size measured = content->measure(tableView.bounds.size.width, INFINITY);
        return std::max(static_cast<CGFloat>(std::ceil(measured.height)), static_cast<CGFloat>(44.0));
    }
    return mapped > 0 ? mapped : static_cast<CGFloat>(44.0);
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
        detach_trampolines(platform);
    }

    void table_view_handler::reload()
    {
        auto* platform = typed_platform_view();
        if (platform == nullptr || platform->native == nullptr)
        {
            return;
        }
        platform->realized.clear(); // re-filled by cellForRow during the layout pass
        platform->section_headers.clear();
        // Record the section headers for each non-empty section (UITableView renders them natively via
        // titleForHeaderInSection; the mirror lets the suites assert the same oracle as headless/apple).
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
