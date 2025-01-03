#ifndef CLIENTCHATWIDGET_H
#define CLIENTCHATWIDGET_H

#include "ClientManager.h"

#include <QDir>
#include <QTcpSocket>
#include <QWidget>

namespace Ui {
class ClientChatWidget;
}

class ClientChatWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ClientChatWidget(QTcpSocket *cleint, QWidget *parent = nullptr);
    void disconnect();
    ~ClientChatWidget();

private slots:

    void clientDisconnected();

    void on_btnSend_clicked();
    void onTyping(QString receiver);

    void onInitReceivingFile(QString clientName, QString fileName, qint64 fileSize);
    void onFileSaved(QString path);
    void on_lblOpenFolder_linkActivated(const QString &link);

signals:
    void clientNameChanged(QString prevName, QString name);
    void isTyping(QString message, QString sender, QString receiver);
    void textForOtherClients(QByteArray encryptedAESKey, const QByteArray encryptedMessage, QString receiver, QString sender);
    void sendFile(QString receiver, QString fileName, qint64 fileSize, QByteArray fileData, QString sender);
    void statusChanged(ChatProtocol::Status status, QString sender);
    void newClient(QString uid, QString email, QString loginStatus, QString publicKey);
    void sendPublicKey(QString publicKey, QString name);
    void groupChat(QString groupName, QStringList memberList, QString sender);
    void textGroupChat(QString groupName, QString message, QString sender);

private:
    Ui::ClientChatWidget *ui;
    ClientManager *_client;
};

#endif // CLIENTCHATWIDGET_H
