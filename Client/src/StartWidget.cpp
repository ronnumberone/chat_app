#include "StartWidget.h"
#include "ui_StartWidget.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>

StartWidget::StartWidget(ClientManager *client, QWidget *parent)
    : QWidget(parent), _client(client)
    , ui(new Ui::StartWidget)
{
    ui->setupUi(this);

    m_networkAccessManager = new QNetworkAccessManager( this );
    ui->confirmPassEdt->hide();
    ui->errorLb->hide();
}

StartWidget::~StartWidget()
{
    m_networkAccessManager->deleteLater();
    delete ui;
}

void StartWidget::signUserUp(const QString &emailAddress, const QString &password)
{
    QString signUpEndpoint = "https://identitytoolkit.googleapis.com/v1/accounts:signUp?key=" + m_apiKey;

    QVariantMap variantPayload;
    variantPayload["email"] = emailAddress;
    variantPayload["password"] = password;
    variantPayload["returnSecureToken"] = true;

    QJsonDocument jsonPayload = QJsonDocument::fromVariant( variantPayload );
    performPOST( signUpEndpoint, jsonPayload );
}

void StartWidget::signUserIn(const QString &emailAddress, const QString &password)
{
    QString signInEndpoint = "https://identitytoolkit.googleapis.com/v1/accounts:signInWithPassword?key=" + m_apiKey;

    QVariantMap variantPayload;
    variantPayload["email"] = emailAddress;
    variantPayload["password"] = password;
    variantPayload["returnSecureToken"] = true;

    QJsonDocument jsonPayload = QJsonDocument::fromVariant( variantPayload );

    performPOST( signInEndpoint, jsonPayload );
}

void StartWidget::networkReplyReadyRead()
{
    QByteArray response = m_networkReply->readAll();
    //qDebug() << response;
    m_networkReply->deleteLater();

    parseResponse( response );
}

void StartWidget::performPOST(const QString &url, const QJsonDocument &payload)
{
    QNetworkRequest newRequest( (QUrl( url )) );
    newRequest.setHeader( QNetworkRequest::ContentTypeHeader, QString( "application/json"));
    m_networkReply = m_networkAccessManager->post( newRequest, payload.toJson());
    connect( m_networkReply, &QNetworkReply::readyRead, this, &StartWidget::networkReplyReadyRead );
}

void StartWidget::parseResponse(const QByteArray &response)
{
    QJsonDocument jsonDocument = QJsonDocument::fromJson( response );
    if ( jsonDocument.object().contains("error") )
    {
        QJsonObject errorObject = jsonDocument.object().value("error").toObject();
        QString errorMessage = errorObject.value("message").toString();
        if(errorMessage == "INVALID_LOGIN_CREDENTIALS") {
            ui->errorLb->setText("Wrong email or password");
            ui->errorLb->show();
        }
        else if(errorMessage == "EMAIL_EXISTS")
        {
            ui->errorLb->setText("Email exists");
            ui->errorLb->show();
        }
        qDebug() << "Error occurred!" << errorMessage;
    }
    else if ( jsonDocument.object().contains("kind"))
    {
        QString kind = jsonDocument.object().value("kind").toString();
        QString uid = jsonDocument.object().value("localId").toString();
        QString email = jsonDocument.object().value("email").toString();
        bool firstLogin = false;
        if (kind == "identitytoolkit#SignupNewUserResponse") {
            qDebug() << "First time registration!";
            firstLogin = true;
        } else if (kind == "identitytoolkit#VerifyPasswordResponse") {
            qDebug() << "User signed in successfully!";
            QString idToken = jsonDocument.object().value("idToken").toString();
            m_idToken = idToken;
            firstLogin = false;
        }
        emit userSignedIn(uid, email, firstLogin);
    }
    else
        qDebug() << "The response was: " << response;
}

void StartWidget::on_loginBtn_clicked()
{
    if(status == "login") {
        if(!isValidEmail(ui->emailEdt->text())) {
            ui->errorLb->show();
            ui->errorLb->setText("Invalid email");
        }
        else if(ui->passEdt->text().trimmed().length() < 6) {
            ui->errorLb->show();
            ui->errorLb->setText("Password must have at least 6 characters");
        }
        else
        {
            signUserIn(ui->emailEdt->text(), ui->passEdt->text());
        }
    }
    else
    {
        if(!isValidEmail(ui->emailEdt->text())) {
            ui->errorLb->show();
            ui->errorLb->setText("Invalid email");
        }
        else if(ui->passEdt->text().trimmed().length() < 6) {
            ui->errorLb->show();
            ui->errorLb->setText("Password must have at least 6 characters");
        }
        else if(ui->passEdt->text().trimmed() != ui->confirmPassEdt->text().trimmed()) {
            ui->errorLb->show();
            ui->errorLb->setText("Password does not match");
        }
        else
        {
            signUserUp(ui->emailEdt->text(), ui->passEdt->text());
        }
    }
}

void StartWidget::on_actionChangeBtn_clicked()
{
    if(status == "login") {
        ui->actionNameLb->setText("Register");
        ui->actionChangeLb->setText("Already have an account? ");
        ui->actionChangeBtn->setText("Login");
        ui->loginBtn->setText("Register");
        ui->confirmPassEdt->show();
        status = "register";
    }
    else
    {
        ui->actionNameLb->setText("Login");
        ui->actionChangeLb->setText("Don't have an account? ");
        ui->actionChangeBtn->setText("Register");
        ui->loginBtn->setText("Login");
        ui->confirmPassEdt->hide();
        status = "login";
    }
}

bool StartWidget::isValidEmail(const QString &email) {
    QRegularExpression regex(R"((\w+)(\.\w+)*@(\w+)(\.\w+)+)");
    return regex.match(email).hasMatch();
}


void StartWidget::on_emailEdt_textChanged(const QString &arg1)
{
    ui->errorLb->hide();
}


void StartWidget::on_passEdt_textChanged(const QString &arg1)
{
    ui->errorLb->hide();
}


void StartWidget::on_confirmPassEdt_textChanged(const QString &arg1)
{
    ui->errorLb->hide();
}

