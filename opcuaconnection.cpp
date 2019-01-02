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
    opcClientTimer_(),
    shownInDesigner_(false),
    serverUrl_(),
    state_(ConnectionState::UnconnectedState),
    subscriptionId_(std::numeric_limits<unsigned int>::max()),
    client_(nullptr)
{
    UA_ClientConfig config = UA_ClientConfig_default;
    config.stateCallback = reinterpret_cast<stateCallbackType>(OpcUaConnection::s_stateCallback);
    config.subscriptionInactivityCallback = reinterpret_cast<subscriptionInactivityCallbackType>(OpcUaConnection::s_subscriptionInactivityCallback);
    config.clientContext = this;
    client_ = UA_Client_new(config);

    opcClientTimer_.setInterval(100);
    opcClientTimer_.setSingleShot(false);
    QObject::connect(&opcClientTimer_, &QTimer::timeout,
                     this, &OpcUaConnection::opcMessagePump);
}

OpcUaConnection::~OpcUaConnection()
{
    opcClientTimer_.stop();
    UA_Client_delete(client_);
}

QUrl OpcUaConnection::serverUrl() const
{
    return serverUrl_;
}

void OpcUaConnection::setServerUrl(QUrl serverUrl)
{
    serverUrl_ = serverUrl;
}

OpcUaConnection::ConnectionState OpcUaConnection::state() const
{
    return state_;
}

void OpcUaConnection::setShownInDesigner(bool inDesigner)
{
    shownInDesigner_ = inDesigner;
    repaint();
}

void OpcUaConnection::connect()
{
    UA_StatusCode retval = UA_Client_connect_username(client_, serverUrl_.toString().toLatin1(), "user1", "password");
    if (retval == UA_STATUSCODE_GOOD) {
        state_ = ConnectionState::ConnectedState;
        opcClientTimer_.start();

        UA_CreateSubscriptionRequest request = UA_CreateSubscriptionRequest_default();
        UA_CreateSubscriptionResponse response = UA_Client_Subscriptions_create(client_, request,
                                                                                nullptr, nullptr, reinterpret_cast<deleteSubscriptionCallbackType>(OpcUaConnection::s_deleteSubscriptionCallback));

        if(response.responseHeader.serviceResult == UA_STATUSCODE_GOOD) {
            qDebug() << "Create subscription succeeded, id" << response.subscriptionId;
            subscriptionId_ = response.subscriptionId;
        }
        else
            return;

        emit connected();
        qDebug() << "Connected to the server";
    }
    else {
        qDebug() << "Couldn't connect client; error code=" << retval;
        UA_Client_delete(client_);
        client_ = nullptr;
        // TODO: capture status code
    }
}

void OpcUaConnection::disconnect()
{
    if (client_) {
        opcClientTimer_.stop();
        UA_Client_disconnect(client_);
        state_ = ConnectionState::UnconnectedState;
        emit disconnected();
    }
}

void OpcUaConnection::paintEvent(QPaintEvent *)
{
    if (!shownInDesigner_)
        return;

    QColor itemColor(255, 0, 0);

    QPainter painter(this);
    painter.setPen(Qt::NoPen);
    painter.setBrush(itemColor);
    painter.drawRect(rect());
}

void OpcUaConnection::opcMessagePump()
{
    UA_Client_run_iterate(client_, 0);
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
