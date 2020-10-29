#ifndef ENCRYPTOR_H
#define ENCRYPTOR_H
#include <QByteArray>
#include <openssl/aes.h>
#include <openssl/evp.h>
#include <QObject>
#include <QDataStream>

class Encryptor: public QObject
{
    Q_OBJECT
public:
    Encryptor();
public slots:
    QByteArray encrypt(const QByteArray& data, const QByteArray& password);
    QByteArray decrypt(const QByteArray& data, const QByteArray& password);
    QByteArray generateSalt();
private slots:
    QByteArray sha256(const QByteArray& text);
private:
    QByteArray m_secretKeyHiddenPart;
    QByteArray m_secretKeyEncryption;
};

#endif // ENCRYPTOR_H
