#include <errno.h>   /* errno */
#include <iostream>	 /* cout */
#include <stdio.h>   /* printf, stderr, fprintf */
#include <stdlib.h>  /* exit */
#include <unistd.h>  /* _exit, fork */
#include <wait.h>    /* waitpid */

using namespace std;

void runTask1();
void runTask2();
void runTask3(char* filename);

int main(int argc, char* argv[])
{      
	// Make sure we have 2 arguments
	if (argc != 2)
	{
		cout << "Usage: " << argv[0]  << " filename" << endl;
		return -1;
	}
	char* filename = argv[1];
	
	// Create 3 child processes
	const int childProcesses = 3;
	int childProcessIds[childProcesses];
	
	int processId = -1;
	int processTask = 0;
	for (int ii = 0; ii < childProcesses && processId != 0; ii++)
	{
		processTask++;
		processId = fork();
		childProcessIds[ii] = processId;
	}
	
	// Parent
	if (processId != 0)
	{
		cout << "Main process. Process ID: " << getpid() << endl;
		
		// Wait for all child processes to finish
		for (int ii = 0; ii < childProcesses; ii++)
		{
			int status;
			waitpid(childProcessIds[ii], &status, 0);
		}
		
		cout << "Main process terminating." << endl;
	}
	
	// Child
	else
	{		
		// Perform the appropriate task
		switch (processTask)
		{
			case 1:
				runTask1();
				break;
				
			case 2:
				runTask2();
				break;
				
			case 3:
				runTask3(filename);
				break;
				
			default:
				break;
		}
		
		cout << "Process " << getpid() << " done." << endl;
	}
	
	return 0;
}

void runTask1()
{
	execl("/bin/ls", "ls", "-l", (char*)0);
}

void runTask2()
{
	execl("/bin/ps", "ps", "-ef", (char*)0);
}

void runTask3(char* filename)
{
	execl("/bin/cat", "cat", filename, (char*)0);
}
