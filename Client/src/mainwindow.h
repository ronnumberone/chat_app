#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include "ClientManager.h"
#include "StartWidget.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_actionConnect_triggered();
    void sendMessage(QString publicKey, QString message, QString receiver);
    void dataReceived(QString message, QString sender);
    void on_lnClientName_editingFinished();
    void on_cmbStatus_currentIndexChanged(int index);
    void onTyping(QString sender);
    void onUserSignedIn(QString uid, QString email, bool firstLogin);
    //void on_btnSendFile_clicked();
    void onReceiveFile(QString sender, QString fileName, qint64 fileSize, QByteArray fileData);
    void onConnectionACK(QString myName, QStringList clientsName);
    void onNewClientConnectedToServer(QString clientName);
    void onClientNameChanged(QString prevName, QString clientName);
    void onStatusChanged(ChatProtocol::Status status, QString sender);
    void onSendPublicKey(QString publicKey, QString sender);
    //void onClientDisconnected(QString clientName);

private:
    Ui::MainWindow *ui;
    ClientManager *_client;
    QStackedWidget *stackedWidget;
    QNetworkAccessManager *networkManager;
    StartWidget *startWidget;
    QString _myClientName;
    void setupClient();
};
#endif // MAINWINDOW_H
