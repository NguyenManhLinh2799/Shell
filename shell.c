#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#define MAXLINE 80

// Ham tach chuoi tu input va gan vao mang a
// tra ve 1 neu xuat hien dau & o cuoi dong lenh
int readInput(char* a[], char* input)
{
	char delim[2] = " ";
	char* token = strtok(input, delim);
	int i = 0;
	while (token != NULL)
	{
		a[i] = (char*)malloc(strlen(token) * sizeof(char) + 1);
		if (a[i] != NULL)
		{
			strcpy(a[i], token);
		}
		token = strtok(NULL, delim);
		i++;
	}

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

int main()
{
    char *args[MAXLINE/2 + 1];
    int flag = 1;
    while (flag)
    {
        printf("osh>");
        fflush(stdout);
        // Doc input
        char* input = (char*)malloc(MAXLINE * sizeof(char));
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
            execvp(args[0], args);
			return 0;
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