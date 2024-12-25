#ifndef CHATPROTOCOL_H
#define CHATPROTOCOL_H

#include <QByteArray>
#include <QString>
#include <QStringList>
#include <QMap>

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

    QByteArray textMessage(QByteArray encryptedAESKey, QByteArray message, QString receiver);
    QByteArray isTypingMessage(QString receiver);
    QByteArray setNameMessage(QString name);
    QByteArray setStatusMessage(Status status);
    QByteArray setInitSendingFileMessage(QString fileName, QString receiver);
    QByteArray setAcceptFileMessage();
    QByteArray setRejectFileMessage();
    QByteArray setFileMessage(QString receiver, QString fileName);
    QByteArray setNewClient(QString uid, QString email, QString loginStatus, QString publicKey);
    QByteArray setGroupChatMessage(QString groupName, QStringList memberList);
    QByteArray textGroupChatMessage(QString message, QString groupName);

    void loadData(QByteArray data);
    const QString &name() const;
    Status status() const;
    MessageType type() const;
    const QString &fileName() const;
    qint64 fileSize() const;
    const QByteArray &fileData() const;
    QString receiver() const;
    const QString &clientName() const;
    const QString &prevName() const;
    const QStringList &clientsName() const;
    const QString &myName() const;
    QString groupName() const;
    QStringList memberList() const;
    QString message() const;

    QString sender() const;

    QString publicKey() const;

    QByteArray encryptedMessage() const;

    QByteArray encryptedAESKey() const;

    QMap<QString, QString> publicKeys() const;

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
    QString _sender;
    QString _clientName;
    QString _prevName;
    QStringList _clientsName;
    QString _myName;
    QString _publicKey;
    QMap<QString, QString> _publicKeys;
    QString _groupName;
    QStringList _memberList;

};

#endif // CHATPROTOCOL_H
