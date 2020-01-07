//
// Created by moqi on 2019/10/25.
//

#include "tty.h"
//#include <stdio.h>
//#include <time.h>
#include <fcntl.h>
#include <unistd.h>
//#include <stdlib.h>
#include <termios.h>
#define ECHOFLAGS (ECHO|ECHOE|ECHOK|ECHONL|ICANON)

static struct termios back;
static int seted = 0;
int  tty_set_noblock()
{
    struct termios ter;
    int flag;

    if (seted) {
        return 0;
    }

    if (tcgetattr(STDIN_FILENO, &back) == -1) {
        return -1;
    }

    ter = back;
    ter.c_lflag &= ~ECHOFLAGS;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &ter) == -1) {
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &back);
        return -1;
    }

    flag = fcntl(STDIN_FILENO, F_GETFL);
    flag |= O_NONBLOCK;

    if (fcntl(STDIN_FILENO, F_SETFL, flag) == -1) {
        return -1;
    }

    seted = 1;
    return 0;
}
void tty_reset()
{
    if (seted) {
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &back);
        seted = 0;
    }

    return;
}