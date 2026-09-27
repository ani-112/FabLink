#include "hsms/hsms_connection.h"
#include "secs/secs_message.h"

#include <QCoreApplication>
#include <QDebug>
#include <QTimer>

using namespace FabLink;

class HostSimulator : public QObject
{
    Q_OBJECT

public:
    explicit HostSimulator(QObject* parent = nullptr)
        : QObject(parent)
    {
        connect(&m_connection, &HsmsConnection::connected,
                this, &HostSimulator::onConnected);
        connect(&m_connection, &HsmsConnection::messageReceived,
                this, &HostSimulator::onMessage);
        connect(&m_connection, &HsmsConnection::logMessage,
                this, [](const QString& message) {
                    qInfo().noquote() << "[HSMS]" << message;
                });
    }

    void start()
    {
        m_connection.connectToHost(QStringLiteral("127.0.0.1"), 5000);
    }

private slots:
    void onConnected()
    {
        qInfo() << "HOST: TCP connected, sending Select.req";
        m_connection.send(HsmsMessage::selectRequest(m_system++));
    }

    void onMessage(const HsmsMessage& message)
    {
        if (!message.isDataMessage())
        {
            if (message.sessionType() == 2)
            {
                qInfo() << "HOST: Select.rsp received, sending S1F1";
                SecsMessage s1f1(1, 1, true, m_system++,
                                 SecsItem::list());
                m_connection.send(HsmsMessage::data(s1f1));
            }
            return;
        }

        const Stream stream = static_cast<Stream>(message.streamByte() & 0x7F);
        const Function function = message.function();

        qInfo() << "HOST: received S" << stream << "F" << function;

        if (stream == 1 && function == 2)
        {
            qInfo() << "HOST: sending S1F13";
            SecsMessage s1f13(1, 13, true, m_system++,
                              SecsItem::list({
                                  SecsItem::ascii(QStringLiteral("FabLinkHost"))
                              }));
            m_connection.send(HsmsMessage::data(s1f13));
        }
        else if (stream == 1 && function == 14)
        {
            qInfo() << "HOST: GEM communication established";
        }
        else if (stream == 6 && function == 11)
        {
            qInfo() << "HOST: S6F11 event received; sending S6F12";
            SecsMessage s6f12(6, 12, false, message.system(),
                              SecsItem::u1(0));
            m_connection.send(HsmsMessage::data(s6f12));
        }
    }

private:
    HsmsConnection m_connection;
    std::uint32_t m_system = 1;
};

#include "host_main.moc"

int main(int argc, char* argv[])
{
    QCoreApplication application(argc, argv);

    HostSimulator host;
    QTimer::singleShot(0, &host, &HostSimulator::start);

    return application.exec();
}
