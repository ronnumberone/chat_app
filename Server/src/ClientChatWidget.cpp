#include "ClientChatWidget.h"
#include "ui_ClientChatWidget.h"
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>

ClientChatWidget::ClientChatWidget(QTcpSocket *client, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ClientChatWidget)
{
    ui->setupUi(this);
    _client = new ClientManager(client, this);
    //    connect(_client, &QTcpSocket::readyRead, this, &ClientChatWidget::dataReceived);
    connect(_client, &ClientManager::disconnected, this, &ClientChatWidget::clientDisconnected);

    connect(_client, &ClientManager::textMessageReceived, [this](QByteArray encryptedAESKey, QByteArray encryptedMessage, QString receiver){
        emit textForOtherClients(encryptedAESKey, encryptedMessage, receiver, _client->name());
    });
    connect(_client, &ClientManager::isTyping, this, &ClientChatWidget::onTyping);

    connect(_client, &ClientManager::nameChanged, [this](QString prevName, QString name){
        emit clientNameChanged(prevName, name);
    });

    connect(_client, &ClientManager::initReceivingFile, this, &ClientChatWidget::onInitReceivingFile);

    connect(_client, &ClientManager::statusChanged, [this](ChatProtocol::Status status){
        emit statusChanged(status, _client->name());
    });

    connect(_client, &ClientManager::newClient, this , [this](QString uid, QString email, QString loginStatus, QString publicKey){
        emit newClient(uid, email, loginStatus, publicKey);
    });
    //connect(_client, &ClientManager::fileSaved, this, &ClientChatWidget::onFileSaved);
    //connect(ui->lnMessage, &QLineEdit::textChanged, _client, &ClientManager::sendIsTyping);

    connect(_client, &ClientManager::fileSend, [this](QString receiver, QString fileName, qint64 fileSize, QByteArray fileData){
        emit sendFile(receiver, fileName, fileSize, fileData, _client->name());
    });

    connect(_client, &ClientManager::groupChat, [this](QString groupName, QStringList memberList){
        emit groupChat(groupName, memberList, _client->name());
    });

    connect(_client, &ClientManager::textGroupChat, [this](QString groupName, QString message){
        emit textGroupChat(groupName, message, _client->name());
    });

    //dir.mkdir(_client->name());
    //dir.setPath("./" + _client->name());
}

void ClientChatWidget::disconnect()
{
    _client->disconnectFromHost();
}

ClientChatWidget::~ClientChatWidget()
{
    delete ui;
}

void ClientChatWidget::clientDisconnected()
{
    ui->wdgSendMessage->setEnabled(false);
}

void ClientChatWidget::on_btnSend_clicked()
{
    auto message = ui->lnMessage->text().trimmed();
    //_client->sendMessage(message);
    ui->lnMessage->setText("");
    ui->lstMessages->addItem(message);
}

void ClientChatWidget::onTyping(QString receiver)
{
    emit isTyping(QString("%1 is typing...").arg(_client->name()), _client->name(), receiver);
}

void ClientChatWidget::onInitReceivingFile(QString clientName, QString fileName, qint64 fileSize)
{
    auto message = QString("Client (%1) wants to send a file. Do you want to accept it?\nFile Name:%2\nFile Size: %3 bytes")
    .arg(clientName, fileName)
        .arg(fileSize);
    auto result = QMessageBox::question(this, "Receiving File", message);
    if (result == QMessageBox::Yes) {
        _client->sendAcceptFile();
    } else {
        _client->sendRejectFile();
    }
}

void ClientChatWidget::onFileSaved(QString path)
{
    auto message = QString("File saved here:\n%1").arg(path);
    QMessageBox::information(this, "File saved", message);
}

void ClientChatWidget::on_lblOpenFolder_linkActivated(const QString &link)
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(_client->name()));
}


