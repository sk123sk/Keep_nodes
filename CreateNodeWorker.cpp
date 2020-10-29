#include "CreateNodeWorker.h"
#include <QDebug>
#include <QThread>
#include <QFile>
#include <QDataStream>
#include "Encryptor.h"
#include "QProcess"
#include "QDir"
#include <QMessageBox>

CreateNodeWorker::CreateNodeWorker(Node node, QString pwd, QString keyStore, QString encryptionPassword)
{
    _currentNode = node;
    _keystore_pwd=pwd;
    _keystore=keyStore;
    _encryptionPassword=encryptionPassword;
    _widget=new QWidget;
}


void CreateNodeWorker::deployNode()
{
    if(_currentNode.isCustomVPS)
        runSSH();
    else {
        _networkManager=new QNetworkAccessManager;
        updateWorkStatus("Renting VPS server...");
        QUrl reqUrl =  QUrl("https://api.vultr.com/v1/server/create");
        QNetworkRequest _request=QNetworkRequest(reqUrl);
        _request.setRawHeader("API-Key", _currentNode.vultr_api.toUtf8());
        _query.addQueryItem("DCID", "5");
        _query.addQueryItem("VPSPLANID", "203");
        _query.addQueryItem("OSID", "270");
        _query.addQueryItem("label", _currentNode.name.toUtf8());
        QNetworkReply* reply = _networkManager->post(_request, _query.toString(QUrl::FullyEncoded).toUtf8());
        connect(reply, SIGNAL(finished()),this, SLOT(deployReply()));
    }
}

void CreateNodeWorker::checkDeployStatus()
{
    QUrl reqUrl =  QUrl("https://api.vultr.com/v1/server/list");
    QNetworkRequest _request=QNetworkRequest(reqUrl);
    _request.setRawHeader("API-Key", _currentNode.vultr_api.toUtf8());
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
        emit updateWorkStatus("-- FATAL error! Chilkat unlock problem");
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
    chillkatUnlock();
    int port = 22;
    success = ssh.Connect(_currentNode.ip.toUtf8(),port);
    if (success != true) {
        std::cout << ssh.lastErrorText() << "\r\n";
        qDebug()<<ssh.lastErrorText();
        emit updateWorkStatus("--FATAL error! SSH connection problem"+QString(ssh.lastErrorText()));
        return;
    }
    //  Authenticate using login/password:
    success = ssh.AuthenticatePw("root",_currentNode.password.toUtf8());
    if (success != true) {
        qDebug()<<ssh.lastErrorText();
        std::cout << ssh.lastErrorText() << "\r\n";
        emit updateWorkStatus("--FATAL error! SSH auth problem" +QString(ssh.lastErrorText()));
        return;
    }
    emit updateWorkStatus("--ssh connected!");
    emit updateWorkStatus("--installing node...");
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
    emit updateWorkStatus("--starting keep node...");
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
    emit updateWorkStatus("--node installation finished");
}

void CreateNodeWorker::startNode()
{
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
    emit updateWorkStatus("\n Congratulations! Keep node is UP! \n");
    emit updateWorkStatus(QString("Node credentials:<br>IP: %1<br> Login: root <br> Password: %2 <br> ..... <br> To see full logs in PuTTY use command <br> %3").arg(_currentNode.ip).arg(_currentNode.password).arg(_currentNode.type=="BEACON"?"sudo docker logs keep-client -f --since 1m":"sudo docker logs keep-ecdsa -f --since 1m"));
    saveNodeInfo();
}

void CreateNodeWorker::deployReply()
{
    QNetworkReply* reply = (QNetworkReply*) sender();
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data=reply->readAll();
        _currentNode.id = QJsonDocument::fromJson(data).object()["SUBID"].toString();
        emit updateWorkStatus(QString("\n --deploying started. VPS ID:%1 <br> --awaiting VPS active status... ").arg(_currentNode.id));
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
            emit updateWorkStatus(QString("\n --VPS is running. IP:"+_currentNode.ip+"<br> --updating VPS configuration..."));
            runSSH();
        }
    }
    else {
        qDebug()<<" Recieving error:"<<QString(reply->readAll())<<" error: "+reply->errorString()<<endl;
    }
}

