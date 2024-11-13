#include "ClientManager.h"

#include <QHostAddress>

ClientManager::ClientManager(QHostAddress ip, ushort port, QObject *parent)
    : QObject{parent},
    _ip(ip),
    _port(port)
{
    if (!generateRSAKeys()) {
        qDebug() << "Failed to generate RSA keys";
    }

    setupClient();
}

ClientManager::~ClientManager()
{
    if (_privateKey) {
        EVP_PKEY_free(_privateKey);
        _privateKey = nullptr;
    }

    if (_publicKey) {
        EVP_PKEY_free(_publicKey);
        _publicKey = nullptr;
    }
}

void ClientManager::connectToServer()
{
    _socket->connectToHost(_ip, _port);
}

void ClientManager::sendMessage(QString publicKey, QString message, QString receiver)
{
    EVP_PKEY *convertedPublicKey = loadPublicKeyFromPEM(publicKey);
    QByteArray aesKey = generateAESKey();

    QByteArray encryptedMessage = encryptAES(message.toUtf8(), aesKey);
    if (encryptedMessage.isEmpty()) {
        qDebug() << "Failed to encrypt message with AES";
        return;
    }

    QByteArray encryptedAESKey = encryptRSA(aesKey, convertedPublicKey);
    if (encryptedAESKey.isEmpty()) {
        qDebug() << "Failed to encrypt AES key with RSA";
        EVP_PKEY_free(convertedPublicKey);
        return;
    }

    EVP_PKEY_free(convertedPublicKey);

    _socket->write(_protocol.textMessage(encryptedAESKey, encryptedMessage, receiver));
    // EVP_PKEY_free(_publicKey);
}

void ClientManager::sendName(QString name)
{
    _socket->write(_protocol.setNameMessage(name));
}

void ClientManager::sendStatus(ChatProtocol::Status status)
{
    _socket->write(_protocol.setStatusMessage(status));
}

void ClientManager::sendIsTyping(QString receiver)
{
    _socket->write(_protocol.isTypingMessage(receiver));
}

void ClientManager::sendFile(QString receiver, QString fileName)
{
    _socket->write(_protocol.setFileMessage(receiver, fileName));
}

void ClientManager::sendNewClient(QString uid, QString email)
{
    _socket->write(_protocol.setNewClient(uid, email));
}

void ClientManager::readyRead()
{
    auto data = _socket->readAll();
    _protocol.loadData(data);
    switch (_protocol.type()) {
    case ChatProtocol::Text: {
        QByteArray encryptedAESKey = _protocol.encryptedAESKey();
        QByteArray encryptedMessage = _protocol.encryptedMessage();
        QByteArray AESKey = decryptRSA(encryptedAESKey, _privateKey);
        QString message = QString::fromUtf8(decryptAES(encryptedMessage, AESKey));
        emit textMessageReceived(message, _protocol.sender());
        break;
    }
    case ChatProtocol::SetName:
        emit nameChanged(_protocol.name());
        break;
    case ChatProtocol::SetStatus:
        emit statusChanged(_protocol.status(), _protocol.sender());
        break;
    case ChatProtocol::IsTyping:
        emit isTyping(_protocol.sender());
        break;
    case ChatProtocol::SendFile:
        emit receiveFile(_protocol.sender(), _protocol.fileName(), _protocol.fileSize(), _protocol.fileData());
        break;
    case ChatProtocol::ConnectionACK:{
        emit connectionACK(_protocol.myName(), _protocol.clientsName());

        QMap<QString, QString> publicKeys = _protocol.publicKeys();
        foreach (auto key, publicKeys.keys()) {
            QString publicKey = publicKeys.value(key);
            emit sendPublicKey(publicKey, key);
        }
        break;
    }
    case ChatProtocol::NewClient:
        emit newClientConnectedToServer(_protocol.clientName());
        break;
    case ChatProtocol::ClientDisconnected:
        emit clientDisconnected(_protocol.clientName());
        break;
    case ChatProtocol::ClientName:
        emit clientNameChanged(_protocol.prevName(), _protocol.clientName());
        break;
    case ChatProtocol::SetPublicKey:
        emit sendPublicKey(_protocol.publicKey(), _protocol.sender());
        break;
    default:
        break;
    }
}

void ClientManager::setupClient()
{
    _socket = new QTcpSocket(this);
    connect(_socket, &QTcpSocket::connected, [this]() {
        _socket->write(_protocol.setPublicKeyMessage(getPublicKeyPEM()));
        emit connected();
    });
    connect(_socket, &QTcpSocket::disconnected, this, &ClientManager::disconnected);
    connect(_socket, &QTcpSocket::readyRead, this, &ClientManager::readyRead);
}

