#include "ui/CheckNodesUi.h"
#include "ui_CheckNodesUi.h"
#include <QGridLayout>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QLabel>
#include <QPlainTextEdit>
#include <QtConcurrent/QtConcurrent>

CheckNodesUi::CheckNodesUi(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::CheckNodesUi)
{
    _checkWorker = new CheckNodeWorker();
     connect(_checkWorker, SIGNAL(checkingStateFinished(QString,int)),this, SLOT(checkingStateFinished(QString,int)));
}

CheckNodesUi::~CheckNodesUi()
{
    delete ui;
}

void CheckNodesUi::setUi(QJsonArray nodes)
{
    _nodes=nodes;
    QGridLayout*  mainLayout= new QGridLayout;
    for(int i=0;i<nodes.size();i++){
        Node node;
        node.name = nodes[i].toObject()["name"].toString();
        node.ip = nodes[i].toObject()["ip"].toString();
        node.password = nodes[i].toObject()["password"].toString();
        node.type = nodes[i].toObject()["type"].toString();
        node.wallet = nodes[i].toObject()["wallet"].toString();
        node.infuraId = nodes[i].toObject()["infura_id"].toString();
        mainLayout->addWidget(createNodeGroupBox(node),i,0);
    }
    QWidget* widget = new QWidget(this);
    this->setCentralWidget(widget);
    widget->setLayout(mainLayout);
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
    CheckNodeWorker::Node node;
    for(int i=0;i<_nodes.size();i++){
        if(_nodes[i].toObject()["name"].toString()==senderNode){
            node.id = _nodes[i].toObject()["id"].toString();
            node.password = _nodes[i].toObject()["password"].toString();
            node.ip = _nodes[i].toObject()["ip"].toString();
            node.type = _nodes[i].toObject()["type"].toString();
            node.name = _nodes[i].toObject()["name"].toString();
            break;
        }
    }
    _checkWorker->checkNodeState(node);
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
    text->setReadOnly(true);
    text->setLineWrapMode(QPlainTextEdit::NoWrap);
    text->appendPlainText(QString("IP: %1").arg(node.ip));
    text->appendPlainText(QString("Password: %1").arg(node.password));
    text->appendPlainText(QString("Type: %1").arg(node.type));
    text->appendPlainText(QString("Wallet: %1").arg(node.wallet));
    text->appendPlainText(QString("Infura ID: %1").arg(node.infuraId));
    text->setFixedWidth(text->document()->size().toSize().width()+3);
    text->setFixedHeight(text->document()->size().toSize().height()*20+70);
    infoLayout->addWidget(text,0,0);
    QPushButton* destroyBtn = new QPushButton("Destroy", this);
    QPushButton* checkBtn = new QPushButton("Check State", this);
    destroyBtn->setObjectName(node.name);
    checkBtn->setObjectName(node.name);
    connect(destroyBtn, &QPushButton::clicked, this, &CheckNodesUi::destroyClicked);
    connect(checkBtn, &QPushButton::clicked, this, &CheckNodesUi::checkClicked);
    buttonsLayout->addWidget(destroyBtn,0,0);
    buttonsLayout->addWidget(checkBtn,0,1);
    nodeLayout->addLayout(infoLayout, 0, 0);
    nodeLayout->addLayout(buttonsLayout, 1, 0);
    nodeGroupBox->setLayout(nodeLayout);
    return nodeGroupBox;
}