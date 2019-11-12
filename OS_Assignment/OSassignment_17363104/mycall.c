#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>

#define _SYSCALL_MYCALL_ 326

int main()
{
    syscall(_SYSCALL_MYCALL_);
    return 0;
}
