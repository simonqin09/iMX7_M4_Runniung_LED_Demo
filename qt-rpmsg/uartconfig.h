#ifndef UARTCONFIG_H
#define UARTCONFIG_H

#include <termios.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <linux/serial.h>
#include <asm-generic/ioctls.h> /* TIOCGRS485 + TIOCSRS485 ioctl definitions */
#include <sys/ioctl.h>

#define TIOCGRS485      0x542E
#define TIOCSRS485      0x542F

int uart_open(char* port);
void uart_close(int fd);
int uart_config(int fd, int bandrate);
int uart_send(int fd, char *buf_send, int buf_len);
int uart_read(int fd, char *buf_read, int buf_len);
int uart_485(int fd);


#endif // UARTCONFIG_H
