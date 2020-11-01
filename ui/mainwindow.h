#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ui/Step_1.h"
#include "ui/Step_2.h"
#include "ui/Step_3.h"
#include "ui/Finalstep.h"
#include "ui/CheckNodesUi.h"
#include <QJsonArray>
#include <QSettings>
#include "Node.h"

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
   void setDataStep1(QString pwd, QString keyStoreData);
   void setDataStep2(QString id, QString api);
   void setDataStep3(int node_type, QString nodeName);
   void setWallet(QString wallet);
   void backClicked();
   void homeClicked();


private slots:
    void on_btn_Run_new_clicked();
    QJsonArray readNodesFile();
    void loadSetting();
    void on_btn_check_nodes_clicked();
    void on_btn_addExisting_clicked();

private:
    enum node_type {
       beacon=0,
       ecdsa
    };
    Ui::MainWindow *ui;
    Node _currentNode;
    QString _keystore_pwd;
    QString _keystore_path;
    QString _keystoreData;
    Step_1* _step1;
    Step_2* _step2;
    Step_3* _step3;
    FinalStep* _finalStep;
    CheckNodesUi* _checkNodes;
    QString _encryptionPassword;
    bool _isLoggedIn;
    QSettings* _settings;
};
#endif // MAINWINDOW_H
