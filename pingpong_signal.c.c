#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

void mysignal(int sig) 
{
    ;
}

int main(int argc, char **argv) 
{
    if (argc != 4) 
    {
        fprintf(stderr, "wrong number of arguments!\n");
        return 1;
    }

    int N = atoi(argv[1]);

    if (N < 2 || N > 5) 
    {
        fprintf(stderr, "wrong range of N!\n");
        return 1;
    }
    int start = atoi(argv[2]);
    int end = atoi(argv[3]);
    int now = start;
	if (start > end) 
	{
        fprintf(stderr, "wrong end!\n");
        return 1;
    }
    int pidArr[N];
    int fd[2];

    if (pipe(fd) == -1) 
    {
        fprintf(stderr, "can not use pipe!\n");
        return 1;
    }

    signal(SIGUSR1, mysignal);
	int index = -1;
	pidArr[0] = getpid();
	pid_t p;
    for (int i = 1; i < N; i++) 
    {
        if ((p = fork()) == -1) 
        {
            fprintf(stderr, "can not use fork!\n");
            return 1;
        }
        if (p == 0) 
        {
			index = i;
			if (i == N - 1)
			{
				printf("curr N: %d, pID: %d\n", now, getpid());
				now++;
				write(fd[1], &now, sizeof(int));
				usleep(50); 
				kill(pidArr[i - 1], SIGUSR1);
			}
            break; //чтобы не создать внука		
        } 
        pidArr[i] = p; //отец
    }
	if (index > -1)
    {
		usleep(10);
		while (1) 
		{
			pause();
			read(fd[0], &now, sizeof(int));
			if (now > end)
			{
				write(fd[1], &now, sizeof(int)); 
				kill(pidArr[0], SIGUSR1);
				close(fd[0]);
				close(fd[1]);
				exit(0);
			}
			printf("curr N: %d, pID: %d\n", now, getpid());
			now++;
			write(fd[1], &now, sizeof(int)); 
			kill(pidArr[index - 1], SIGUSR1); 
			usleep(10);
		}
    }
    else
    {
		while (1)
		{
			pause();
			read(fd[0], &now, sizeof(int));
			if (now > end)
			{
				for (int i = 1; i < N; i++)
				{
					write(fd[i], &now, sizeof(int));
					kill(pidArr[i], SIGUSR1);
				}
				break;
			}
			printf("curr N: %d, pID: %d\n", now, getpid());
			now++;
			write(fd[1], &now, sizeof(int)); 
			usleep(10);
			kill(pidArr[N - 1], SIGUSR1); 
		}
		for (int i = 1; i < N; i++)
		{
			wait(0);
			close(fd[0]);
            close(fd[1]);
            exit(0);			
		}
	}

    return 0;
}

