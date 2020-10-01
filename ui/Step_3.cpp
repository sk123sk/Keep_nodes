#include "ui/Step_3.h"
#include "ui_Step_3.h"

Step_3::Step_3(QWidget *parent) :
    QMainWindow(parent),_next(0),
    ui(new Ui::Step_3)
{
    ui->setupUi(this);
}

Step_3::~Step_3()
{
    if (!_next)
        parentWidget()->deleteLater();
    delete ui;
}

void Step_3::on_btn_next_clicked()
{
    _next=1;
    QString name = ui->le_nodeName->text();
    int type = ui->cb_node_type->currentIndex();
    emit setData(type, name);
    this->hide();
}

void Step_3::on_btn_back_clicked()
{
    this->hide();
    emit backBtnClicked();
}
