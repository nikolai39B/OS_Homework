/*
 * Will Hauber
 * Homework 4
 * November 29, 2016
 */

#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

int main(int argc, char* argv[]) 
{ 
	// Check that we have the correct arguments
	if (argc != 3)
	{
		printf("Usage: ./a.out programName textFile\n");
		return 1;
	}
	
	char* programName = argv[1];
	char* fileName = argv[2];
	
	// Fork a child process
	if (fork() == 0)
	{
		// Open the file and redirect output to that file
		int fd = open(fileName, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
		dup2(fd, 1);
		close(fd);
		
		// Execute the program
		execl(programName, programName, (char*) NULL);
	}
	
	return 0;
}
