#include "ChatItemWidget.h"
#include "ClientChatWidget.h"
#include "ui_ClientChatWidget.h"
#include "FileItemWidget.h"
#include "RecordItemWidget.h"
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
        emit sendMessage(getPublicKey(), ui->lnMessage->text().trimmed(), clientName);
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
    auto listWidgetItem = new QListWidgetItem();
    ui->lstMessages->addItem(listWidgetItem);
    listWidgetItem->setBackground(QColor(173, 215, 227));

    if(fileName.contains(".wav"))
    {
        auto recordWidget = new RecordItemWidget(this);
        recordWidget->setRecordInfo(sender, fileData);
        listWidgetItem->setSizeHint(QSize(0, 70));
        ui->lstMessages->setItemWidget(listWidgetItem, recordWidget);
    }
    else
    {
        auto fileWidget = new FileItemWidget(this);
        fileWidget->setFileInfo(sender, fileName, fileSize, fileData);
        listWidgetItem->setSizeHint(QSize(0, 90));
        ui->lstMessages->setItemWidget(listWidgetItem, fileWidget);
    }
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
        listWidgetItem->setSizeHint(QSize(0, 90));
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
        emit sendMessage(getPublicKey(), ui->lnMessage->text().trimmed(), clientName);
        auto chatWidget = new ChatItemWidget(this);
        chatWidget->setMessage(ui->lnMessage->text().trimmed(), "", true);
        ui->lnMessage->setText("");
        ui->lnMessage->setFocus();

        auto listWidgetItem = new QListWidgetItem();
        listWidgetItem->setSizeHint(QSize(0, 60));
        ui->lstMessages->addItem(listWidgetItem);
        ui->lstMessages->setItemWidget(listWidgetItem, chatWidget);
    }
}

void ClientChatWidget::on_recordBtn_clicked()
{
    if (!isRecording) {
        if (!_audioRecorder) {
            _audioRecorder = new QAudioRecorder(this);
            _audioProbe = new QAudioProbe(this);
            setupAudioProbe(_audioRecorder, _audioProbe);
        }
        _audioRecorder->record();
        ui->recordBtn->setIcon(QIcon(":/icons/recording.png"));
        isRecording = true;
    }
    else
    {
        _audioRecorder->stop();
        ui->recordBtn->setIcon(QIcon(":/icons/record.png"));
        isRecording = false;
        QString fileName = QCoreApplication::applicationDirPath() + "/record.wav";

        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly)) {
            QByteArray fileData = file.readAll();
            file.close();

            auto listWidgetItem = new QListWidgetItem();
            ui->lstMessages->addItem(listWidgetItem);

            auto recordWidget = new RecordItemWidget(this);
            recordWidget->setRecordInfo("", fileData, true);

            listWidgetItem->setSizeHint(QSize(0, 70));
            ui->lstMessages->setItemWidget(listWidgetItem, recordWidget);
        }

        emit sendFile(clientName, fileName);
    }
}

QString ClientChatWidget::getPublicKey() const
{
    return publicKey;
}

void ClientChatWidget::setPublicKey(const QString &newPublicKey)
{
    publicKey = newPublicKey;
}

void ClientChatWidget::setupAudioProbe(QAudioRecorder *&audioRecorder, QAudioProbe *&audioProbe)
{
    audioProbe->setSource(audioRecorder);

    QAudioEncoderSettings settings;
    settings.setCodec("audio/pcm");
    settings.setSampleRate(0);
    settings.setBitRate(0);
    settings.setChannelCount(-1);
    settings.setQuality(QMultimedia::EncodingQuality::VeryLowQuality);
    settings.setEncodingMode(QMultimedia::ConstantQualityEncoding);
    QString container = "audio/x-wav";

    audioRecorder->setOutputLocation(QUrl::fromLocalFile(QCoreApplication::applicationDirPath() + "/record.wav"));
    audioRecorder->setEncodingSettings(settings, QVideoEncoderSettings(), container);
}


QString ClientChatWidget::getClientName() const
{
    return clientName;
}

void ClientChatWidget::on_lnMessage_textChanged()
{
    emit isTyping(clientName);
}

void ClientChatWidget::setClientName(const QString &newClientName)
{
    clientName = newClientName;
}

