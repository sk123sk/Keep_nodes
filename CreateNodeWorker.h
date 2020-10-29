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
#include "Node.h"
#include <QWidget>

class CreateNodeWorker : public QObject
{
    Q_OBJECT


public:
    CreateNodeWorker(Node node, QString pwd, QString keyStore, QString encryptionPassword);

public slots:
   void get_eth_balance();


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
    void getGrant();
    void replyKeepGrantFinished();
    void delegateKeep();
    void beacon_auth();
    void ecdsa_tbtc_auth();
    void bond_eth(double amount);

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
    bool _isNodeStatusActive;
    Node _currentNode;
    QString _encryptionPassword;
    QWidget* _widget;
};

#endif // NODEWORKER_H
