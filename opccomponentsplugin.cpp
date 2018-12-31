#include "opccomponentsplugin.h"
#include "opcuaconnectionplugin.h"
#include "opcuamonitoreditemplugin.h"

OpcUaComponents::OpcUaComponents(QObject *parent) :
    QObject(parent)
{
    widgets.append(new OpcUaConnectionPlugin(this));
    widgets.append(new OpcUaMonitoredItemPlugin(this));
}

QList<QDesignerCustomWidgetInterface *> OpcUaComponents::customWidgets() const
{
    return widgets;
}
