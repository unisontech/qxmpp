/*
 * Copyright (C) 2008-2012 The QXmpp developers
 *
 * Author:
 *  Manjeet Dahiya
 *
 * Source:
 *  http://code.google.com/p/qxmpp
 *
 * This file is a part of QXmpp library.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 */

#include "QXmppPresence.h"
#include "QXmppUtils.h"
#include <QtDebug>
#include <QDomElement>
#include <QXmlStreamWriter>
#include "QXmppConstants.h"

static const char* presence_types[] = {
    "error",
    "",
    "unavailable",
    "subscribe",
    "subscribed",
    "unsubscribe",
    "unsubscribed",
    "probe"
};

static const char* presence_shows[] = {
    "",
    "away",
    "xa",
    "dnd",
    "chat",
    "invisible"
};

static const char* presence_info[] = {
    "",
    "autoaway",
    "onphone",
    "inliveroom"
};

class QXmppPresencePrivate : public QSharedData
{
public:
    QXmppPresence::AvailableStatusType availableStatusType;
    QXmppPresence::Type type;
    QXmppPresence::Status status;

    /// XEP-0153: vCard-Based Avatars

    /// photoHash: the SHA1 hash of the avatar image data itself (not the base64-encoded version)
    /// in accordance with RFC 3174
    QByteArray photoHash;
    QXmppPresence::VCardUpdateType vCardUpdateType;

    // XEP-0115: Entity Capabilities
    QString capabilityHash;
    QString capabilityNode;
    QByteArray capabilityVer;
    // Legacy XEP-0115: Entity Capabilities
    QStringList capabilityExt;

    // XEP-0045: Multi-User Chat
    QXmppMucItem mucItem;
    QString mucPassword;
    QList<int> mucStatusCodes;
    bool mucSupported;
};

/// Constructs a QXmppPresence.
///
/// \param type

QXmppPresence::QXmppPresence(QXmppPresence::Type type)
    : d(new QXmppPresencePrivate)
{
    d->type = type;
    d->mucSupported = false;
    d->vCardUpdateType = VCardUpdateNone;
}

/// Constructs a copy of \a other.

QXmppPresence::QXmppPresence(const QXmppPresence &other)
    : QXmppStanza(other)
    , d(other.d)
{
}

/// Destroys a QXmppPresence.

QXmppPresence::~QXmppPresence()
{

}

/// Assigns \a other to this presence.

QXmppPresence &QXmppPresence::operator=(const QXmppPresence &other)
{
    QXmppStanza::operator=(other);
    d = other.d;
    return *this;
}

/// Returns the availability status type, for instance busy or away.
///
/// This will not tell you whether a contact is connected, check whether
/// type() is QXmppPresence::Available instead.

QXmppPresence::AvailableStatusType QXmppPresence::availableStatusType() const
{
    QXmppPresence::Status::Info info = d->status.info();
    if (info) {
        switch (info) {
        case QXmppPresence::Status::Autoaway:
            return QXmppPresence::Autoaway;
        case QXmppPresence::Status::OnPhone:
            return QXmppPresence::OnPhone;
        case QXmppPresence::Status::InLiveRoom:
            return QXmppPresence::InLiveRoom;
        }
    }
    return static_cast<QXmppPresence::AvailableStatusType>(d->status.type());
}

/// Sets the availability status type, for instance busy or away.

void QXmppPresence::setAvailableStatusType(AvailableStatusType type)
{
    d->status.setType(static_cast<QXmppPresence::Status::Type>(type));
}

/// Returns the priority level of the resource.

int QXmppPresence::priority() const
{
    return d->status.priority();
}

/// Sets the \a priority level of the resource.

void QXmppPresence::setPriority(int priority)
{
    d->status.setPriority(priority);
}

/// Returns the status text, a textual description of the user's status.

QString QXmppPresence::statusText() const
{
    return d->status.statusText();
}

/// Sets the status text, a textual description of the user's status.
///
/// \param statusText The status text, for example "Gone fishing".

void QXmppPresence::setStatusText(const QString& statusText)
{
    d->status.setStatusText(statusText);
}

bool QXmppPresence::isMobile() const
{
    return d->status.isMobile();
}

void QXmppPresence::setIsMobile(bool isMobile)
{
    d->status.setIsMobile(isMobile);
}

const QDateTime &QXmppPresence::stamp() const
{
    return d->status.stamp();
}

void QXmppPresence::setStamp(const QDateTime& stamp)
{
    d->status.setStamp(stamp);
}

const QString &QXmppPresence::onPhoneWith() const
{
    return d->status.onPhoneWith();
}

