#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QVariantMap>

class DatabaseManager : public QObject
{
    Q_OBJECT
public:
    explicit DatabaseManager(QObject *parent = nullptr);

    void updateUserName(const QString &prevName, const QString &name);
    void registerNewClient(const QString &uid, const QString &email);
    void getClientData(const QString &uid, std::function<void(QVariantMap)> callback);

private:
    QNetworkAccessManager *m_networkManager;
};

#endif // DATABASEMANAGER_H
