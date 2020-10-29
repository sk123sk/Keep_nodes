#ifndef CREATEPASSWORDDIALOG_H
#define CREATEPASSWORDDIALOG_H

#include <QDialog>

namespace Ui {
class CreatePasswordDialog;
}

class CreatePasswordDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CreatePasswordDialog(QWidget *parent = nullptr);
    ~CreatePasswordDialog();
signals:
    void setPassword(QString);
private slots:
    void on_buttonBox_accepted();

private:
    Ui::CreatePasswordDialog *ui;
};

#endif // CREATEPASSWORDDIALOG_H
