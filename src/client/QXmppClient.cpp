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

#include <QSslSocket>
#include <QTimer>
#include <QUuid>
#include <QElapsedTimer>

#include "QXmppClient.h"
#include "QXmppClientExtension.h"
#include "QXmppConstants.h"
#include "QXmppLogger.h"
#include "QXmppOutgoingClient.h"
#include "QXmppMessage.h"
#include "QXmppUtils.h"

#include "QXmppVCardManager.h"
#include "QXmppVersionManager.h"
#include "QXmppEntityTimeManager.h"
#include "QXmppDiscoveryManager.h"
#include "QXmppDiscoveryIq.h"
#include "QXmppEntityTimeIq.h"

class QXmppClientPrivate
{
public:
    QXmppClientPrivate(QXmppClient *qq);

    QXmppPresence clientPresence;                   ///< Current presence of the client
    QList<QXmppClientExtension*> extensions;
    QXmppLogger *logger;
    QXmppOutgoingClient *stream;                    ///< Pointer to the XMPP stream

    // reconnection
    bool receivedConflict;
    int reconnectionTries;
    QTimer *reconnectionTimer;

    // managers
    QXmppVCardManager *vCardManager;
    QXmppVersionManager *versionManager;
    QXmppEntityTimeManager *timeManager;

    void addProperCapability(QXmppPresence& presence);
    int getNextReconnectTime() const;

    QString nextId() const;
    static unsigned long long lastId;
    QString id;

    QDateTime serverTime;
    QElapsedTimer elapsedTime;
    QTimer timeRequestTimer;

private:
    QXmppClient *q;
};

unsigned long long QXmppClientPrivate::lastId = 0;

QXmppClientPrivate::QXmppClientPrivate(QXmppClient *qq)
    : clientPresence(QXmppPresence::Available)
    , logger(0)
    , stream(0)
    , receivedConflict(false)
    , reconnectionTries(0)
    , reconnectionTimer(0)
    , vCardManager(0)
    , versionManager(0)
    , timeManager(0)
    , id(QUuid::createUuid().toString().mid(1, 8))
    , q(qq)
{
}

void QXmppClientPrivate::addProperCapability(QXmppPresence& presence)
{
    QXmppDiscoveryManager* ext = q->findExtension<QXmppDiscoveryManager>();
    if(ext) {
        presence.setCapabilityHash("sha-1");
        presence.setCapabilityNode(ext->clientCapabilitiesNode());
        presence.setCapabilityVer(ext->capabilities().verificationString());
    }
}

int QXmppClientPrivate::getNextReconnectTime() const
{
    if (reconnectionTries < 5)
        return 10 * 1000;
    else if (reconnectionTries < 10)
        return 20 * 1000;
    else if (reconnectionTries < 15)
        return 40 * 1000;
    else
        return 60 * 1000;
}

QString QXmppClientPrivate::nextId() const
{
    return id + "_" + QString::number(++lastId);
}

/// \mainpage
///
/// QXmpp is a cross-platform C++ XMPP client library based on the Qt
/// framework. It tries to use Qt's programming conventions in order to ease
/// the learning curve for new programmers.
///
/// QXmpp based clients are built using QXmppClient instances which handle the
/// establishment of the XMPP connection and provide a number of high-level
/// "managers" to perform specific tasks. You can write your own managers to
/// extend QXmpp by subclassing QXmppClientExtension.
///
/// <B>Main Class:</B>
/// - QXmppClient
///
/// <B>Managers to perform specific tasks:</B>
/// - QXmppVCardManager
/// - QXmppTransferManager
/// - QXmppMucManager
/// - QXmppCallManager
/// - QXmppArchiveManager
/// - QXmppVersionManager
/// - QXmppDiscoveryManager
/// - QXmppEntityTimeManager
///
/// <B>XMPP stanzas:</B> If you are interested in a more low-level API, you can refer to these
/// classes.
/// - QXmppIq
/// - QXmppMessage
/// - QXmppPresence
///
/// <BR><BR>
/// <B>Project Details:</B>
///
/// Project Page: http://code.google.com/p/qxmpp/
/// <BR>
/// Report Issues: http://code.google.com/p/qxmpp/issues/
/// <BR>
/// New Releases: http://code.google.com/p/qxmpp/downloads/
///

