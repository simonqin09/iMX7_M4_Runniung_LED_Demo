#ifndef THREADREAD_H
#define THREADREAD_H

#include <QThread>
#include <QMutex>
#include "uartconfig.h"

class ThreadRead : public QThread
{
    Q_OBJECT
public:
    explicit ThreadRead(QObject *parent = 0);
    //ThreadRead();
    void run();
    //void stop();

    int fd;
    //char data_send[1024];
    //char data_read[1024];
    //char uport[20];
    //QString str;

signals:
    void send_Rev_Data(QString msg);

public slots:
    void stopImmediately();
private:
    //int fd,len,ch;
    int len,ch;
    char data_read[1024];
    QString str;
    QMutex thread_lock;
    bool thread_isCanRun;
};

#endif // THREADREAD_H
