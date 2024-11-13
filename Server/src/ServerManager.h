#ifndef SERVERMANAGER_H
#define SERVERMANAGER_H

#include "ChatProtocol.h"

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QNetworkAccessManager>
#include "ChatProtocol.h"

class ServerManager : public QObject
{
    Q_OBJECT
public:
    explicit ServerManager(ushort port = 4500, QObject *parent = nullptr);
    void notifyOtherClients(QString prevName, QString name);
    void onClientTyping(QString sender, QString receiver);

public slots:
    void onTextForOtherClients(QByteArray encryptedAESKey, QByteArray encryptedMessage, QString receiver, QString sender);
    void onSendFile(QString receiver, QString fileName, qint64 fileSize, QByteArray fileData, QString sender);
    void onSetStatus(ChatProtocol::Status status, QString sender);
    void onNewClient(QString uid, QString email, QString name);
    void onSendPublicKey(QString publicKey, QString sender);

signals:
    void newClientConnected(QTcpSocket *client);
    void clientDisconnected(QTcpSocket *client);

private slots:
    void newClientConnectionReceived();
    void onClientDisconnected();

private: // fields
    QTcpServer *_server;
    QMap<QString, QTcpSocket *> _clients;
    QMap<QString, QString> _clientPublicKeys;
    ChatProtocol _protocol;
    QNetworkAccessManager *m_networkManager;

private: //mehtods
    void setupServer(ushort port);
};

#endif // SERVERMANAGER_H
