#include "ChatItemWidget.h"
#include "ClientChatWidget.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "GroupChatDialog.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setupClient();

    stackedWidget = new QStackedWidget(this);

    networkManager = new QNetworkAccessManager(this);

    startWidget = new StartWidget(_client, this);
    connect(startWidget, &StartWidget::userSignedIn, this, &MainWindow::onUserSignedIn);

    stackedWidget->addWidget(startWidget);
    stackedWidget->addWidget(ui->centralwidget);
    setCentralWidget(stackedWidget);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupClient()
{
    _client = new ClientManager();
    connect(_client , &ClientManager::connected, [this](){
        ui->centralwidget->setEnabled(true);
    });
    connect(_client, &ClientManager::disconnected, [this](){
        ui->centralwidget->setEnabled(false);
    });

    connect(_client, &ClientManager::textMessageReceived, this, &MainWindow::dataReceived);
    connect(_client, &ClientManager::isTyping, this, &MainWindow::onTyping);
    connect(_client, &ClientManager::receiveFile, this, &MainWindow::onReceiveFile);
    connect(_client, &ClientManager::statusChanged, this, &MainWindow::onStatusChanged);

    connect(_client, &ClientManager::connectionACK, this, &MainWindow::onConnectionACK);
    connect(_client, &ClientManager::newClientConnectedToServer, this, &MainWindow::onNewClientConnectedToServer);
    //onnect(_client, &ClientManager::clientDisconnected, this, &MainWindow::onClientDisconnected);
    connect(_client, &ClientManager::clientNameChanged, this, &MainWindow::onClientNameChanged);
    connect(_client, &ClientManager::sendPublicKey, this, &MainWindow::onSendPublicKey);
}

void MainWindow::on_actionConnect_triggered()
{
    _client->connectToServer();
}

void MainWindow::on_actionAdd_group_chat_triggered()
{
    QStringList members;
    for (int i = 0; i < ui->tbClients->count(); ++i) {
        QString memberName = ui->tbClients->tabText(i);
        members.append(memberName);
    }

    GroupChatDialog dialog(this, members);
    if (dialog.exec() == QDialog::Accepted) {
        QString groupName = dialog.getGroupName();
        QStringList selectedMembers = dialog.getSelectedMembers();
    }
}


void MainWindow::sendMessage(QString publicKey, QString message, QString receiver)
{
    _client->sendMessage(publicKey, message, receiver);
}

void MainWindow::dataReceived(QString message, QString sender)
{
    ClientChatWidget* chatWidget;
    for (int i = 0; i < ui->tbClients->count(); ++i) {
        QString tabReceiver = ui->tbClients->tabText(i);
        if (tabReceiver == sender) {
            chatWidget = qobject_cast<ClientChatWidget*>(ui->tbClients->widget(i));
            if (chatWidget) {
                chatWidget->dataReceived(message, sender);
            }
            break;
        }
    }

    QUrl url("http://localhost:5005/webhooks/rest/webhook");

    QJsonObject json;
    json["sender"] = sender;  // REST API sử dụng "sender" thay vì "recipient_id"
    json["message"] = message;

    QJsonDocument doc(json);
    QByteArray data = doc.toJson();

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply *reply = networkManager->post(request, data);

    connect(reply, &QNetworkReply::finished, this, [reply, chatWidget]() {
        if (reply->error() != QNetworkReply::NoError) {
            qDebug() << "Error:" << reply->errorString();
            reply->deleteLater();
            return;
        }

        QByteArray response = reply->readAll();

        QJsonDocument responseDoc = QJsonDocument::fromJson(response);
        if (responseDoc.isArray()) {
            QJsonArray responseArray = responseDoc.array();
            for (const QJsonValue &value : responseArray) {
                if (value.isObject()) {
                    QJsonObject obj = value.toObject();
                    if (obj.contains("text")) {
                        QString botMessage = obj["text"].toString();
                        if(!botMessage.isEmpty()) chatWidget->suggestionReceived(botMessage);
                    }
                }
            }
        }

        reply->deleteLater();
    });
}

void MainWindow::on_lnClientName_editingFinished()
{
    auto name = ui->lnClientName->text().trimmed();
    _client->sendName(name);
}


void MainWindow::on_cmbStatus_currentIndexChanged(int index)
{
    auto status = (ChatProtocol::Status)index;
    _client->sendStatus(status);
}

void MainWindow::onTyping(QString sender)
{
    for (int i = 0; i < ui->tbClients->count(); ++i) {
        if (ui->tbClients->tabText(i) == sender) {
            auto* chatWidget = qobject_cast<ClientChatWidget*>(ui->tbClients->widget(i));
            if (chatWidget) {
                chatWidget->onTyping(sender);
            }
            break;
        }
    }
}

