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

// Ham redirect input hay output dua tren reCheck
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

// Ham main
int main()
{
	char *args[MAXLINE / 2 + 1];
	int flag = 1;
	while (flag)
	{
		printf("osh>");
		fflush(stdout);
		// Doc input
		char *input = (char *)malloc(MAXLINE * sizeof(char));
		if (input != NULL)
		{
			gets(input);
		}

		// Neu go exit -> thoat chuong trinh
		if (strcmp(input, "exit") == 0)
		{
			flag = 0;
			break;
		}

		int includeAnd = readInput(args, input);

		if (fork() == 0) // Tien trinh con
		{
			// Kiem tra redirect I/O
			char *fileName = NULL;
			int reCheck = redirectCheck(args, &fileName);
			if (reCheck != 0)
			{
				redirectIO(reCheck, fileName);
			}

			// Goi command voi args da cho
			execvp(args[0], args);
			// Thoat tien trinh con
			exit(0);
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