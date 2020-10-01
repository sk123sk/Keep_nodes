#include "ui/Step_1.h"
#include "ui_Step_1.h"
#include <QFileDialog>
#include <QMessageBox>

Step_1::Step_1(QWidget *parent) :
    QMainWindow(parent), _next(0),_keepGrant(0),
    ui(new Ui::Step_1)
{
    ui->setupUi(this);
}

Step_1::~Step_1()
{
    if (!_next)
        parentWidget()->deleteLater();
    delete ui;
}

void Step_1::on_btn_next_clicked()
{
    _next=1;
    QString wallet = ui->le_wallet->text();
    QString pwd = ui->le_pwd->text();
    _networkManager = new QNetworkAccessManager();
    _request.setUrl(QUrl(QString("https://us-central1-keep-test-f3e0.cloudfunctions.net/keep-faucet-ropsten?account=%1").arg(wallet)));
    QNetworkReply* reply = _networkManager->get(_request);
    connect(reply, SIGNAL(finished()),this, SLOT(replyKeepGrantFinished()));
}

void Step_1::on_btn_select_keystore_clicked()
{
    _path = QFileDialog::getOpenFileName(this, QString("Keystore file..."),
                                         QString(_path.isEmpty()?"./":_path));
    ui->l_path_name->setText(_path);
}

void Step_1::on_btn_back_clicked()
{
    this->hide();
    emit backBtnClicked();
}

void Step_1::replyKeepGrantFinished()
{
    QNetworkReply* reply = (QNetworkReply*) sender();
    if (reply->error()) {
        qDebug() << reply->errorString();
        if(QString(reply->readAll()).contains("maximum testnet KEEP")){
            qDebug()<<"Already got 300k";
            _keepGrant=true;
        }
        else {
             qDebug()<<"Keep 300 tokens claim error. Try again. "+reply->readAll();
        return;
        }
    }
    QString answer = reply->readAll();
    qDebug() << answer;
    if (answer.contains("Created")){
        qDebug()<<"Successfully got 300k KEEP(test)";
        _keepGrant = true;
    }
    this->hide();
    QString wallet = ui->le_wallet->text();
    QString pwd = ui->le_pwd->text();
    emit setData(wallet, pwd, _path);
}
