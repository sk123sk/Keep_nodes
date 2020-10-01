#ifndef STEP_3_H
#define STEP_3_H

#include <QMainWindow>

namespace Ui {
class Step_3;
}

class Step_3 : public QMainWindow
{
    Q_OBJECT

signals:
    void setData(int, QString);
    void backBtnClicked();
public:
    explicit Step_3(QWidget *parent = nullptr);
    ~Step_3();

private slots:
    void on_btn_next_clicked();
    void on_btn_back_clicked();

private:
    Ui::Step_3 *ui;
    bool _next;
};

#endif // STEP_3_H
