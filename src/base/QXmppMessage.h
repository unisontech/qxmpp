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


#ifndef QXMPPMESSAGE_H
#define QXMPPMESSAGE_H

#include <QDateTime>
#include "QXmppLogger.h"
#include "QXmppStanza.h"

class QXmppMessagePrivate;
class QXmppAmpPrivate;
class QXmppAmpRulePrivate;

/// \brief The QXmppMessage class represents an XMPP message.
///
/// \ingroup Stanzas
///

class QXMPP_EXPORT QXmppMessage : public QXmppStanza, public QXmppLoggable
{
public:
    // XEP-0079: Advanced Message Processing
    class QXMPP_EXPORT QXmppAmp
    {
    public:
        class QXMPP_EXPORT QXmppAmpRule
        {
        public:
            enum Actions
            {
                alert = 0,
                drop,
                error,
                notify
            };
            enum Conditions
            {
                deliver = 0,
                expire_at,
                match_resource
            };
            enum Values
            {
                direct = 0,
                forward,
                gateway,
                none,
                stored,
                any,
                exact,
                other
            };

            QXmppAmpRule();
            QXmppAmpRule(const QXmppMessage::QXmppAmp::QXmppAmpRule &other);
            QXmppAmpRule(QXmppMessage::QXmppAmp::QXmppAmpRule::Actions action,
                QXmppMessage::QXmppAmp::QXmppAmpRule::Conditions condition,
                QXmppMessage::QXmppAmp::QXmppAmpRule::Values value);
            QXmppAmpRule(QXmppMessage::QXmppAmp::QXmppAmpRule::Actions action,
                QXmppMessage::QXmppAmp::QXmppAmpRule::Conditions condition,
                const QDateTime &value);
            ~QXmppAmpRule();

            QXmppMessage::QXmppAmp::QXmppAmpRule& operator=(const QXmppMessage::QXmppAmp::QXmppAmpRule& other);

            QXmppMessage::QXmppAmp::QXmppAmpRule::Actions action() const;
            QXmppMessage::QXmppAmp::QXmppAmpRule::Conditions condition() const;
            QXmppMessage::QXmppAmp::QXmppAmpRule::Values value() const;
            QDateTime expireAt() const;

        private:
            QSharedDataPointer<QXmppAmpRulePrivate> d;
        };

        QXmppAmp();
        QXmppAmp(const QXmppMessage::QXmppAmp &other);
        QXmppAmp(const QList<QXmppMessage::QXmppAmp::QXmppAmpRule> &rules);
        ~QXmppAmp();

        QXmppMessage::QXmppAmp& operator=(const QXmppMessage::QXmppAmp& other);

        void setRules(const QList<QXmppMessage::QXmppAmp::QXmppAmpRule> &rules);
        QList<QXmppMessage::QXmppAmp::QXmppAmpRule> rules() const;

        bool isStatus() const;
        void setStatus(QXmppMessage::QXmppAmp::QXmppAmpRule::Actions status);
        QXmppMessage::QXmppAmp::QXmppAmpRule::Actions status() const;

        void setTo(const QString &);
        QString to() const;

        void setFrom(const QString &);
        QString from() const;

        void setPerHop(bool);
        bool perHop() const;

    private:
        QSharedDataPointer<QXmppAmpPrivate> d;
    };

    /// This enum described a message type.
    enum Type
    {
        Error = 0,
        Normal,
        Chat,
        GroupChat,
        Headline
    };

    /// This enum describes a chat state as defined by
    /// XEP-0085 : Chat State Notifications.
    enum State
    {
        None = 0,   ///< The message does not contain any chat state information.
        Active,     ///< User is actively participating in the chat session.
        Inactive,   ///< User has not been actively participating in the chat session.
        Gone,       ///< User has effectively ended their participation in the chat session.
        Composing,  ///< User is composing a message.
        Paused,     ///< User had been composing but now has stopped.
    };

    QXmppMessage(const QString& from = "", const QString& to = "",
                 const QString& body = "", const QString& thread = "");
    QXmppMessage(const QXmppMessage &other);
    ~QXmppMessage();

    QXmppMessage& operator=(const QXmppMessage &other);

    QString body() const;
    void setBody(const QString&);

    bool isAttentionRequested() const;
    void setAttentionRequested(bool requested);

    bool isReceiptRequested() const;
    void setReceiptRequested(bool requested);

    bool isReceiptReceived() const;
    void setReceiptReceived(bool received);

    // Unison Extension: custom receipt read
    bool isReceiptRead() const;
    void setReceiptRead(bool read);

    QString mucInvitationJid() const;
    void setMucInvitationJid(const QString &jid);

    QString mucInvitationPassword() const;
    void setMucInvitationPassword(const QString &password);

    QString mucInvitationReason() const;
    void setMucInvitationReason(const QString &reason);

    QString receiptId() const;
    void setReceiptId(const QString &id);

    QDateTime stamp() const;
    void setStamp(const QDateTime &stamp);

    QXmppMessage::State state() const;
    void setState(QXmppMessage::State);

    QString subject() const;
    void setSubject(const QString&);

    QString thread() const;
    void setThread(const QString&);

    QXmppMessage::Type type() const;
    void setType(QXmppMessage::Type);

    QString xhtml() const;
    void setXhtml(const QString &xhtml);

    // XEP-0079: Advanced Message Processing
    bool isAmp() const;
    QXmppMessage::QXmppAmp amp() const;
    void setAmp(const QXmppAmp &);

    // Unison Extension: Chat History
    QString chatHistoryId() const;
    void setChatHistoryId(const QString& id);

    // Unison Extension: Attachments
    QString attachment() const;
    void setAttachment(const QString &);
    QStringList attachments() const;
    void setAttachments(const QStringList &);

    /// \cond
    void parse(const QDomElement &element);
    void toXml(QXmlStreamWriter *writer) const;
    /// \endcond

private:
    QSharedDataPointer<QXmppMessagePrivate> d;
};

#endif // QXMPPMESSAGE_H
