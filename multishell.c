#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>

/* these should be the same as multishell.c */
#define MY_FILE_SIZE 1024
#define MY_SHARED_FILE_NAME "/sharedlogfile"
#define MAX_SHELL 10
#define DEFAULT_NSHELL 2

char *addr = NULL; 
int fd = -1;       /* fd for shared file object */

int initmem() {
    fd = shm_open(MY_SHARED_FILE_NAME, O_CREAT | O_RDWR | O_TRUNC, 0666);
    if (fd < 0) {
        perror("multishell.c:open file:");
        exit(1);
    }
    if (ftruncate(fd, 1024) == -1) {
        perror("ftruncate");
        exit(1);
    }

    addr = mmap(NULL, MY_FILE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (addr == NULL) {
        perror("mmap:");
        exit(1);
    }
    return 0;
}

int main(int argc, char **argv) {
    int nshell;
    char logfilename[64];
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    initmem();
    pid_t pid[MAX_SHELL];

    /* get number of shells from argument */
    if (argc == 1) {
        nshell = DEFAULT_NSHELL;
    } else {
        nshell = atoi(argv[1]);
        if (nshell > MAX_SHELL) {
            printf("too many shells, set to default: %d\n", DEFAULT_NSHELL);
            nshell = DEFAULT_NSHELL;
        } else if (nshell < 1) {
            printf("invalid number of shells, set to default: %d\n", DEFAULT_NSHELL);
            nshell = DEFAULT_NSHELL;
        }
    }

    for (int i = 0; i < nshell*nshell; i++) {
        if ((pid[i] = fork()) == 0) {
            /* child process */
            char shellname[16];
            sprintf(shellname, "shell%d", i);
            execlp("xterm", "xterm", "-e", "./singleshell", shellname, NULL);
            perror("exec");
            exit(1);
        }
    }
    /* parent process */
    int status;
    for (int i = 0; i < nshell*nshell; i++) {
        waitpid(pid[i], &status, 0);
	printf("shell %d den cikis yapildi. %d\n", (i+1), WEXITSTATUS(status));
      
    }
    /* create logfile */
    sprintf(logfilename, "shelllog-%04d%02d%02d-%02d%02d%02d.txt",
            tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    int logfile_fd = open(logfilename, O_CREAT | O_WRONLY | O_APPEND, 0666);
    if (logfile_fd == -1) {
	    
            perror("open");
            exit(1);
}
    int wfd = write(logfile_fd, addr, strlen(addr));
    if (write(logfile_fd, addr, strlen(addr)) == -1) {
            perror("write");
            exit(1);
}
/* write data in shared memory to logfile */
    fprintf(stdout, "%s\n", addr);

/* close logfile and free mmap */
    if (addr) {
    munmap(addr, MY_FILE_SIZE);
}
if (fd >= 0) {
    close(fd);
    shm_unlink(MY_SHARED_FILE_NAME);
}

    return 0;
}