void MainWindow::onUserSignedIn(QString uid, QString email, bool firstLogin)
{
    connect(_client, &ClientManager::setPublicKey, [this, uid, email, firstLogin](QString publicKey) {
        _client->sendNewClient(uid, email, firstLogin ? "REGISTER" : "LOGIN", publicKey);
    });

    // connect(_client->socket(), &QTcpSocket::connected, this, [this, uid, email, firstLogin, sentPublicKey]() {
    //     _client->sendNewClient(uid, email, firstLogin ? "REGISTER" : "LOGIN", sentPublicKey);
    //     qDebug() << "Sending" << (firstLogin ? "REGISTER:" : "LOGIN:") << uid << email;
    // });

    // Kết nối đến server
    _client->connectToServer();
    stackedWidget->setCurrentWidget(ui->centralwidget);
}

void MainWindow::onReceiveFile(QString sender, QString fileName, qint64 fileSize, QByteArray fileData)
{
    for (int i = 0; i < ui->tbClients->count(); ++i) {
        QString tabReceiver = ui->tbClients->tabText(i);
        if (tabReceiver == sender) {
            ClientChatWidget* chatWidget = qobject_cast<ClientChatWidget*>(ui->tbClients->widget(i));
            if (chatWidget) {
                chatWidget->fileReceived(sender, fileName, fileSize, fileData);
            }
            break;
        }
    }
}

void MainWindow::onConnectionACK(QString myName, QStringList clientsName)
{
    for (const QString& clientName : clientsName) {
        if (myName != clientName) {
            auto chatWidget = new ClientChatWidget(clientName, ui->tbClients);
            connect(chatWidget, &ClientChatWidget::sendMessage, this, &MainWindow::sendMessage);
            connect(chatWidget, &ClientChatWidget::isTyping, _client, &ClientManager::sendIsTyping);
            connect(chatWidget, &ClientChatWidget::sendFile, _client, &ClientManager::sendFile);
            ui->tbClients->addTab(chatWidget, clientName);
        }
    }
}

void MainWindow::onNewClientConnectedToServer(QString clientName)
{
    if (clientName != _myClientName) {
        bool tabExists = false;
        for (int i = 0; i < ui->tbClients->count(); ++i) {
            if (ui->tbClients->tabText(i) == clientName) {
                tabExists = true;
                break;
            }
        }

        if (!tabExists) {
            auto chatWidget = new ClientChatWidget(clientName, ui->tbClients);
            connect(chatWidget, &ClientChatWidget::sendMessage, this, &MainWindow::sendMessage);
            connect(chatWidget, &ClientChatWidget::isTyping, _client, &ClientManager::sendIsTyping);
            connect(chatWidget, &ClientChatWidget::sendFile, _client, &ClientManager::sendFile);
            ui->tbClients->addTab(chatWidget, clientName);
        }
    }
}


void MainWindow::onClientNameChanged(QString prevName, QString clientName)
{
    for (int i = 0; i < ui->tbClients->count(); ++i) {
        if (ui->tbClients->tabText(i) == prevName) {
            ui->tbClients->setTabText(i, clientName);
            auto* chatWidget = qobject_cast<ClientChatWidget*>(ui->tbClients->widget(i));
            if (chatWidget) {
                chatWidget->setClientName(clientName);
            }
            break;
        }
    }
}

void MainWindow::onStatusChanged(ChatProtocol::Status status, QString sender)
{
    for (int i = 0; i < ui->tbClients->count(); ++i) {
        if (ui->tbClients->tabText(i) == sender) {
            auto* chatWidget = qobject_cast<ClientChatWidget*>(ui->tbClients->widget(i));
            auto index = ui->tbClients->indexOf(chatWidget);
            QString iconName = ":/icons/";
            switch (status) {
            case ChatProtocol::Available:
                iconName.append("available.png");
                break;
            case ChatProtocol::Away:
                iconName.append("away.png");
                break;
            case ChatProtocol::Busy:
                iconName.append("busy.png");
                break;
            default:
                iconName = "";
                break;
            }

            auto icon = QIcon(iconName);
            ui->tbClients->setTabIcon(index, icon);
            break;
        }
    }
}

void MainWindow::onSendPublicKey(QString publicKey, QString sender)
{
    for (int i = 0; i < ui->tbClients->count(); ++i) {
        if (ui->tbClients->tabText(i) == sender) {
            auto* chatWidget = qobject_cast<ClientChatWidget*>(ui->tbClients->widget(i));
            if (chatWidget) {
                chatWidget->setPublicKey(publicKey);
            }
            break;
        }
    }
}



