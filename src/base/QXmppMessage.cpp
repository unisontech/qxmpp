/*
 * Copyright (C) 2008-2012 The QXmpp developers
 *
 * Authors:
 *  Manjeet Dahiya
 *  Jeremy Lainé
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

#include <QDomElement>
#include <QTextStream>
#include <QXmlStreamWriter>
#include <QStringList>

#include "QXmppConstants.h"
#include "QXmppMessage.h"
#include "QXmppUtils.h"

// XEP-0079: Advanced Message Processing
const char* ns_amp = "http://jabber.org/protocol/amp";
// Unison Extension: Attachments
const char* ns_attachments = "jabber:info:unison";

static const char* actions_types[] = {
    "alert",
    "drop",
    "error",
    "notify"
};

static const char* conditions_types[] = {
    "deliver",
    "expire_at",
    "match_resource"
};

static const char* values_types[] = {
    "direct",
    "forward",
    "gateway",
    "none",
    "stored",
    "any",
    "exact",
    "other"
};

class QXmppAmpRulePrivate : public QSharedData
{
public:
    QXmppMessage::QXmppAmp::QXmppAmpRule::Actions action;
    QXmppMessage::QXmppAmp::QXmppAmpRule::Conditions condition;
    QXmppMessage::QXmppAmp::QXmppAmpRule::Values value;
    QDateTime expire;
};

QXmppMessage::QXmppAmp::QXmppAmpRule::QXmppAmpRule(const QXmppMessage::QXmppAmp::QXmppAmpRule &other)
    : d(other.d)
{
}

QXmppMessage::QXmppAmp::QXmppAmpRule::QXmppAmpRule(
    QXmppMessage::QXmppAmp::QXmppAmpRule::Actions action,
    QXmppMessage::QXmppAmp::QXmppAmpRule::Conditions condition,
    QXmppMessage::QXmppAmp::QXmppAmpRule::Values value)
    : d(new QXmppAmpRulePrivate)
{
    d->action = action;
    d->condition = condition;
    d->value = value;
}

QXmppMessage::QXmppAmp::QXmppAmpRule::QXmppAmpRule(
    QXmppMessage::QXmppAmp::QXmppAmpRule::Actions action,
    QXmppMessage::QXmppAmp::QXmppAmpRule::Conditions condition,
    const QDateTime &value)
    : d(new QXmppAmpRulePrivate)
{
    d->action = action;
    d->condition = condition;
    d->expire = value;
}

QXmppMessage::QXmppAmp::QXmppAmpRule::~QXmppAmpRule()
{
}

QXmppMessage::QXmppAmp::QXmppAmpRule& QXmppMessage::QXmppAmp::QXmppAmpRule::operator=(const QXmppMessage::QXmppAmp::QXmppAmpRule& other)
{
    d = other.d;
    return *this;
}

QXmppMessage::QXmppAmp::QXmppAmpRule::Actions QXmppMessage::QXmppAmp::QXmppAmpRule::action() const
{
    return d->action;
}

QXmppMessage::QXmppAmp::QXmppAmpRule::Conditions QXmppMessage::QXmppAmp::QXmppAmpRule::condition() const
{
    return d->condition;
}

QXmppMessage::QXmppAmp::QXmppAmpRule::Values QXmppMessage::QXmppAmp::QXmppAmpRule::value() const
{
    return d->value;
}

QDateTime QXmppMessage::QXmppAmp::QXmppAmpRule::expireAt() const
{
    return d->expire;
}

class QXmppAmpPrivate : public QSharedData
{
public:
    QList <QXmppMessage::QXmppAmp::QXmppAmpRule> rules;
    QXmppMessage::QXmppAmp::QXmppAmpRule::Actions status;
    bool isStatus;
    QString from;
    QString to;
    bool perHop;
};

QXmppMessage::QXmppAmp::QXmppAmp()
    : d(new QXmppAmpPrivate)
{
    d->isStatus = false;
    d->perHop = false;
}

QXmppMessage::QXmppAmp::QXmppAmp(const QList<QXmppMessage::QXmppAmp::QXmppAmpRule> &rules)
    : d(new QXmppAmpPrivate)
{
    setRules(rules);
    d->isStatus = false;
    d->perHop = false;
}

QXmppMessage::QXmppAmp::QXmppAmp(const QXmppMessage::QXmppAmp &other)
    : d(other.d)
{
}

QXmppMessage::QXmppAmp::~QXmppAmp()
{
}

QXmppMessage::QXmppAmp &QXmppMessage::QXmppAmp::operator=(const QXmppMessage::QXmppAmp &other)
{
    d = other.d;
    return *this;
}

void QXmppMessage::QXmppAmp::setRules(const QList<QXmppMessage::QXmppAmp::QXmppAmpRule> &rules)
{
    d->rules = rules;
}

QList<QXmppMessage::QXmppAmp::QXmppAmpRule> QXmppMessage::QXmppAmp::rules() const
{
    return d->rules;
}

bool QXmppMessage::QXmppAmp::isStatus() const
{
    return d->isStatus;
}

void QXmppMessage::QXmppAmp::setStatus(QXmppMessage::QXmppAmp::QXmppAmpRule::Actions status)
{
    d->status = status;
    d->isStatus = true;
}

QXmppMessage::QXmppAmp::QXmppAmpRule::Actions QXmppMessage::QXmppAmp::status() const
{
    return d->status;
}

void QXmppMessage::QXmppAmp::setTo(const QString &to)
{
    d->to = to;
}

QString QXmppMessage::QXmppAmp::to() const
{
    return d->to;
}

void QXmppMessage::QXmppAmp::setFrom(const QString &from)
{
    d->from = from;
}

QString QXmppMessage::QXmppAmp::from() const
{
    return d->from;
}

void QXmppMessage::QXmppAmp::setPerHop(bool perHop)
{
    d->perHop = perHop;
}

bool QXmppMessage::QXmppAmp::perHop() const
{
    return d->perHop;
}

static const char* chat_states[] = {
    "",
    "active",
    "inactive",
    "gone",
    "composing",
    "paused",
};

static const char* message_types[] = {
    "error",
    "normal",
    "chat",
    "groupchat",
    "headline"
};

static const char *ns_xhtml = "http://www.w3.org/1999/xhtml";

enum StampType
{
    LegacyDelayedDelivery,  // XEP-0091: Legacy Delayed Delivery
    DelayedDelivery,        // XEP-0203: Delayed Delivery
};

class QXmppMessagePrivate : public QSharedData
{
public:
    QXmppMessage::Type type;
    QDateTime stamp;
    StampType stampType;
    QXmppMessage::State state;

    bool attentionRequested;
    QString body;
    QString subject;
    QString thread;

    // XEP-0071: XHTML-IM
    QString xhtml;

    // XEP-0079: Advanced Message Processing
    bool isAmp;
    QXmppMessage::QXmppAmp amp;

    // Unison Extension: Chat History
    QString chatHistoryId;

    // Request message receipt as per XEP-0184.
    QString receiptId;
    bool receiptRequested;
    bool receiptReceived;
    // Unison Extension: custom receipt read
    bool receiptRead;

    // XEP-0249: Direct MUC Invitations
    QString mucInvitationJid;
    QString mucInvitationPassword;
    QString mucInvitationReason;

    // Unison Extension: Attachments
    QString attachment;
    QStringList attachments;
};

/// Constructs a QXmppMessage.
///
/// \param from
/// \param to
/// \param body
/// \param thread

QXmppMessage::QXmppMessage(const QString& from, const QString& to, const
                         QString& body, const QString& thread)
    : QXmppStanza(from, to)
    , QXmppLoggable()
    , d(new QXmppMessagePrivate)
{
    d->type = Chat;
    d->stampType = DelayedDelivery;
    d->state = None;
    d->attentionRequested = false;
    d->body = body;
    d->thread = thread;
    d->receiptRequested = false;
    d->isAmp = false;
    d->chatHistoryId = "";
    d->receiptReceived = false;
    d->receiptRead = false;
}

/// Constructs a copy of \a other.

QXmppMessage::QXmppMessage(const QXmppMessage &other)
    : QXmppStanza(other)
    , QXmppLoggable()
    , d(other.d)
{
}

QXmppMessage::~QXmppMessage()
{

}

/// Assigns \a other to this message.

QXmppMessage& QXmppMessage::operator=(const QXmppMessage &other)
{
    QXmppStanza::operator=(other);
    d = other.d;
    return *this;
}

/// Returns the message's body.
///

QString QXmppMessage::body() const
{
    return d->body;
}

/// Sets the message's body.
///
/// \param body

void QXmppMessage::setBody(const QString& body)
{
    d->body = body;
}

/// Returns true if the user's attention is requested, as defined
/// by XEP-0224: Attention.

bool QXmppMessage::isAttentionRequested() const
{
    return d->attentionRequested;
}

/// Sets whether the user's attention is requested, as defined
/// by XEP-0224: Attention.
///
/// \a param requested

void QXmppMessage::setAttentionRequested(bool requested)
{
    d->attentionRequested = requested;
}

/// Returns true if a delivery receipt is requested, as defined
/// by XEP-0184: Message Delivery Receipts.

bool QXmppMessage::isReceiptRequested() const
{
    return d->receiptRequested;
}

/// Sets whether a delivery receipt is requested, as defined
/// by XEP-0184: Message Delivery Receipts.
///
/// \a param requested

void QXmppMessage::setReceiptRequested(bool requested)
{
    d->receiptRequested = requested;
    if (requested && id().isEmpty())
        generateAndSetNextId();
}

bool QXmppMessage::isReceiptReceived() const
{
    return d->receiptReceived;
}

void QXmppMessage::setReceiptReceived(bool received)
{
    d->receiptReceived = received;
}

bool QXmppMessage::isReceiptRead() const
{
    return d->receiptRead;
}

void QXmppMessage::setReceiptRead(bool read)
{
    d->receiptRead = read;
}

/// If this message is a delivery receipt, returns the ID of the
/// original message.

QString QXmppMessage::receiptId() const
{
    return d->receiptId;
}

/// Make this message a delivery receipt for the message with
/// the given \a id.

void QXmppMessage::setReceiptId(const QString &id)
{
    d->receiptId = id;
}

/// Returns the JID for a multi-user chat direct invitation as defined
/// by XEP-0249: Direct MUC Invitations.

QString QXmppMessage::mucInvitationJid() const
{
    return d->mucInvitationJid;
}

/// Sets the JID for a multi-user chat direct invitation as defined
/// by XEP-0249: Direct MUC Invitations.

void QXmppMessage::setMucInvitationJid(const QString &jid)
{
    d->mucInvitationJid = jid;
}

/// Returns the password for a multi-user chat direct invitation as defined
/// by XEP-0249: Direct MUC Invitations.

QString QXmppMessage::mucInvitationPassword() const
{
    return d->mucInvitationPassword;
}

/// Sets the \a password for a multi-user chat direct invitation as defined
/// by XEP-0249: Direct MUC Invitations.

void QXmppMessage::setMucInvitationPassword(const QString &password)
{
    d->mucInvitationPassword = password;
}

/// Returns the reason for a multi-user chat direct invitation as defined
/// by XEP-0249: Direct MUC Invitations.

QString QXmppMessage::mucInvitationReason() const
{
    return d->mucInvitationReason;
}

/// Sets the \a reason for a multi-user chat direct invitation as defined
/// by XEP-0249: Direct MUC Invitations.

void QXmppMessage::setMucInvitationReason(const QString &reason)
{
    d->mucInvitationReason = reason;
}

/// Returns the message's type.
///

QXmppMessage::Type QXmppMessage::type() const
{
    return d->type;
}

/// Sets the message's type.
///
/// \param type

void QXmppMessage::setType(QXmppMessage::Type type)
{
    d->type = type;
}

/// Returns the message's timestamp (if any).

QDateTime QXmppMessage::stamp() const
{
    return d->stamp;
}

/// Sets the message's timestamp.
///
/// \param stamp

void QXmppMessage::setStamp(const QDateTime &stamp)
{
    d->stamp = stamp;
}

/// Returns the message's chat state.
///

QXmppMessage::State QXmppMessage::state() const
{
    return d->state;
}

/// Sets the message's chat state.
///
/// \param state

void QXmppMessage::setState(QXmppMessage::State state)
{
    d->state = state;
}

/// Returns the message's subject.
///

QString QXmppMessage::subject() const
{
    return d->subject;
}

/// Sets the message's subject.
///
/// \param subject

void QXmppMessage::setSubject(const QString& subject)
{
    d->subject = subject;
}

/// Returns the message's thread.

QString QXmppMessage::thread() const
{
    return d->thread;
}

/// Sets the message's thread.
///
/// \param thread

void QXmppMessage::setThread(const QString& thread)
{
    d->thread = thread;
}

/// Returns the message's XHTML body as defined by
/// XEP-0071: XHTML-IM.

QString QXmppMessage::xhtml() const
{
    return d->xhtml;
}

/// Sets the message's XHTML body as defined by
/// XEP-0071: XHTML-IM.

void QXmppMessage::setXhtml(const QString &xhtml)
{
    d->xhtml = xhtml;
}

bool QXmppMessage::isAmp() const
{
    return d->isAmp;
}

QXmppMessage::QXmppAmp QXmppMessage::amp() const
{
    return d->amp;
}

void QXmppMessage::setAmp(const QXmppAmp &amp)
{
    d->amp = amp;
    d->isAmp = true;
}

QString QXmppMessage::chatHistoryId() const
{
    return d->chatHistoryId;
}

void QXmppMessage::setChatHistoryId(const QString& id)
{
    d->chatHistoryId = id;
}

QString QXmppMessage::attachment() const
{
    return d->attachment;
}

void QXmppMessage::setAttachment(const QString &value)
{
    d->attachment = value;
}

QStringList QXmppMessage::attachments() const
{
    return d->attachments;
}

void QXmppMessage::setAttachments(const QStringList &value)
{
    d->attachments = value;
}

/// \cond
void QXmppMessage::parse(const QDomElement &element)
{
    QXmppStanza::parse(element);

    const QString type = element.attribute("type");
    d->type = Normal;
    for (int i = Error; i <= Headline; i++) {
        if (type == message_types[i]) {
            d->type = static_cast<Type>(i);
            break;
        }
    }

    d->body = element.firstChildElement("body").text();
    d->subject = element.firstChildElement("subject").text();
    d->thread = element.firstChildElement("thread").text();

    // chat states
    for (int i = Active; i <= Paused; i++)
    {
        QDomElement stateElement = element.firstChildElement(chat_states[i]);
        if (!stateElement.isNull() &&
            stateElement.namespaceURI() == ns_chat_states)
        {
            d->state = static_cast<QXmppMessage::State>(i);
            break;
        }
    }

    // XEP-0071: XHTML-IM
    QDomElement htmlElement = element.firstChildElement("html");
    if (!htmlElement.isNull() && htmlElement.namespaceURI() == ns_xhtml_im) {
        QDomElement bodyElement = htmlElement.firstChildElement("body");
        if (!bodyElement.isNull() && bodyElement.namespaceURI() == ns_xhtml) {
            QTextStream stream(&d->xhtml, QIODevice::WriteOnly);
            bodyElement.save(stream, 0);

            d->xhtml = d->xhtml.mid(d->xhtml.indexOf('>') + 1);
            d->xhtml.replace(" xmlns=\"http://www.w3.org/1999/xhtml\"", "");
            d->xhtml.replace("</body>", "");
            d->xhtml = d->xhtml.trimmed();
        }
    }

    // XEP-0079: Advanced Message Processing
    QDomElement ampElement = element.firstChildElement("amp");
    if (!ampElement.isNull() && ampElement.namespaceURI() == ns_amp) {
        QList<QXmppMessage::QXmppAmp::QXmppAmpRule> rules;
        for (QDomElement ruleElement = ampElement.firstChildElement("rule");
            !ruleElement.isNull();
            ruleElement = ruleElement.nextSiblingElement("rule"))
        {

            QString attributeAction = ruleElement.attribute("action");
            if (attributeAction.isEmpty()) {
                warning("QXmppMessage : element 'rule' missing required attribute 'action'");
                continue;
            }
            QString attributeCondition = ruleElement.attribute("condition");
            if (attributeCondition.isEmpty()) {
                warning("QXmppMessage : element 'rule' missing required attribute 'condition'");
                continue;
            }
            QString attributeValue = ruleElement.attribute("value");
            if (attributeValue.isEmpty()) {
                warning("QXmppMessage : element 'rule' missing required attribute 'value'");
                continue;
            }

            QXmppMessage::QXmppAmp::QXmppAmpRule::Actions action;
            if (attributeAction == actions_types[QXmppMessage::QXmppAmp::QXmppAmpRule::alert]) {
                action = QXmppMessage::QXmppAmp::QXmppAmpRule::alert;
            } else if (attributeAction == actions_types[QXmppMessage::QXmppAmp::QXmppAmpRule::drop]) {
                action = QXmppMessage::QXmppAmp::QXmppAmpRule::drop;
            } else if (attributeAction == actions_types[QXmppMessage::QXmppAmp::QXmppAmpRule::error]) {
                action = QXmppMessage::QXmppAmp::QXmppAmpRule::error;
            } else if (attributeAction == actions_types[QXmppMessage::QXmppAmp::QXmppAmpRule::notify]) {
                action = QXmppMessage::QXmppAmp::QXmppAmpRule::notify;
            } else {
                warning("QXmppMessage : element 'rule' invalid combination");
                continue;
            }

            QXmppMessage::QXmppAmp::QXmppAmpRule::Conditions condition;
            if (attributeCondition == conditions_types[QXmppMessage::QXmppAmp::QXmppAmpRule::deliver]) {
                condition = QXmppMessage::QXmppAmp::QXmppAmpRule::deliver;
                QXmppMessage::QXmppAmp::QXmppAmpRule::Values value;
                if (attributeValue == values_types[QXmppMessage::QXmppAmp::QXmppAmpRule::direct]) {
                    value = QXmppMessage::QXmppAmp::QXmppAmpRule::direct;
                } else if (attributeValue == values_types[QXmppMessage::QXmppAmp::QXmppAmpRule::forward]) {
                    value = QXmppMessage::QXmppAmp::QXmppAmpRule::forward;
                } else if (attributeValue == values_types[QXmppMessage::QXmppAmp::QXmppAmpRule::gateway]) {
                    value = QXmppMessage::QXmppAmp::QXmppAmpRule::gateway;
                } else if (attributeValue == values_types[QXmppMessage::QXmppAmp::QXmppAmpRule::none]) {
                    value = QXmppMessage::QXmppAmp::QXmppAmpRule::none;
                } else if (attributeValue == values_types[QXmppMessage::QXmppAmp::QXmppAmpRule::stored]) {
                    value = QXmppMessage::QXmppAmp::QXmppAmpRule::stored;
                } else {
                    warning("QXmppMessage : element 'rule' invalid combination");
                    continue;
                }
                rules.append(QXmppMessage::QXmppAmp::QXmppAmpRule(action, condition, value));
            } else if (attributeCondition == conditions_types[QXmppMessage::QXmppAmp::QXmppAmpRule::expire_at]) {
                condition = QXmppMessage::QXmppAmp::QXmppAmpRule::expire_at;
                QDateTime value = QDateTime::fromString(attributeValue, Qt::ISODate);
                if (!value.isValid()) {
                    warning("QXmppMessage : element 'rule' invalid attribute 'value', MUST be a DateTime");
                    continue;
                }
                rules.append(QXmppMessage::QXmppAmp::QXmppAmpRule(action, condition, value));
            } else if (attributeCondition == conditions_types[QXmppMessage::QXmppAmp::QXmppAmpRule::match_resource]) {
                condition = QXmppMessage::QXmppAmp::QXmppAmpRule::match_resource;
                QXmppMessage::QXmppAmp::QXmppAmpRule::Values value;
                if (attributeValue == values_types[QXmppMessage::QXmppAmp::QXmppAmpRule::any]) {
                    value = QXmppMessage::QXmppAmp::QXmppAmpRule::any;
                } else if (attributeValue == values_types[QXmppMessage::QXmppAmp::QXmppAmpRule::exact]) {
                    value = QXmppMessage::QXmppAmp::QXmppAmpRule::exact;
                } else if (attributeValue == values_types[QXmppMessage::QXmppAmp::QXmppAmpRule::other]) {
                    value = QXmppMessage::QXmppAmp::QXmppAmpRule::other;
                } else {
                    warning("QXmppMessage : element 'rule' invalid combination");
                    continue;
                }
                rules.append(QXmppMessage::QXmppAmp::QXmppAmpRule(action, condition, value));
            } else {
                warning("QXmppMessage : element 'rule' invalid attribute 'condition'");
                continue;
            }
        }
        if (rules.count()) {
            d->isAmp = true;
            d->amp.setRules(rules);
            QString attributeAction = ampElement.attribute("status");
            if (!attributeAction.isEmpty()) {
                if (attributeAction == actions_types[QXmppMessage::QXmppAmp::QXmppAmpRule::alert]) {
                    d->amp.setStatus(QXmppMessage::QXmppAmp::QXmppAmpRule::alert);
                } else if (attributeAction == actions_types[QXmppMessage::QXmppAmp::QXmppAmpRule::drop]) {
                    d->amp.setStatus(QXmppMessage::QXmppAmp::QXmppAmpRule::drop);
                } else if (attributeAction == actions_types[QXmppMessage::QXmppAmp::QXmppAmpRule::error]) {
                    d->amp.setStatus(QXmppMessage::QXmppAmp::QXmppAmpRule::error);
                } else if (attributeAction == actions_types[QXmppMessage::QXmppAmp::QXmppAmpRule::notify]) {
                    d->amp.setStatus(QXmppMessage::QXmppAmp::QXmppAmpRule::notify);
                }
            }
            d->amp.setTo(ampElement.attribute("to"));
            d->amp.setFrom(ampElement.attribute("from"));
            d->amp.setPerHop(ampElement.attribute("per-hop").isEmpty() ? false : true);
        }
    }

    // Unison Extension: Chat History
    d->chatHistoryId = element.attribute("chat_history_id");

    // XEP-0184: Message Delivery Receipts
    QDomElement receivedElement = element.firstChildElement("received");
    if (!receivedElement.isNull() && receivedElement.namespaceURI() == ns_message_receipts) {
        d->receiptId = receivedElement.attribute("id");
        d->receiptReceived = true;
    }

    // Unison Extension: custom receipt read
    QDomElement receiptReadElement = element.firstChildElement("read");
    if (!receiptReadElement.isNull() && receiptReadElement.namespaceURI() == ns_message_receipts) {
        d->receiptId = receiptReadElement.attribute("id");
        d->receiptRead = true;
    }

    if (receivedElement.isNull() && receiptReadElement.isNull()) {
        // compatibility with old-style XEP
        d->receiptId = QString();
    } else {
        if (d->receiptId.isEmpty())
            d->receiptId = id();
    }

    d->receiptRequested = element.firstChildElement("request").namespaceURI() == ns_message_receipts;

    // XEP-0203: Delayed Delivery
    QDomElement delayElement = element.firstChildElement("delay");
    if (!delayElement.isNull() && delayElement.namespaceURI() == ns_delayed_delivery)
    {
        const QString str = delayElement.attribute("stamp");
        d->stamp = QXmppUtils::datetimeFromString(str);
        d->stampType = DelayedDelivery;
    }

    // XEP-0224: Attention
    d->attentionRequested = element.firstChildElement("attention").namespaceURI() == ns_attention;

    // Unison Extension: Attachments
    d->attachment = element.firstChildElement("attachment").text();
    QDomElement attachmentsElement = element.firstChildElement("attachments");
    if (!attachmentsElement.isNull() && attachmentsElement.namespaceURI() == ns_attachments)
    {
        QDomElement attachmentElement = attachmentsElement.firstChildElement("attachment");
        while (!attachmentElement.isNull())
        {
            QString id = attachmentElement.attribute("id");
            if (!id.isEmpty())
                d->attachments.append(id);

            attachmentElement = attachmentElement.nextSiblingElement("attachment");
        }
    }

    QXmppElementList extensions;
    QDomElement xElement = element.firstChildElement("x");
    while (!xElement.isNull())
    {
        if (xElement.namespaceURI() == ns_legacy_delayed_delivery)
        {
            // XEP-0091: Legacy Delayed Delivery
            const QString str = xElement.attribute("stamp");
            d->stamp = QDateTime::fromString(str, "yyyyMMddThh:mm:ss");
            d->stamp.setTimeSpec(Qt::UTC);
            d->stampType = LegacyDelayedDelivery;
        } else if (xElement.namespaceURI() == ns_conference) {
            // XEP-0249: Direct MUC Invitations
            d->mucInvitationJid = xElement.attribute("jid");
            d->mucInvitationPassword = xElement.attribute("password");
            d->mucInvitationReason = xElement.attribute("reason");
        } else {
            // other extensions
            extensions << QXmppElement(xElement);
        }
        xElement = xElement.nextSiblingElement("x");
    }
    setExtensions(extensions);
}

void QXmppMessage::toXml(QXmlStreamWriter *xmlWriter) const
{
    xmlWriter->writeStartElement("message");
    helperToXmlAddAttribute(xmlWriter, "xml:lang", lang());
    helperToXmlAddAttribute(xmlWriter, "id", id());
    if (!d->receiptId.isEmpty()) { // send this id only if this message is receipt
        // Unison Extension: custom id
        xmlWriter->writeAttribute("chat_history_id", d->chatHistoryId);
    }
    helperToXmlAddAttribute(xmlWriter, "to", to());
    helperToXmlAddAttribute(xmlWriter, "from", from());
    helperToXmlAddAttribute(xmlWriter, "type", message_types[d->type]);
    if (!d->subject.isEmpty())
        helperToXmlAddTextElement(xmlWriter, "subject", d->subject);
    if (!d->body.isEmpty())
        helperToXmlAddTextElement(xmlWriter, "body", d->body);
    if (!d->thread.isEmpty())
        helperToXmlAddTextElement(xmlWriter, "thread", d->thread);
    error().toXml(xmlWriter);

    // chat states
    if (d->state > None && d->state <= Paused)
    {
        xmlWriter->writeStartElement(chat_states[d->state]);
        xmlWriter->writeAttribute("xmlns", ns_chat_states);
        xmlWriter->writeEndElement();
    }

    // XEP-0071: XHTML-IM
    if (!d->xhtml.isEmpty()) {
        xmlWriter->writeStartElement("html");
        xmlWriter->writeAttribute("xmlns", ns_xhtml_im);
        xmlWriter->writeStartElement("body");
        xmlWriter->writeAttribute("xmlns", ns_xhtml);
        xmlWriter->writeCharacters("");
        xmlWriter->device()->write(d->xhtml.toUtf8());
        xmlWriter->writeEndElement();
        xmlWriter->writeEndElement();
    }

    // XEP-0079: Advanced Message Processing
    if (d->isAmp)
    {
        xmlWriter->writeStartElement("amp");
        xmlWriter->writeAttribute("xmlns", ns_amp);
        QString from = d->amp.from();
        if (!from.isEmpty())
        {
            helperToXmlAddAttribute(xmlWriter, "from", from);
        }
        QString to = d->amp.to();
        if (!to.isEmpty())
        {
            helperToXmlAddAttribute(xmlWriter, "to", to);
        }
        bool perHop = d->amp.perHop();
        if (perHop)
        {
            helperToXmlAddAttribute(xmlWriter, "per-hop", "true");
        }
        if (d->amp.isStatus())
        {
            helperToXmlAddAttribute(xmlWriter, "status", actions_types[d->amp.status()]);
        }
        Q_FOREACH(const QXmppAmp::QXmppAmpRule &rule, d->amp.rules())
        {
            xmlWriter->writeStartElement("rule");
            xmlWriter->writeAttribute("action", actions_types[rule.action()]);
            xmlWriter->writeAttribute("condition", conditions_types[rule.condition()]);
            if (rule.condition() == QXmppMessage::QXmppAmp::QXmppAmpRule::deliver
                || rule.condition() == QXmppMessage::QXmppAmp::QXmppAmpRule::match_resource)
            {
                xmlWriter->writeAttribute("value", values_types[rule.value()]);
            }
            else {
                xmlWriter->writeAttribute("value", rule.expireAt().toString(Qt::ISODate));
            }
            xmlWriter->writeEndElement();
        }
        xmlWriter->writeEndElement();
    }

    // time stamp
    if (d->stamp.isValid())
    {
        QDateTime utcStamp = d->stamp.toUTC();
        if (d->stampType == DelayedDelivery)
        {
            // XEP-0203: Delayed Delivery
            xmlWriter->writeStartElement("delay");
            xmlWriter->writeAttribute("xmlns", ns_delayed_delivery);
            helperToXmlAddAttribute(xmlWriter, "stamp", QXmppUtils::datetimeToString(utcStamp));
            xmlWriter->writeEndElement();
        } else {
            // XEP-0091: Legacy Delayed Delivery
            xmlWriter->writeStartElement("x");
            xmlWriter->writeAttribute("xmlns", ns_legacy_delayed_delivery);
            helperToXmlAddAttribute(xmlWriter, "stamp", utcStamp.toString("yyyyMMddThh:mm:ss"));
            xmlWriter->writeEndElement();
        }
    }

    // XEP-0184: Message Delivery Receipts
    if (!d->receiptId.isEmpty()) {
        // Unison Extension: custom receipt read
        if (d->receiptRead)
            xmlWriter->writeStartElement("read");
        else
            xmlWriter->writeStartElement("received");
        xmlWriter->writeAttribute("xmlns", ns_message_receipts);
        xmlWriter->writeAttribute("id", d->receiptId);
        xmlWriter->writeEndElement();
    }
    if (d->receiptRequested) {
        xmlWriter->writeStartElement("request");
        xmlWriter->writeAttribute("xmlns", ns_message_receipts);
        xmlWriter->writeEndElement();
    }

    // XEP-0224: Attention
    if (d->attentionRequested) {
        xmlWriter->writeStartElement("attention");
        xmlWriter->writeAttribute("xmlns", ns_attention);
        xmlWriter->writeEndElement();
    }

    // XEP-0249: Direct MUC Invitations
    if (!d->mucInvitationJid.isEmpty()) {
        xmlWriter->writeStartElement("x");
        xmlWriter->writeAttribute("xmlns", ns_conference);
        xmlWriter->writeAttribute("jid", d->mucInvitationJid);
        if (!d->mucInvitationPassword.isEmpty())
            xmlWriter->writeAttribute("password", d->mucInvitationPassword);
        if (!d->mucInvitationReason.isEmpty())
            xmlWriter->writeAttribute("reason", d->mucInvitationReason);
        xmlWriter->writeEndElement();
    }

    // Unison Extension: Attachments
    if (!d->attachment.isEmpty())
    {
        helperToXmlAddTextElement(xmlWriter, "attachment", d->attachment);
    }
    if (d->attachments.count())
    {
        xmlWriter->writeStartElement("attachments");
        xmlWriter->writeAttribute("xmlns", ns_attachments);
        Q_FOREACH(const QString &id, d->attachments)
        {
            if (!id.isEmpty()) {
                xmlWriter->writeStartElement("attachment");
                xmlWriter->writeAttribute("id", id);
                xmlWriter->writeEndElement();
            }
        }
        xmlWriter->writeEndElement();
    }

    // other extensions
    QXmppStanza::extensionsToXml(xmlWriter);

    xmlWriter->writeEndElement();
}
/// \endcond