void QXmppPresence::setOnPhoneWith(const QString& onPhoneWith)
{
    d->status.setOnPhoneWith(onPhoneWith);
}

/// Returns the presence type.
///
/// You can use this method to determine the action which needs to be
/// taken in response to receiving the presence. For instance, if the type is
/// QXmppPresence::Available or QXmppPresence::Unavailable, you could update
/// the icon representing a contact's availability.

QXmppPresence::Type QXmppPresence::type() const
{
    return d->type;
}

/// Sets the presence type.
///
/// \param type

void QXmppPresence::setType(QXmppPresence::Type type)
{
    d->type = type;
}

/// \cond
void QXmppPresence::parse(const QDomElement &element)
{
    QXmppStanza::parse(element);

    const QString type = element.attribute("type");
    for (int i = Error; i <= Probe; i++) {
        if (type == presence_types[i]) {
            d->type = static_cast<Type>(i);
            break;
        }
    }
    d->status.parse(element);

    QXmppElementList extensions;
    QDomElement xElement = element.firstChildElement();
    d->vCardUpdateType = VCardUpdateNone;
    while(!xElement.isNull())
    {
        if (xElement.tagName() == "info")
        {
        }
        else if (xElement.tagName() == "show")
        {
        }
        else if (xElement.tagName() == "status")
        {
        }
        else if (xElement.tagName() == "error")
        {
        }
        // XEP-0045: Multi-User Chat
        else if(xElement.namespaceURI() == ns_muc) {
            d->mucSupported = true;
            d->mucPassword = xElement.firstChildElement("password").text();
        }
        else if(xElement.namespaceURI() == ns_muc_user)
        {
            QDomElement itemElement = xElement.firstChildElement("item");
            d->mucItem.parse(itemElement);
            QDomElement statusElement = xElement.firstChildElement("status");
            d->mucStatusCodes.clear();
            while (!statusElement.isNull()) {
                d->mucStatusCodes << statusElement.attribute("code").toInt();
                statusElement = statusElement.nextSiblingElement("status");
            }
        }
        // XEP-0153: vCard-Based Avatars
        else if(xElement.namespaceURI() == ns_vcard_update)
        {
            QDomElement photoElement = xElement.firstChildElement("photo");
            if(!photoElement.isNull())
            {
                d->photoHash = QByteArray::fromHex(photoElement.text().toLatin1());
                if(d->photoHash.isEmpty())
                    d->vCardUpdateType = VCardUpdateNoPhoto;
                else
                    d->vCardUpdateType = VCardUpdateValidPhoto;
            }
            else
            {
                d->photoHash = QByteArray();
                d->vCardUpdateType = VCardUpdateNotReady;
            }
        }
        // XEP-0115: Entity Capabilities
        else if(xElement.tagName() == "c" && xElement.namespaceURI() == ns_capabilities)
        {
            d->capabilityNode = xElement.attribute("node");
            d->capabilityVer = QByteArray::fromBase64(xElement.attribute("ver").toLatin1());
            d->capabilityHash = xElement.attribute("hash");
            d->capabilityExt = xElement.attribute("ext").split(" ", QString::SkipEmptyParts);
        }
        else if (xElement.tagName() == "addresses")
        {
        }
        else if (xElement.tagName() == "priority")
        {
        }
        else
        {
            // other extensions
            extensions << QXmppElement(xElement);
        }
        xElement = xElement.nextSiblingElement();
    }
    setExtensions(extensions);
}

