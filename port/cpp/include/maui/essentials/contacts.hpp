#pragma once
// maui::application_model::communication::contacts    <=  Microsoft.Maui.ApplicationModel.Communication.Contacts
// (static facade) maui::application_model::communication::i_contacts  <=
// Microsoft.Maui.ApplicationModel.Communication.IContacts
//
// Lets the user pick a contact and read all device contacts. The C# `Task<Contact?> PickContactAsync()`
// and `Task<IEnumerable<Contact>> GetAllAsync(CancellationToken)` become the library's callback
// convention: pick delivers an optional contact (std::nullopt = the user cancelled), get_all delivers
// a vector + honours a cancellation_token (a cancelled query yields an empty vector, the CNContactStore
// cancellation analog). Backends complete inline.
//
// PickContactAsync is a UI seam: it presents CNContactPickerViewController, which is NOT drivable in
// the spawned simulator gtest process (no key window / root view controller - the same constraint the
// browser/share UI hit). So the iOS PickContact path is a DOCUMENTED service seam: the contract +
// presentation wiring exist, but the on-simulator suite cannot drive a selection; the headless fake is
// the behavioral test path (a staged optional contact). GetAllAsync reads CNContactStore directly (no
// UI) and is exercised on the simulator behaviorally where contacts exist.
//
// Backends (suffix oracle): apple/macOS - GetAllAsync REAL (Contacts.ios.macos.cs - CNContactStore),
// PickContactAsync NOT SUPPORTED (the macOS arm throws NotSupportedOrImplementedException - macOS has
// no CNContactPickerViewController); ios - both REAL (PickContact via CNContactPickerViewController,
// GetAll via CNContactStore), PickContact UI not sim-drivable (above). Headless mirrors netstandard
// (throws) until the fake stages a picked contact + a contact list.

#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include "maui/core/cancellation_token.hpp"
#include "maui/core/move_only_function.hpp"
#include "maui/essentials/contact.hpp"

namespace maui::application_model::communication
{
    // Receives the picked contact (std::nullopt = the user cancelled) - the Task<Contact?> result.
    using pick_contact_callback = maui::core::move_only_function<void(std::optional<contact>)>;
    // Receives every device contact - the Task<IEnumerable<Contact>> result.
    using all_contacts_callback = maui::core::move_only_function<void(std::vector<contact>)>;

    class i_contacts
    {
    public:
        virtual ~i_contacts() = default;

        // PickContactAsync: present the OS contact picker.
        virtual void pick_contact_async(pick_contact_callback on_complete) = 0;
        // GetAllAsync: read every device contact (a cancelled token yields no contacts).
        virtual void get_all_async(maui::core::cancellation_token token, all_contacts_callback on_complete) = 0;

    protected:
        i_contacts() = default;
        i_contacts(const i_contacts&) = default;
        i_contacts(i_contacts&&) = default;
        i_contacts& operator=(const i_contacts&) = default;
        i_contacts& operator=(i_contacts&&) = default;
    };

    namespace detail
    {
        // The platform partial's factory (ContactsImplementation), one per backend under
        // src/platform/<backend>/essentials_contacts.{cpp,mm}.
        [[nodiscard]] std::shared_ptr<i_contacts> make_contacts();
    } // namespace detail

    // The static facade over contacts::default_() (C# Contacts).
    class contacts final
    {
    public:
        contacts() = delete;

        static void pick_contact_async(pick_contact_callback on_complete)
        {
            default_().pick_contact_async(std::move(on_complete));
        }
        // GetAllAsync() with no cancellation (the C# default CancellationToken).
        static void get_all_async(all_contacts_callback on_complete)
        {
            default_().get_all_async(maui::core::cancellation_token{}, std::move(on_complete));
        }
        static void get_all_async(maui::core::cancellation_token token, all_contacts_callback on_complete)
        {
            default_().get_all_async(token, std::move(on_complete));
        }

        // Contacts.Default (lazy platform default) + SetDefault (the C# internal test seam made
        // public; nullptr resets to the lazy platform default).
        [[nodiscard]] static i_contacts& default_();
        static void set_default(std::shared_ptr<i_contacts> implementation);
    };
} // namespace maui::application_model::communication
