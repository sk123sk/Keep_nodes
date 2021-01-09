#ifndef CHECKNODESUI_H
#define CHECKNODESUI_H

#include <QMainWindow>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QGroupBox>
#include "CheckNodeWorker.h"
#include <Node.h>
#include <QWidget>

namespace Ui {
class CheckNodesUi;
}

class CheckNodesUi : public QMainWindow
{
    Q_OBJECT


public:
    explicit CheckNodesUi(QWidget *parent = nullptr, QString encryptionPassword="" );
    ~CheckNodesUi();

signals:
    void backClicked();
    void homeClicked();
    void destroyNode(QJsonArray, QString);
    void checkNodeState(Node);
    void deleteNode(QJsonArray);
    void checkLiveLogs(Node);
    void stopLogs();

public slots:
    void setUi(QJsonArray nodes);
    void destroyClicked();
    void checkClicked();
    void homeBtnClicked();
    void deleteClicked();
    void logsBtnClicked();
    void checkingStateFinished(QString name, int peers);
    void fixWindowSize();

private slots:
    QGroupBox* createNodeGroupBox(Node node);

private:
    Ui::CheckNodesUi *ui;
    QJsonArray _nodes;
    CheckNodeWorker* _checkWorker;
    QString _encryptionPassword;
    QWidget* _currentWidget;
    bool _isRunningLogs;



};

#endif // CHECKNODESUI_H
