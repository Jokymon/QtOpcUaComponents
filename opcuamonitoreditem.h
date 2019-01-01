#ifndef OPCUAMONITOREDITEM_H
#define OPCUAMONITOREDITEM_H

#include <QVariant>
#include <QWidget>
#include <QtUiPlugin/QDesignerExportWidget>
#include <variant>

class OpcUaConnection;

class QDESIGNER_WIDGET_EXPORT OpcUaMonitoredItem : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString nodeId READ nodeId WRITE setNodeId DESIGNABLE true)

signals:
    void valueChanged(QVariant value);

public:
    explicit OpcUaMonitoredItem(QWidget *parent = nullptr);
    virtual ~OpcUaMonitoredItem() override;

    void setConnection(OpcUaConnection *connection);

    QString nodeId() const;
    void setNodeId(QString nodeId);

    QVariant value() const;

public slots:
    void subscribe();
    void unsubscribe();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    static void s_dataChangeCallback(void *client, uint32_t subscriptionId, void *subscriptionContext, uint32_t monitoringId, void *monitoringContext, void *value);

    void dataChangeCallback(uint32_t monitoringId, void *value);

private:
    OpcUaConnection *_opcConnection;
    QString _nodeIdString;
    int _namespaceIndex;
    std::variant<int, QString> _nodeId;
    QVariant _value;
};


#endif // OPCUAMONITOREDITEM_H
