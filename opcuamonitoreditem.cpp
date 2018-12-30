#include "opcuamonitoreditem.h"
#include "opcuaconnection.h"
#include <ua_client_subscriptions.h>
#include <QDateTime>
#include <QPainter>
#include <QDebug>

typedef void (*dataChangeNotificationCallbackType)(UA_Client *client, UA_UInt32 subId, void *subContext, UA_UInt32 monId, void *monContext, UA_DataValue *value);

OpcUaMonitoredItem::OpcUaMonitoredItem(QWidget *parent) :
    QWidget(parent),
    _opcConnection(nullptr),
    _nodeId()
{
}

OpcUaMonitoredItem::~OpcUaMonitoredItem()
{
}

void OpcUaMonitoredItem::setConnection(OpcUaConnection *connection)
{
    _opcConnection = connection;
}

QString OpcUaMonitoredItem::nodeId() const
{
    return _nodeId;
}

void OpcUaMonitoredItem::setNodeId(QString nodeId)
{
    _nodeId = nodeId;
}

QVariant OpcUaMonitoredItem::value() const
{
    return QVariant();
}

void OpcUaMonitoredItem::subscribe()
{
    if (!_opcConnection) {
        qWarning() << "Connection not available, can't subscribe to MonitoredItem";
        return;
    }

    UA_MonitoredItemCreateRequest monRequest =
        UA_MonitoredItemCreateRequest_default(UA_NODEID_STRING_ALLOC(1, "current-time-datasource")/*nodeId.toUaNodeId()*/);
    UA_MonitoredItemCreateResult monResponse =
        UA_Client_MonitoredItems_createDataChange(_opcConnection->_client, _opcConnection->_subscriptionId,
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
        _value = QVariant(static_cast<QChar>(byte));
    } else if (UA_Variant_hasScalarType(&ua_value->value, &UA_TYPES[UA_TYPES_BOOLEAN])) {
        UA_Boolean boolean = *static_cast<UA_Boolean*>(ua_value->value.data);
        _value = QVariant(static_cast<bool>(boolean));
    } else if(UA_Variant_hasScalarType(&ua_value->value, &UA_TYPES[UA_TYPES_DATETIME])) {
        UA_DateTime raw_date = *static_cast<UA_DateTime*>(ua_value->value.data);
        UA_DateTimeStruct dts = UA_DateTime_toStruct(raw_date);
        _value = QVariant(
                    QDateTime(
                        QDate(dts.year, dts.month, dts.day),
                        QTime(dts.hour, dts.min, dts.sec, dts.milliSec)));
    }

    emit valueChanged(_value);
}
