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
#include <QTimer>


class CheckNodeWorker: public QObject
{
    Q_OBJECT

public:   
    CheckNodeWorker(QString encryptionPassword);

signals:
    void checkingStateFinished(QString,int);
    void updateLogs(QString);

public slots:
    void destroyNode(QJsonArray nodes, QString id);
    void checkNodeState(Node node);
    void checkLiveLogs(Node node);
    void deleteNode(QJsonArray nodes);
    void stopLogs();

private slots:
    void destroyReply();
    void saveNodeInfo(QJsonArray nodes);
    void readSshChannel();
private:
    QNetworkAccessManager* _networkManager;
    QUrlQuery _query;
    bool _isReplyFinished;
    CkSsh ssh;
    bool success;
    QString _encryptionPassword;
    std::atomic<bool> _stopLogs;
    int _sshChannel;
    QTimer* _logsTimer;
};

#endif // CHECKNODEWORKER_H
