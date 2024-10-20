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

    void connectToServer();
    void sendMessage(QString message, QString receiver);
    void sendName(QString name);
    void sendStatus(ChatProtocol::Status status);
    void sendIsTyping(QString receiver);
    void sendFile(QString receiver, QString fileName);

    ChatProtocol _protocol;

signals:
    void connected();
    void disconnected();
    //    void dataReceived(QByteArray data);
    void textMessageReceived(QString message, QString sender);
    void isTyping(QString sender);
    void nameChanged(QString name);
    void statusChanged(ChatProtocol::Status status, QString sender);
    void receiveFile(QString sender, QString fileName, qint64 fileSize, QByteArray fileData);

    void connectionACK(QString myName, QStringList clientsName);
    void newClientConnectedToServer(QString clienName);
    void clientNameChanged(QString prevName, QString clientName);
    void clientDisconnected(QString clientName);

private slots:
    void readyRead();

private:
    //fields
    QTcpSocket *_socket;
    QHostAddress _ip;
    ushort _port;

    QString _tmpFileName;

    //methods
    void setupClient();
};

#endif // CLIENTMANAGER_H
