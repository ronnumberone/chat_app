#ifndef STARTWIDGET_H
#define STARTWIDGET_H

#include "ClientManager.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QWidget>

namespace Ui {
class StartWidget;
}

class StartWidget : public QWidget
{
    Q_OBJECT

public:
    explicit StartWidget(ClientManager *client, QWidget *parent = nullptr);
    ~StartWidget();
    void signUserUp( const QString & emailAddress, const QString & password );
    void signUserIn( const QString & emailAddress, const QString & password );

public slots:
    void networkReplyReadyRead();

signals:
    void userSignedIn(QString uid, QString email, bool firstLogin);

private slots:
    void on_loginBtn_clicked();

    void on_actionChangeBtn_clicked();

    void on_emailEdt_textChanged(const QString &arg1);

    void on_passEdt_textChanged(const QString &arg1);

    void on_confirmPassEdt_textChanged(const QString &arg1);

private:
    Ui::StartWidget *ui;
    void performPOST( const QString & url, const QJsonDocument & payload );
    void parseResponse( const QByteArray & reponse );
    QString m_apiKey = "AIzaSyClD7QbSKHs-Wk2Xbr4kpUXTa-287bXBOY";
    ClientManager *_client;
    QNetworkAccessManager * m_networkAccessManager;
    QNetworkReply * m_networkReply;
    QString m_idToken;
    QString status = "login";
    bool isValidEmail(const QString &email);
};

#endif // STARTWIDGET_H
