#pragma once
// maui::application_model::communication::contact        <=  Microsoft.Maui.ApplicationModel.Communication.Contact
// maui::application_model::communication::contact_email  <=  Microsoft.Maui.ApplicationModel.Communication.ContactEmail
// maui::application_model::communication::contact_phone  <=  Microsoft.Maui.ApplicationModel.Communication.ContactPhone
//
// The pure value model behind the Contacts API (Types/Contact.shared.cs). Backend-independent: a
// contact's name parts + phone/email collections, with the C# DisplayName fallback rule (an inferred
// "<given> <family>" when no display name was set). Lives in its own namespace
// (maui::application_model::communication) matching the C# Communication namespace; contacts.hpp's
// facade reuses these.

#include <string>
#include <utility>
#include <vector>

namespace maui::application_model::communication
{
    // ContactEmail: a single email address associated with a contact.
    class contact_email
    {
    public:
        contact_email() = default;
        explicit contact_email(std::string email_address) : email_address_(std::move(email_address))
        {
        }

        [[nodiscard]] const std::string& email_address() const
        {
            return email_address_;
        }
        void set_email_address(std::string value)
        {
            email_address_ = std::move(value);
        }

        // ToString() => EmailAddress.
        [[nodiscard]] std::string to_string() const
        {
            return email_address_;
        }

    private:
        std::string email_address_;
    };

    // ContactPhone: a single phone number associated with a contact.
    class contact_phone
    {
    public:
        contact_phone() = default;
        explicit contact_phone(std::string phone_number) : phone_number_(std::move(phone_number))
        {
        }

        [[nodiscard]] const std::string& phone_number() const
        {
            return phone_number_;
        }
        void set_phone_number(std::string value)
        {
            phone_number_ = std::move(value);
        }

        // ToString() => PhoneNumber.
        [[nodiscard]] std::string to_string() const
        {
            return phone_number_;
        }

    private:
        std::string phone_number_;
    };

    // Contact: a contact on the user's device.
    class contact
    {
    public:
        contact() = default;

        // The full-data ctor (Contact(id, namePrefix, givenName, middleName, familyName, nameSuffix,
        // phones, emails, displayName = null)).
        contact(std::string id, std::string name_prefix, std::string given_name, std::string middle_name,
                std::string family_name, std::string name_suffix, std::vector<contact_phone> phones,
                std::vector<contact_email> emails, std::string display_name = {})
            : id_(std::move(id)), name_prefix_(std::move(name_prefix)), given_name_(std::move(given_name)),
              middle_name_(std::move(middle_name)), family_name_(std::move(family_name)),
              name_suffix_(std::move(name_suffix)), phones_(std::move(phones)), emails_(std::move(emails)),
              display_name_(std::move(display_name))
        {
        }

        [[nodiscard]] const std::string& id() const
        {
            return id_;
        }
        void set_id(std::string value)
        {
            id_ = std::move(value);
        }

        [[nodiscard]] const std::string& name_prefix() const
        {
            return name_prefix_;
        }
        void set_name_prefix(std::string value)
        {
            name_prefix_ = std::move(value);
        }

        [[nodiscard]] const std::string& given_name() const
        {
            return given_name_;
        }
        void set_given_name(std::string value)
        {
            given_name_ = std::move(value);
        }

        [[nodiscard]] const std::string& middle_name() const
        {
            return middle_name_;
        }
        void set_middle_name(std::string value)
        {
            middle_name_ = std::move(value);
        }

        [[nodiscard]] const std::string& family_name() const
        {
            return family_name_;
        }
        void set_family_name(std::string value)
        {
            family_name_ = std::move(value);
        }

        [[nodiscard]] const std::string& name_suffix() const
        {
            return name_suffix_;
        }
        void set_name_suffix(std::string value)
        {
            name_suffix_ = std::move(value);
        }

        [[nodiscard]] std::vector<contact_phone>& phones()
        {
            return phones_;
        }
        [[nodiscard]] const std::vector<contact_phone>& phones() const
        {
            return phones_;
        }

        [[nodiscard]] std::vector<contact_email>& emails()
        {
            return emails_;
        }
        [[nodiscard]] const std::vector<contact_email>& emails() const
        {
            return emails_;
        }

        // DisplayName: the explicit display name, or the inferred "<given> <family>" fallback
        // (BuildDisplayName: given-only / family-only / "given family"). The C# setter is private,
        // so the port sets it only through the ctor / set_display_name.
        [[nodiscard]] std::string display_name() const
        {
            return !is_blank(display_name_) ? display_name_ : build_display_name();
        }
        void set_display_name(std::string value)
        {
            display_name_ = std::move(value);
        }

        // ToString() => DisplayName.
        [[nodiscard]] std::string to_string() const
        {
            return display_name();
        }

    private:
        // string.IsNullOrWhiteSpace: empty or all-whitespace.
        [[nodiscard]] static bool is_blank(const std::string& value)
        {
            return value.find_first_not_of(" \t\n\v\f\r") == std::string::npos;
        }

        [[nodiscard]] std::string build_display_name() const
        {
            if (is_blank(given_name_))
            {
                return family_name_;
            }
            if (is_blank(family_name_))
            {
                return given_name_;
            }
            return given_name_ + " " + family_name_;
        }

        std::string id_;
        std::string name_prefix_;
        std::string given_name_;
        std::string middle_name_;
        std::string family_name_;
        std::string name_suffix_;
        std::vector<contact_phone> phones_;
        std::vector<contact_email> emails_;
        std::string display_name_;
    };
} // namespace maui::application_model::communication