bool ClientManager::generateRSAKeys(int keyLength)
{
    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL);
    if (!ctx) {
        qDebug() << "EVP_PKEY_CTX_new_id failed";
        return false;
    }

    if (EVP_PKEY_keygen_init(ctx) <= 0) {
        qDebug() << "EVP_PKEY_keygen_init failed";
        EVP_PKEY_CTX_free(ctx);
        return false;
    }

    if (EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, keyLength) <= 0) {
        qDebug() << "EVP_PKEY_CTX_set_rsa_keygen_bits failed";
        EVP_PKEY_CTX_free(ctx);
        return false;
    }

    EVP_PKEY *pkey = nullptr;
    if (EVP_PKEY_keygen(ctx, &pkey) <= 0) {
        qDebug() << "EVP_PKEY_keygen failed";
        EVP_PKEY_CTX_free(ctx);
        return false;
    }

    EVP_PKEY_CTX_free(ctx);

    _privateKey = pkey;
    _publicKey = EVP_PKEY_dup(_privateKey);
    if (!_publicKey) {
        qDebug() << "EVP_PKEY_dup failed";
        EVP_PKEY_free(_privateKey);
        _privateKey = nullptr;
        return false;
    }

    return true;
}

QString ClientManager::getPublicKeyPEM() const
{
    if (!_publicKey) {
        qDebug() << "Public key not available";
        return QString();
    }

    BIO *bio = BIO_new(BIO_s_mem());
    if (!PEM_write_bio_PUBKEY(bio, _publicKey)) {
        qDebug() << "PEM_write_bio_PUBKEY failed";
        BIO_free(bio);
        return QString();
    }

    char *pemData;
    long pemLen = BIO_get_mem_data(bio, &pemData);
    QByteArray pemArray(pemData, pemLen);
    QString pemString = QString::fromUtf8(pemArray);

    BIO_free(bio);
    return pemString;
}

EVP_PKEY* ClientManager::loadPublicKeyFromPEM(const QString& publicKeyPEM) const
{
    if (publicKeyPEM.isEmpty()) {
        qDebug() << "Public key PEM string is empty";
        return nullptr;
    }

    QByteArray pemBytes = publicKeyPEM.toUtf8();
    BIO* bio = BIO_new_mem_buf(pemBytes.constData(), pemBytes.size());
    if (!bio) {
        qDebug() << "Failed to create BIO for public key";
        return nullptr;
    }

    EVP_PKEY* pubKey = PEM_read_bio_PUBKEY(bio, NULL, NULL, NULL);
    if (!pubKey) {
        qDebug() << "Failed to read public key from PEM string";
        BIO_free(bio);
        return nullptr;
    }

    BIO_free(bio);
    return pubKey;
}

QByteArray ClientManager::generateAESKey() {
    const int keyLength = 32;
    QByteArray aesKey(keyLength, 0);

    if (RAND_bytes(reinterpret_cast<unsigned char*>(aesKey.data()), keyLength) != 1) {
        qDebug() << "Failed to generate AES key";
        return QByteArray();
    }

    return aesKey;
}

QByteArray ClientManager::encryptAES(const QByteArray& plaintext, const QByteArray& key)
{
    // Initialize encryption context
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        qDebug() << "Failed to create EVP_CIPHER_CTX";
        return QByteArray();
    }

    // Initialize encryption operation
    if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL,
                                reinterpret_cast<const unsigned char*>(key.data()), NULL)) {
        qDebug() << "Failed to initialize AES encryption";
        EVP_CIPHER_CTX_free(ctx);
        return QByteArray();
    }

    // Provide the message to be encrypted and obtain the encrypted output
    QByteArray ciphertext;
    ciphertext.resize(plaintext.size() + EVP_CIPHER_block_size(EVP_aes_256_cbc()));
    int len;

    if (1 != EVP_EncryptUpdate(ctx,
                               reinterpret_cast<unsigned char*>(ciphertext.data()), &len,
                               reinterpret_cast<const unsigned char*>(plaintext.data()), plaintext.size())) {
        qDebug() << "Failed to encrypt message";
        EVP_CIPHER_CTX_free(ctx);
        return QByteArray();
    }
    int ciphertext_len = len;

    // Finalize encryption
    if (1 != EVP_EncryptFinal_ex(ctx,
                                 reinterpret_cast<unsigned char*>(ciphertext.data()) + len, &len)) {
        qDebug() << "Failed to finalize AES encryption";
        EVP_CIPHER_CTX_free(ctx);
        return QByteArray();
    }
    ciphertext_len += len;

    EVP_CIPHER_CTX_free(ctx);

    ciphertext.resize(ciphertext_len);
    return ciphertext;
}

