#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <QMessageBox>
//#include <QTimer>
#include <QString>
#include "threadread.h"
#include "uartconfig.h"

namespace Ui {
class MainWindow;
class QLineEdit;
class QLable;
class QPushButton;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:

    void on_DecreaseButton_clicked();
    void on_IncreaseButton_clicked();
    void DateRevUpdate(QString);

private:
    Ui::MainWindow *ui;
    Ui::QLineEdit *uartoutput;
    Ui::QLable *lable;
    Ui::QPushButton *IncreaseButton;
    Ui::QPushButton *DecreaseButton;
    QMessageBox msgBox;
    ThreadRead* ReadThread;

    int len,ch;
    char data_send[1024];
    char uport[20];

};

#endif // MAINWINDOW_H
