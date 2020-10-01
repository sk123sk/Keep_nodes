#ifndef STEP_2_H
#define STEP_2_H

#include <QMainWindow>
#include "ui/Step_3.h"

namespace Ui {
class Step_2;
}

class Step_2 : public QMainWindow
{
    Q_OBJECT

signals:
    void setData(QString, QString);
    void backBtnClicked();

public:
    explicit Step_2(QWidget *parent = nullptr);
    ~Step_2();

private slots:
    void on_btn_next_clicked();
    void on_btn_back_clicked();

private:
    Ui::Step_2 *ui;
    bool _next;
};

#endif // STEP_2_H
