#include <errno.h>   /* errno */
#include <iostream>	 /* cout */
#include <stdio.h>   /* printf, stderr, fprintf */
#include <stdlib.h>  /* exit */
#include <unistd.h>  /* _exit, fork */

using namespace std;

int main(int argc, char* argv[])
{      
	// Make sure we have 2 arguments
	if (argc != 2)
	{
		cout << "Usage: " << argv[0]  << " N" << endl;
		return -1;
	}
	
	// Parse the command line argument into an integer
	int n = atoi(argv[1]);	
	cout << "n: " << n << endl;
	
	// Create 3 child processes
	int processId = -1;
	for (int ii = 0; ii < 3 && processId != 0; ii++)
	{
		processId = fork();
	}
	
	// Print infomation about thread
	for (int ii = 0; ii < n; ii++)
	{
		// Parent
		if (processId != 0)
		{
			cout << "Main process. Process ID: " << getpid() << endl;
		}
		
		// Child
		else
		{
			cout << "Child process. Process ID: " << getpid() << 
			". Parent process ID: " << getppid() << endl;
		}
		
		// Wait 2 seconds
		sleep(2);
	}
	
	return 0;
}
 
