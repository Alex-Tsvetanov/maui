// contacts - Apple (AppKit / macOS) platform partial. Ported from Contacts.ios.macos.cs: GetAllAsync
// reads CNContactStore (enumerate containers -> unified contacts -> convert each), honouring the
// cancellation token (a cancelled query yields no contacts). PickContactAsync is NOT SUPPORTED on
// macOS (the C# macOS arm throws NotSupportedOrImplementedException - there is no
// CNContactPickerViewController on macOS). Reading contacts requests authorization implicitly through
// CNContactStore; in an unbundled / unauthorized process the enumerate returns nothing, so GetAll
// completes with an empty list rather than throwing. Compiled as Objective-C++ with ARC for the apple
// backend.

#import <Contacts/Contacts.h>
#import <Foundation/Foundation.h>

#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "maui/essentials/contacts.hpp"
#include "maui/essentials/feature_not_supported.hpp"

#include "src/platform/apple_shared/essentials_url.hpp"

namespace maui::application_model::communication
{
    namespace
    {
        using maui::platform::apple_shared::to_std_string;

        // ConvertContact(CNContact): the name parts + the phone/email value strings. Index iteration
        // (vs typed fast-enumeration) keeps each loop variable initialized for clang-tidy.
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

        class apple_contacts final : public i_contacts
        {
        public:
            // macOS: PickContactAsync throws NotSupportedOrImplementedException.
            void pick_contact_async(pick_contact_callback /*on_complete*/) override
            {
                throw maui::application_model::feature_not_supported();
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
            return std::make_shared<apple_contacts>();
        }
    } // namespace detail
} // namespace maui::application_model::communication
