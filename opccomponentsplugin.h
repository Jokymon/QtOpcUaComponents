#ifndef OPCCOMPONENTSPLUGIN_H
#define OPCCOMPONENTSPLUGIN_H

#include <QtUiPlugin/QDesignerCustomWidgetCollectionInterface>

class MyCustomWidgets: public QObject, public QDesignerCustomWidgetCollectionInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QDesignerCustomWidgetCollectionInterface" FILE "OpcUaComponents.json")
    Q_INTERFACES(QDesignerCustomWidgetCollectionInterface)

public:
    MyCustomWidgets(QObject *parent = nullptr);

    QList<QDesignerCustomWidgetInterface*> customWidgets() const override;

private:
    QList<QDesignerCustomWidgetInterface*> widgets;
};

#endif // OPCCOMPONENTSPLUGIN_H
