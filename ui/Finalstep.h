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
    void homeClicked();

public slots:
    void setData(Node node, QString pwd, QString keystore);
    void updateWorkStatus(QString status);

public:
    explicit FinalStep(QWidget *parent = nullptr, QString encryptionPassword="");
    ~FinalStep();


private slots:
    void on_btn_back_clicked();
    void on_btn_deploy_clicked();

    void on_pb_home_clicked();

private:
    Ui::FinalStep *ui;
    bool _backPushed;
    Node _currentNode;
    QString _keystore_pwd;
    QString _keystore;
    bool _isNodeUp;
    int _timePassed;
    QTimer* _timerTimePassed;
    CreateNodeWorker* _nodeWorker;
    QString _encryptionPassword;
};

#endif // FINALSTEP_H
