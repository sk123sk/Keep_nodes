#include "ui/mainwindow.h"
#include "ui_mainwindow.h"
#include <QFile>
#include <QDataStream>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow), _node_type(beacon), _step1(NULL)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setDataStep1(QString wallet, QString pwd, QString path)
{
    _eth_wallet= wallet;
    _keystore_path = path;
    _keystore_pwd = pwd;
    _step2->show();
}

void MainWindow::setDataStep2(QString id, QString api)
{
    _infura_project_ID = id;
    _apiKey=api;
    _step3->show();
}

void MainWindow::setDataStep3(int type, QString nodeName)
{
    _node_type = node_type(type);
    _node_name = nodeName;
    readKeyStoreFile();
    _finalStep->setData(_eth_wallet, _infura_project_ID, _node_type==0?"BEACON":"ECSDA", _node_name, _keystore_pwd, _keystoreData, _apiKey);
    _finalStep->show();    
}

void MainWindow::backClicked()
{

    QString sender_class = sender()->metaObject()->className();
    if (sender_class == "Step_1" || sender_class == "CheckNodesUi"){
        this->show();
    }
    else if (sender_class == "Step_2"){
        _step1->show();
    }
    else if (sender_class == "Step_3"){
        _step2->show();
    }
    else {
        _step3->show();
    }
}


void MainWindow::on_btn_Run_new_clicked()
{
    if (_step1 != NULL){
        delete _step1;
        delete _step2;
        delete _step3;
        delete _finalStep;
    }

    _step1= new Step_1(this);
    _step2= new Step_2(this);
    _step3= new Step_3(this);
    _finalStep= new FinalStep(this);
    connect(_step1, SIGNAL(setData(QString,QString,QString)), this, SLOT(setDataStep1(QString,QString,QString)));
    connect(_step2, SIGNAL(setData(QString, QString)), this, SLOT(setDataStep2(QString, QString)));
    connect(_step3, SIGNAL(setData(int, QString)), this, SLOT(setDataStep3(int, QString)));
    connect(_step1, SIGNAL(backBtnClicked()), this, SLOT(backClicked()));
    connect(_step2, SIGNAL(backBtnClicked()), this, SLOT(backClicked()));
    connect(_step3, SIGNAL(backBtnClicked()), this, SLOT(backClicked()));
    connect(_finalStep, SIGNAL(backBtnClicked()), this, SLOT(backClicked()));
    this->hide();
    _step1->show();
}

void MainWindow::on_btn_check_nodes_clicked()
{
   QJsonArray nodes = readNodesFile();
   if (nodes.count()>5){
       QMessageBox msgBox;
       msgBox.setText("You have reached max amount of nodes");
       msgBox.exec();
   }
   else if (nodes.count()==0){
       QMessageBox msgBox;
       msgBox.setText("You don't have any nodes");
       msgBox.exec();
   }
   else {
       _checkNodes = new CheckNodesUi(this);
       this->hide();
       _checkNodes->setUi(nodes);
       _checkNodes->show();
       connect(_checkNodes,SIGNAL(backClicked()),this,SLOT(backClicked()));
   }
}

void MainWindow::readKeyStoreFile()
{
    QFile file(_keystore_path);
    if (!file.exists() || !file.open(QIODevice::ReadOnly))
        QMessageBox::information(this,"Cannot open file","Incorrect path or file unavailable");
    else {
        QByteArray data;
        data=file.readAll();
        file.close();
        _keystoreData=QString(data);
    }
}

QJsonArray MainWindow::readNodesFile()
{
    QJsonArray nodes;
    QFile fileToRead("nodes.dat");
    if (!fileToRead.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug()<<"file error";
        return nodes;
    }
     QByteArray data = fileToRead.readAll();
     fileToRead.close();
     nodes = QJsonDocument::fromJson(data).array();
     return nodes;
}

