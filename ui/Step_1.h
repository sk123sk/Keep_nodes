#ifndef STEP_1_H
#define STEP_1_H

#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QTimer>
#include <QThread>

namespace Ui {
class Step_1;
}

class Step_1 : public QMainWindow
{
    Q_OBJECT

signals:
    void setData(QString,QString,QString);
    void backBtnClicked();

public:
    explicit Step_1(QWidget *parent = nullptr);
    ~Step_1();

private slots:
    void on_btn_next_clicked();
    void on_btn_select_keystore_clicked();
    void on_btn_back_clicked();
    void replyKeepGrantFinished();

private:
    Ui::Step_1 *ui;
    QString _path;
    bool _next;
    bool _keepGrant;
    QNetworkAccessManager* _networkManager;
    QNetworkRequest _request;
    QUrl _reqUrl;
};

#endif // STEP_1_H
