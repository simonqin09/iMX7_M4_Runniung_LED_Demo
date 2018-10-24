#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->uartoutput->setText("Interval=100");

    ReadThread = new ThreadRead(this);
    QObject::connect(ReadThread,SIGNAL(send_Rev_Data(QString)),this,SLOT(DateRevUpdate(QString)));

    if(ReadThread->isRunning())
    {
        return;
    }
    ReadThread->start();
}

MainWindow::~MainWindow()
{
    ReadThread->stopImmediately();//由于此线程的父对象是Widget，因此退出时需要进行判断
    ReadThread->wait();
    delete ui;
}


void MainWindow::on_DecreaseButton_clicked()
{
    /*capture uartoutput content QString and change to char[]*/
    this->len = sprintf(this->data_send, ui->uartoutput->text().toLocal8Bit().constData());
    /*judge if uartoutput lineedit is blank*/
    if(this->len == 0)
    {
        /*config default message for sending once lineedit is blank*/
        //this->len = sprintf(this->data_send,"this is a test program");
        this->len = sprintf(this->data_send,"Interval=100");
        this->data_send[this->len]='\0';
        /*popup message box to choose*/
        msgBox.setText("Current interval is blank, default interval 100ms");
        //msgBox.setInformativeText("this is a test program");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::Yes);
        if(msgBox.exec() == QMessageBox::Yes)
        {
            /*send default message*/
            this->ch = uart_send(ReadThread->fd,this->data_send,this->len);
            if(this->ch == -1)
            {
                QMessageBox::warning(NULL, "warning", "send error!");
            }
            ui->label->setText("send default message successfully!");
        }
    }
    else
    {
        /*send input message*/
        if(this->len != -1)
        {
            //this->data_send.section('=',1,1)
            QString dataSendNew = QString(QLatin1String(this->data_send));
            dataSendNew = dataSendNew.section('=',1,1);
            //qDebug("%s",qUtf8Printable(dataSendNew));
            int dataSendInt = dataSendNew.toInt();
            if (dataSendInt >100)
            {
                dataSendInt = dataSendInt - 100;
                dataSendNew = "Interval=" + QString::number(dataSendInt);
                //qDebug("%s",qUtf8Printable(dataSendNew));
                this->len = sprintf(this->data_send, dataSendNew.toLocal8Bit().constData());
                //this->data_send[this->len]='\0';
                //qDebug(this->data_send);
            }
            else
            {
                this->len = sprintf(this->data_send,"Interval=100");
                this->data_send[this->len]='\0';
                //qDebug(this->data_send);
            }
            this->ch = uart_send(ReadThread->fd,this->data_send,this->len);
            if(this->ch == -1)
            {
                QMessageBox::warning(NULL, "warning", "send error!");
            }
            ui->label->setText("send input message successfully!");
        }
        else
            QMessageBox::warning(NULL, "warning", "error set data for sending!");
    }
}

void MainWindow::on_IncreaseButton_clicked()
{

    /*capture uartoutput content QString and change to char[]*/
    this->len = sprintf(this->data_send, ui->uartoutput->text().toLocal8Bit().constData());
    /*judge if uartoutput lineedit is blank*/
    if(this->len == 0)
    {
        /*config default message for sending once lineedit is blank*/
        //this->len = sprintf(this->data_send,"this is a test program");
        this->len = sprintf(this->data_send,"Interval=100");
        this->data_send[this->len]='\0';
        /*popup message box to choose*/
        msgBox.setText("Current interval is blank, default interval 100ms");
        //msgBox.setInformativeText("this is a test program");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::Yes);
        if(msgBox.exec() == QMessageBox::Yes)
        {
            /*send default message*/
            this->ch = uart_send(ReadThread->fd,this->data_send,this->len);
            if(this->ch == -1)
            {
                QMessageBox::warning(NULL, "warning", "send error!");
            }
            ui->label->setText("send default message successfully!");
        }
    }
    else
    {
        /*send input message*/
        if(this->len != -1)
        {
            //this->data_send.section('=',1,1)
            QString dataSendNew = QString(QLatin1String(this->data_send));
            dataSendNew = dataSendNew.section('=',1,1);
            //qDebug("%s",qUtf8Printable(dataSendNew));
            int dataSendInt = dataSendNew.toInt();
            if (dataSendInt <1000)
            {
                dataSendInt = dataSendInt + 100;
                dataSendNew = "Interval=" + QString::number(dataSendInt);
                //qDebug("%s",qUtf8Printable(dataSendNew));
                this->len = sprintf(this->data_send, dataSendNew.toLocal8Bit().constData());
                //this->data_send[this->len]='\0';
                //qDebug(this->data_send);
            }
            else
            {
                this->len = sprintf(this->data_send,"Interval=100");
                this->data_send[this->len]='\0';
                //qDebug(this->data_send);
            }
            this->ch = uart_send(ReadThread->fd,this->data_send,this->len);
            if(this->ch == -1)
            {
                QMessageBox::warning(NULL, "warning", "send error!");
            }
            ui->label->setText("send input message successfully!");
        }
        else
            QMessageBox::warning(NULL, "warning", "error set data for sending!");
    }
}


void MainWindow::DateRevUpdate(QString msg)
{
    ui->uartoutput->setText(msg);
    qDebug("date updated");
}
