#include "hsms/hsms_connection.h"

#include <QHostAddress>

namespace FabLink
{
HsmsConnection::HsmsConnection(QObject* parent)
    : QObject(parent)
{
    connect(&m_server, &QTcpServer::newConnection,
            this, &HsmsConnection::onNewConnection);
}

bool HsmsConnection::listen(quint16 port)
{
    const bool ok = m_server.listen(QHostAddress::LocalHost, port);
    if (ok)
        emit logMessage(QStringLiteral("HSMS server listening on port %1").arg(port));
    else
        emit logMessage(QStringLiteral("HSMS listen failed: %1").arg(m_server.errorString()));
    return ok;
}

void HsmsConnection::connectToHost(const QString& host, quint16 port)
{
    if (m_socket)
        m_socket->deleteLater();

    m_socket = new QTcpSocket(this);
    attachSocket(m_socket);
    m_socket->connectToHost(host, port);
    emit logMessage(QStringLiteral("Connecting to %1:%2").arg(host).arg(port));
}

void HsmsConnection::close()
{
    if (m_socket)
        m_socket->disconnectFromHost();
    m_server.close();
}

bool HsmsConnection::isConnected() const
{
    return m_socket && m_socket->state() == QAbstractSocket::ConnectedState;
}

bool HsmsConnection::isListening() const
{
    return m_server.isListening();
}

void HsmsConnection::send(const HsmsMessage& message)
{
    if (!isConnected())
    {
        emit logMessage(QStringLiteral("Cannot send HSMS message: socket is not connected"));
        return;
    }

    m_socket->write(message.encode());
    m_socket->flush();
}

void HsmsConnection::onNewConnection()
{
    if (m_socket)
    {
        m_socket->disconnectFromHost();
        m_socket->deleteLater();
    }

    m_socket = m_server.nextPendingConnection();
    attachSocket(m_socket);
    emit logMessage(QStringLiteral("Host connected to equipment"));
}

void HsmsConnection::attachSocket(QTcpSocket* socket)
{
    connect(socket, &QTcpSocket::readyRead,
            this, &HsmsConnection::onReadyRead);
    connect(socket, &QTcpSocket::connected,
            this, [this]() {
                emit connected();
                emit logMessage(QStringLiteral("HSMS TCP connection established"));
            });
    connect(socket, &QTcpSocket::disconnected,
            this, &HsmsConnection::onSocketDisconnected);
}

void HsmsConnection::onReadyRead()
{
    m_buffer.append(m_socket->readAll());
    processFrames();
}

void HsmsConnection::processFrames()
{
    while (m_buffer.size() >= 4)
    {
        const auto b0 = static_cast<std::uint8_t>(m_buffer.at(0));
        const auto b1 = static_cast<std::uint8_t>(m_buffer.at(1));
        const auto b2 = static_cast<std::uint8_t>(m_buffer.at(2));
        const auto b3 = static_cast<std::uint8_t>(m_buffer.at(3));
        const std::uint32_t length =
            (static_cast<std::uint32_t>(b0) << 24)
            | (static_cast<std::uint32_t>(b1) << 16)
            | (static_cast<std::uint32_t>(b2) << 8)
            | b3;

        if (length > 1024 * 1024)
        {
            emit logMessage(QStringLiteral("HSMS frame rejected: too large"));
            m_buffer.clear();
            return;
        }

        if (m_buffer.size() < static_cast<int>(length + 4))
            return;

        const QByteArray frame = m_buffer.left(static_cast<int>(length + 4));
        m_buffer.remove(0, static_cast<int>(length + 4));

        HsmsMessage message;
        if (HsmsMessage::decode(frame, message))
            emit messageReceived(message);
        else
            emit logMessage(QStringLiteral("Invalid HSMS frame received"));
    }
}

void HsmsConnection::onSocketDisconnected()
{
    emit disconnected();
    emit logMessage(QStringLiteral("HSMS TCP connection closed"));
}
}
