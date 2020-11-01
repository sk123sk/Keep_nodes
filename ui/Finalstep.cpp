#include "ui/Finalstep.h"
#include "ui_Finalstep.h"
#include <QDebug>
#include <QThread>
#include <QMessageBox>
#include <QPalette>

FinalStep::FinalStep(QWidget *parent,QString encryptionPassword) :
    QMainWindow(parent),
    ui(new Ui::FinalStep), _backPushed(0), _isNodeUp(0), _encryptionPassword(encryptionPassword)
{
    ui->setupUi(this);
    QPalette p = ui->plain_te->palette();
    p.setColor(QPalette::Base, Qt::black);
    ui->plain_te->setPalette(p);
}

FinalStep::~FinalStep()
{
    if (!_backPushed)
        parentWidget()->deleteLater();
    delete ui;

}


void FinalStep::setData(Node node, QString pwd, QString keystore)
{
    ui->l_wallet->setTextFormat(Qt::RichText);
    ui->l_wallet->setText(QString("<font color=\"#000000\" size=3>ETH wallet: </font>").arg(node.wallet));
    ui->le_wallet->setText(node.wallet);
    ui->l_infuraID->setTextFormat(Qt::RichText);
    ui->l_infuraID->setText(QString("<font color=\"#000000\" size=3>Infura id: </font><font color=\"#0000A0\" size=3>%1</font>").arg(node.infuraId));
    ui->l_nodeName->setTextFormat(Qt::RichText);
    ui->l_nodeName->setText(QString("<font color=\"#000000\" size=3>Node name: </font><font color=\"#0000A0\" size=3>%1</font>").arg(node.name));
    ui->l_nodeType->setTextFormat(Qt::RichText);
    ui->l_nodeType->setText(QString("<font color=\"#000000\" size=3>Node type: </font><font color=\"#0000A0\" size=3>%1</font>").arg(node.type.toLower()));
    _currentNode=node;
    _keystore_pwd=pwd;
    _keystore=keystore;

}

void FinalStep::updateWorkStatus(QString status)
{
    if (status.contains("..."))
        ui->plain_te->appendHtml(QString("<font color=\"#ADD8E6\" size=3>%1</font>").arg(status));
    else if (status.contains("FATAL")){
        if (status.contains("Wallet balance")){
            status.chop(5);
            _timerTimePassed->stop();
            ui->plain_te->appendHtml(QString("<font color=\"#FF0000\" size=3>%1</font>").arg(status));
        }
        else
            ui->plain_te->appendHtml(QString("<font color=\"#FF0000\" size=3>%1</font>").arg(status));
    }
    else
        ui->plain_te->appendHtml(QString("<font color=\"#00FF00\" size=3>%1</font>").arg(status));
    if (status.contains("Keep node is UP")){
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
    if (_currentNode.wallet=="" || _currentNode.infuraId=="" || _keystore==""){
        QMessageBox msgBox;
        msgBox.setText("Input all fields! Missing smth");
        msgBox.exec();
        return;
    }
    ui->plain_te->clear();
    ui->plain_te->appendHtml(QString("<font color=\"#ADD8E6\" size=3>--checking ETH testnet wallet balance...</font>"));
    _nodeWorker = new CreateNodeWorker(_currentNode,_keystore_pwd,_keystore, _encryptionPassword);
    QThread* workerThread = new QThread();
    _nodeWorker->moveToThread(workerThread);
    connect(workerThread, SIGNAL(started()),_nodeWorker,SLOT(get_eth_balance()));
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

void FinalStep::on_pb_home_clicked()
{
    this->hide();
    emit homeClicked();
}
