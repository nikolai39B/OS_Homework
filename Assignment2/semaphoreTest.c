#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/times.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/wait.h> 

//-----------------//
// Semaphore Stuff //
//-----------------//
// A semaphore value of 0 means that the data is not being used
// A semaphore value > 1 is the number of threads that are reading the data
// A semaphore value of -1 means that a thread is writing the data
int semId = -1;
struct sembuf sem_lock = { 0, -1, SEM_UNDO };
struct sembuf sem_unlock = { 0, 1, SEM_UNDO };

#define READERS_COUNT_SIZE 1
char* sharedReadersCount;

union semun
{
	int val;
	struct semid_ds *buf;
	ushort *array;
} arg;

//--------------//
// Reader Stuff //
//--------------//
int readerID = 0;
void reader()
{
	// Increment the number of readers
	int res = semop(semId, &sem_lock, 1);
	// Make sure that no one is writing currently
	while (atoi(sharedReadersCount) == -1)
	{
		// Wait and try later
		semop(semId, &sem_unlock, 1);
		sleep(1);
		semop(semId, &sem_lock, 1);
	}
	sprintf(sharedReadersCount, "%d", atoi(sharedReadersCount) + 1);
	printf("reader count: %s\n", sharedReadersCount);
	res = semop(semId, &sem_unlock, 1);
	
	// Read
	printf("reading\n");
	sleep(3);
	printf("read\n");
	
	// Decrement the number of readers
	res = semop(semId, &sem_lock, 1);
	sprintf(sharedReadersCount, "%d", atoi(sharedReadersCount) - 1);
	printf("reader count: %s\n", sharedReadersCount);
	res = semop(semId, &sem_unlock, 1);
}

void create_reader()
{
    if (0 == fork()) 
    {
        reader();
        exit(0);
    }
    readerID++;
}

//--------------//
// Writer Stuff //
//--------------//
int writerID = 0;
void writer()
{
	// Set the reader count to -1
	int res = semop(semId, &sem_lock, 1);
	// Make sure that no one is using the data currently
	while (atoi(sharedReadersCount) != 0)
	{
		// Wait and try later
		semop(semId, &sem_unlock, 1);
		sleep(1);
		semop(semId, &sem_lock, 1);
	}
	sprintf(sharedReadersCount, "%d", -1);
	printf("reader count: %s\n", sharedReadersCount);
	res = semop(semId, &sem_unlock, 1);
	
	printf("writing\n");
	sleep(2);
	printf("written\n");
	res = semop(semId, &sem_unlock, 1);
	
	// Set the reader count to 0
	res = semop(semId, &sem_lock, 1);
	sprintf(sharedReadersCount, "%d", 0);
	printf("reader count: %s\n", sharedReadersCount);
	res = semop(semId, &sem_unlock, 1);
}

void create_writer()
{
    if (0 == fork()) 
    {
        writer();
        exit(0);
    }
    writerID++;
}
 
//------//
// Main //
//------//
int main(int argc, char* argv[]) 
{ 
	// Create and init semaphore
	semId = semget(IPC_PRIVATE, 1, IPC_CREAT | 0600);
	semun init;
	init.val = 1;
	int res = semctl(semId, 0, SETVAL, init);
	if (res == -1)
	{
		perror("failed: ");
	}
	
	
	// Create a shared readers count
	int readersCountFd = open("readersCount.dat", O_RDWR | O_CREAT | O_TRUNC, 0600);
	if (readersCountFd < 0)
	{
		perror("readersCount.dat ");
		exit(1);
	}
	
	char* readersCountInitialData = (char*)"0\n";
	write(readersCountFd, readersCountInitialData, READERS_COUNT_SIZE);
	unlink("readersCount.dat");
	
	sharedReadersCount = (char*)mmap(0, READERS_COUNT_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, readersCountFd, 0);
	if (sharedReadersCount == (caddr_t)-1)
	{
		perror("mmap");
		exit(2);
	}
	
	printf("%d\n", atoi(sharedReadersCount));
	
	// Fork processes
	for (int ii = 0; ii < 5; ii++)
	{
		//create_reader();
		create_writer();
		create_writer();
	}
	
	// Wait for children
	for (int ii = 0; ii < readerID + writerID; ii++)
	{
		wait(NULL);
	}
	  
	return 0;
	
	/* Get the value of the semaphore
	int value = semctl(semId, 0, GETVAL, 0);
	if (res == -1)
	{
		perror("failed: ");
	}
	printf("%d\n", value);*/
}

