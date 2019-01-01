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
    _opcConnection(nullptr),
    _nodeIdString(),
    _namespaceIndex(-1)
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
    return _nodeIdString;
}

void OpcUaMonitoredItem::setNodeId(QString nodeId)
{
    _nodeIdString = nodeId;
    _namespaceIndex = -1;

    static QRegExp numericNotation(R"((ns=(\d+);)?i=(\d+))");
    static QRegExp stringNotation(R"((ns=(\d+);)?s=([a-zA-Z0-9_\-]+))");

    int numericNotationPos = numericNotation.indexIn(nodeId);
    int stringNotationPos = stringNotation.indexIn(nodeId);
    if (numericNotationPos > -1) {
        if (numericNotation.cap(2).isEmpty()) {
            _namespaceIndex = 0;
        }
        else {
            _namespaceIndex = numericNotation.cap(2).toInt();
        }
        _nodeId = numericNotation.cap(3).toInt();
    }
    else if (stringNotationPos > -1) {
        if (stringNotation.cap(2).isEmpty()) {
            _namespaceIndex = 0;
        }
        else {
            _namespaceIndex = stringNotation.cap(2).toInt();
        }
        _nodeId = stringNotation.cap(3);
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
    if (!_opcConnection) {
        qWarning() << "Connection not available, can't subscribe to MonitoredItem";
        return;
    }

    if (_namespaceIndex<0) {
        qWarning() << "Invalid node Id" << _nodeIdString << "; OPC-UA node Id must be specified in correct XML notation with NUMERIC or STRING type.";
        return;
    }

    UA_MonitoredItemCreateRequest monRequest =
        UA_MonitoredItemCreateRequest_default(variantToNodeId(_namespaceIndex, _nodeId));
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
