#include "opcuamonitoreditemplugin.h"
#include "opcuamonitoreditem.h"

OpcUaMonitoredItemPlugin::OpcUaMonitoredItemPlugin(QObject *parent) :
    QObject(parent)
{
}

bool OpcUaMonitoredItemPlugin::isContainer() const
{
    return false;
}

bool OpcUaMonitoredItemPlugin::isInitialized() const
{
    return initialized_;
}

QIcon OpcUaMonitoredItemPlugin::icon() const
{
    return QIcon();
}

QString OpcUaMonitoredItemPlugin::domXml() const
{
    return R"(<ui language="c++">
             <widget class="OpcUaMonitoredItem" name="opcUaMonitoredItem">
              <property name="geometry">
               <rect>
                <x>0</x>
                <y>0</y>
                <width>10</width>
                <height>10</height>
               </rect>
              </property>
             </widget>
           </ui>)";
}

QString OpcUaMonitoredItemPlugin::group() const
{
    return QStringLiteral("OPC-UA Components");
}

QString OpcUaMonitoredItemPlugin::includeFile() const
{
    return QStringLiteral("opcuamonitoreditem.h");
}

QString OpcUaMonitoredItemPlugin::name() const
{
    return QStringLiteral("OpcUaMonitoredItem");
}

QString OpcUaMonitoredItemPlugin::toolTip() const
{
    return QString();
}

QString OpcUaMonitoredItemPlugin::whatsThis() const
{
    return QString();
}

QWidget *OpcUaMonitoredItemPlugin::createWidget(QWidget *parent)
{
    auto *instance = new OpcUaMonitoredItem(parent);
    instance->setShownInDesigner(true);
    return instance;
}

void OpcUaMonitoredItemPlugin::initialize(QDesignerFormEditorInterface *core)
{
    if (initialized_)
        return;

    initialized_ = true;
}
