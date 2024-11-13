#include "ChatProtocol.h"
#include <QDataStream>
#include <QFileInfo>
#include <QIODevice>
#include <QMap>

ChatProtocol::ChatProtocol()
{

}

QByteArray ChatProtocol::textMessage(QByteArray encryptedAESKey, QByteArray encryptedMessage, QString sender)
{
    QByteArray ba;
    QDataStream out(&ba, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_15);
    out << Text << encryptedAESKey << encryptedMessage << sender;
    return ba;
}

QByteArray ChatProtocol::isTypingMessage(QString sender)
{
    return getData(IsTyping, sender);
}

QByteArray ChatProtocol::setNameMessage(QString name)
{
    return getData(SetName, name);
}

QByteArray ChatProtocol::setStatusMessage(Status status, QString sender)
{
    QByteArray ba;
    QDataStream out(&ba, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_15);
    out << SetStatus << status << sender;
    return ba;
}

QByteArray ChatProtocol::setInitSendingFileMessage(QString fileName)
{
    QByteArray ba;
    QFileInfo info(fileName);
    QDataStream out(&ba, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_15);
    out << InitSendingFile << info.fileName() << info.size();
    return ba;
}

QByteArray ChatProtocol::setAcceptFileMessage()
{
    return getData(AcceptSendingFile, "");
}

QByteArray ChatProtocol::setRejectFileMessage()
{
    return getData(RejectSendingFile, "");
}

QByteArray ChatProtocol::setFileMessage(QString sender, QString fileName, qint64 fileSize, QByteArray fileData)
{
    QByteArray ba;
    QDataStream out(&ba, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_15);
    out << SendFile << sender << fileName << fileSize << fileData;
    return ba;
}

QByteArray ChatProtocol::setClientNameMessage(QString prevName, QString name)
{
    QByteArray ba;
    QDataStream out(&ba, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_15);
    out << ClientName << prevName << name;
    return ba;
}

QByteArray ChatProtocol::setConnectionACKMessage(QString clientName, QStringList otherClients, QMap<QString, QString> publickeys)
{
    QByteArray ba;
    QDataStream out(&ba, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_15);
    out << ConnectionACK << clientName << otherClients << publickeys;
    return ba;
}

QByteArray ChatProtocol::setNewClientMessage(QString clientName)
{
    return getData(NewClient, clientName);
}

QByteArray ChatProtocol::setClinetDisconnectedMessage(QString clientName)
{
    return getData(ClientDisconnected, clientName);
}

void ChatProtocol::loadData(QByteArray data)
{
    QDataStream in(&data, QIODevice::ReadOnly);
    in.setVersion(QDataStream::Qt_5_15);
    in >> _type;
    switch (_type) {
    case Text:
        in >> _receiver >> _encryptedAESKey >> _encryptedMessage;
        break;
    case SetName:
        in >> _name;
        break;
    case IsTyping:
        in >> _receiver;
        break;
    case SetStatus:
        in >> _status;
        break;
    case NewClient:
        in >> _uid >> _email;
        break;
    case InitSendingFile:
        in >> _receiver >> _fileName >> _fileSize;
        break;
    case SendFile:
        in >> _receiver >> _fileName >> _fileSize >> _fileData;
        break;
    case SetPublicKey:
        in >> _publicKey;
        break;
    default:
        break;
    }
}

QByteArray ChatProtocol::getData(MessageType type, QString data)
{
    QByteArray ba;
    QDataStream out(&ba, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_15);
    out << type << data;
    return ba;
}

QByteArray ChatProtocol::setPublicKeyMessage(QString publicKey, QString name)
{
    QByteArray ba;
    QDataStream out(&ba, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_15);
    out << SetPublicKey << publicKey << name;
    return ba;
}

QString ChatProtocol::publicKey() const
{
    return _publicKey;
}

QByteArray ChatProtocol::encryptedAESKey() const
{
    return _encryptedAESKey;
}

QByteArray ChatProtocol::encryptedMessage() const
{
    return _encryptedMessage;
}

QString ChatProtocol::email() const
{
    return _email;
}

QString ChatProtocol::uid() const
{
    return _uid;
}

const QString &ChatProtocol::receiver() const
{
    return _receiver;
}

const QByteArray &ChatProtocol::fileData() const
{
    return _fileData;
}

qint64 ChatProtocol::fileSize() const
{
    return _fileSize;
}

const QString &ChatProtocol::fileName() const
{
    return _fileName;
}

ChatProtocol::MessageType ChatProtocol::type() const
{
    return _type;
}

ChatProtocol::Status ChatProtocol::status() const
{
    return _status;
}

const QString &ChatProtocol::name() const
{
    return _name;
}

