#include "ui/Step_1.h"
#include "ui_Step_1.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QProcess>

Step_1::Step_1(QWidget *parent) :
    QMainWindow(parent), _next(0),
    ui(new Ui::Step_1)
{
    ui->setupUi(this);
    ui->le_pwd->setEchoMode(QLineEdit::Password);
}

Step_1::~Step_1()
{
    if (!_next)
        parentWidget()->deleteLater();
    delete ui;
}

void Step_1::on_btn_next_clicked()
{        
    _keystore_pwd = ui->le_pwd->text();
    _keystore = readKeystoreFile();
    this->hide();
    emit setData(_keystore_pwd, _keystore);
    getWallet(_keystore, _keystore_pwd);
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

QString Step_1::readKeystoreFile()
{
    QFile file(_path);
    if (!file.exists() || !file.open(QIODevice::ReadOnly))
        QMessageBox::information(this,"Cannot open file","Incorrect path or file unavailable");
    else {
        QByteArray data;
        data=file.readAll();
        file.close();
        return QString(data);
    }
}

void Step_1::getWallet(QString keyStore,QString keystore_pwd)
{
    QProcess* pythonProccess = new QProcess;
    QStringList params;
    params << keyStore;
    params << keystore_pwd;
    QObject::connect(pythonProccess, &QProcess::readyRead, [pythonProccess, this] () {
        QByteArray data = pythonProccess->readAll();
        QString wallet  = QString(data);
        wallet.chop(2);
        qDebug()<<wallet;
        emit setWallet(wallet);
    });
    QObject::connect(pythonProccess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                     [=](int exitCode, QProcess::ExitStatus /*exitStatus*/){
        pythonProccess->deleteLater();
    });
    pythonProccess->start(QDir::currentPath()+"/scripts/getWallet.exe", params);
}

