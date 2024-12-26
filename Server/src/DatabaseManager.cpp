#include "DatabaseManager.h"
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QDebug>

DatabaseManager::DatabaseManager(QObject *parent)
    : QObject(parent)
{
    m_networkManager = new QNetworkAccessManager(this);
}

void DatabaseManager::updateUserName(const QString &prevName, const QString &name)
{
    QNetworkRequest request(QUrl("https://qt-chat-app-default-rtdb.asia-southeast1.firebasedatabase.app/users.json"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply *reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply, prevName, name]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray responseData = reply->readAll();
            QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
            QJsonObject jsonObject = jsonDoc.object();

            QString targetUid;
            QJsonObject targetUser;
            for (auto it = jsonObject.begin(); it != jsonObject.end(); ++it) {
                QJsonObject userObj = it.value().toObject();
                QString currentName = userObj.value("name").toString();
                if (currentName == prevName) {
                    targetUid = userObj.value("uid").toString();
                    targetUser = userObj;
                    break;
                }
            }

            if (!targetUid.isEmpty()) {
                targetUser["name"] = name;

                QNetworkRequest updateRequest(QUrl("https://qt-chat-app-default-rtdb.asia-southeast1.firebasedatabase.app/users/" + targetUid + ".json"));
                updateRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

                QJsonDocument updateJsonDoc(targetUser);
                m_networkManager->put(updateRequest, updateJsonDoc.toJson());
            }
        } else {
            qDebug() << "Lỗi khi tìm kiếm người dùng cũ trong Firebase: " << reply->errorString();
        }

        reply->deleteLater();
    });
}

void DatabaseManager::registerNewClient(const QString &uid, const QString &email)
{
    qint64 currentTime = QDateTime::currentDateTime().toMSecsSinceEpoch();

    QVariantMap newClient;
    newClient["email"] = email;
    newClient["name"] = email;
    newClient["uid"] = uid;
    newClient["createdAt"] = currentTime;
    newClient["updatedAt"] = currentTime;

    QJsonDocument jsonDoc = QJsonDocument::fromVariant(newClient);

    QNetworkRequest newClientRequest(QUrl("https://qt-chat-app-default-rtdb.asia-southeast1.firebasedatabase.app/users/" + uid + ".json"));
    newClientRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    m_networkManager->put(newClientRequest, jsonDoc.toJson());
}

void DatabaseManager::getClientData(const QString &uid, std::function<void(QVariantMap)> callback)
{
    QNetworkRequest loginRequest(QUrl("https://qt-chat-app-default-rtdb.asia-southeast1.firebasedatabase.app/users/" + uid + ".json"));
    QNetworkReply *reply = m_networkManager->get(loginRequest);

    connect(reply, &QNetworkReply::finished, this, [this, reply, callback]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray responseData = reply->readAll();
            QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
            QVariantMap clientData = jsonDoc.toVariant().toMap();

            if (callback) {
                callback(clientData);
            }
        } else {
            qDebug() << "Lỗi khi lấy thông tin từ Firebase:" << reply->errorString();
            if (callback) {
                callback(QVariantMap());
            }
        }

        reply->deleteLater();
    });
}

void DatabaseManager::saveMessageToDatabase(QByteArray encryptedAESKey, QByteArray encryptedMessage, QString receiver, QString sender)
{
    if (encryptedAESKey.isEmpty() || encryptedMessage.isEmpty() || receiver.isEmpty() || sender.isEmpty()) {
        qDebug() << "Dữ liệu đầu vào không hợp lệ. Không thể lưu tin nhắn.";
        return;
    }

    QString uniqueMessageId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    QString chatId = sender + "_" + receiver;
    QString aesKeyBase64 = encryptedAESKey.toBase64();
    QString encryptedMessageBase64 = encryptedMessage.toBase64();

    qint64 currentTime = QDateTime::currentDateTime().toMSecsSinceEpoch();

    QVariantMap messageData;
    messageData["chatId"] = chatId;
    messageData["senderId"] = sender;
    messageData["receiverId"] = receiver;
    messageData["encryptedAESKey"] = aesKeyBase64;
    messageData["encryptedMessage"] = encryptedMessageBase64;
    messageData["createdAt"] = currentTime;
    messageData["updatedAt"] = currentTime;

    QJsonDocument jsonDoc = QJsonDocument::fromVariant(messageData);
    QNetworkRequest request(QUrl("https://qt-chat-app-default-rtdb.asia-southeast1.firebasedatabase.app/messages/" + uniqueMessageId + ".json"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    m_networkManager->put(request, jsonDoc.toJson());
}


void DatabaseManager::saveGroupToDatabase(QString groupName, QStringList memberList, QString adminId)
{
    QString groupId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    // Thời gian hiện tại
    qint64 currentTime = QDateTime::currentDateTime().toMSecsSinceEpoch();

    // Tạo đối tượng JSON để lưu thông tin nhóm
    QVariantMap groupData;
    groupData["groupId"] = groupId;           // ID duy nhất của nhóm
    groupData["groupName"] = groupName;       // Tên nhóm
    groupData["participants"] = memberList;   // Danh sách thành viên
    groupData["adminId"] = adminId;           // Người tạo nhóm
    groupData["createdAt"] = currentTime;     // Thời gian tạo
    groupData["updatedAt"] = currentTime;     // Thời gian cập nhật

    // Tạo yêu cầu gửi tới Firebase
    QJsonDocument jsonDoc = QJsonDocument::fromVariant(groupData);
    QNetworkRequest request(QUrl("https://qt-chat-app-default-rtdb.asia-southeast1.firebasedatabase.app/groups/" + groupId + ".json"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    m_networkManager->put(request, jsonDoc.toJson());
}

