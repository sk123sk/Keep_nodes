#ifndef ADDNODEDIALOG_H
#define ADDNODEDIALOG_H

#include <QDialog>
#include "Node.h"

namespace Ui {
class AddNodeDialog;
}

class AddNodeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddNodeDialog(QWidget *parent = nullptr);
    ~AddNodeDialog();

signals:
    void setNodeData(Node node);

private slots:
    void on_buttonBox_accepted();

private:
    Ui::AddNodeDialog *ui;
};

#endif // ADDNODEDIALOG_H
