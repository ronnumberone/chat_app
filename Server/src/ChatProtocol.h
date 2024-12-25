#ifndef CHATPROTOCOL_H
#define CHATPROTOCOL_H

#include <QByteArray>
#include <QString>
#include <QStringList>

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
        GroupChat,
        TextGroupChat
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
    QByteArray setGroupChatMessage(QString groupName, QStringList memberList, QString clientName);

    QByteArray setClientNameMessage(QString prevName, QString name);
    QByteArray setConnectionACKMessage(QString clientName, QStringList otherClients, QMap<QString, QString> publickeys);
    QByteArray setNewClientMessage(QString clientName, QString publicKey);
    QByteArray setClinetDisconnectedMessage(QString clientName);
    QByteArray textGroupChatMessage(QString message, QString groupName, QString sender);

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

    QString loginStatus() const;

    QString groupName() const;

    QStringList memberList() const;

    QString message() const;

private:
    QByteArray getData(MessageType type, QString data);

    QString _message;
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
    QString _loginStatus;
    QString _publicKey;
    QString _groupName;
    QStringList _memberList;

};

#endif // CHATPROTOCOL_H