/// Creates a QXmppClient object.
/// \param parent is passed to the QObject's constructor.
/// The default value is 0.

QXmppClient::QXmppClient(QObject *parent)
    : QXmppLoggable(parent),
    d(new QXmppClientPrivate(this))
{
    bool check;
    Q_UNUSED(check);

    d->stream = new QXmppOutgoingClient(this);
    d->addProperCapability(d->clientPresence);

    check = connect(d->stream, SIGNAL(elementReceived(QDomElement,bool&)),
                    this, SLOT(_q_elementReceived(QDomElement,bool&)));
    Q_ASSERT(check);

    check = connect(d->stream, SIGNAL(messageReceived(QXmppMessage)),
                    this, SIGNAL(messageReceived(QXmppMessage)));
    Q_ASSERT(check);

    check = connect(d->stream, SIGNAL(presenceReceived(QXmppPresence)),
                    this, SIGNAL(presenceReceived(QXmppPresence)));
    Q_ASSERT(check);

    check = connect(d->stream, SIGNAL(iqReceived(QXmppIq)),
                    this, SIGNAL(iqReceived(QXmppIq)));
    Q_ASSERT(check);

    check = connect(d->stream->socket(), SIGNAL(stateChanged(QAbstractSocket::SocketState)),
                    this, SLOT(_q_socketStateChanged(QAbstractSocket::SocketState)));
    Q_ASSERT(check);

    check = connect(d->stream, SIGNAL(connected()),
                    this, SLOT(_q_streamConnected()));
    Q_ASSERT(check);

    check = connect(d->stream, SIGNAL(disconnected()),
                    this, SLOT(_q_streamDisconnected()));
    Q_ASSERT(check);

    check = connect(d->stream, SIGNAL(error(QXmppClient::Error)),
                    this, SLOT(_q_streamError(QXmppClient::Error)));
    Q_ASSERT(check);

    // reconnection
    d->reconnectionTimer = new QTimer(this);
    d->reconnectionTimer->setSingleShot(true);
    connect(d->reconnectionTimer, SIGNAL(timeout()),
            this, SLOT(_q_reconnect()));
    Q_ASSERT(check);

    // logging
    setLogger(QXmppLogger::getLogger());

    d->vCardManager = new QXmppVCardManager;
    addExtension(d->vCardManager);

    d->versionManager = new QXmppVersionManager;
    addExtension(d->versionManager);

    _q_timeManagerInit();

    addExtension(new QXmppDiscoveryManager());
}

/// Destructor, destroys the QXmppClient object.
///

QXmppClient::~QXmppClient()
{
    delete d;
}

/// Registers a new extension with the client.
///
/// \param extension

bool QXmppClient::addExtension(QXmppClientExtension* extension)
{
    if (d->extensions.contains(extension))
    {
        qWarning("Cannot add extension, it has already been added");
        return false;
    }

    extension->setParent(this);
    extension->setClient(this);
    d->extensions << extension;
    return true;
}

/// Unregisters the given extension from the client. If the extension
/// is found, it will be destroyed.
///
/// \param extension

bool QXmppClient::removeExtension(QXmppClientExtension* extension)
{
    if (d->extensions.contains(extension))
    {
        d->extensions.removeAll(extension);
        delete extension;
        return true;
    } else {
        qWarning("Cannot remove extension, it was never added");
        return false;
    }
}

/// Returns a list containing all the client's extensions.
///

QList<QXmppClientExtension*> QXmppClient::extensions()
{
    return d->extensions;
}

/// Returns a modifiable reference to the current configuration of QXmppClient.
/// \return Reference to the QXmppClient's configuration for the connection.

QXmppConfiguration& QXmppClient::configuration()
{
    return d->stream->configuration();
}

