#include "CreatePasswordDialog.h"
#include "ui_CreatePasswordDialog.h"
#include <QMessageBox>

CreatePasswordDialog::CreatePasswordDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CreatePasswordDialog)
{
    ui->setupUi(this);
    ui->le_confirm->setEchoMode(QLineEdit::Password);
    ui->le_pass->setEchoMode(QLineEdit::Password);

}

CreatePasswordDialog::~CreatePasswordDialog()
{
    delete ui;
}

void CreatePasswordDialog::on_buttonBox_accepted()
{
if (this->ui->le_pass->text()=="")
   QMessageBox::information(this,"Input incorrect","password is empty");
else if (this->ui->le_pass->text()!= this->ui->le_confirm->text())
   QMessageBox::information(this,"Input incorrect","Passwords do not match");
else
    emit setPassword(this->ui->le_pass->text());
}
