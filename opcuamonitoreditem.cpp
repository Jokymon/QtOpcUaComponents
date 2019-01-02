#include "opcuamonitoreditem.h"
#include "opcuaconnection.h"
#include <ua_client_subscriptions.h>
#include <QDateTime>
#include <QPainter>
#include <QDebug>

UA_NodeId variantToNodeId(int nsIndex, std::variant<int, QString> nodeId)
{
    if (auto intValue = std::get_if<int>(&nodeId)) {
        return UA_NODEID_NUMERIC(nsIndex, *intValue);
    }
    else if (auto stringValue = std::get_if<QString>(&nodeId)) {
        return UA_NODEID_STRING_ALLOC(nsIndex, stringValue->toLocal8Bit());
    }
    return UA_NodeId{};
}

typedef void (*dataChangeNotificationCallbackType)(UA_Client *client, UA_UInt32 subId, void *subContext, UA_UInt32 monId, void *monContext, UA_DataValue *value);

OpcUaMonitoredItem::OpcUaMonitoredItem(QWidget *parent) :
    QWidget(parent),
    opcConnection_(nullptr),
    nodeIdString_(),
    namespaceIndex_(-1)
{
}

OpcUaMonitoredItem::~OpcUaMonitoredItem()
{
}

void OpcUaMonitoredItem::setConnection(OpcUaConnection *connection)
{
    opcConnection_ = connection;
}

QString OpcUaMonitoredItem::nodeId() const
{
    return nodeIdString_;
}

void OpcUaMonitoredItem::setNodeId(QString nodeId)
{
    nodeIdString_ = nodeId;
    namespaceIndex_ = -1;

    static QRegExp numericNotation(R"((ns=(\d+);)?i=(\d+))");
    static QRegExp stringNotation(R"((ns=(\d+);)?s=([a-zA-Z0-9_\-]+))");

    int numericNotationPos = numericNotation.indexIn(nodeId);
    int stringNotationPos = stringNotation.indexIn(nodeId);
    if (numericNotationPos > -1) {
        if (numericNotation.cap(2).isEmpty()) {
            namespaceIndex_ = 0;
        }
        else {
            namespaceIndex_ = numericNotation.cap(2).toInt();
        }
        nodeId_ = numericNotation.cap(3).toInt();
    }
    else if (stringNotationPos > -1) {
        if (stringNotation.cap(2).isEmpty()) {
            namespaceIndex_ = 0;
        }
        else {
            namespaceIndex_ = stringNotation.cap(2).toInt();
        }
        nodeId_ = stringNotation.cap(3);
    }
    else {
        qWarning() << "This is an invalid notation for the nodeId: " << nodeId;
    }
}

QVariant OpcUaMonitoredItem::value() const
{
    return QVariant();
}

void OpcUaMonitoredItem::subscribe()
{
    if (!opcConnection_) {
        qWarning() << "Connection not available, can't subscribe to MonitoredItem";
        return;
    }

    if (namespaceIndex_<0) {
        qWarning() << "Invalid node Id" << nodeIdString_ << "; OPC-UA node Id must be specified in correct XML notation with NUMERIC or STRING type.";
        return;
    }

    UA_MonitoredItemCreateRequest monRequest =
        UA_MonitoredItemCreateRequest_default(variantToNodeId(namespaceIndex_, nodeId_));
    UA_MonitoredItemCreateResult monResponse =
        UA_Client_MonitoredItems_createDataChange(opcConnection_->client_, opcConnection_->subscriptionId_,
                                                  UA_TIMESTAMPSTORETURN_BOTH,
                                                  monRequest, this, reinterpret_cast<dataChangeNotificationCallbackType>(OpcUaMonitoredItem::s_dataChangeCallback), nullptr);
    if (monResponse.statusCode == UA_STATUSCODE_GOOD) {
        qDebug() << "Monitoring UA_NS0ID_SERVER_SERVERSTATUS_CURRENTTIME', id" <<
                    monResponse.monitoredItemId;
    }
    else {
        qWarning() << "Failed to create a MonitoredItem; code " << QString("%1").arg(monResponse.statusCode, 0, 16);
    }
}

void OpcUaMonitoredItem::unsubscribe()
{

}

void OpcUaMonitoredItem::paintEvent(QPaintEvent *)
{
    QPoint dummyOffset(0, 0);
    if (!redirected(&dummyOffset))
        return;

    QPainter painter(this);
    QColor itemColor(255, 0, 0);
    painter.setPen(Qt::NoPen);
    painter.setBrush(itemColor);
    painter.drawRect(rect());
}

void OpcUaMonitoredItem::s_dataChangeCallback(void *, uint32_t, void *, uint32_t monitoringId, void *monitoringContext, void *value)
{
    OpcUaMonitoredItem* opcMonitoredItem = static_cast<OpcUaMonitoredItem*>(monitoringContext);
    opcMonitoredItem->dataChangeCallback(monitoringId, value);
}

void OpcUaMonitoredItem::dataChangeCallback(uint32_t monitoringId, void *value)
{
    UA_DataValue *ua_value = reinterpret_cast<UA_DataValue*>(value);
    if (UA_Variant_hasScalarType(&ua_value->value, &UA_TYPES[UA_TYPES_BYTE])) {
        UA_Byte byte = *static_cast<UA_Byte*>(ua_value->value.data);
        value_ = QVariant(static_cast<QChar>(byte));
    } else if (UA_Variant_hasScalarType(&ua_value->value, &UA_TYPES[UA_TYPES_BOOLEAN])) {
        UA_Boolean boolean = *static_cast<UA_Boolean*>(ua_value->value.data);
        value_ = QVariant(static_cast<bool>(boolean));
    } else if(UA_Variant_hasScalarType(&ua_value->value, &UA_TYPES[UA_TYPES_DATETIME])) {
        UA_DateTime raw_date = *static_cast<UA_DateTime*>(ua_value->value.data);
        UA_DateTimeStruct dts = UA_DateTime_toStruct(raw_date);
        value_ = QVariant(
                    QDateTime(
                        QDate(dts.year, dts.month, dts.day),
                        QTime(dts.hour, dts.min, dts.sec, dts.milliSec)));
    }

    emit valueChanged(value_);
}
