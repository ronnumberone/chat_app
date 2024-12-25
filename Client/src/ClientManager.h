#ifndef CLIENTMANAGER_H
#define CLIENTMANAGER_H

#include "ChatProtocol.h"

#include <QHostAddress>
#include <QObject>
#include <QTcpSocket>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rand.h>

class ClientManager : public QObject
{
    Q_OBJECT
public:
    explicit ClientManager(QHostAddress ip = QHostAddress::LocalHost, ushort port = 4500, QObject *parent = nullptr);

    ~ClientManager();

    void connectToServer();
    void sendMessage(QString publicKey, QString message, QString receiver);
    void sendName(QString name);
    void sendStatus(ChatProtocol::Status status);
    void sendIsTyping(QString receiver);
    void sendFile(QString receiver, QString fileName);
    void sendNewClient(QString uid, QString email, QString loginStatus, QString publicKey);
    void sendGroupChat(QString groupName, QStringList memberList);
    void sendGroupMessage(QString message, QString groupName);

    ChatProtocol _protocol;

    QTcpSocket *socket() const;

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
    void sendPublicKey(QString publicKey, QString sender);
    void setPublicKey(QString publicKey);
    void groupChat(QString groupName, QStringList memberList, QString clientName);
    void textGroupChat(QString groupName, QString message, QString sender);

private slots:
    void readyRead();

private:
    //fields
    QTcpSocket *_socket;
    QHostAddress _ip;
    ushort _port;

    EVP_PKEY *_privateKey = nullptr;
    EVP_PKEY *_publicKey = nullptr;

    QString _tmpFileName;

    //methods
    void setupClient();
    bool generateRSAKeys(int keyLength = 2048);
    QString getPublicKeyPEM() const;
    EVP_PKEY* loadPublicKeyFromPEM(const QString& publicKeyPEM) const;
    QByteArray generateAESKey();
    QByteArray encryptAES(const QByteArray& plaintext, const QByteArray& key);
    QByteArray decryptAES(const QByteArray& ciphertext, const QByteArray& key);
    QByteArray encryptRSA(const QByteArray& data, EVP_PKEY* publicKey);
    QByteArray decryptRSA(const QByteArray& encryptedData, EVP_PKEY* privateKey);
};

#endif // CLIENTMANAGER_H
