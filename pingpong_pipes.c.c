#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

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
	
	pid_t p;
	int start = atoi(argv[2]);
	int end = atoi(argv[3]);
	int now = start;
	
	int pidArr[N]; 
	int fd[N][2]; 
	
	for (int i = 0; i < N; i++)
	{
		if (pipe(fd[i]) == -1)
		{
			fprintf(stderr, "can not use pipe!\n");
			return 1;
		}
	}
	pidArr[0] = getpid();
	for (int i = 0; i < N - 1; i++)
	{
		if ((p = fork()) == -1)
		{
			fprintf(stderr, "can not use fork!\n");
			return 1;
		}
		int id = getpid();
		pidArr[id - pidArr[0]] = id;
		if (p == 0) // чтобы не создать внуков
			break; 
	}
	if (p > 0) // father - initialization
	{
		close(fd[0][0]);
		for (int i = 1; i < N; i++)
        {
			if (i != N - 1)
				close(fd[i][0]);
            close(fd[i][1]);
        }
		write(fd[0][1], &start, sizeof(int));	
	
	}
	if (pidArr[0] - getpid() == 0) // father - fd[0][1] - write, fd[N - 1][0] - read
	{
		close(fd[N - 1][1]);
		while (read(fd[N - 1][0], &now, sizeof(int)) > 0)
		{
			if (now <= end)
				printf("curr N: %d, pID: %d\n", now, getpid());
			now++;
			if (now >= end)
			{
				close(fd[0][0]);
				write(fd[0][1], &now, sizeof(int));
				close(fd[0][1]); // !!!!
				break;
				
			}
			else if (now < end)
			{
				close(fd[0][0]);
				write(fd[0][1], &now, sizeof(int));
			}
		}
		wait(0);
	
	}
	else // sons - fd[pidId][1] - write, fd[pidId - 1][0] - read
	{
		int chInd = getpid() - pidArr[0];
		for (int i = 0; i < N; i++)
        {
            if (i != chInd && i != chInd - 1)
            {
                close(fd[i][0]);
                close(fd[i][1]);
            }
        }
		close(fd[chInd - 1][1]);
		while (read(fd[chInd - 1][0], &now, sizeof(int)) > 0)
		{
			if (now <= end)
				printf("curr N: %d, pID: %d\n", now, getpid());
			now++;
			if (now >= end)
			{
				close(fd[chInd][0]);
				write(fd[chInd][1], &now, sizeof(int));
				close(fd[chInd][1]); // !!
				exit(0);	
			}
			else if (now < end)
			{
				close(fd[chInd][0]);
				write(fd[chInd][1], &now, sizeof(int));
			}
		}
	}
	for (int i = 0; i < N; i++)
	{
		close(fd[i][0]);
		close(fd[i][1]);
	}
	if (p > 0)
    {   
        for (int i = 0; i < N - 1; i++)
            wait(0); 
        exit(0);
    } 

	
	return 0;
}















