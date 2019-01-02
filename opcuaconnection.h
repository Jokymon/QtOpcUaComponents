#ifndef OPCUACONNECTION_H
#define OPCUACONNECTION_H

#include <QUrl>
#include <QWidget>
#include <QTimer>
#include <QtUiPlugin/QDesignerExportWidget>

struct UA_Client;

class QDESIGNER_WIDGET_EXPORT OpcUaConnection : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QUrl serverUrl READ serverUrl WRITE setServerUrl DESIGNABLE true)

signals:
    void connected();
    void disconnected();

public:
    friend class OpcUaMonitoredItem;
    enum ConnectionState { UnconnectedState, ConnectedState };

    explicit OpcUaConnection(QWidget *parent = nullptr);
    virtual ~OpcUaConnection() override;

    QUrl serverUrl() const;
    void setServerUrl(QUrl serverUrl);
    ConnectionState state() const;

    void setShownInDesigner(bool inDesigner);

public slots:
    void connect();
    void disconnect();

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void opcMessagePump();

private:
    static void s_stateCallback(void *client, int clientState);
    static void s_subscriptionInactivityCallback(void *client, uint32_t subscriptionId, void *subscriptionContext);
    static void s_deleteSubscriptionCallback(void *client, uint32_t subscriptionId, void *subscriptionContext);

    void stateCallback(int clientState);
    void subscriptionInactivityCallback(uint32_t subscriptionId, void *subscriptionContext);
    void deleteSubscriptionCallback(uint32_t subscriptionId, void *subscriptionContext);

private:
    QTimer opcClientTimer_;
    bool shownInDesigner_;
    QUrl serverUrl_;
    ConnectionState state_;
    unsigned int subscriptionId_;

    UA_Client *client_;
};

#endif // OPCUACONNECTION_H
