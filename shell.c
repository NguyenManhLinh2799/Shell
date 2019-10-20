#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#define MAXLINE 80

// Ham tra ve so tham so dong lenh trong args
int argsCount(char *a[])
{
	int i;
	for (i = 0; a[i] != NULL; i++)
		;
	return i;
}

// Ham tach chuoi tu input va gan vao mang a
// tra ve 1 neu xuat hien dau & o cuoi dong lenh, nguoc lai tra ve 0
int readInput(char *a[], char *input)
{
	char delim[2] = " ";
	char *token = strtok(input, delim);
	int i = 0;
	while (token != NULL)
	{
		a[i] = (char *)malloc(strlen(token) * sizeof(char) + 1);
		if (a[i] != NULL)
		{
			strcpy(a[i], token);
		}
		token = strtok(NULL, delim);
		i++;
	}

	// Kiem tra va xoa dau &
	if (strcmp(a[i - 1], "&") == 0)
	{
		free(a[i - 1]);
		a[i - 1] = NULL;
		return 1;
	}
	else
	{
		a[i] = NULL;
		return 0;
	}
}

// Ham kiem tra dong lenh co yeu cau redirect input va output khong
// tra ve 0 neu khong co yeu cau
// tra ve 1 neu la output (>)
// tra ve 2 neu la input (<)
// ten file duoc su dung se luu vao fileName
int redirectCheck(char *a[], char **fileName)
{
	int n = argsCount(a);
	int flag = 0;
	if (n >= 3 && (strcmp(a[n - 2], ">") == 0 || strcmp(a[n - 2], "<") == 0))
	{
		free(*fileName);
		*fileName = (char *)malloc(strlen(a[n - 1]) * sizeof(char) + 1);
		if (*fileName != NULL)
		{
			strcpy(*fileName, a[n - 1]);
		}

		if (strcmp(a[n - 2], ">") == 0)
		{
			flag = 1;
		}
		else
		{
			flag = 2;
		}

		// Xoa ten file va redirect mark
		free(a[n - 1]);
		free(a[n - 2]);
		a[n - 2] = NULL;
	}
	return flag;
}

// Ham redirect input hoac output dua tren reCheck
void redirectIO(int reCheck, char *fileName)
{
	int fd;
	if (reCheck == 1)
	{
		fd = open(fileName, O_WRONLY | O_APPEND);
		dup2(fd, STDOUT_FILENO);
	}
	else if (reCheck == 2)
	{
		fd = open(fileName, O_RDONLY);
		dup2(fd, STDIN_FILENO);
	}
}

// Ham thuc thi command trong tien trinh con, co kiem tra redirect
void executeChild(char *args[])
{
	// Kiem tra redirect I/O
	char *fileName = NULL;
	int reCheck = redirectCheck(args, &fileName);
	if (reCheck != 0)
	{
		redirectIO(reCheck, fileName);
	}

	execvp(args[0], args);
}

// Ham kiem tra co pipe character (|) trong command khong
// tra ve vi tri cua pipe character neu co, nguoc lai tra ve 0
int pipeCheck(char *a[])
{
	int n = argsCount(a);
	if (n >= 3)
	{
		int i;
		for (i = 1; i < n; i++)
		{
			if (strcmp(a[i], "|") == 0)
			{
				return i;
			}
		}
	}
	return 0;
}

// Ham tach command thanh 2 command dua vao vi tri của pipe character
void separate(char *a[], char *a1[], char *a2[], int pipePos)
{
	int n = argsCount(a);
	int i;
	int j = 0;
	for (i = 0; i < pipePos; i++)
	{
		a1[j++] = a[i];
	}
	a1[j] = NULL;
	j = 0;
	for (i = pipePos + 1; i < n; i++)
	{
		a2[j++] = a[i];
	}
	a2[j] = NULL;
}

// Ham tao pipe giua hai tien trinh
void pipeCreate(char *a1[], char *a2[])
{
	int p[2];
	pipe(p);

	if (fork() == 0)
	{
		dup2(p[1], STDOUT_FILENO);
		close(p[0]);
		executeChild(a1);
	}
	else
	{
		int n3 = fork();
		if (n3 == 0)
		{
			dup2(p[0], STDIN_FILENO);
			close(p[1]);
			executeChild(a2);
		}
		else
		{
			close(p[1]);
			wait(NULL);
		}
	}
	close(p[0]);
	wait(NULL);
	exit(0);
}

// Ham main
int main()
{
	char *args[MAXLINE / 2 + 1];
	char *history = NULL;

	while (1)
	{
		printf("osh>");
		fflush(stdout);
		// Nhap command
		char *input = (char *)malloc(MAXLINE * sizeof(char));
		if (input != NULL)
		{
			gets(input);
		}

		// Neu go exit -> thoat chuong trinh
		if (strcmp(input, "exit") == 0)
		{
			break;
		}

		// Kiem tra lenh !!
		if (strcmp(input, "!!") == 0)
		{
			if (history == NULL) // khong co command trong history
			{
				printf("No commands in history\n");
				continue;
			}
			else
			{
				strcpy(input, history);
			}
		}
		else // Neu khong phai lenh !! thi copy input vao history
		{
			free(history);
			history = (char *)malloc(strlen(input) * sizeof(char) + 1);
			strcpy(history, input);
		}

		// Doc input
		int includeAnd = readInput(args, input);

		if (fork() == 0) // Tien trinh con
		{
			// Kiem tra pipe character
			int pipePos = pipeCheck(args);
			// Neu co thi tach args thanh 2 args
			// tao pipe va tien trinh con khac
			if (pipePos != 0)
			{
				char *argsChild1[MAXLINE / 2 + 1];
				char *argsChild2[MAXLINE / 2 + 1];
				separate(args, argsChild1, argsChild2, pipePos);
				pipeCreate(argsChild1, argsChild2);
			}
			else // Neu la command binh thuong -> thuc thi voi args da cho
			{
				executeChild(args);
			}
		}
		else // Tien trinh cha
		{
			if (!includeAnd) // Neu khong co dau & thi wait
			{
				wait(NULL);
			}
		}
	}
	return 0;
}