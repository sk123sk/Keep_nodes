#include "ui/Finalstep.h"
#include "ui_Finalstep.h"
#include <QDebug>
#include <QThread>
#include <QMessageBox>

FinalStep::FinalStep(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::FinalStep), _backPushed(0), _isNodeUp(0)
{
    ui->setupUi(this);
}

FinalStep::~FinalStep()
{
    if (!_backPushed)
        parentWidget()->deleteLater();
    delete ui;

}


void FinalStep::setData(QString wallet, QString infuraId, QString nodeType, QString nodeName, QString pwd, QString keystore, QString api)
{
    ui->l_wallet->setText("ETH WALLET: "+wallet);
    ui->l_infuraID->setText("INFURA ID: "+infuraId);
    ui->l_nodeName->setText("NODE NAME: "+nodeName);
    ui->l_nodeType->setText("NODE TYPE: "+nodeType);
    _wallet=wallet;
    _infuraID=infuraId;
    _nodeName=nodeName;
    _nodeType=nodeType;
    _keystore_pwd=pwd;
    _keystore=keystore;
    _apiKey=api;
}

void FinalStep::updateWorkStatus(QString status)
{
   ui->plain_te->appendPlainText(status);
   if (status.contains("KEEP NODE is UP")){
       _isNodeUp=true;
       _timerTimePassed->stop();
       _timerTimePassed->deleteLater();
   }
}

void FinalStep::on_btn_back_clicked()
{
    this->hide();
    emit backBtnClicked();
}

void FinalStep::on_btn_deploy_clicked()
{  
    if (_wallet=="" || _infuraID=="" || _keystore==""){
        QMessageBox msgBox;
        msgBox.setText("Input all fields! Missing smth");
        msgBox.exec();
        return;
    }
    _nodeWorker = new CreateNodeWorker(_wallet,_keystore_pwd,_keystore,_infuraID, _nodeName, _nodeType, _apiKey);
    QThread* workerThread = new QThread();
    _nodeWorker->moveToThread(workerThread);
    connect(workerThread, SIGNAL(started()),_nodeWorker,SLOT(deployNode()));
    connect(_nodeWorker, SIGNAL(updateWorkStatus(QString)),this, SLOT(updateWorkStatus(QString)));

    workerThread->start();
    _timerTimePassed =new QTimer();
    connect(_timerTimePassed, &QTimer::timeout, this,  [=](){
        _timePassed++;
        ui->l_deployingStatus->setText(QString("Time passed since start: "+QString::number(_timePassed) + "s. Avg. deploying time 5min."));
    });
    _timePassed=0;
    _timerTimePassed->start(1000);

}
