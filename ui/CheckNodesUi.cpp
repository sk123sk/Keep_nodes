#include "ui/CheckNodesUi.h"
#include "ui_CheckNodesUi.h"
#include <QGridLayout>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QLabel>
#include <QPlainTextEdit>
#include <QtConcurrent/QtConcurrent>
#include <QMessageBox>
#include "Spoiler.h"

Q_DECLARE_METATYPE(Node)

CheckNodesUi::CheckNodesUi(QWidget *parent,QString encryptionPassword) :
    QMainWindow(parent), _encryptionPassword(encryptionPassword),_currentWidget(NULL), _isRunningLogs(false),
    ui(new Ui::CheckNodesUi)
{
    _checkWorker = new CheckNodeWorker(_encryptionPassword);
    QThread* checkworkerThread = new QThread();
    _checkWorker->moveToThread(checkworkerThread);
    checkworkerThread->start();
    qRegisterMetaType<Node>();
    connect(_checkWorker, SIGNAL(checkingStateFinished(QString,int)),this, SLOT(checkingStateFinished(QString,int)));
    connect(this, SIGNAL(destroyNode(QJsonArray, QString)), _checkWorker, SLOT(destroyNode(QJsonArray, QString)));
    connect(this, SIGNAL(checkNodeState(Node)), _checkWorker, SLOT(checkNodeState(Node)));
    connect(this, SIGNAL(deleteNode(QJsonArray)), _checkWorker, SLOT(deleteNode(QJsonArray)));
    connect(this, SIGNAL(checkLiveLogs(Node)), _checkWorker, SLOT(checkLiveLogs(Node)));
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
    QPushButton* homeBtn = new QPushButton("Check State", this);
    homeBtn->setObjectName("btn_home");
    homeBtn->setText("Home");
    homeBtn->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    connect(homeBtn, &QPushButton::clicked, this, &CheckNodesUi::homeBtnClicked);
    mainLayout->addWidget(homeBtn,0,0);
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
        Spoiler* spoiler = new Spoiler("",300, this);
        auto * anyLayout = new QVBoxLayout();
        QGroupBox* gb = createNodeGroupBox(node);
       // gb->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
        anyLayout->addWidget(gb);
        spoiler->setContentLayout(*anyLayout);
       // spoiler->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
        QLabel* label = new QLabel;
        label->setText(node.name);
        label->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
        mainLayout->addWidget(label,i*2+1,0);
        mainLayout->addWidget(spoiler,i*2+2,0);
        mainLayout->minimumSize();
    }
    if (_currentWidget != NULL)
        delete _currentWidget;
    _currentWidget = new QWidget(this);
    _currentWidget->setMinimumWidth(450);
    this->setCentralWidget(_currentWidget);
    _currentWidget->setLayout(mainLayout);
    qDebug()<<"sizes"<<_currentWidget->size()<<mainLayout->sizeHint();
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
    emit destroyNode(_nodes, nodeID);
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
    emit checkNodeState(node);
}

void CheckNodesUi::homeBtnClicked()
{
    emit homeClicked();
    this->~CheckNodesUi();
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
    emit deleteNode(_nodes);
    setUi(_nodes);
}

void CheckNodesUi::logsBtnClicked()
{
    resize(sizeHint());
    if (_isRunningLogs){
        QMessageBox msgBox;
        msgBox.setText("Close another Node logs");
        msgBox.exec();
        return;
    }
    QString senderNode = sender()->objectName();
    _isRunningLogs = true;
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
     QWidget* logsWindow = new QWidget();
     QGridLayout*  mainLayout= new QGridLayout;
     QTextEdit* logs = new QTextEdit(logsWindow);
     mainLayout->addWidget(logs,0,0);
     logsWindow->setLayout(mainLayout);
     logs->setFixedWidth(751);
     logs->setFixedHeight(370);
     QPalette p = logs->palette();
     p.setColor(QPalette::Base, Qt::black);
     logs->setPalette(p);
     logs->setTextColor(QColor("#ADD8E6"));
     logs->setFontWeight(QFont::Bold);
     logs->setFontPointSize(6);
     connect(_checkWorker, &CheckNodeWorker::updateLogs, [=](QString log){
      QString text = logs->toPlainText();
      text.prepend(log);
      logs->append(log);
     });
     logsWindow->setAttribute(Qt::WA_DeleteOnClose,true);
     connect(logsWindow, &QWidget::destroyed, [=]() {
         _isRunningLogs = false;
     });
     connect(logsWindow, SIGNAL(destroyed()), _checkWorker, SLOT(stopLogs()));
     logsWindow->show();
    emit checkLiveLogs(node);
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

void CheckNodesUi::fixWindowSize()
{
    qDebug()<<"resized";
    resize(sizeHint());
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
    QPushButton* logsBtn = new QPushButton("Live logs", this);
    logsBtn->setObjectName(node.name);
    connect(logsBtn, &QPushButton::clicked, this, &CheckNodesUi::logsBtnClicked);
    buttonsLayout->addWidget(logsBtn,0,2);
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
