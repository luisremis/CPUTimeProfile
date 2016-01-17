#include "userapp.h"
#include <fcntl.h>
#include "stdio.h"

#define DURATION_SEC 8

int main(int argc, char* argv[])
{

    int fd;
    int pid;
    unsigned int loop, loop2;
    int duration;

    pid = getpid(); // Take the process id

    printf("New Procees PID: %d \n", pid); // Print process id for reference

    fd = open("/proc/mp1/status", O_RDWR );// Open the status file

    write(fd, &pid, 4); // Writes process id. 4 bytes because the pid is a I32 data type. 

    /* The application starts doing some job, until 
    killed by the test.sh script file. 
    We choosed also this way to make sure the program is always running and we 
    have control over the userapp from the test.sh file */
    while(1);

/*
    for(loop = 0; loop < 100000; loop = loop + 2)
        for(loop2 = 0; loop2 < 10000; ++loop2)
            duration++;
*/
/*
    for (duration = 0; duration < DURATION_SEC; ++duration)
    {
        system("sleep 1");
    }
//*/

    return 0;
}
