#include "AddNodeDialog.h"
#include "ui_AddNodeDialog.h"
#include "Encryptor.h"
#include "QMessageBox"
#include <QDebug>

AddNodeDialog::AddNodeDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddNodeDialog)
{
    ui->setupUi(this);
}

AddNodeDialog::~AddNodeDialog()
{
    delete ui;
}

void AddNodeDialog::on_buttonBox_accepted()
{
    Node node;
    node.ip=ui->le_ip->text();
    node.login=ui->le_login->text();
    node.password=ui->le_pass->text();
    node.type=ui->cb_node_type->currentIndex()==0?"BEACON":"ECSDA";
    node.name=ui->le_name->text();
    node.wallet=ui->le_wallet->text();
    if(ui->le_vultrAPI->text()!="" && ui->le_vultrAPI->text()!="(optional)")
        node.vultr_api=ui->le_vultrAPI->text();
    else
        node.vultr_api="";
    qDebug()<<node.vultr_api;
    emit setNodeData(node);
    QMessageBox::information(this,"Success","Node has been added!");
}
