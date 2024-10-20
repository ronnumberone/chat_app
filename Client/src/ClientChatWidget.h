#ifndef CLIENTCHATWIDGET_H
#define CLIENTCHATWIDGET_H

#include "ClientManager.h"

#include <QTcpSocket>
#include <QWidget>
#include <QStatusBar>

namespace Ui {
class ClientChatWidget;
}

class ClientChatWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ClientChatWidget(QString clientName, QWidget *parent = nullptr);
    ~ClientChatWidget();

    QString getClientName() const;
    void dataReceived(QString message, QString sender);
    void onTyping(QString sender);
    void fileReceived(QString sender, QString fileName, qint64 fileSize, QByteArray fileData);
    void setClientName(const QString &newClientName);

signals:
    void sendMessage(QString message, QString receiver);
    void isTyping(QString receiver);
    void sendFile(QString receiver, QString fileName);

private slots:
    void on_btnSend_clicked();

    void on_lnMessage_textChanged(const QString &arg1);

    void on_lnMessage_returnPressed();

    void on_pushButton_clicked();

private:
    Ui::ClientChatWidget *ui;
    QStatusBar *statusBar;
    ClientManager *_client;
    QString clientName;
};

#endif // CLIENTCHATWIDGET_H
