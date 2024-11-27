#include "ServerManager.h"
#include <QDebug>
#include <QVariantMap>

ServerManager::ServerManager(ushort port, QObject *parent)
    : QObject{parent}
{
    setupServer(port);
    m_databaseManager = new DatabaseManager(this);
}

void ServerManager::notifyOtherClients(QString prevName, QString name)
{
    if (_clientPublicKeys.contains(prevName)) {
        QString publicKey = _clientPublicKeys.take(prevName);
        _clientPublicKeys[name] = publicKey;
    }

    auto message = _protocol.setClientNameMessage(prevName, name);
    foreach (auto cl, _clients) {
        auto clientName = cl->property("clientName").toString();
        if (clientName != name) {
            cl->write(message);
        }
    }

    if (_clients.contains(prevName)) {
        auto client = _clients.take(prevName);
        _clients[name] = client;
    }

    m_databaseManager->updateUserName(prevName, name);
}

void ServerManager::onTextForOtherClients(QByteArray encryptedAESKey, QByteArray encryptedMessage, QString receiver, QString sender)
{
    auto msg = _protocol.textMessage(encryptedAESKey, encryptedMessage, sender);
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

void ServerManager::onNewClient(QString uid, QString email, QString loginStatus, QString publicKey)
{
    if (loginStatus == "REGISTER") {
        m_databaseManager->registerNewClient(uid, email);

        _clientPublicKeys[email] = publicKey;

        for (auto it = _temporaryClients.begin(); it != _temporaryClients.end(); ++it) {
            QTcpSocket *client = *it;

            client->setProperty("clientName", email);
            client->setProperty("uid", uid);
            client->setProperty("email", email);

            _clientPublicKeys[email] = publicKey;

            if (_clients.count() > 0) {
                QMap<QString, QString> publicKeys;
                foreach (auto key, _clientPublicKeys.keys()) {
                    QString publicKey = _clientPublicKeys.value(key);
                    if (key != email) {
                        publicKeys[key] = publicKey;
                    }
                }

                auto message = _protocol.setConnectionACKMessage(email, _clients.keys(), publicKeys);

                client->write(message);

                auto newClientMessage = _protocol.setNewClientMessage(email, publicKey);
                foreach (auto cl, _clients) {
                    cl->write(newClientMessage);
                }
            }
            _clients[email] = client;
            _temporaryClients.erase(it);
            break;
        }
    } else if (loginStatus == "LOGIN") {
        m_databaseManager->getClientData(uid, [this, uid, publicKey](QVariantMap clientData) {
            QString clientName = clientData.value("name").toString();
            QString email = clientData.value("email").toString();

            for (auto it = _temporaryClients.begin(); it != _temporaryClients.end(); ++it) {
                QTcpSocket *client = *it;

                client->setProperty("clientName", clientName);
                client->setProperty("uid", uid);
                client->setProperty("email", email);

                _clientPublicKeys[clientName] = publicKey;

                if (_clients.count() > 0) {
                    QMap<QString, QString> publicKeys;
                    foreach (auto key, _clientPublicKeys.keys()) {
                        QString publicKey = _clientPublicKeys.value(key);
                        if (key != clientName) {
                            publicKeys[key] = publicKey;
                        }
                    }

                    auto message = _protocol.setConnectionACKMessage(clientName, _clients.keys(), publicKeys);

                    client->write(message);

                    auto newClientMessage = _protocol.setNewClientMessage(clientName, publicKey);
                    foreach (auto cl, _clients) {
                        cl->write(newClientMessage);
                    }
                }
                _clients[clientName] = client;
                _temporaryClients.erase(it);
                break;
            }
        });
    }
}

void ServerManager::onSendPublicKey(QString publicKey, QString sender)
{
    _clientPublicKeys[sender] = publicKey;
    auto sendPublicKeyMessage = _protocol.setPublicKeyMessage(publicKey, sender);
    foreach (auto cl, _clients) {
        auto clientName = cl->property("clientName").toString();
        if (clientName != sender) {
            cl->write(sendPublicKeyMessage);
        }
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
    _temporaryClients.append(client);

    // auto id = _clients.count() + 1;
    // auto clientName = QString("Client (%1)").arg(id);
    // client->setProperty("id", id);
    // client->setProperty("clientName", clientName);

    connect(client, &QTcpSocket::disconnected, this, &ServerManager::onClientDisconnected);
    emit newClientConnected(client);

    // if (id > 1) {
    //     QMap<QString, QString> publicKeys;
    //     foreach (auto key, _clientPublicKeys.keys()) {
    //         QString publicKey = _clientPublicKeys.value(key);
    //         if(key != clientName) {
    //             publicKeys[key] = publicKey;
    //         }
    //     }

    //     auto message = _protocol.setConnectionACKMessage(clientName, _clients.keys(), publicKeys);
    //     client->write(message);

    //     auto newClientMessage = _protocol.setNewClientMessage(clientName);
    //     foreach (auto cl, _clients) {
    //         cl->write(newClientMessage);
    //     }

    // }
    // _clients[clientName] = client;
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