void QXmppPresence::toXml(QXmlStreamWriter *xmlWriter) const
{
    xmlWriter->writeStartElement("presence");
    helperToXmlAddAttribute(xmlWriter,"xml:lang", lang());
    helperToXmlAddAttribute(xmlWriter,"id", id());
    helperToXmlAddAttribute(xmlWriter,"to", to());
    helperToXmlAddAttribute(xmlWriter,"from", from());
    helperToXmlAddAttribute(xmlWriter,"type", presence_types[d->type]);
    d->status.toXml(xmlWriter);

    error().toXml(xmlWriter);

    // XEP-0045: Multi-User Chat
    if(d->mucSupported) {
        xmlWriter->writeStartElement("x");
        xmlWriter->writeAttribute("xmlns", ns_muc);
        if (!d->mucPassword.isEmpty())
            xmlWriter->writeTextElement("password", d->mucPassword);
        xmlWriter->writeEndElement();
    }

    if(!d->mucItem.isNull() || !d->mucStatusCodes.isEmpty())
    {
        xmlWriter->writeStartElement("x");
        xmlWriter->writeAttribute("xmlns", ns_muc_user);
        if (!d->mucItem.isNull())
            d->mucItem.toXml(xmlWriter);
        foreach (int code, d->mucStatusCodes) {
            xmlWriter->writeStartElement("status");
            xmlWriter->writeAttribute("code", QString::number(code));
            xmlWriter->writeEndElement();
        }
        xmlWriter->writeEndElement();
    }

    // XEP-0153: vCard-Based Avatars
    if(d->vCardUpdateType != VCardUpdateNone)
    {
        xmlWriter->writeStartElement("x");
        xmlWriter->writeAttribute("xmlns", ns_vcard_update);
        switch(d->vCardUpdateType)
        {
        case VCardUpdateNoPhoto:
            helperToXmlAddTextElement(xmlWriter, "photo", "");
            break;
        case VCardUpdateValidPhoto:
            helperToXmlAddTextElement(xmlWriter, "photo", d->photoHash.toHex());
            break;
        case VCardUpdateNotReady:
            break;
        default:
            break;
        }
        xmlWriter->writeEndElement();
    }

    if(!d->capabilityNode.isEmpty() && !d->capabilityVer.isEmpty()
        && !d->capabilityHash.isEmpty())
    {
        xmlWriter->writeStartElement("c");
        xmlWriter->writeAttribute("xmlns", ns_capabilities);
        helperToXmlAddAttribute(xmlWriter, "hash", d->capabilityHash);
        helperToXmlAddAttribute(xmlWriter, "node", d->capabilityNode);
        helperToXmlAddAttribute(xmlWriter, "ver", d->capabilityVer.toBase64());
        xmlWriter->writeEndElement();
    }

    // other extensions
    QXmppStanza::extensionsToXml(xmlWriter);

    xmlWriter->writeEndElement();
}
/// \endcond

/// Returns the photo-hash of the VCardUpdate.
///
/// \return QByteArray

QByteArray QXmppPresence::photoHash() const
{
    return d->photoHash;
}

/// Sets the photo-hash of the VCardUpdate.
///
/// \param photoHash as QByteArray

void QXmppPresence::setPhotoHash(const QByteArray& photoHash)
{
    d->photoHash = photoHash;
}

/// Returns the type of VCardUpdate
///
/// \return VCardUpdateType

QXmppPresence::VCardUpdateType QXmppPresence::vCardUpdateType() const
{
    return d->vCardUpdateType;
}

/// Sets the type of VCardUpdate
///
/// \param type VCardUpdateType

void QXmppPresence::setVCardUpdateType(VCardUpdateType type)
{
    d->vCardUpdateType = type;
}

/// XEP-0115: Entity Capabilities
QString QXmppPresence::capabilityHash() const
{
    return d->capabilityHash;
}

/// XEP-0115: Entity Capabilities
void QXmppPresence::setCapabilityHash(const QString& hash)
{
    d->capabilityHash = hash;
}

/// XEP-0115: Entity Capabilities
QString QXmppPresence::capabilityNode() const
{
    return d->capabilityNode;
}

/// XEP-0115: Entity Capabilities
void QXmppPresence::setCapabilityNode(const QString& node)
{
    d->capabilityNode = node;
}

/// XEP-0115: Entity Capabilities
QByteArray QXmppPresence::capabilityVer() const
{
    return d->capabilityVer;
}

/// XEP-0115: Entity Capabilities
void QXmppPresence::setCapabilityVer(const QByteArray& ver)
{
    d->capabilityVer = ver;
}

/// Legacy XEP-0115: Entity Capabilities
QStringList QXmppPresence::capabilityExt() const
{
    return d->capabilityExt;
}

/// Returns the MUC item.

QXmppMucItem QXmppPresence::mucItem() const
{
    return d->mucItem;
}

/// Sets the MUC item.
///
/// \param item

void QXmppPresence::setMucItem(const QXmppMucItem &item)
{
    d->mucItem = item;
}

/// Returns the password used to join a MUC room.

QString QXmppPresence::mucPassword() const
{
    return d->mucPassword;
}

/// Sets the password used to join a MUC room.

void QXmppPresence::setMucPassword(const QString &password)
{
    d->mucPassword = password;
}

/// Returns the MUC status codes.

QList<int> QXmppPresence::mucStatusCodes() const
{
    return d->mucStatusCodes;
}

/// Sets the MUC status codes.
///
/// \param codes

void QXmppPresence::setMucStatusCodes(const QList<int> &codes)
{
    d->mucStatusCodes = codes;
}

/// Returns true if the sender has indicated MUC support.

bool QXmppPresence::isMucSupported() const
{
    return d->mucSupported;
}

/// Sets whether MUC is \a supported.

void QXmppPresence::setMucSupported(bool supported)
{
    d->mucSupported = supported;
}

