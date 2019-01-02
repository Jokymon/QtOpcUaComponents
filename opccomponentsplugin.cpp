#include "opccomponentsplugin.h"
#include "opcuaconnectionplugin.h"
#include "opcuamonitoreditemplugin.h"

OpcUaComponents::OpcUaComponents(QObject *parent) :
    QObject(parent)
{
    widgets_.append(new OpcUaConnectionPlugin(this));
    widgets_.append(new OpcUaMonitoredItemPlugin(this));
}

QList<QDesignerCustomWidgetInterface *> OpcUaComponents::customWidgets() const
{
    return widgets_;
}
