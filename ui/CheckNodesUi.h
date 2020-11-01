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

public slots:
    void setUi(QJsonArray nodes);
    void destroyClicked();
    void checkClicked();
    void homeBtnClicked();
    void deleteClicked();
    void checkingStateFinished(QString name, int peers);

private slots:
    QGroupBox* createNodeGroupBox(Node node);

private:
    Ui::CheckNodesUi *ui;
    QJsonArray _nodes;
    CheckNodeWorker* _checkWorker;
    QString _encryptionPassword;
    QWidget* _currentWidget;



};

#endif // CHECKNODESUI_H
