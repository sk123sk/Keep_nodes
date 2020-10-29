#ifndef NODE_H
#define NODE_H
#include <QString>

struct Node{
    QString name;
    QString ip;
    QString login="root";
    QString password;
    QString id="";
    QString type="BEACON";
    QString wallet;
    QString infuraId;
    QString vultr_api="";
    bool isCustomVPS =false;
    int connectedPeers=0;
};
#endif // NODE_H
