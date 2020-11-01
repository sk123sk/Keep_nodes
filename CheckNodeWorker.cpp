#include "CheckNodeWorker.h"
#include <QFile>
#include <QThread>
#include "Encryptor.h"

CheckNodeWorker::CheckNodeWorker(QString encryptionPassword): _encryptionPassword(encryptionPassword)
{
    _networkManager=new QNetworkAccessManager;
}

void CheckNodeWorker::destroyNode(QJsonArray nodes, QString id)
{
    qDebug()<<"Starting destroy "<<id;
    QUrl reqUrl =  QUrl("https://api.vultr.com/v1/server/destroy");
    QNetworkRequest _request=QNetworkRequest(reqUrl);
    _request.setRawHeader("API-Key", "5DVIS4NUNFJQQFSL27L34HHZVREIFAVSOV7Q");
    _query.addQueryItem("SUBID", id);
    _isReplyFinished=0;
    QNetworkReply* reply = _networkManager->post(_request, _query.toString(QUrl::FullyEncoded).toUtf8());
    connect(reply, SIGNAL(finished()),this, SLOT(destroyReply()));
    saveNodeInfo(nodes);
}

void CheckNodeWorker::checkNodeState(Node node)
{
    int port = 22;
    qDebug()<<"ip:"<<node.ip<<" login:"<<node.login<<" pass:"<<node.password;
    success = ssh.Connect(node.ip.toUtf8(),port);
    if (success != true) {
        std::cout << ssh.lastErrorText() << "\r\n";
        qDebug()<<ssh.lastErrorText();
        return;
    }
    //  Authenticate using login/password:
    success = ssh.AuthenticatePw(node.login.toUtf8(),node.password.toUtf8());
    if (success != true) {
        qDebug()<<ssh.lastErrorText();
        std::cout << ssh.lastErrorText() << "\r\n";
        return;
    }
    QList<QString> commands;
    if (node.type=="BEACON")
        commands.push_back("sudo docker logs  keep-client --tail 100 2>&1 | grep \"number of connected peers:\"");
    else
        commands.push_back("sudo docker logs  keep-ecdsa --tail 100 2>&1 | grep \"number of connected peers:\"");
    foreach(QString cmd, commands){
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
                QString output = ssh.getReceivedText(channel,"ansi");
                int peers = output.mid(output.lastIndexOf("[")+1,output.lastIndexOf("]")-output.lastIndexOf("[")-1).toInt();
                numFinished = numFinished + 1;
                qDebug()<<"emit stateFinish";
                emit checkingStateFinished(node.name, peers);
            }
        }
    }
    qDebug()<<"SSH end";
}

void CheckNodeWorker::deleteNode(QJsonArray nodes)
{
    saveNodeInfo(nodes);
}

void CheckNodeWorker::destroyReply()
{
    qDebug()<<"reply:";
    _isReplyFinished=true;
    QNetworkReply* reply = (QNetworkReply*) sender();
    if (reply->error() == QNetworkReply::NoError){
        qDebug()<<reply->readAll();
        qDebug()<<reply->attribute( QNetworkRequest::HttpStatusCodeAttribute );
    }
}


void CheckNodeWorker::saveNodeInfo(QJsonArray( nodes))
{
    Encryptor enc;
    QFile file("nodes.dat");
    if (!file.open(QIODevice::WriteOnly)) {
        qDebug()<<"file error";
        return;
    }
    QDataStream out(&file);
    out <<enc.encrypt(QJsonDocument(nodes).toJson(), _encryptionPassword.toUtf8());
    file.close();
}
