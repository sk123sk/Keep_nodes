#include "CreateNodeWorker.h"
#include <QDebug>
#include <QThread>
#include <QFile>
#include <QTextStream>


CreateNodeWorker::CreateNodeWorker(): _isNodeStatusActive(0)
{

}

CreateNodeWorker::CreateNodeWorker(QString wallet, QString pwd, QString keyStore, QString infuraId, QString nodeLabel, QString nodeType, QString api)
{
    _currentNode.wallet=wallet;
    _keystore_pwd=pwd;
    _keystore=keyStore;
    _currentNode.infuraId=infuraId;
    _currentNode.name=nodeLabel;
    _currentNode.type=nodeType;
    _apiKey=api;
}


void CreateNodeWorker::deployNode()
{
    _networkManager=new QNetworkAccessManager;
    qDebug()<<"Worker started. Deploying..";
    QUrl reqUrl =  QUrl("https://api.vultr.com/v1/server/create");
    QNetworkRequest _request=QNetworkRequest(reqUrl);
    _request.setRawHeader("API-Key", _apiKey.toUtf8());
    _query.addQueryItem("DCID", "5");
    _query.addQueryItem("VPSPLANID", "203");
    _query.addQueryItem("OSID", "270");
    _query.addQueryItem("label", _currentNode.name.toUtf8());
    QNetworkReply* reply = _networkManager->post(_request, _query.toString(QUrl::FullyEncoded).toUtf8());
    connect(reply, SIGNAL(finished()),this, SLOT(deployReply()));
}

void CreateNodeWorker::checkDeployStatus()
{
    QUrl reqUrl =  QUrl("https://api.vultr.com/v1/server/list");
    QNetworkRequest _request=QNetworkRequest(reqUrl);
    _request.setRawHeader("API-Key", _apiKey.toUtf8());
    QNetworkReply* reply = _networkManager->get(_request);
    connect(reply, SIGNAL(finished()),this, SLOT(statusCheckReply()));
}

void CreateNodeWorker::chillkatUnlock()
{
    qDebug()<<"unlock..";
    CkGlobal glob;
    bool success = glob.UnlockBundle("free unlock");
    if (success != true) {
        qDebug()<<"Chilkat unlock UNsuccess:"<<glob.lastErrorText();
        emit updateWorkStatus("-Chilkat unlock UNsuccess:"+QString(glob.lastErrorText()));
        return;
    }
    int status = glob.get_UnlockStatus();
    if (status == 2) {
        qDebug()<<"Unlocked using purchased unlock code.";
    }
    else {
        qDebug()<<"Unlocked in trial mode";
    }
}

void CreateNodeWorker::runSSH()
{
    int port = 22;
    success = ssh.Connect(_currentNode.ip.toUtf8(),port);
    if (success != true) {
        std::cout << ssh.lastErrorText() << "\r\n";
        qDebug()<<ssh.lastErrorText();
        emit updateWorkStatus("-SSH erorr"+QString(ssh.lastErrorText()));
        return;
    }
    //  Authenticate using login/password:
    success = ssh.AuthenticatePw("root",_currentNode.password.toUtf8());
    if (success != true) {
        qDebug()<<ssh.lastErrorText();
        std::cout << ssh.lastErrorText() << "\r\n";
        emit updateWorkStatus("-SSH pass error!" +QString(ssh.lastErrorText()));
        return;
    }
    emit updateWorkStatus("-SSH connection success! Installing...");
    QList<QString> commandsSetUp;
    QThread::currentThread()->msleep(15000);
    commandsSetUp.push_back("sudo apt update && sudo apt upgrade -y");
    commandsSetUp.push_back("yes | sudo apt install git");
    commandsSetUp.push_back("sudo apt install docker.io curl -y");
    commandsSetUp.push_back("sudo systemctl start docker");
    commandsSetUp.push_back("sudo systemctl enable docker");
    commandsSetUp.push_back("sudo ufw allow 22 && sudo ufw allow 3919 && sudo ufw allow 3920 && yes | sudo ufw enable");
    commandsSetUp.push_back("sudo ufw status");
    commandsSetUp.push_back("git clone https://github.com/icohigh/keep-nodes.git");
    commandsSetUp.push_back(QString("echo '%1' >> $HOME/keep-nodes/data/eth-address.txt").arg(_currentNode.wallet));
    commandsSetUp.push_back(QString("echo '%1' >> $HOME/keep-nodes/data/eth-address-pass.txt").arg(_keystore_pwd));
    commandsSetUp.push_back(QString("echo '%1' >> $HOME/keep-nodes/data/keep_wallet.json").arg(_keystore));
    commandsSetUp.push_back("echo 'export ETH_PASSWORD=$(cat $HOME/keep-nodes/data/eth-address-pass.txt)' >> $HOME/.profile");
    commandsSetUp.push_back("echo 'export SERVER_IP=$(curl ifconfig.co)' >> $HOME/.profile");
    if (_currentNode.type == "BEACON")
        commandsSetUp.push_back(QString("grep -rl INFURA_BEACON_ID $HOME/keep-nodes/beacon/config* | xargs perl -p -i -e 's/INFURA_BEACON_ID/%1/g'").arg(_currentNode.infuraId));
    else
        commandsSetUp.push_back(QString("grep -rl INFURA_ECDSA_ID $HOME/keep-nodes/ecdsa/config* | xargs perl -p -i -e 's/INFURA_ECDSA_ID/%1/g'").arg(_currentNode.infuraId));
    commandsSetUp.push_back("export KEEP_ETHEREUM_PASSWORD=$(cat $HOME/keep-nodes/data/eth-address-pass.txt)");
    setupNode(commandsSetUp);
    emit updateWorkStatus("-Starting keep node...");
    startNode();
}

