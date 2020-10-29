#ifndef CHECKNODEWORKER_H
#define CHECKNODEWORKER_H

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QUrlQuery>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <Chilkat_C.h>
#include <CkSsh.h>
#include <CkStringBuilder.h>
#include <iostream>
#include <CkGlobal.h>
#include "Node.h"


class CheckNodeWorker: public QObject
{
    Q_OBJECT

public:   
    CheckNodeWorker(QString encryptionPassword);

signals:
    void checkingStateFinished(QString,int);

public slots:
    void destroyNode(QJsonArray nodes, QString id);
    void checkNodeState(Node node);
    void deleteNode(QJsonArray nodes);

private slots:
    void destroyReply();
    void saveNodeInfo(QJsonArray nodes);

private:
    QNetworkAccessManager* _networkManager;
    QUrlQuery _query;
    bool _isReplyFinished;
    CkSsh ssh;
    bool success;
    QString _encryptionPassword;
};

#endif // CHECKNODEWORKER_H
