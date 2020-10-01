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

class CheckNodeWorker: public QObject
{
    Q_OBJECT

public:   
    CheckNodeWorker();
    struct Node{
           QString name;
           QString ip;
           QString password;
           QString id;
           QString type;
           QString wallet;
           QString infuraId;
           int connectedPeers=0;
       };
signals:
    void checkingStateFinished(QString,int);

public slots:
    void destroyNode(QJsonArray nodes, QString id);
    void checkNodeState(Node node);

private slots:
    void destroyReply();
    void saveNodeInfo(QJsonArray nodes);

private:
    QNetworkAccessManager* _networkManager;
    QUrlQuery _query;
    bool _isReplyFinished;
    CkSsh ssh;
    bool success;
};

#endif // CHECKNODEWORKER_H