void CreateNodeWorker::setupNode(QList<QString> commands)
{
    foreach(QString cmd, commands){
        QThread::currentThread()->msleep(1000);
        int channel1 = ssh.QuickCmdSend(cmd.toUtf8());
        if (channel1 < 0) {
            qDebug()<< ssh.lastErrorText() << "\r\n";
            return;
        }
        int pollTimeoutMs = 1000;
        int numFinished = 0;
        while (numFinished < 1) {
            int channel = ssh.QuickCmdCheck(pollTimeoutMs);
            if (channel == -2) {
                qDebug()<<channel<<"erorr:"<< ssh.lastErrorText();
                return;
            }
            if (channel >= 0) {
                qDebug()<<"---- channel "<<channel<<" finished ----";
                qDebug()<<"result:"<<ssh.getReceivedText(channel,"ansi");
                numFinished = numFinished + 1;
            }
        }
    }
    emit updateWorkStatus("\n -Configuration has been done. \n Starting...");
}

void CreateNodeWorker::startNode()
{
    qDebug()<<"Starting node..";
    QList<QString> commands;
    if(_currentNode.type=="BEACON")
        commands.push_back(QString("sudo docker run -d --entrypoint /usr/local/bin/keep-client --restart always --volume $HOME/keep-nodes/data:/mnt/data --volume $HOME/keep-nodes/beacon/config:/mnt/beacon/config --volume $HOME/keep-nodes/beacon/persistence:/mnt/beacon/persistence -e  KEEP_ETHEREUM_PASSWORD='%1' -e LOG_LEVEL=debug --name keep-client -p 3919:3919 keepnetwork/keep-client:v1.3.0-rc.4 --config /mnt/beacon/config/config.toml start").arg(_keystore_pwd).toUtf8());
    else
        commands.push_back(QString("sudo docker run -d --entrypoint /usr/local/bin/keep-ecdsa --restart always --volume $HOME/keep-nodes/data:/mnt/data --volume $HOME/keep-nodes/ecdsa/config:/mnt/ecdsa/config --volume $HOME/keep-nodes/ecdsa/persistence:/mnt/ecdsa/persistence -e KEEP_ETHEREUM_PASSWORD='%1' -e LOG_LEVEL=debug --name keep-ecdsa -p 3920:3919 keepnetwork/keep-ecdsa-client:v1.2.0-rc.5 --config /mnt/ecdsa/config/config.toml start").arg(_keystore_pwd).toUtf8());
    foreach(QString cmd, commands){
        QThread::currentThread()->msleep(2000);
        int channel1 = ssh.QuickCmdSend(cmd.toUtf8());
        if (channel1 < 0) {
            qDebug()<< ssh.lastErrorText() << "\r\n";
            return;
        }
        int pollTimeoutMs = 30000;
        int numFinished = 0;
        while (numFinished < 1) {
            int channel = ssh.QuickCmdCheck(pollTimeoutMs);
            if (channel == -2) {
                qDebug()<<channel<<"erorr:"<< ssh.lastErrorText();
                return;
            }
            if (channel >= 0) {
                qDebug()<<"---- channel "<<channel<<" finished ----";
                qDebug()<<"result:"<<ssh.getReceivedText(channel,"ansi");
                numFinished = numFinished + 1;
            }
        }
    }
    emit updateWorkStatus(QString("\n CONGRATULATIONS! KEEP NODE IS UP! \n  In case you want to manage it use PuTTy app and these credentials:\n \n IP: %1\n Login: root \n Password: %2 \n \n To see full logs in PuTTY use command \n %3").arg(_currentNode.ip).arg(_currentNode.password).arg(_currentNode.type=="BEACON"?"sudo docker logs keep-client -f --since 1m":"sudo docker logs keep-ecdsa -f --since 1m"));
    saveNodeInfo();
}

