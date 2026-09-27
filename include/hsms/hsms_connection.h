#pragma once

#include "hsms/hsms_message.h"

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <memory>

namespace FabLink
{
class HsmsConnection : public QObject
{
    Q_OBJECT

public:
    explicit HsmsConnection(QObject* parent = nullptr);

    bool listen(quint16 port);
    void connectToHost(const QString& host, quint16 port);
    void close();

    bool isConnected() const;
    bool isListening() const;

    void send(const HsmsMessage& message);

signals:
    void connected();
    void disconnected();
    void messageReceived(const FabLink::HsmsMessage& message);
    void logMessage(const QString& message);

private slots:
    void onNewConnection();
    void onReadyRead();
    void onSocketDisconnected();

private:
    void attachSocket(QTcpSocket* socket);
    void processFrames();

    QTcpServer m_server;
    QTcpSocket* m_socket = nullptr;
    QByteArray m_buffer;
};
}
