#ifndef CREATENODEWORKER_H
#define CREATENODEWORKER_H
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QUrlQuery>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTimer>
#include <Chilkat_C.h>
#include <CkSsh.h>
#include <CkStringBuilder.h>
#include <iostream>
#include <CkGlobal.h>

class CreateNodeWorker : public QObject
{
    Q_OBJECT

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

public:
    CreateNodeWorker();
    CreateNodeWorker(QString wallet, QString pwd, QString keyStore, QString infuraId, QString nodeLabel, QString nodeType, QString api);

signals:
    void updateWorkStatus(QString);

private slots:
    void deployNode();
    void checkDeployStatus();
    void chillkatUnlock();
    void runSSH();
    void setupNode(QList<QString> commands);
    void startNode();
    void deployReply();
    void statusCheckReply();
    void saveNodeInfo();

private:
    CkSsh ssh;
    bool success;
    QNetworkAccessManager* _networkManager;
    QUrlQuery _query;
    QTimer* _timer;
    QTimer* _timerTimePassed;
    int _timePassed;
    QString _keystore;
    QString _keystore_pwd;
    QString _apiKey;
    bool _isNodeStatusActive;
    Node _currentNode;
};

#endif // NODEWORKER_H
