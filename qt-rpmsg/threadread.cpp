#include "threadread.h"

ThreadRead::ThreadRead(QObject *parent) :
    QThread(parent)
{

}

void ThreadRead::stopImmediately()
{
    QMutexLocker locker(&thread_lock);
    thread_isCanRun = false;
}
void ThreadRead::run()
{

    thread_isCanRun = true;
    bool fdFlag = true;
    fd = uart_open("/dev/ttyRPMSG");
    if(this->fd < 0)
    {

        fdFlag = false;
        qDebug("uart port input error!");
    }
    else{
            ch = uart_config(fd, B38400);
            if(ch == -1)
            {

                fdFlag = false;
                qDebug("uart setup error!");
            }
         }

    while(1)
    {
        if(!fdFlag)//在每次循环判断是否可以运行，如果不行就退出循环
        {
            qDebug("fd error quit");
            return;
        }

        str = "";
        memset(data_read,0,sizeof(data_read));
        for(int i=0;;i++)
        {
            qDebug("data start to read");
            len = uart_read(fd, data_read, sizeof(data_read));
            if(len > 0)
            {
                data_read[len]='\0';
                //qDebug(data_read);
                for (int j=0;j<this->len;j++)
                {
                    str +=QString(data_read[j]);
                }
            }
            else
            {
                qDebug("can't receive data!");
                break;
            }


            if('\n' == data_read[len-1])
            {
                qDebug("data read finished");
                emit send_Rev_Data(str);
                break;
            }
        }

        QMutexLocker locker(&thread_lock);
        if(!thread_isCanRun)//在每次循环判断是否可以运行，如果不行就退出循环
        {
            qDebug("thread quit");
            uart_close(fd);
            return;
        }

        //msleep(100);
    }
}
