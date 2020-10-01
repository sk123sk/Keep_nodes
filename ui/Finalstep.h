#ifndef FINALSTEP_H
#define FINALSTEP_H

#include <QMainWindow>
#include <QTimer>
#include "CreateNodeWorker.h"


namespace Ui {
class FinalStep;
}

class FinalStep : public QMainWindow
{
    Q_OBJECT

signals:
    void backBtnClicked();

public slots:
    void setData(QString wallet, QString infuraId, QString nodeType, QString nodeName, QString pwd, QString keystore, QString api);
    void updateWorkStatus(QString status);

public:
    explicit FinalStep(QWidget *parent = nullptr);
    ~FinalStep();

private slots:
    void on_btn_back_clicked();
    void on_btn_deploy_clicked();

private:
    Ui::FinalStep *ui;
    bool _backPushed;
    QString _wallet;
    QString _keystore_pwd;
    QString _keystore;
    QString _infuraID;
    QString _nodeType;
    QString _nodeName;
    QString _apiKey;
    bool _isNodeUp;
    int _timePassed;
    QTimer* _timerTimePassed;
    CreateNodeWorker* _nodeWorker;
};

#endif // FINALSTEP_H