/// Attempts to connect to the XMPP server. Server details and other configurations
/// are specified using the config parameter. Use signals connected(), error(QXmppClient::Error)
/// and disconnected() to know the status of the connection.
/// \param config Specifies the configuration object for connecting the XMPP server.
/// This contains the host name, user, password etc. See QXmppConfiguration for details.
/// \param initialPresence The initial presence which will be set for this user
/// after establishing the session. The default value is QXmppPresence::Available

void QXmppClient::connectToServer(const QXmppConfiguration& config,
                                  const QXmppPresence& initialPresence)
{
    d->stream->configuration() = config;
    d->clientPresence = initialPresence;
    d->addProperCapability(d->clientPresence);

    d->stream->connectToHost();
}

/// Overloaded function to simply connect to an XMPP server with a JID and password.
///
/// \param jid JID for the account.
/// \param password Password for the account.

void QXmppClient::connectToServer(const QString &jid, const QString &password)
{
    QXmppConfiguration config;
    config.setJid(jid);
    config.setPassword(password);
    connectToServer(config);
}

/// After successfully connecting to the server use this function to send
/// stanzas to the server. This function can solely be used to send various kind
/// of stanzas to the server. QXmppStanza is a parent class of all the stanzas
/// QXmppMessage, QXmppPresence, QXmppIq, QXmppBind, QXmppRosterIq, QXmppSession
/// and QXmppVCard.
///
/// \return Returns true if the packet was sent, false otherwise.
///
/// Following code snippet illustrates how to send a message using this function:
/// \code
/// QXmppMessage message(from, to, message);
/// client.sendPacket(message);
/// \endcode
///
/// \param packet A valid XMPP stanza. It can be an iq, a message or a presence stanza.
///

bool QXmppClient::sendPacket(const QXmppStanza& packet)
{
    return d->stream->sendPacket(packet);
}

/// Disconnects the client and the current presence of client changes to
/// QXmppPresence::Unavailable and status text changes to "Logged out".
///
/// \note Make sure that the clientPresence is changed to
/// QXmppPresence::Available, if you are again calling connectToServer() after
/// calling the disconnectFromServer() function.
///

void QXmppClient::disconnectFromServer()
{
    // cancel reconnection
    d->reconnectionTimer->stop();

    d->clientPresence.setType(QXmppPresence::Unavailable);
    d->clientPresence.setStatusText("Logged out");
    if (d->stream->isConnected())
        sendPacket(d->clientPresence);

    d->stream->disconnectFromHost();
}

/// Returns true if the client has authenticated with the XMPP server.

bool QXmppClient::isAuthenticated() const
{
    return d->stream->isAuthenticated();
}

/// Returns true if the client is connected to the XMPP server.
///

bool QXmppClient::isConnected() const
{
    return d->stream->isConnected();
}

/// Utility function to send message to all the resources associated with the
/// specified bareJid. If there are no resources available, that is the contact
/// is offline or not present in the roster, it will still send a message to
/// the bareJid.
///
/// \param bareJid bareJid of the receiving entity
/// \param message Message string to be sent.
/// \param message XHTML Message body to be sent. If empty, it will be same as Message.
///
/// \return stanzaIds of messages that was sent

QStringList QXmppClient::sendMessage(const QString& bareJid, const QString& message, const QString& xhtmlMessage,
    const QStringList& attachments, const QString& attachment)
{
    QString xhtml = xhtmlMessage;
    if (xhtmlMessage.isEmpty())
        xhtml = message;

    QStringList stanzas;

    stanzas.append(_q_sendMessage(bareJid, message, xhtml, attachments, attachment));

    return stanzas;
}

///

/// Returns the client's current state.

QXmppClient::State QXmppClient::state() const
{
    if (d->stream->isConnected())
        return QXmppClient::ConnectedState;
    else if (d->stream->socket()->state() != QAbstractSocket::UnconnectedState &&
             d->stream->socket()->state() != QAbstractSocket::ClosingState)
        return QXmppClient::ConnectingState;
    else
        return QXmppClient::DisconnectedState;
}

/// Returns the client's current presence.
///

QXmppPresence QXmppClient::clientPresence() const
{
    return d->clientPresence;
}