QByteArray ClientManager::decryptAES(const QByteArray& ciphertext, const QByteArray& key)
{
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        qDebug() << "Failed to create EVP_CIPHER_CTX";
        return QByteArray();
    }

    if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL,
                                reinterpret_cast<const unsigned char*>(key.data()), NULL)) {
        qDebug() << "Failed to initialize AES decryption";
        EVP_CIPHER_CTX_free(ctx);
        return QByteArray();
    }

    QByteArray plaintext;
    plaintext.resize(ciphertext.size());
    int len;

    if (1 != EVP_DecryptUpdate(ctx,
                               reinterpret_cast<unsigned char*>(plaintext.data()), &len,
                               reinterpret_cast<const unsigned char*>(ciphertext.data()), ciphertext.size())) {
        qDebug() << "Failed to decrypt message";
        EVP_CIPHER_CTX_free(ctx);
        return QByteArray();
    }
    int plaintext_len = len;

    if (1 != EVP_DecryptFinal_ex(ctx,
                                 reinterpret_cast<unsigned char*>(plaintext.data()) + len, &len)) {
        qDebug() << "Failed to finalize AES decryption";
        EVP_CIPHER_CTX_free(ctx);
        return QByteArray();
    }
    plaintext_len += len;

    EVP_CIPHER_CTX_free(ctx);

    plaintext.resize(plaintext_len);
    return plaintext;
}



QByteArray ClientManager::encryptRSA(const QByteArray& data, EVP_PKEY* publicKey)
{
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(publicKey, NULL);
    if (!ctx) {
        qDebug() << "Failed to create EVP_PKEY_CTX";
        return QByteArray();
    }

    if (EVP_PKEY_encrypt_init(ctx) <= 0) {
        qDebug() << "Failed to initialize RSA encryption";
        EVP_PKEY_CTX_free(ctx);
        return QByteArray();
    }

    size_t outlen;
    if (EVP_PKEY_encrypt(ctx, NULL, &outlen,
                         reinterpret_cast<const unsigned char*>(data.data()), data.size()) <= 0) {
        qDebug() << "Failed to determine encrypted data length";
        EVP_PKEY_CTX_free(ctx);
        return QByteArray();
    }

    QByteArray out;
    out.resize(outlen);
    if (EVP_PKEY_encrypt(ctx, reinterpret_cast<unsigned char*>(out.data()), &outlen,
                         reinterpret_cast<const unsigned char*>(data.data()), data.size()) <= 0) {
        qDebug() << "Failed to encrypt data with RSA";
        EVP_PKEY_CTX_free(ctx);
        return QByteArray();
    }

    EVP_PKEY_CTX_free(ctx);
    out.resize(outlen);
    return out;
}

QByteArray ClientManager::decryptRSA(const QByteArray& encryptedData, EVP_PKEY* privateKey)
{
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(privateKey, NULL);
    if (!ctx) {
        qDebug() << "Failed to create EVP_PKEY_CTX";
        return QByteArray();
    }

    if (EVP_PKEY_decrypt_init(ctx) <= 0) {
        qDebug() << "Failed to initialize RSA decryption";
        EVP_PKEY_CTX_free(ctx);
        return QByteArray();
    }

    size_t outlen;
    if (EVP_PKEY_decrypt(ctx, NULL, &outlen,
                         reinterpret_cast<const unsigned char*>(encryptedData.data()), encryptedData.size()) <= 0) {
        qDebug() << "Failed to determine decrypted data length";
        EVP_PKEY_CTX_free(ctx);
        return QByteArray();
    }

    QByteArray out;
    out.resize(outlen);
    if (EVP_PKEY_decrypt(ctx, reinterpret_cast<unsigned char*>(out.data()), &outlen,
                         reinterpret_cast<const unsigned char*>(encryptedData.data()), encryptedData.size()) <= 0) {
        qDebug() << "Failed to decrypt data with RSA";
        EVP_PKEY_CTX_free(ctx);
        return QByteArray();
    }

    EVP_PKEY_CTX_free(ctx);
    out.resize(outlen);
    return out;
}






