#include "ChatItemWidget.h"
#include "ClientChatWidget.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <iostream>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setupClient();
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
}

void MainWindow::on_actionConnect_triggered()
{
    _client->connectToServer();
}

void MainWindow::sendMessage(QString message, QString receiver)
{
    _client->sendMessage(message, receiver);
}

void MainWindow::dataReceived(QString message, QString sender)
{
    for (int i = 0; i < ui->tbClients->count(); ++i) {
        QString tabReceiver = ui->tbClients->tabText(i);
        if (tabReceiver == sender) {
            ClientChatWidget* chatWidget = qobject_cast<ClientChatWidget*>(ui->tbClients->widget(i));
            if (chatWidget) {
                chatWidget->dataReceived(message, sender);
            }
            break;
        }
    }
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
