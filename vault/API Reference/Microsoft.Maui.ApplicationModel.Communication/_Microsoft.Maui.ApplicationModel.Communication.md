---
title: "Microsoft.Maui.ApplicationModel.Communication"
tags:
  - api
  - namespace
  - ns/Microsoft-Maui-ApplicationModel-Communication
---

# Microsoft.Maui.ApplicationModel.Communication

> [!info] Namespace
> `Microsoft.Maui.ApplicationModel.Communication` — 16 public types.

- [Online namespace docs](https://learn.microsoft.com/dotnet/api/microsoft.maui.applicationmodel.communication)

## Overview

`Microsoft.Maui.ApplicationModel.Communication` provides cross-platform access to the device's native communication features, letting a .NET MAUI app reach into the operating system to send messages, compose emails, place calls, and read contact information without writing platform-specific code. Rather than embedding mail servers or telephony stacks, these APIs hand off to the OS-provided UIs and apps — the default email composer, the SMS app, the phone dialer, and the contact picker — so the user stays in control of the interaction.

The namespace groups four loosely related capability areas. For email, [[Email|Email]] (and its interface [[IEmail|IEmail]]) opens the default mail composer using an [[EmailMessage|EmailMessage]] that carries recipients, an [[EmailBodyFormat|EmailBodyFormat]], and one or more [[EmailAttachment|EmailAttachment]] items. For text messaging, [[Sms (Communication)|Sms]] / [[ISms|ISms]] launch the default SMS app pre-populated from an [[SmsMessage|SmsMessage]]. For voice, [[PhoneDialer|PhoneDialer]] / [[IPhoneDialer|IPhoneDialer]] open a phone number in the system dialer.

Contacts round out the set: [[Contacts|Contacts]] / [[IContacts|IContacts]] present the OS contact picker and return a [[Contact|Contact]], whose associated [[ContactPhone|ContactPhone]] and [[ContactEmail|ContactEmail]] values can feed directly back into the dialer, SMS, or email flows. Most static entry points also expose support checks, so apps can detect whether a given capability is available before invoking it.

## Key types

- [[Email|Email]] — Provides an easy way to allow the user to send emails via the default composer.
- [[EmailMessage|EmailMessage]] — Represents a single email message, including recipients, body, and attachments.
- [[EmailAttachment|EmailAttachment]] — A file attached to an email message.
- [[EmailBodyFormat|EmailBodyFormat]] — Represents the available email body formats (for example plain text).
- [[Sms (Communication)|Sms]] — Composes SMS messages through the device's default messaging app.
- [[SmsMessage|SmsMessage]] — Represents a single SMS message to send to a recipient.
- [[PhoneDialer|PhoneDialer]] — Opens a phone number in the system dialer.
- [[Contacts|Contacts]] — Opens the OS default UI for picking a contact from the device.
- [[Contact|Contact]] — Represents a contact on the user's device.
- [[ContactPhone|ContactPhone]] — A phone number associated with a `Contact`.
- [[ContactEmail|ContactEmail]] — An email address associated with a `Contact`.
- [[IContacts|IContacts]] — The Contacts API for picking a contact and retrieving its information.


## Classes

| Type | Summary |
|---|---|
| [[Contact\|Contact]] | Represents a contact on the user's device. |
| [[ContactEmail\|ContactEmail]] | Represents an email address that is associated with a `Contact`. |
| [[ContactPhone\|ContactPhone]] | Represents a phone number that is associated with a `Contact`. |
| [[Contacts\|Contacts]] | Opens the operating system's default UI for picking a contact from the device. |
| [[Email\|Email]] | Provides an easy way to allow the user to send emails. |
| [[EmailAttachment\|EmailAttachment]] | The email message body is plain text. |
| [[EmailExtensions\|EmailExtensions]] | Gets a value indicating whether composing an email is supported on this device. |
| [[EmailMessage\|EmailMessage]] | Represents a single email message. |
| [[PhoneDialer\|PhoneDialer]] | Gets a value indicating whether using the phone dialer is supported on this device. |
| [[Sms (Communication)\|Sms (Communication)]] | Gets a value indicating whether composing of SMS messages is supported on this device. |
| [[SmsMessage\|SmsMessage]] | Represents a single SMS message. |

## Interfaces

| Type | Summary |
|---|---|
| [[IContacts\|IContacts]] | The Contacts API lets a user pick a contact and retrieve information about it. |
| [[IEmail\|IEmail]] | Provides an easy way to allow the user to send emails. |
| [[IPhoneDialer\|IPhoneDialer]] | The PhoneDialer API enables an application to open a phone number in the dialer. |
| [[ISms\|ISms]] | The SMS API enables an application to open the default SMS application with a specified message to send to a recipient. |

## Enums

| Type | Summary |
|---|---|
| [[EmailBodyFormat\|EmailBodyFormat]] | Represents various types of email body formats. |

## See also

- [[_API Reference]]