/// Changes the presence of the connected client.
///
/// The connection to the server will be updated accordingly:
///
/// \li If the presence type is QXmppPresence::Unavailable, the connection
/// to the server will be closed.
///
/// \li Otherwise, the connection to the server will be established
/// as needed.
///
/// \param presence QXmppPresence object
///

void QXmppClient::setClientPresence(const QXmppPresence& presence)
{
    d->clientPresence = presence;
    d->addProperCapability(d->clientPresence);

    if (presence.type() == QXmppPresence::Unavailable)
    {
        // cancel reconnection
        d->reconnectionTimer->stop();

        // NOTE: we can't call disconnect() because it alters
        // the client presence
        if (d->stream->isConnected())
            sendPacket(d->clientPresence);

        d->stream->disconnectFromHost();
    }
    else if (d->stream->isConnected())
        sendPacket(d->clientPresence);
    else
        connectToServer(d->stream->configuration(), presence);
}

/// Returns the socket error if error() is QXmppClient::SocketError.
///

QAbstractSocket::SocketError QXmppClient::socketError()
{
    return d->stream->socket()->error();
}

/// Returns the XMPP stream error if QXmppClient::Error is QXmppClient::XmppStreamError.
///

QXmppStanza::Error::Condition QXmppClient::xmppStreamError()
{
    return d->stream->xmppStreamError();
}

/// Returns the reference to QXmppVCardManager, implementation of XEP-0054.
/// http://xmpp.org/extensions/xep-0054.html
///

QXmppVCardManager& QXmppClient::vCardManager()
{
    return *d->vCardManager;
}

/// Returns the reference to QXmppVersionManager, implementation of XEP-0092.
/// http://xmpp.org/extensions/xep-0092.html
///

QXmppVersionManager& QXmppClient::versionManager()
{
    return *d->versionManager;
}

const QDateTime QXmppClient::currentServerTime() const
{
    return d->serverTime.addMSecs(d->elapsedTime.elapsed());
}

/// Give extensions a chance to handle incoming stanzas.
///
/// \param element
/// \param handled

void QXmppClient::_q_elementReceived(const QDomElement &element, bool &handled)
{
    foreach (QXmppClientExtension *extension, d->extensions)
    {
        if (extension->handleStanza(element))
        {
            handled = true;
            return;
        }
    }
}

void QXmppClient::_q_reconnect()
{
    if (d->stream->configuration().autoReconnectionEnabled()) {
        debug("Reconnecting to server");
        d->stream->connectToHost();
    }
}

void QXmppClient::_q_socketStateChanged(QAbstractSocket::SocketState socketState)
{
    Q_UNUSED(socketState);
    emit stateChanged(state());
}

/// At connection establishment, send initial presence.

void QXmppClient::_q_streamConnected()
{
    d->receivedConflict = false;
    d->reconnectionTries = 0;

    // notify managers
    emit connected();
    emit stateChanged(QXmppClient::ConnectedState);

    _q_serverTimeRequest();
}

void QXmppClient::_q_streamDisconnected()
{
    _q_serverTimeAbort();

    // notify managers
    emit disconnected();
    emit stateChanged(QXmppClient::DisconnectedState);
}

void QXmppClient::_q_streamError(QXmppClient::Error err)
{
    if (d->stream->configuration().autoReconnectionEnabled()) {
        if (err == QXmppClient::XmppStreamError) {
            // if we receive a resource conflict, inhibit reconnection
            if (d->stream->xmppStreamError() == QXmppStanza::Error::Conflict)
                d->receivedConflict = true;
        } else if (err == QXmppClient::SocketError && !d->receivedConflict) {
            // schedule reconnect
            d->reconnectionTimer->start(d->getNextReconnectTime());
        } else if (err == QXmppClient::KeepAliveError) {
            // if we got a keepalive error, reconnect in one second
            d->reconnectionTimer->start(1000);
        }
    }

    // notify managers
    emit error(err);
}

void QXmppClient::_q_serverTimeReceived(const QXmppEntityTimeIq &time)
{
    QDateTime utcTime = time.utc();
    utcTime.setTimeSpec(Qt::UTC);
    if (!utcTime.isNull() && utcTime.isValid()) {
        d->serverTime = utcTime;
        d->elapsedTime.restart();
    }

    d->timeRequestTimer.start();
}

