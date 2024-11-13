#include "ChatProtocol.h"

#include <QDataStream>
#include <QFileInfo>
#include <QIODevice>
#include <QDebug>

ChatProtocol::ChatProtocol()
{

}

QByteArray ChatProtocol::textMessage(QByteArray encryptedAESKey, QByteArray encryptedMessage, QString receiver)
{
    QByteArray ba;
    QDataStream out(&ba, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_15);
    out << Text << receiver << encryptedAESKey << encryptedMessage;
    return ba;
}

QByteArray ChatProtocol::isTypingMessage(QString receiver)
{
    return getData(IsTyping, receiver);
}

QByteArray ChatProtocol::setNameMessage(QString name)
{
    return getData(SetName, name);
}

QByteArray ChatProtocol::setStatusMessage(Status status)
{
    QByteArray ba;
    QDataStream out(&ba, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_15);
    out << SetStatus << status;
    return ba;
}

QByteArray ChatProtocol::setInitSendingFileMessage(QString fileName, QString receiver)
{
    QByteArray ba;
    QDataStream out(&ba, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_15);
    QFileInfo info(fileName);
    out << InitSendingFile << receiver << info.fileName() << info.size();
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

QByteArray ChatProtocol::setFileMessage(QString receiver, QString fileName)
{
    QByteArray ba;
    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly)) {
        QDataStream out(&ba, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_5_15);
        QFileInfo info(fileName);
        out << SendFile << receiver << info.fileName() << info.size() << file.readAll();
        file.close();
    }
    return ba;
}

QByteArray ChatProtocol::setNewClient(QString uid, QString email)
{
    QByteArray ba;
    QDataStream out(&ba, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_15);
    out << NewClient << uid << email;
    return ba;
}

QByteArray ChatProtocol::setPublicKeyMessage(QString publicKey)
{
    return getData(SetPublicKey, publicKey);
}

void ChatProtocol::loadData(QByteArray data)
{
    QDataStream in(&data, QIODevice::ReadOnly);
    in.setVersion(QDataStream::Qt_5_15);
    in >> _type;
    switch (_type) {
    case Text:
        in  >> _encryptedAESKey >> _encryptedMessage >> _sender;
        break;
    case SetName:
        in >> _name;
        break;
    case IsTyping:
        in >> _sender;
        break;
    case SetStatus:
        in >> _status >> _sender;
        break;
    case InitSendingFile:
        in >> _fileName >> _fileSize;
        break;
    case SendFile:
        in >> _sender >> _fileName >> _fileSize >> _fileData;
        break;
    case ClientName:
        in >> _prevName >> _clientName;
        break;
    case NewClient:
    case ClientDisconnected:
        in >> _clientName;
        break;
    case ConnectionACK:
        in >> _myName >> _clientsName >> _publicKeys;
        break;
    case SetPublicKey:
        in >> _publicKey >> _sender;
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

QMap<QString, QString> ChatProtocol::publicKeys() const
{
    return _publicKeys;
}

QByteArray ChatProtocol::encryptedAESKey() const
{
    return _encryptedAESKey;
}

QByteArray ChatProtocol::encryptedMessage() const
{
    return _encryptedMessage;
}

QString ChatProtocol::publicKey() const
{
    return _publicKey;
}

QString ChatProtocol::sender() const
{
    return _sender;
}

const QString &ChatProtocol::myName() const
{
    return _myName;
}

const QStringList &ChatProtocol::clientsName() const
{
    return _clientsName;
}

const QString &ChatProtocol::prevName() const
{
    return _prevName;
}

const QString &ChatProtocol::clientName() const
{
    return _clientName;
}

QString ChatProtocol::receiver() const
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
