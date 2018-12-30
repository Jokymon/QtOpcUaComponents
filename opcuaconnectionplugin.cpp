#include "opcuaconnectionplugin.h"
#include "opcuaconnection.h"
#include <QDebug>

OpcUaConnectionPlugin::OpcUaConnectionPlugin(QObject *parent) :
    QObject(parent)
{
}

bool OpcUaConnectionPlugin::isContainer() const
{
    return false;
}

bool OpcUaConnectionPlugin::isInitialized() const
{
    return initialized;
}

QIcon OpcUaConnectionPlugin::icon() const
{
    return QIcon();
}

QString OpcUaConnectionPlugin::domXml() const
{
    return R"(<ui language="c++">
             <widget class="OpcUaConnection" name="opcUaConnection">
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

QString OpcUaConnectionPlugin::group() const
{
    return QStringLiteral("OPC-UA Components");
}

QString OpcUaConnectionPlugin::includeFile() const
{
    return QStringLiteral("opcuaconnection.h");
}

QString OpcUaConnectionPlugin::name() const
{
    return QStringLiteral("OpcUaConnection");
}

QString OpcUaConnectionPlugin::toolTip() const
{
    return QString();
}

QString OpcUaConnectionPlugin::whatsThis() const
{
    return QString();
}

QWidget *OpcUaConnectionPlugin::createWidget(QWidget *parent)
{
    auto *instance = new OpcUaConnection(parent);
    instance->setShownInDesigner(true);
    return instance;
}

void OpcUaConnectionPlugin::initialize(QDesignerFormEditorInterface *)
{
    if (initialized)
        return;

    initialized = true;
}