/// \cond
const QXmppPresence::Status& QXmppPresence::status() const
{
    return d->status;
}

QXmppPresence::Status& QXmppPresence::status()
{
    return d->status;
}

void QXmppPresence::setStatus(const QXmppPresence::Status& status)
{
    d->status = status;
}

QXmppPresence::Status::Status()
    : m_type(QXmppPresence::Status::Online)
    , m_info(QXmppPresence::Status::NoInfo)
    , m_statusText()
    , m_priority(0)
    , m_isMobile(false)
    , m_stamp()
    , m_onPhoneWith()
{
}

QXmppPresence::Status::Type QXmppPresence::Status::type() const
{
    return m_type;
}

void QXmppPresence::Status::setType(QXmppPresence::Status::Type type)
{
    m_type = type;
}

QXmppPresence::Status::Info QXmppPresence::Status::info() const
{
    return m_info;
}

void QXmppPresence::Status::setInfo(QXmppPresence::Status::Info info)
{
    m_info = info;
}

QString QXmppPresence::Status::statusText() const
{
    return m_statusText;
}

void QXmppPresence::Status::setStatusText(const QString& str)
{
    m_statusText = str;
}

bool QXmppPresence::Status::isMobile() const
{
    return m_isMobile;
}

void QXmppPresence::Status::setIsMobile(bool isMobile)
{
    m_isMobile = isMobile;
}

int QXmppPresence::Status::priority() const
{
    return m_priority;
}

void QXmppPresence::Status::setPriority(int priority)
{
    m_priority = priority;
}

const QDateTime &QXmppPresence::Status::stamp() const
{
    return m_stamp;
}

void QXmppPresence::Status::setStamp(const QDateTime & stamp)
{
    m_stamp = stamp;
}

const QString &QXmppPresence::Status::onPhoneWith() const
{
    return m_onPhoneWith;
}

void QXmppPresence::Status::setOnPhoneWith(const QString& onPhoneWith)
{
    m_onPhoneWith = onPhoneWith;
}

void QXmppPresence::Status::parse(const QDomElement &element)
{
    m_type = QXmppPresence::Status::Online;
    const QString show = element.firstChildElement("show").text();
    if (!show.isEmpty()) {
        for (int i = Online; i <= Invisible; i++) {
            if (show == presence_shows[i]) {
                m_type = static_cast<Type>(i);
                break;
            }
        }
    }

    m_info = QXmppPresence::Status::NoInfo;
    const QDomElement xElement = element.firstChildElement("info");
    if (xElement.namespaceURI() == ns_unison) {
        const QString info = xElement.text();
        if (!info.isEmpty()) {
            for (int i = Autoaway; i <= InLiveRoom; i++) {
                if (info == presence_info[i]) {
                    m_info = static_cast<Info>(i);
                    break;
                }
            }
        }
        m_isMobile = xElement.attribute("is_mobile") == "true" ? 1 : 0;
        m_onPhoneWith = xElement.attribute("to");
    }

    QDomElement statusText = element.firstChildElement("status");
    if (!statusText.isNull())
        m_statusText = statusText.text();

    m_priority = element.firstChildElement("priority").text().toInt();

    // XEP-0203: Delayed Delivery
    m_stamp = QDateTime();
    QDomElement stamp = element.firstChildElement("delay");
    if (!stamp.isNull() && stamp.namespaceURI() == ns_delayed_delivery)
        m_stamp = QXmppUtils::datetimeFromString(stamp.attribute("stamp"));
}

void QXmppPresence::Status::toXml(QXmlStreamWriter *xmlWriter) const
{
    const QString show = presence_shows[m_type];
    if (!show.isEmpty())
        helperToXmlAddTextElement(xmlWriter, "show", show);

    const QString info = presence_info[m_info];
    if (!info.isEmpty() || m_isMobile) {
        xmlWriter->writeStartElement(ns_unison, "info");
        if (m_isMobile)
            xmlWriter->writeAttribute("is_mobile", "true");
        if (!m_onPhoneWith.isEmpty())
            xmlWriter->writeAttribute("to", m_onPhoneWith);
        xmlWriter->writeEmptyElement(info);
        xmlWriter->writeEndElement();
    }

    if (!m_statusText.isNull())
        helperToXmlAddTextElement(xmlWriter, "status", m_statusText);

    if (m_priority != 0)
        helperToXmlAddTextElement(xmlWriter, "priority", QString::number(m_priority));

    if (m_stamp.isValid()) {
        xmlWriter->writeStartElement(ns_delayed_delivery, "info");
        xmlWriter->writeAttribute("stamp", m_stamp.toString(Qt::ISODate));
        xmlWriter->writeEndElement();
    }
}
/// \endcond
