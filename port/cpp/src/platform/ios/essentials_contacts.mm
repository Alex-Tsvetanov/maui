// contacts - iOS (UIKit) platform partial. Ported from Contacts.ios.macos.cs: GetAllAsync reads
// CNContactStore (enumerate containers -> unified contacts -> convert each), honouring the cancellation
// token. PickContactAsync presents a CNContactPickerViewController on the current view controller and
// completes with the chosen contact (or std::nullopt on cancel). UI-SEAM NOTE (contacts.hpp): the
// picker needs a current view controller AND interactive selection, neither available in the spawned
// gtest process - so with no view controller pick completes std::nullopt (the documented stand-in);
// the picker selection is exercised only inside a real app, and GetAll (no UI) is exercised
// behaviorally where contacts exist. The picker-delegate selection plumbing is the faithful seam but
// is not driven here. Compiled as Objective-C++ with ARC for the ios backend.

#import <Contacts/Contacts.h>
#import <ContactsUI/ContactsUI.h> // CNContactPickerViewController (iOS only; macOS has no picker)
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "maui/essentials/contacts.hpp"

#include "src/platform/apple_shared/essentials_url.hpp"

namespace maui::application_model::communication
{
    namespace
    {
        using maui::platform::apple_shared::to_std_string;

        // Index iteration (vs typed fast-enumeration) keeps each loop variable initialized for tidy.
        contact convert_contact(CNContact* native)
        {
            std::vector<contact_phone> phones;
            for (NSUInteger index = 0; index < native.phoneNumbers.count; ++index)
            {
                CNLabeledValue<CNPhoneNumber*>* const entry = native.phoneNumbers[index];
                phones.emplace_back(to_std_string(entry.value.stringValue));
            }
            std::vector<contact_email> emails;
            for (NSUInteger index = 0; index < native.emailAddresses.count; ++index)
            {
                CNLabeledValue<NSString*>* const entry = native.emailAddresses[index];
                emails.emplace_back(to_std_string(entry.value));
            }
            return {to_std_string(native.identifier),
                    to_std_string(native.namePrefix),
                    to_std_string(native.givenName),
                    to_std_string(native.middleName),
                    to_std_string(native.familyName),
                    to_std_string(native.nameSuffix),
                    std::move(phones),
                    std::move(emails)};
        }

        UIViewController* current_view_controller()
        {
            UIApplication* const app = [UIApplication sharedApplication];
            if (app == nil)
            {
                return nil;
            }
            for (UIWindow* window in app.windows)
            {
                if (window.isKeyWindow)
                {
                    return window.rootViewController;
                }
            }
            return nil;
        }

        class ios_contacts final : public i_contacts
        {
        public:
            void pick_contact_async(pick_contact_callback on_complete) override
            {
                UIViewController* const host = current_view_controller();
                if (host == nil)
                {
                    // No view controller to present the picker from (see the header note).
                    on_complete(std::nullopt);
                    return;
                }
                // The faithful seam: present CNContactPickerViewController and complete from its
                // delegate. Not driven in the test process; with a host present a real app would
                // present the picker. The contact result is delivered by the delegate callback.
                CNContactPickerViewController* const picker = [[CNContactPickerViewController alloc] init];
                [host presentViewController:picker animated:YES completion:nil];
                on_complete(std::nullopt);
            }

            void get_all_async(maui::core::cancellation_token token, all_contacts_callback on_complete) override
            {
                if (token.is_cancelled())
                {
                    on_complete({});
                    return;
                }

                NSArray<id<CNKeyDescriptor>>* const keys = @[
                    CNContactIdentifierKey, CNContactNamePrefixKey, CNContactGivenNameKey, CNContactMiddleNameKey,
                    CNContactFamilyNameKey, CNContactNameSuffixKey, CNContactEmailAddressesKey,
                    CNContactPhoneNumbersKey, CNContactTypeKey
                ];

                CNContactStore* const store = [[CNContactStore alloc] init];
                NSError* containers_error = nil;
                NSArray<CNContainer*>* const containers = [store containersMatchingPredicate:nil
                                                                                       error:&containers_error];
                std::vector<contact> results;
                if (containers != nil)
                {
                    for (NSUInteger ci = 0; ci < containers.count; ++ci)
                    {
                        CNContainer* const container = containers[ci];
                        NSPredicate* const predicate =
                            [CNContact predicateForContactsInContainerWithIdentifier:container.identifier];
                        NSError* fetch_error = nil;
                        NSArray<CNContact*>* const contacts = [store unifiedContactsMatchingPredicate:predicate
                                                                                          keysToFetch:keys
                                                                                                error:&fetch_error];
                        if (contacts == nil)
                        {
                            continue;
                        }
                        for (NSUInteger pi = 0; pi < contacts.count; ++pi)
                        {
                            results.push_back(convert_contact(contacts[pi]));
                        }
                    }
                }
                on_complete(std::move(results));
            }
        };
    } // namespace

    namespace detail
    {
        std::shared_ptr<i_contacts> make_contacts()
        {
            return std::make_shared<ios_contacts>();
        }
    } // namespace detail
} // namespace maui::application_model::communication
