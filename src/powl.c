#include "powl.h"
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>

void    olfich(void)
{
    static int  ol = 0;

    if (ol)
        return ;
    switch (fork())
    {
        case -1 :
            perror("fork");
            _exit(1);
        case 0 :
            if (-1 == execlp("bin/esep.exe", "./bin/esep.exe", ".mlgr.qtc", NULL))
            {
                perror("execlp");
                _exit(2);
            }
            _exit(0);
        default :
            ol = 1;
            return ;
    }
}

void    alfich(void)
{
    static int  al = 0;

    if (al)
        return ;
    switch (fork())
    {
        case -1 :
            perror("fork");
            _exit(1);
        case 0 :
            if (-1 == execlp("bin/esep.exe", "./bin/esep.exe", ".jpp.qtc", NULL))
            {
                perror("execlp");
                _exit(2);
            }
            _exit(0);
        default :
            al = 1;
            return ;
    }
}
