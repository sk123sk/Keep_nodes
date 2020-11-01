#include "ui/mainwindow.h"
#include "ui_mainwindow.h"
#include <QFile>
#include <QDataStream>
#include <QMessageBox>
#include "CreatePasswordDialog.h"
#include "AddNodeDialog.h"
#include "Encryptor.h"
#include <QInputDialog>
#include <QLineEdit>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QDir>
#include <QCryptographicHash>
#include <QProcess>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow), _step1(NULL), _isLoggedIn(0)
{
   ui->setupUi(this);
    _settings=new QSettings("keep_nodes.conf", QSettings::IniFormat);
    _settings->setPath(QSettings::IniFormat,QSettings::UserScope, QDir::currentPath());
    loadSetting();
}

MainWindow::~MainWindow()
{
    delete ui;
}



void MainWindow::setDataStep1(QString pwd, QString keyStoreData)
{
    _keystoreData = keyStoreData;
    _keystore_pwd = pwd;
    if(_currentNode.isCustomVPS)
        _step2->setApiVisible(false);
    _step2->show();
}

void MainWindow::setDataStep2(QString id, QString api)
{
    _currentNode.infuraId = id;
    if (_currentNode.isCustomVPS && api=="5DVIS4NUNFJQQFSL27L34HHZVREIFAVSOV7Q")
        _currentNode.vultr_api="";
    else
        _currentNode.vultr_api=api;;
    _step3->show();
}

void MainWindow::setDataStep3(int type, QString nodeName)
{
    _currentNode.type = type==0?"BEACON":"ECSDA";
    _currentNode.name = nodeName;
    _finalStep->setData(_currentNode, _keystore_pwd, _keystoreData);
    _finalStep->show();
}

void MainWindow::setWallet(QString wallet)
{
    _currentNode.wallet = wallet;
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

void MainWindow::homeClicked()
{
   this->show();
}


void MainWindow::on_btn_Run_new_clicked()
{
    if (_encryptionPassword==""){
        loadSetting();
        return;
    }
    if (_step1 != NULL){
        delete _step1;
        delete _step2;
        delete _step3;
        delete _finalStep;
    }
    _currentNode = Node();
    QMessageBox msgBox(QMessageBox::Question, tr("Create new node"),
                       "Do you have your own VPS server for the node?", 0, this);
    msgBox.addButton(tr("&Yes"), QMessageBox::AcceptRole);
    msgBox.addButton(tr("&No"), QMessageBox::RejectRole);
    if (msgBox.exec() == QMessageBox::AcceptRole)
    {
        QDialog dialog(this);
        QFormLayout form(&dialog);
        form.addRow(new QLabel("Enter VPS credentials:"));
        QLineEdit *lineEdit_ip = new QLineEdit(&dialog);
        QString label_ip=QString("VPS IP");
        form.addRow(label_ip, lineEdit_ip);
        QLineEdit *lineEdit_login = new QLineEdit(&dialog);
        QString label_login=QString("Login");
        form.addRow(label_login, lineEdit_login);
        QLineEdit *lineEdit_pass = new QLineEdit(&dialog);
        lineEdit_pass->setEchoMode(QLineEdit::Password);
        QString label_pass=QString("VPS Password");
        form.addRow(label_pass, lineEdit_pass);

        QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                                   Qt::Horizontal, &dialog);
        form.addRow(&buttonBox);
        QObject::connect(&buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
        QObject::connect(&buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));
        if (dialog.exec() == QDialog::Accepted) {
            _currentNode.ip=lineEdit_ip->text();
            _currentNode.login=lineEdit_login->text();
            _currentNode.password=lineEdit_pass->text();
            _currentNode.isCustomVPS=true;
        }
        else
            _currentNode.isCustomVPS=false;

    }
    else
        _currentNode.isCustomVPS=false;
    _step1= new Step_1(this);
    _step2= new Step_2(this);
    _step3= new Step_3(this);
    _finalStep= new FinalStep(this, _encryptionPassword);
    connect(_step1, SIGNAL(setData(QString,QString)), this, SLOT(setDataStep1(QString,QString)));
    connect(_step1, SIGNAL(setWallet(QString)), this, SLOT(setWallet(QString)));
    connect(_step2, SIGNAL(setData(QString, QString)), this, SLOT(setDataStep2(QString, QString)));
    connect(_step3, SIGNAL(setData(int, QString)), this, SLOT(setDataStep3(int, QString)));
    connect(_step1, SIGNAL(backBtnClicked()), this, SLOT(backClicked()));
    connect(_step2, SIGNAL(backBtnClicked()), this, SLOT(backClicked()));
    connect(_step3, SIGNAL(backBtnClicked()), this, SLOT(backClicked()));
    connect(_finalStep, SIGNAL(backBtnClicked()), this, SLOT(backClicked()));
    connect(_finalStep, SIGNAL(homeClicked()), this, SLOT(homeClicked()));
    this->hide();
    _step1->show();
}

