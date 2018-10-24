#include "uartconfig.h"

int uart_open(char* port)
{
    int fd_temp;
    int flags = O_RDWR | O_NOCTTY | O_NONBLOCK;
    fd_temp = open(port, flags);
    if(fd_temp < 0)
        {
            perror("Can't open Serial Port");
            return(-1);
        }
    return fd_temp;
}

void uart_close(int fd)
{
    close(fd);
}

int uart_config(int fd, int bandrate)
{
    struct termios tty;

    if(tcgetattr(fd, &tty) != 0)
        {
            perror("failed setup Serial");
            return(-1);
        }

    /*config bandrate*/
    cfsetispeed(&tty,bandrate);
    cfsetospeed(&tty,bandrate);

    /*config data bits = 8*/
    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;

    /*config parity = none*/
    tty.c_cflag &= ~PARENB;

    /*config stop bits = 1*/
    tty.c_cflag &= ~CSTOPB;

    /* config flow control = none*/
    tty.c_cflag &= ~CRTSCTS;

    /*config control mode*/
    tty.c_cflag |= CLOCAL;
    tty.c_cflag |= CREAD;

    /* config RAW data mode input and output*/
    //tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);  /*Input*/
    tty.c_lflag &= ~(ICANON | ECHOE | ISIG);
    tty.c_oflag &= ~OPOST;   /*Output*/

    /*config wait time and minimal receive characters*/
    tty.c_cc[VTIME] = 100; /* wait time 100*(1/10)s */
    tty.c_cc[VMIN] = 1; /* minimal receive characters = 1 */

    /*data overrun processing*/
    tcflush(fd,TCIFLUSH);

    /* active above configuration*/
    if (tcsetattr(fd, TCSANOW, &tty) != 0)
    {
        fprintf (stderr, "error %d from tcsetattr", errno);
        return -1;
    }
    return(1);
}

int uart_send(int fd, char *buf_send, int buf_len)
{
    int len;
    len = write(fd,buf_send,buf_len);
    if(len == buf_len)
    {
        printf("send successflly\n");
        return len;
    }
    else
    {
        tcflush(fd,TCOFLUSH);
        printf("send error\n");
        return -1;
    }
}

int uart_read(int fd, char *buf_read, int buf_len)
{
    int len,fs_sel;
    fd_set fs_read;
    //struct timeval time;


    FD_ZERO(&fs_read);
    FD_SET(fd,&fs_read);

    //time.tv_sec = 600;
    //time.tv_usec = 0;

    fs_sel = select(fd+1,&fs_read,NULL,NULL,NULL);
    if(fs_sel)
    {
       len = read(fd,buf_read,buf_len);
       return len;
    }
    else
    {
        return -1;
    }
}

int uart_485(int fd)
{
    struct serial_rs485 rs485conf;

    /* Enable RS485 mode: */
    rs485conf.flags |= SER_RS485_ENABLED;

    /* Set logical level for RTS pin equal to 0 when sending: */
    rs485conf.flags |= SER_RS485_RTS_ON_SEND;
    /* or, set logical level for RTS pin equal to 1 when sending: */
    //rs485conf.flags &= ~(SER_RS485_RTS_ON_SEND);

    /* Set logical level for RTS pin equal to 0 after sending: */
    //rs485conf.flags |= SER_RS485_RTS_AFTER_SEND;
    /* or, set logical level for RTS pin equal to 1 after sending: */
    rs485conf.flags &= ~(SER_RS485_RTS_AFTER_SEND);

    /* Set rts delay before send, if needed:(in microseconds) */
    rs485conf.delay_rts_before_send = 100;

    /* Set rts delay after send, if needed: (in microseconds) */
    rs485conf.delay_rts_after_send = 100;

    /* Set this flag if you want to receive data even whilst sending data */
    rs485conf.flags |= SER_RS485_RX_DURING_TX;

    if (ioctl(fd, TIOCSRS485, &rs485conf) < 0) {
        printf("Error: TIOCSRS485 ioctl not supported.\n");
        return -1;
    }
    else
        return 1;
}
