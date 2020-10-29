#include "Encryptor.h"
#include <QRandomGenerator64>;

Encryptor::Encryptor()
{

}

QByteArray Encryptor::sha256(const QByteArray &text)
{
    unsigned int outLen = 0;
    QByteArray dataBuff;
    dataBuff.resize(EVP_MAX_MD_SIZE);
    EVP_MD_CTX* evpMdCtx = EVP_MD_CTX_new();
    EVP_DigestInit(evpMdCtx, EVP_sha256());
    EVP_DigestUpdate(evpMdCtx, text.data(), text.size());
    EVP_DigestFinal_ex(evpMdCtx, (unsigned char*)dataBuff.data(), &outLen);
    EVP_MD_CTX_free(evpMdCtx);
    dataBuff.resize(outLen);
    return dataBuff.toHex();
}

QByteArray Encryptor::encrypt(const QByteArray &data, const QByteArray &password)
{
    int outLen = 0;
    QByteArray dataBuff;
    dataBuff.resize(data.size() + AES_BLOCK_SIZE);
    EVP_CIPHER_CTX* evpCipherCtx = EVP_CIPHER_CTX_new();
    EVP_CIPHER_CTX_init(evpCipherCtx);
    EVP_EncryptInit(evpCipherCtx, EVP_aes_256_cbc(), (const unsigned char*)sha256(password).data(),
                    (const unsigned char*)sha256(m_secretKeyHiddenPart + password).data());
    EVP_EncryptUpdate(evpCipherCtx, (unsigned char*)dataBuff.data(), &outLen, (const unsigned char*)data.data(),
                      data.size());
    int resultLen = outLen;
    EVP_EncryptFinal(evpCipherCtx, (unsigned char*)dataBuff.data() + resultLen, &outLen);
    resultLen += outLen;
    EVP_CIPHER_CTX_free(evpCipherCtx);
    dataBuff.resize(resultLen);
    return dataBuff;
}

QByteArray Encryptor::decrypt(const QByteArray &data, const QByteArray &password)
{
    int outLen = 0;
    QByteArray dataBuff;
    dataBuff.resize(data.size() + AES_BLOCK_SIZE);
    EVP_CIPHER_CTX* evpCipherCtx = EVP_CIPHER_CTX_new();
    EVP_CIPHER_CTX_init(evpCipherCtx);
    EVP_DecryptInit(evpCipherCtx, EVP_aes_256_cbc(), (const unsigned char*)sha256(password).data(),
                    (const unsigned char*)sha256(m_secretKeyHiddenPart + password).data());
    EVP_DecryptUpdate(evpCipherCtx, (unsigned char*)dataBuff.data(), &outLen, (const unsigned char*)data.data(),
                      data.size());
    int resultLen = outLen;
    EVP_DecryptFinal(evpCipherCtx, (unsigned char*)dataBuff.data() + resultLen, &outLen);
    resultLen += outLen;
    EVP_CIPHER_CTX_free(evpCipherCtx);
    dataBuff.resize(resultLen);
    return dataBuff;
}

QByteArray Encryptor::generateSalt()
{
    const QString possibleCharacters("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");
       const int randomStringLength = 64; // assuming you want random strings of 12 characters

       QString randomString;
       for(int i=0; i<randomStringLength; ++i)
       {
           int index = QRandomGenerator64::global()->generate()%possibleCharacters.size();
           QChar nextChar = possibleCharacters.at(index);
           randomString.append(nextChar);
       }
       return randomString.toUtf8();
}
