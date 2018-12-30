#include "opcuaconnection.h"
#include <QPainter>
#include <ua_client_highlevel.h>
#include <ua_config_default.h>
#include <ua_client_subscriptions.h>
#include <QDebug>

#undef max

typedef void (*stateCallbackType)(UA_Client *client, UA_ClientState clientState);
typedef void (*subscriptionInactivityCallbackType)(UA_Client *client, UA_UInt32 subId, void *subContext);
typedef void (*deleteSubscriptionCallbackType)(UA_Client *client, UA_UInt32 subscriptionId, void *);

OpcUaConnection::OpcUaConnection(QWidget *parent) :
    QWidget(parent),
    _opcClientTimer(),
    _shownInDesigner(false),
    _serverUrl(),
    _state(ConnectionState::UnconnectedState),
    _subscriptionId(std::numeric_limits<unsigned int>::max()),
    _client(nullptr)
{
    UA_ClientConfig config = UA_ClientConfig_default;
    config.stateCallback = reinterpret_cast<stateCallbackType>(OpcUaConnection::s_stateCallback);
    config.subscriptionInactivityCallback = reinterpret_cast<subscriptionInactivityCallbackType>(OpcUaConnection::s_subscriptionInactivityCallback);
    config.clientContext = this;
    _client = UA_Client_new(config);

    _opcClientTimer.setInterval(100);
    _opcClientTimer.setSingleShot(false);
    QObject::connect(&_opcClientTimer, &QTimer::timeout,
                     this, &OpcUaConnection::opcMessagePump);
}

OpcUaConnection::~OpcUaConnection()
{
    _opcClientTimer.stop();
    UA_Client_delete(_client);
}

QUrl OpcUaConnection::serverUrl() const
{
    return _serverUrl;
}

void OpcUaConnection::setServerUrl(QUrl serverUrl)
{
    _serverUrl = serverUrl;
}

OpcUaConnection::ConnectionState OpcUaConnection::state() const
{
    return _state;
}

void OpcUaConnection::setShownInDesigner(bool inDesigner)
{
    _shownInDesigner = inDesigner;
    repaint();
}

void OpcUaConnection::connect()
{
    UA_StatusCode retval = UA_Client_connect_username(_client, _serverUrl.toString().toLatin1(), "user1", "password");
    if (retval == UA_STATUSCODE_GOOD) {
        _state = ConnectionState::ConnectedState;
        _opcClientTimer.start();

        UA_CreateSubscriptionRequest request = UA_CreateSubscriptionRequest_default();
        UA_CreateSubscriptionResponse response = UA_Client_Subscriptions_create(_client, request,
                                                                                nullptr, nullptr, reinterpret_cast<deleteSubscriptionCallbackType>(OpcUaConnection::s_deleteSubscriptionCallback));

        if(response.responseHeader.serviceResult == UA_STATUSCODE_GOOD) {
            qDebug() << "Create subscription succeeded, id" << response.subscriptionId;
            _subscriptionId = response.subscriptionId;
        }
        else
            return;

        emit connected();
        qDebug() << "Connected to the server";
    }
    else {
        qDebug() << "Couldn't connect client; error code=" << retval;
        UA_Client_delete(_client);
        _client = nullptr;
        // TODO: capture status code
    }
}

void OpcUaConnection::disconnect()
{
    if (_client) {
        _opcClientTimer.stop();
        UA_Client_disconnect(_client);
        _state = ConnectionState::UnconnectedState;
        emit disconnected();
    }
}

void OpcUaConnection::paintEvent(QPaintEvent *)
{
    QPoint dummyOffset(0, 0);
    if (!redirected(&dummyOffset))
        return;

    if (_shownInDesigner)
    {
        QColor itemColor(255, 0, 0);

        QPainter painter(this);
        painter.setPen(Qt::NoPen);
        painter.setBrush(itemColor);
        painter.drawRect(rect());
    }
}

void OpcUaConnection::opcMessagePump()
{
    UA_Client_run_iterate(_client, 0);
}

void OpcUaConnection::stateCallback(int clientState)
{
    qDebug() << "Client state: " << clientState;
}

void OpcUaConnection::subscriptionInactivityCallback(uint32_t subscriptionId, void *)
{
    qDebug() << "Inactivity for subscription" << subscriptionId;
}

void OpcUaConnection::deleteSubscriptionCallback(uint32_t subscriptionId, void *)
{
    qDebug() << "Subscription Id" << subscriptionId << "was deleted";
}

void OpcUaConnection::s_stateCallback(void *client, int clientState)
{
    auto ua_client = reinterpret_cast<UA_Client*>(client);
    OpcUaConnection* opcConnection = static_cast<OpcUaConnection*>(UA_Client_getContext(ua_client));
    opcConnection->stateCallback(clientState);
}

void OpcUaConnection::s_subscriptionInactivityCallback(void *client, uint32_t subscriptionId, void *subscriptionContext)
{
    auto ua_client = reinterpret_cast<UA_Client*>(client);
    OpcUaConnection* opcConnection = static_cast<OpcUaConnection*>(UA_Client_getContext(ua_client));
    opcConnection->subscriptionInactivityCallback(subscriptionId, subscriptionContext);
}

void OpcUaConnection::s_deleteSubscriptionCallback(void *client, uint32_t subscriptionId, void *subscriptionContext)
{
    auto ua_client = reinterpret_cast<UA_Client*>(client);
    OpcUaConnection* opcConnection = static_cast<OpcUaConnection*>(UA_Client_getContext(ua_client));
    opcConnection->deleteSubscriptionCallback(subscriptionId, subscriptionContext);
}