void QXmppClient::_q_serverTimeRequest()
{
    d->timeManager->requestTime("u1");
}

void QXmppClient::_q_serverTimeAbort()
{
    d->timeRequestTimer.stop();
}

QString QXmppClient::_q_sendMessage(const QString& to, const QString& message, const QString& xhtmlMessage,
    const QStringList& attachments, const QString& attachment)
{
    QXmppMessage m("", to, message);
    m.setXhtml(xhtmlMessage);
    QList<QXmppMessage::QXmppAmp::QXmppAmpRule> rules;
    rules.append(QXmppMessage::QXmppAmp::QXmppAmpRule(
        QXmppMessage::QXmppAmp::QXmppAmpRule::notify,
        QXmppMessage::QXmppAmp::QXmppAmpRule::deliver,
        QXmppMessage::QXmppAmp::QXmppAmpRule::stored));
    rules.append(QXmppMessage::QXmppAmp::QXmppAmpRule(
        QXmppMessage::QXmppAmp::QXmppAmpRule::notify,
        QXmppMessage::QXmppAmp::QXmppAmpRule::deliver,
        QXmppMessage::QXmppAmp::QXmppAmpRule::direct));
    rules.append(QXmppMessage::QXmppAmp::QXmppAmpRule(
        QXmppMessage::QXmppAmp::QXmppAmpRule::notify,
        QXmppMessage::QXmppAmp::QXmppAmpRule::deliver,
        QXmppMessage::QXmppAmp::QXmppAmpRule::none));
    QXmppMessage::QXmppAmp amp(rules);
    m.setAmp(amp);
    m.setId("chat_" + d->nextId());
    m.setReceiptRequested(true);
    m.setAttachments(attachments);
    m.setAttachment(attachment);
    sendPacket(m);
    return m.id();
}

void QXmppClient::_q_timeManagerInit()
{
    bool check;
    Q_UNUSED(check);

    d->timeManager = new QXmppEntityTimeManager();
    addExtension(d->timeManager);

    check = connect(d->timeManager, SIGNAL(timeReceived(QXmppEntityTimeIq)),
                    this, SLOT(_q_serverTimeReceived(QXmppEntityTimeIq)));
    Q_ASSERT(check);

    d->serverTime = QDateTime::currentDateTimeUtc();
    d->elapsedTime.start();

    d->timeRequestTimer.setInterval(60 * 60 * 1000);
    d->timeRequestTimer.setSingleShot(true);
    check = QObject::connect(&d->timeRequestTimer, SIGNAL(timeout()), this, SLOT(_q_serverTimeRequest()), Qt::UniqueConnection);
    Q_ASSERT(check);
}

/// Returns the QXmppLogger associated with the current QXmppClient.

QXmppLogger *QXmppClient::logger() const
{
    return d->logger;
}

/// Sets the QXmppLogger associated with the current QXmppClient.

void QXmppClient::setLogger(QXmppLogger *logger)
{
    if (logger != d->logger) {
        if (d->logger) {
            disconnect(this, SIGNAL(logMessage(QXmppLogger::MessageType,QString)),
                       d->logger, SLOT(log(QXmppLogger::MessageType,QString)));
            disconnect(this, SIGNAL(setGauge(QString,double)),
                       d->logger, SLOT(setGauge(QString,double)));
            disconnect(this, SIGNAL(updateCounter(QString,qint64)),
                       d->logger, SLOT(updateCounter(QString,qint64)));
        }

        d->logger = logger;
        if (d->logger) {
            connect(this, SIGNAL(logMessage(QXmppLogger::MessageType,QString)),
                    d->logger, SLOT(log(QXmppLogger::MessageType,QString)));
            connect(this, SIGNAL(setGauge(QString,double)),
                    d->logger, SLOT(setGauge(QString,double)));
            connect(this, SIGNAL(updateCounter(QString,qint64)),
                    d->logger, SLOT(updateCounter(QString,qint64)));
        }

        emit loggerChanged(d->logger);
    }
}

