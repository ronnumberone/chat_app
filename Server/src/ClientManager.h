#ifndef CLIENTMANAGER_H
#define CLIENTMANAGER_H

#include "ChatProtocol.h"

#include <QHostAddress>
#include <QObject>
#include <QTcpSocket>

class ClientManager : public QObject
{
    Q_OBJECT

public:
    explicit ClientManager(QHostAddress ip = QHostAddress::LocalHost, ushort port = 4500, QObject *parent = nullptr);
    explicit ClientManager(QTcpSocket *client, QObject *parent = nullptr);

    void connectToServer();
    void disconnectFromHost();

    //void sendMessage(QString message);
    void sendName(QString name);
    void sendStatus(ChatProtocol::Status status);
    QString name() const;
    void sendIsTyping(QString receiver);

    void sendInitSendingFile(QString fileName);
    void sendAcceptFile();
    void sendRejectFile();

signals:
    void connected();
    void disconnected();
    //    void dataReceived(QByteArray data);
    void textMessageReceived(QByteArray encryptedAESKey, const QByteArray encryptedMessage, QString receiver);
    void isTyping(QString receiver);
    void nameChanged(QString prevName, QString name);
    void statusChanged(ChatProtocol::Status status);
    void rejectReceivingFile();
    void initReceivingFile(QString clientName, QString fileName, qint64 fileSize);
    void fileSaved(QString path);
    void fileSend(QString receiver, QString fileName, qint64 fileSize, QByteArray fileData);
    void newClient(QString uid, QString email, QString loginStatus, QString publicKey);
    void groupChat(QString groupName, QStringList memberList);
    void textGroupChat(QString groupName, QString message);

private slots:
    void readyRead();

private:
    //fields
    QTcpSocket *_socket;
    QHostAddress _ip;
    ushort _port;
    ChatProtocol _protocol;
    QString _tmpFileName;

    //methods
    void setupClient();
    void sendFile();
    void saveFile();
};

#endif // CLIENTMANAGER_H
