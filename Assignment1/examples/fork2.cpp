#include <stdio.h>   /* printf, stderr, fprintf */
#include <unistd.h>  /* _exit, fork */
#include <stdlib.h>  /* exit */
#include <errno.h>   /* errno */

int main()
{      
          int PID;
          int *data = new int;
          *data = 10; 
          for (int i = 0; i < 2; i++) //creates 3 children process
          {
              PID = fork();
          }
          *data = *data + 1;

          if (PID == 0) {
             printf("The is a parent. I've created a process with PID %d\n", PID);
          } else {
             printf("This is a child  process\n");
          }

          printf("my PID is :%d \n", getpid());
          printf("my parent PID is :%d \n", getppid());
          
          printf("data = %d\n", *data);
          return 0;
}
