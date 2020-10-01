#ifndef CHECKNODESUI_H
#define CHECKNODESUI_H

#include <QMainWindow>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QGroupBox>
#include "CheckNodeWorker.h"

namespace Ui {
class CheckNodesUi;
}

class CheckNodesUi : public QMainWindow
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
    explicit CheckNodesUi(QWidget *parent = nullptr);
    ~CheckNodesUi();

signals:
    void backClicked();

public slots:
    void setUi(QJsonArray nodes);
    void destroyClicked();
    void checkClicked();
    void checkingStateFinished(QString name, int peers);

private slots:
    QGroupBox* createNodeGroupBox(Node node);

private:
    Ui::CheckNodesUi *ui;
    QJsonArray _nodes;
    CheckNodeWorker* _checkWorker;

};

#endif // CHECKNODESUI_H