void MainWindow::on_btn_check_nodes_clicked()
{
    if (_encryptionPassword==""){
        loadSetting();
        return;
    }
    QJsonArray nodes = readNodesFile();
    qDebug()<<"Node:"<<nodes;
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
        _checkNodes = new CheckNodesUi(this, _encryptionPassword);
        this->hide();
        _checkNodes->setUi(nodes);
        _checkNodes->show();
        connect(_checkNodes,SIGNAL(homeClicked()),this,SLOT(homeClicked()));
    }
}

void MainWindow::on_btn_addExisting_clicked()
{
    AddNodeDialog* addDialog=new AddNodeDialog;
    connect(addDialog, &AddNodeDialog::setNodeData, this, [=](const Node &newNode){
        Encryptor enc;
        QJsonArray nodes;
        QFile fileToRead("nodes.dat");
        if (!fileToRead.open(QIODevice::ReadOnly))
            qDebug()<<"No saved nodes detected";
        else {
            QByteArray encryptedData;
            QDataStream in(&fileToRead);
            in>>encryptedData;
            QByteArray decriptedData = enc.decrypt(encryptedData, _encryptionPassword.toUtf8());
            fileToRead.close();
            nodes = QJsonDocument::fromJson(decriptedData).array();
        }
        QJsonObject node;
        node.insert("name", newNode.name);
        node.insert("ip", newNode.ip);
        node.insert("login", newNode.login);
        node.insert("password", newNode.password);
        node.insert("id", newNode.id);
        node.insert("type", newNode.type);
        node.insert("wallet", newNode.wallet);
        node.insert("vultr_api",newNode.vultr_api);
        nodes.push_back(node);
        QFile file("nodes.dat");
        if (!file.open(QIODevice::WriteOnly)) {
            qDebug()<<"file error";
            return;
        }
        QDataStream out(&file);
        out <<enc.encrypt(QJsonDocument(nodes).toJson(), _encryptionPassword.toUtf8());
        file.close();
    });
    addDialog->show();
}


QJsonArray MainWindow::readNodesFile()
{
    Encryptor enc;
    QJsonArray nodes;
    QFile fileToRead("nodes.dat");
    if (!fileToRead.open(QIODevice::ReadOnly)) {
        qDebug()<<"file error";
        return nodes;
    }
    QByteArray encryptedData;
    QDataStream in(&fileToRead);
    in>>encryptedData;
    QByteArray decriptedData = enc.decrypt(encryptedData, _encryptionPassword.toUtf8());
    fileToRead.close();
    nodes = QJsonDocument::fromJson(decriptedData).array();
    return nodes;
}

void MainWindow::loadSetting()
{
    this->hide();
    if (_isLoggedIn)
        return;
    QString pwdHash = _settings->value("user_verification").toString();
    if (pwdHash==""){
        CreatePasswordDialog* pwdDialog=new CreatePasswordDialog(this);
        connect(pwdDialog, &CreatePasswordDialog::setPassword, this, [=](const QString &pwd){
            _encryptionPassword=pwd;
            Encryptor encryptor;
            QByteArray salt = encryptor.generateSalt();
            _settings->setValue("user_verification", QCryptographicHash::hash(_encryptionPassword.toUtf8().append(salt), QCryptographicHash::Sha256).toHex());
            _settings->setValue("salt", salt);
        });
        pwdDialog->show();
        return;
    }

    bool isInputCorrect;
    _encryptionPassword = (QInputDialog::getText( 0,"Log in","Enter your password:",
                                                  QLineEdit::Password,"", &isInputCorrect)).toUtf8();
    if (!isInputCorrect) {
        //  Cancel or dialog was closed
        QMessageBox::information(this,"Input incorrect","You have to enter password. Try again");
        return;
    }

    if (QCryptographicHash::hash(_encryptionPassword.toUtf8().append(_settings->value("salt").toString()), QCryptographicHash::Sha256).toHex()==_settings->value("user_verification").toByteArray()){
        _isLoggedIn = true;
    }
    else {
        QMessageBox::information(this,"Wrong password","Incorrect password. Try again");
        loadSetting();
    }
}


