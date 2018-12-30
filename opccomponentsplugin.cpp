#include "opccomponentsplugin.h"
#include "opcuaconnectionplugin.h"
#include "opcuamonitoreditemplugin.h"

MyCustomWidgets::MyCustomWidgets(QObject *parent) :
    QObject(parent)
{
    widgets.append(new OpcUaConnectionPlugin(this));
    widgets.append(new OpcUaMonitoredItemPlugin(this));
}

QList<QDesignerCustomWidgetInterface *> MyCustomWidgets::customWidgets() const
{
    return widgets;
}
