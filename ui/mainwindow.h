#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ui/Step_1.h"
#include "ui/Step_2.h"
#include "ui/Step_3.h"
#include "ui/Finalstep.h"
#include "ui/CheckNodesUi.h"
#include <QJsonArray>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
signals:

public slots:
   void setDataStep1(QString wallet, QString pwd, QString path);
   void setDataStep2(QString id, QString api);
   void setDataStep3(int node_type, QString nodeName);
   void backClicked();


private slots:
    void on_btn_Run_new_clicked();
    void readKeyStoreFile();
    QJsonArray readNodesFile();
    void on_btn_check_nodes_clicked();

private:
    enum node_type {
       beacon=0,
       ecdsa
    };
    Ui::MainWindow *ui;
    QString _eth_wallet;
    QString _keystore_pwd;
    QString _keystore_path;
    QString _infura_project_ID;
    QString _node_name;
    QString _keystoreData;
    QString _apiKey;
    node_type _node_type;
    Step_1* _step1;
    Step_2* _step2;
    Step_3* _step3;
    FinalStep* _finalStep;
    CheckNodesUi* _checkNodes;
};
#endif // MAINWINDOW_H
