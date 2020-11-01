#include "ui/CheckNodesUi.h"
#include "ui_CheckNodesUi.h"
#include <QGridLayout>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QLabel>
#include <QPlainTextEdit>
#include <QtConcurrent/QtConcurrent>
#include <QMessageBox>

CheckNodesUi::CheckNodesUi(QWidget *parent,QString encryptionPassword) :
    QMainWindow(parent), _encryptionPassword(encryptionPassword),_currentWidget(NULL),
    ui(new Ui::CheckNodesUi)
{
    _checkWorker = new CheckNodeWorker(_encryptionPassword);
    connect(_checkWorker, SIGNAL(checkingStateFinished(QString,int)),this, SLOT(checkingStateFinished(QString,int)));
}

CheckNodesUi::~CheckNodesUi()
{
    delete ui;
}

void CheckNodesUi::setUi(QJsonArray nodes)
{
    _nodes=nodes;
    if(_nodes.isEmpty()){
        this->hide();
        QMessageBox::information(this,"Nodes Manager","You don't have active nodes. Trasfering to HOME screen");
        emit backClicked();
        return;
    }
    QGridLayout*  mainLayout= new QGridLayout;
    for(int i=0;i<nodes.size();i++){
        Node node;
        node.name = nodes[i].toObject()["name"].toString();
        node.ip = nodes[i].toObject()["ip"].toString();
        node.password = nodes[i].toObject()["password"].toString();
        node.type = nodes[i].toObject()["type"].toString();
        node.wallet = nodes[i].toObject()["wallet"].toString();
        node.infuraId = nodes[i].toObject()["infura_id"].toString();
        node.vultr_api = nodes[i].toObject()["vultr_api"].toString();
        node.login = nodes[i].toObject()["vultr_api"].toString();
        mainLayout->addWidget(createNodeGroupBox(node),i,0);
    }
    if (_currentWidget != NULL)
        delete _currentWidget;
    _currentWidget = new QWidget(this);
    this->setCentralWidget(_currentWidget);
    _currentWidget->setLayout(mainLayout);
    resize(sizeHint());
}

void CheckNodesUi::destroyClicked()
{
    QString senderNode = sender()->objectName();
    QString nodeID;
    for(int i=0;i<_nodes.size();i++){
        if(_nodes[i].toObject()["name"].toString()==senderNode){
            nodeID = _nodes[i].toObject()["id"].toString();
            _nodes.removeAt(i);
            break;
        }
    }
    _checkWorker->destroyNode(_nodes, nodeID);
    setUi(_nodes);
}

void CheckNodesUi::checkClicked()
{

    QString senderNode = sender()->objectName();

    Node node;
    for(int i=0;i<_nodes.size();i++){
        if(_nodes[i].toObject()["name"].toString()==senderNode){
            node.id = _nodes[i].toObject()["id"].toString();
            node.password = _nodes[i].toObject()["password"].toString();
            node.ip = _nodes[i].toObject()["ip"].toString();
            node.type = _nodes[i].toObject()["type"].toString();
            node.name = _nodes[i].toObject()["name"].toString();
            node.login = _nodes[i].toObject()["login"].toString();
            break;
        }
    }
    _checkWorker->checkNodeState(node);
}

void CheckNodesUi::deleteClicked()
{
    QString senderNode = sender()->objectName();
    QString nodeID;
    for(int i=0;i<_nodes.size();i++){
        if(_nodes[i].toObject()["name"].toString()==senderNode){
            nodeID = _nodes[i].toObject()["id"].toString();
            _nodes.removeAt(i);
            break;
        }
    }
    _checkWorker->deleteNode(_nodes);
    setUi(_nodes);
}

void CheckNodesUi::checkingStateFinished(QString name, int peers)
{
    if(peers>0)
        for(int i=0;i<this->centralWidget()->layout()->count();i++){
            QGroupBox* gb = (QGroupBox*)this->centralWidget()->layout()->itemAt(i)->widget();
            if (gb->objectName()=="node"+name){
                QGridLayout* infoLayout = (QGridLayout*)gb->layout()->itemAt(0)->layout();
                QPlainTextEdit* text = (QPlainTextEdit*)infoLayout->itemAt(0)->widget();
                text->appendPlainText("---------------------\n -Status OK. Connected peers: "+QString::number(peers));
            }
        }
    else
        for(int i=0;i<this->centralWidget()->layout()->count();i++){
            QGroupBox* gb = (QGroupBox*)this->centralWidget()->layout()->itemAt(i)->widget();
            if (gb->objectName()=="node"+name){
                QGridLayout* infoLayout = (QGridLayout*)gb->layout()->itemAt(0)->layout();
                QPlainTextEdit* text = (QPlainTextEdit*)infoLayout->itemAt(0)->widget();
                text->appendPlainText("---------------------\n -Status FAIL. Connected peers: 0. If this is new node, wait 5 mins before checking status");
            }
        }
}

QGroupBox* CheckNodesUi::createNodeGroupBox(Node node)
{
    QGroupBox* nodeGroupBox = new QGroupBox(tr(QString("Node %1").arg(node.name).toUtf8()),this);
    qDebug()<<nodeGroupBox;
    nodeGroupBox->setObjectName("node"+node.name);
    qDebug()<<nodeGroupBox->objectName();
    QGridLayout* nodeLayout = new QGridLayout();
    QGridLayout* buttonsLayout = new QGridLayout();
    QGridLayout* infoLayout = new QGridLayout();
    qDebug()<<infoLayout;
    QPlainTextEdit* text = new QPlainTextEdit(this);
    text->setLineWrapMode(QPlainTextEdit::WidgetWidth);
    text->appendPlainText(QString("IP: %1 \n").arg(node.ip));
    text->appendPlainText(QString("Login: %1 \n").arg(node.login));
    text->appendPlainText(QString("Password: %1 \n").arg(node.password));
    text->appendPlainText(QString("Type: %1 \n").arg(node.type));
    text->appendPlainText(QString("Wallet: %1 \n").arg(node.wallet));
    text->setFixedWidth(351);
    text->setFixedHeight(170);
    qDebug()<<node.name<<node.isCustomVPS<<node.vultr_api;
    infoLayout->addWidget(text,0,0);
    QPushButton* checkBtn = new QPushButton("Check State", this);
    checkBtn->setObjectName(node.name);
    connect(checkBtn, &QPushButton::clicked, this, &CheckNodesUi::checkClicked);
    buttonsLayout->addWidget(checkBtn,0,0);
    QPushButton* delBtn = new QPushButton("Delete", this);
    delBtn->setObjectName(node.name);
    connect(delBtn, &QPushButton::clicked, this, &CheckNodesUi::deleteClicked);
    buttonsLayout->addWidget(delBtn,0,1);
    if (node.vultr_api != "" && node.id != ""){
        QPushButton* destroyBtn = new QPushButton("Destroy VPS", this);
        destroyBtn->setObjectName(node.name);
        connect(destroyBtn, &QPushButton::clicked, this, &CheckNodesUi::destroyClicked);
        buttonsLayout->addWidget(destroyBtn,0,2);
    }
    nodeLayout->addLayout(infoLayout, 0, 0);
    nodeLayout->addLayout(buttonsLayout, 1, 0);
    nodeGroupBox->setLayout(nodeLayout);
    return nodeGroupBox;
}