void CreateNodeWorker::saveNodeInfo()
{
    qDebug()<<"savenig new node";
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
    node.insert("name",_currentNode.name);
    node.insert("ip",_currentNode.ip);
    node.insert("password",_currentNode.password);
    node.insert("id",_currentNode.id);
    node.insert("type",_currentNode.type);
    node.insert("wallet",_currentNode.wallet);
    node.insert("infura_id",_currentNode.infuraId);
    node.insert("isCustomVPS",_currentNode.isCustomVPS);
    node.insert("vultr_api",_currentNode.vultr_api);
    qDebug()<<node;
    nodes.push_back(node);
    qDebug()<<nodes;
    QFile file("nodes.dat");
    if (!file.open(QIODevice::WriteOnly)) {
        qDebug()<<"file error";
        return;
    }
    QDataStream out(&file);
    out <<enc.encrypt(QJsonDocument(nodes).toJson(), _encryptionPassword.toUtf8());
    file.close();
}

void CreateNodeWorker::get_eth_balance()
{
    QProcess* pythonProccess = new QProcess;
    QStringList params;
    params << _currentNode.wallet;
    params << _currentNode.infuraId;
    QObject::connect(pythonProccess, &QProcess::readyRead, [pythonProccess, this] () {
        QByteArray data = pythonProccess->readAll();
        QString balance  = QString(data);
        balance.chop(2);
        qDebug()<<"wallet balance:"<<balance;
        if (balance.toDouble()<1)
            updateWorkStatus(QString("Wallet balance is %1 ETH. You need at least 1 testnet ETH to continue. Try again when ready. FATAL").arg(balance));

        else
            getGrant();
    });
    QObject::connect(pythonProccess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                     [=](int exitCode, QProcess::ExitStatus /*exitStatus*/){
        pythonProccess->deleteLater();
    });
    pythonProccess->start(QDir::currentPath()+"/scripts/get_eth_balance.exe", params);
}

void CreateNodeWorker::getGrant()
{
    _networkManager = new QNetworkAccessManager();
    QNetworkRequest request = QNetworkRequest();
    request.setUrl(QUrl(QString("https://us-central1-keep-test-f3e0.cloudfunctions.net/keep-faucet-ropsten?account=%1").arg(_currentNode.wallet)));
    QNetworkReply* reply = _networkManager->get(request);
    connect(reply, SIGNAL(finished()),this, SLOT(replyKeepGrantFinished()));
}

void CreateNodeWorker::replyKeepGrantFinished()
{
    QNetworkReply* reply = (QNetworkReply*) sender();
    if (reply->error()) {
        qDebug() << reply->errorString();
        if(QString(reply->readAll()).contains("maximum testnet KEEP")){
            updateWorkStatus("--wallet already got 300k KEEP grant...");
            delegateKeep();
            return;
        }
        else {
            qDebug()<<"Keep 300k tokens grant claiming error. Try again!"+reply->readAll();
            getGrant();
            return;
        }
    }
    QString grantStatus = reply->readAll();
    if (grantStatus.contains("Created")){
        qDebug()<<"--claimed 300k KEEP grant";
        updateWorkStatus("--successfully claimed 300k KEEP grant");
    }
    delegateKeep();
}


void CreateNodeWorker::delegateKeep()
{
    updateWorkStatus("--delegating KEEP...");
    QProcess* pythonProccess = new QProcess;
    QStringList params;
    params << _keystore;
    params << _keystore_pwd;
    params << _currentNode.infuraId;
    QObject::connect(pythonProccess, &QProcess::readyRead, [pythonProccess, this] () {
        QByteArray data = pythonProccess->readAll();
        qDebug() <<  data;
        if (QString(data).contains("transaction confirmed")) {
            updateWorkStatus("--delegated KEEP");
            beacon_auth();
        }
        else {
            updateWorkStatus("--FATAL ERROR!"+QString(data).remove("\r\n"));
            return ;
        }
    });
    QObject::connect(pythonProccess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                     [=](int exitCode, QProcess::ExitStatus /*exitStatus*/){
        pythonProccess->deleteLater();
    });
    pythonProccess->start(QDir::currentPath()+"/scripts/delegateKeep.exe", params);
    pythonProccess->waitForFinished(-1);
}

