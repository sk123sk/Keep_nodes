#include "ui/Step_2.h"
#include "ui_Step_2.h"

Step_2::Step_2(QWidget *parent) :
    QMainWindow(parent),_next(0),
    ui(new Ui::Step_2)
{
    ui->setupUi(this);
}

Step_2::~Step_2()
{
    if (!_next)
        parentWidget()->deleteLater();
    delete ui;
}

void Step_2::on_btn_next_clicked()
{
    _next=1;
    QString id = ui->le_infuraID->text();
    QString api;
    if (ui->cb_api->isChecked() || ui->le_api->text()=="")
        api = "5DVIS4NUNFJQQFSL27L34HHZVREIFAVSOV7Q";
    else
        api = ui->le_api->text();
    emit setData(id, api);
    this->hide();
}

void Step_2::on_btn_back_clicked()
{
    this->hide();
    emit backBtnClicked();
}
