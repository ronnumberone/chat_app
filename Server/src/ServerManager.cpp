#include "ServerManager.h"
#include <QDebug>

ServerManager::ServerManager(ushort port, QObject *parent)
    : QObject{parent}
{
    setupServer(port);
}

void ServerManager::notifyOtherClients(QString prevName, QString name)
{
    auto message = _protocol.setClientNameMessage(prevName, name);
    foreach (auto cl, _clients) {
        auto clientName = cl->property("clientName").toString();
        if (clientName != name) {
            cl->write(message);
        }
    }
}

void ServerManager::onTextForOtherClients(QString message, QString receiver, QString sender)
{
    auto msg = _protocol.textMessage(message, sender);
    foreach (auto cl, _clients) {
        auto clientName = cl->property("clientName").toString();
        if (clientName == receiver) {
            cl->write(msg);
            return;
        }
    }
}

void ServerManager::onSendFile(QString receiver, QString fileName, qint64 fileSize, QByteArray fileData, QString sender)
{
    auto fileMessage = _protocol.setFileMessage(sender, fileName, fileSize, fileData);
    foreach (auto cl, _clients) {
        auto clientName = cl->property("clientName").toString();
        if (clientName == receiver) {
            cl->write(fileMessage);
            return;
        }
    }
}

void ServerManager::onSetStatus(ChatProtocol::Status status, QString sender)
{
    auto statusMessage = _protocol.setStatusMessage(status, sender);
    foreach (auto cl, _clients) {
        cl->write(statusMessage);
    }
}

void ServerManager::onClientTyping(QString sender, QString receiver)
{
    auto typingMessage = _protocol.isTypingMessage(sender);
    foreach (auto cl, _clients) {
        auto clientName = cl->property("clientName").toString();
        if (clientName == receiver) {
            cl->write(typingMessage);
        }
    }
}

void ServerManager::newClientConnectionReceived()
{
    auto client = _server->nextPendingConnection();

    auto id = _clients.count() + 1;
    auto clientName = QString("Client (%1)").arg(id);
    client->setProperty("id", id);
    client->setProperty("clientName", clientName);

    connect(client, &QTcpSocket::disconnected, this, &ServerManager::onClientDisconnected);
    emit newClientConnected(client);

    if (id > 1) {
        auto message = _protocol.setConnectionACKMessage(clientName, _clients.keys());
        client->write(message);

        auto newClientMessage = _protocol.setNewClientMessage(clientName);
        foreach (auto cl, _clients) {
            cl->write(newClientMessage);
        }
    }
    _clients[clientName] = client;
}

void ServerManager::onClientDisconnected()
{
    auto client = qobject_cast<QTcpSocket *>(sender());
    auto clientName = client->property("clientName").toString();
    _clients.remove(clientName);
    auto message = _protocol.setClinetDisconnectedMessage(clientName);
    foreach (auto cl, _clients) {
        cl->write(message);
    }

    emit clientDisconnected(client);
}

void ServerManager::setupServer(ushort port)
{
    _server = new QTcpServer(this);
    connect(_server, &QTcpServer::newConnection, this, &ServerManager::newClientConnectionReceived);
    _server->listen(QHostAddress::Any, port);
}