void CreateNodeWorker::beacon_auth()
{
    updateWorkStatus("--authorizing Operator Contract...");
    QProcess* pythonProccess = new QProcess;
    QStringList params;
    params << _keystore;
    params << _keystore_pwd;
    params << _currentNode.infuraId;
    QObject::connect(pythonProccess, &QProcess::readyRead, [pythonProccess, this] () {
        QByteArray data = pythonProccess->readAll();
        qDebug() <<  data;
        if (QString(data).contains("transaction confirmed")) {
            updateWorkStatus("--authorized Operator Contract");
            ecdsa_tbtc_auth();
            return;
        }
        else {
            updateWorkStatus("--FATAL ERROR! Authorizing Operator Contract failed!");
            return ;
        }
    });
    QObject::connect(pythonProccess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                     [=](int exitCode, QProcess::ExitStatus /*exitStatus*/){
        pythonProccess->deleteLater();
    });
    pythonProccess->start(QDir::currentPath()+"/scripts/beaconAuth.exe", params);
    pythonProccess->waitForFinished(-1);
}

void CreateNodeWorker::ecdsa_tbtc_auth()
{
    updateWorkStatus("--authorizing tBTC contracts...");
    QProcess* pythonProccess = new QProcess;
    QStringList params;
    params << _keystore;
    params << _keystore_pwd;
    params << _currentNode.infuraId;
    QObject::connect(pythonProccess, &QProcess::readyRead, [pythonProccess, this] () {
        QByteArray data = pythonProccess->readAll();
        qDebug() <<  data;
        if (QString(data).contains("transaction confirmed operator") && QString(data).contains("transaction confirmed pool")) {
            updateWorkStatus("--authorized Operator Contract (ECDSA)");
            updateWorkStatus("--authorized Sortition Pool Contract (ECDSA)");
            bond_eth(0.1);
            return;
        }
        else {
            if (QString(data).contains("authOperatorContract failed"))
                updateWorkStatus("--FATAL ERROR! Authorizing Operator Contract (ECDSA) failed!"+QString(data).remove("\r\n"));
            else
                updateWorkStatus("--FATAL ERROR! Authorizing Sortition Pool Contract (ECDSA)"+QString(data).remove("\r\n"));
            return ;
        }
    });
    QObject::connect(pythonProccess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                     [=](int exitCode, QProcess::ExitStatus /*exitStatus*/){
        pythonProccess->deleteLater();
    });
    pythonProccess->start(QDir::currentPath()+"/scripts/ecdsa_tbtc_auth.exe", params);
    pythonProccess->waitForFinished(-1);
}

void CreateNodeWorker::bond_eth(double amount)
{
    updateWorkStatus("--bonding ETH...");
    QProcess* pythonProccess = new QProcess;
    QStringList params;
    params << _keystore;
    params << _keystore_pwd;
    params << _currentNode.infuraId;
    params << QString::number(amount, 'f', 2);
    QObject::connect(pythonProccess, &QProcess::readyRead, [pythonProccess, amount, this] () {
        QByteArray data = pythonProccess->readAll();
        qDebug() <<  data;
        if (QString(data).contains("transaction confirmed")) {
            updateWorkStatus(QString("--successfully bonded %1 ETH").arg(amount));
            deployNode();
            return;
        }
        else {
            updateWorkStatus("--FATAL ERROR!Bonding ETH failed!");
            return ;
        }
    });
    QObject::connect(pythonProccess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                     [=](int exitCode, QProcess::ExitStatus /*exitStatus*/){
        pythonProccess->deleteLater();
    });
    pythonProccess->start(QDir::currentPath()+"/scripts/bond_eth.exe", params);
    pythonProccess->waitForFinished(-1);
}
