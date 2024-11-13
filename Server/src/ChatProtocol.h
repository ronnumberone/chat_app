#ifndef CHATPROTOCOL_H
#define CHATPROTOCOL_H

#include <QByteArray>
#include <QString>

class ChatProtocol
{
public:
    enum MessageType{
        Text,
        IsTyping,
        SetName,
        SetStatus,
        InitSendingFile,
        AcceptSendingFile,
        RejectSendingFile,
        SendFile,
        ClientName,
        ConnectionACK,
        NewClient,
        ClientDisconnected,
        SetPublicKey
    };

    enum Status{
        None,
        Available,
        Away,
        Busy
    };

    ChatProtocol();

    QByteArray textMessage(QByteArray encryptedAESKey, QByteArray encryptedMessage, QString sender);
    QByteArray isTypingMessage(QString sender);
    QByteArray setNameMessage(QString name);
    QByteArray setStatusMessage(Status status, QString sender);
    QByteArray setInitSendingFileMessage(QString fileName);
    QByteArray setAcceptFileMessage();
    QByteArray setRejectFileMessage();
    QByteArray setFileMessage(QString sender, QString fileName, qint64 fileSize, QByteArray fileData);

    QByteArray setClientNameMessage(QString prevName, QString name);
    QByteArray setConnectionACKMessage(QString clientName, QStringList otherClients, QMap<QString, QString> publickeys);
    QByteArray setNewClientMessage(QString clientName);
    QByteArray setClinetDisconnectedMessage(QString clientName);
    QByteArray setPublicKeyMessage(QString publicKey, QString name);

    void loadData(QByteArray data);
    const QString &name() const;
    Status status() const;
    MessageType type() const;
    const QString &fileName() const;
    qint64 fileSize() const;
    const QByteArray &fileData() const;
    const QString &receiver() const;

    QString uid() const;

    QString email() const;

    QString publicKey() const;

    QByteArray encryptedMessage() const;

    QByteArray encryptedAESKey() const;

private:
    QByteArray getData(MessageType type, QString data);

    MessageType _type;
    QByteArray _encryptedMessage;
    QByteArray _encryptedAESKey;
    QString _name;
    Status _status;
    QString _fileName;
    qint64 _fileSize;
    QByteArray _fileData;
    QString _receiver;
    QString _uid;
    QString _email;
    QString _publicKey;

};

#endif // CHATPROTOCOL_H
