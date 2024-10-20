#include "ChatItemWidget.h"
#include "ClientChatWidget.h"
#include "ui_ClientChatWidget.h"
#include "FileItemWidget.h"
#include <QDebug>
#include <QFileDialog>

ClientChatWidget::ClientChatWidget(QString clientName, QWidget *parent)
    :clientName(clientName), QWidget(parent)
    , ui(new Ui::ClientChatWidget)
{
    ui->setupUi(this);
    statusBar = new QStatusBar(this);
    ui->verticalLayout->addWidget(statusBar);
}

ClientChatWidget::~ClientChatWidget()
{
    delete ui;
}

void ClientChatWidget::on_btnSend_clicked()
{
    if(ui->lnMessage->text().trimmed() != ""){
        emit sendMessage(ui->lnMessage->text().trimmed(), clientName);
        auto chatWidget = new ChatItemWidget();
        chatWidget->setMessage(ui->lnMessage->text().trimmed(), "", true);
        ui->lnMessage->setText("");
        ui->lnMessage->setFocus();

        auto listWidgetItem = new QListWidgetItem();
        listWidgetItem->setSizeHint(QSize(0, 60));
        ui->lstMessages->addItem(listWidgetItem);
        ui->lstMessages->setItemWidget(listWidgetItem, chatWidget);
    }
}

void ClientChatWidget::dataReceived(QString message, QString sender)
{
    auto chatWidget = new ChatItemWidget();
    chatWidget->setMessage(message, sender);
    auto listWidgetItem = new QListWidgetItem();
    listWidgetItem->setSizeHint(QSize(0, 60));
    ui->lstMessages->addItem(listWidgetItem);
    listWidgetItem->setBackground(QColor(173, 215, 227));
    ui->lstMessages->setItemWidget(listWidgetItem, chatWidget);
}

void ClientChatWidget::onTyping(QString sender)
{
    statusBar->showMessage(sender + " is typing...", 750);
}

void ClientChatWidget::fileReceived(QString sender, QString fileName, qint64 fileSize, QByteArray fileData)
{
    auto fileWidget = new FileItemWidget();
    fileWidget->setFileInfo(sender, fileName, fileSize, fileData);
    auto listWidgetItem = new QListWidgetItem();
    listWidgetItem->setSizeHint(QSize(0, 80));
    ui->lstMessages->addItem(listWidgetItem);
    listWidgetItem->setBackground(QColor(173, 215, 227));
    ui->lstMessages->setItemWidget(listWidgetItem, fileWidget);
    ui->lstMessages->update();
}

void ClientChatWidget::on_pushButton_clicked()
{
    auto fileName = QFileDialog::getOpenFileName(this, "Select a file", "/home");
    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly)) {
        QFileInfo info(fileName);
        auto fileWidget = new FileItemWidget();
        fileWidget->setFileInfo("", info.fileName(), info.size(), file.readAll(), true);
        auto listWidgetItem = new QListWidgetItem();
        listWidgetItem->setSizeHint(QSize(0, 80));
        ui->lstMessages->addItem(listWidgetItem);
        ui->lstMessages->setItemWidget(listWidgetItem, fileWidget);
        ui->lstMessages->update();
        file.close();
    }
    emit sendFile(clientName, fileName);
}

void ClientChatWidget::on_lnMessage_returnPressed()
{
    if(ui->lnMessage->text().trimmed() != ""){
        emit sendMessage(ui->lnMessage->text().trimmed(), clientName);
        auto chatWidget = new ChatItemWidget();
        chatWidget->setMessage(ui->lnMessage->text().trimmed(), "", true);
        ui->lnMessage->setText("");
        ui->lnMessage->setFocus();

        auto listWidgetItem = new QListWidgetItem();
        listWidgetItem->setSizeHint(QSize(0, 60));
        ui->lstMessages->addItem(listWidgetItem);
        ui->lstMessages->setItemWidget(listWidgetItem, chatWidget);
    }
}

QString ClientChatWidget::getClientName() const
{
    return clientName;
}


void ClientChatWidget::on_lnMessage_textChanged(const QString &arg1)
{
    emit isTyping(clientName);
}

void ClientChatWidget::setClientName(const QString &newClientName)
{
    clientName = newClientName;
}