void CreateNodeWorker::deployReply()
{
    QNetworkReply* reply = (QNetworkReply*) sender();
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data=reply->readAll();
        _currentNode.id = QJsonDocument::fromJson(data).object()["SUBID"].toString();
        qDebug()<<"NODE ID:"<<_currentNode.id;
        emit updateWorkStatus(QString("\n -Deploying Started. Node ID:%1 \n -Awaiting node active status... ").arg(_currentNode.id));
        _timer = new QTimer();
        connect(_timer, SIGNAL(timeout()), this, SLOT (checkDeployStatus()));
        _timer->start(5000);
        _timerTimePassed =new QTimer();
        connect(_timerTimePassed, &QTimer::timeout, this,  [=](){
            _timePassed++;
        });
        _timePassed=0;
        _timerTimePassed->start(1000);
    }
    else {
        qDebug()<<" Recieving error:"<<QString(reply->readAll())<<" error: "+reply->errorString()<<endl;
    }
}

void CreateNodeWorker::statusCheckReply()
{
    QNetworkReply* reply = (QNetworkReply*) sender();
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data=reply->readAll();
        QString status=QJsonDocument::fromJson(data).object()[_currentNode.id].toObject()["power_status"].toString();
        if (status=="running" && _timePassed>30){
            _isNodeStatusActive=true;
            _timer->stop();
            _timer->deleteLater();
            _currentNode.ip = QJsonDocument::fromJson(data).object()[_currentNode.id].toObject()["main_ip"].toString();
            _currentNode.password = QJsonDocument::fromJson(data).object()[_currentNode.id].toObject()["default_password"].toString();
            emit updateWorkStatus(QString("\n -Node is running. IP:"+_currentNode.ip+" Updating configuration..."));
            chillkatUnlock();
            runSSH();
        }
    }
    else {
        qDebug()<<" Recieving error:"<<QString(reply->readAll())<<" error: "+reply->errorString()<<endl;
    }
}

void CreateNodeWorker::saveNodeInfo()
{
    QJsonArray nodes;
    QFile fileToRead("nodes.dat");
    if (!fileToRead.open(QIODevice::ReadOnly | QIODevice::Text))
        qDebug()<<"No saved nodes detected";
    else {
        QByteArray data = fileToRead.readAll();
        fileToRead.close();
        nodes = QJsonDocument::fromJson(data).array();
    }
    QJsonObject node;
    node.insert("name",_currentNode.name);
    node.insert("ip",_currentNode.ip);
    node.insert("password",_currentNode.password);
    node.insert("id",_currentNode.id);
    node.insert("type",_currentNode.type);
    node.insert("wallet",_currentNode.wallet);
    node.insert("infura_id",_currentNode.infuraId);
    nodes.push_back(node);
    QFile file("nodes.dat");
    if (!file.open(QIODevice::WriteOnly  | QIODevice::Text)) {
        qDebug()<<"file error";
        return;
    }
    file.write(QJsonDocument(nodes).toJson());
    file.close();
}
