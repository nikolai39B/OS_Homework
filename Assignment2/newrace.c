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

#define FILE_SIZE 11
#define NO_PROC   10

int DelayCount = 0;

int readerID = 0;
int writerID = 0;

char* shared_buffer;

int Delay100ms = 0;

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

/*-------------------------------------------
    Delay routines
 --------------------------------------------*/
void basic_delay()
{
   long i,j,k;
   for (i=0;i<200L;i++) {
       for (j=0;j<400L;j++) {
          k = k+i;
       }
   }
}

/* do some delays (in 100 ms/tick) */
void delay(int delay_time)
{
   int i,j;

   for (i=0; i<delay_time; i++) {
      for (j=0;j<Delay100ms;j++) {
          basic_delay();
      }
   }
}


int stop_alarm = 0;

static void
sig_alrm(int signo)
{
    stop_alarm = 1;
}


/*------------------------------------------
 * Since the speed of differet systems vary,
 * we need to calculate the delay factor 
 *------------------------------------------
 */
void calcuate_delay()
{
    int i;
    struct  tms t1;
    struct  tms t2;
    clock_t t;
    long    clktck;
    double  td;

    printf(".... Calculating delay factor ......\n");
    stop_alarm = 0;
    if (signal(SIGALRM, sig_alrm) == SIG_ERR)
       perror("Set SIGALRM");
    alarm(5);  /* stop the following loop after 5 seconds */

    times(&t1);
    while (stop_alarm == 0) {
        DelayCount++;
        basic_delay();
    }    
    times(&t2);
    alarm(0);   /* turn off the timer */

    /* Calcluate CPU time */
    t = t2.tms_utime - t1.tms_utime; 

    /* fetch clock ticks per second */
    if ( (clktck = sysconf(_SC_CLK_TCK)) < 0 )
       perror("sysconf error");

    /* actual delay in seconds */
    td = t / (double)clktck;
    
    Delay100ms = DelayCount/td/10;

    if (Delay100ms == 0)
       Delay100ms++;

    printf(".... End calculating delay factor\n");
}




/*-------------------------------------------
   The reader
 --------------------------------------------*/
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
    fflush(stdout);
	res = semop(semId, &sem_unlock, 1);
	
  int i,j,n;
  char results[FILE_SIZE];

  srand(2);
  for (i=0; i<1; i++) {
      printf("Reader %d (pid = %d) arrives\n", readerID, getpid()); 

      /* read data from shared data */
      for (j=0; j<FILE_SIZE; j++) {
         results[j] = shared_buffer[j]; 
         delay(4);  
      }
 
      /* display result */
      results[j] = 0;
      printf("      Reader %d gets results = %s\n", 
              readerID, results);
  }
  
    fflush(stdout);
	
	// Decrement the number of readers
	res = semop(semId, &sem_lock, 1);
	sprintf(sharedReadersCount, "%d", atoi(sharedReadersCount) - 1);
	printf("reader count: %s\n", sharedReadersCount);
    fflush(stdout);
	res = semop(semId, &sem_unlock, 1);
}



/*-------------------------------------------
   The writer. It tries to fill the buffer
   repeatly with the same digit 
 --------------------------------------------*/
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
	printf("before update reader count: %s\n", sharedReadersCount);
	sprintf(sharedReadersCount, "%d", -1);
	printf("after update reader count: %s\n", sharedReadersCount);
    fflush(stdout);
	res = semop(semId, &sem_unlock, 1);

	
  int i,j,n;
  char data[FILE_SIZE];

  srand(1);

  for (j=0; j<FILE_SIZE-1; j++) {
      data[j]= writerID + '0';
  }
  data[j]= 0;

  for (i=0; i<1; i++) {
      printf("Writer %d (pid = %d) arrives, writing %s to buffer\n", 
              writerID, getpid(), data);

      /* write to shared buffer */
      for (j=0; j<FILE_SIZE-1; j++) {
          shared_buffer[j]= data[j]; 
          delay(3);  
      }

      printf("Writer %d finishes\n", writerID);
  }
  
    fflush(stdout);
	
	// Set the reader count to 0
	res = semop(semId, &sem_lock, 1);
	sprintf(sharedReadersCount, "%d", 0);
	printf("reader count: %s\n", sharedReadersCount);
    fflush(stdout);
	res = semop(semId, &sem_unlock, 1);
}


/*-------------------------------------------

      Routines for creating readers and writers

*-------------------------------------------*/
void create_reader()
{
    if (0 == fork()) {
        reader();
        exit(0);
    }

    readerID++;
}

void create_writer()
{
    if (0 == fork()) {
        writer();
        exit(0);
    }

    writerID++;
}



/*-------------------------------------------
 --------------------------------------------*/
int main() 
{ 
  int return_value;
  char InitData[]="0000000000\n";
  int i;
  int fd;

  calcuate_delay();

  /*-------------------------------------------------------
   
       The following code segment creates a memory
     region shared by all child processes
       If you can't completely understand the code, don't
     worry. You don't have to understand the detail
     of mmap() to finish the homework
  
  -------------------------------------------------------*/ 

  fd = open("race.dat", O_RDWR | O_CREAT | O_TRUNC, 0600);
  if ( fd < 0 ) {
     perror("race.dat ");
     exit(1);
  }

  write(fd, InitData, FILE_SIZE);

  unlink("race.dat");

  shared_buffer = (char*)mmap(0, FILE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if ( shared_buffer == (caddr_t) -1) {
     perror("mmap");
     exit(2);
  }
  
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
	
	//printf("%d\n", atoi(sharedReadersCount));

  /*------------------------------------------------------- 
  
       Create some readers and writes (processes)
  
  -------------------------------------------------------*/ 
  create_reader();
  delay(1);
  create_writer();
  delay(1);
  create_reader();
  create_reader();
  create_reader();
  delay(1);
  create_writer();
  delay(1);
  create_reader();

  /* delay 15 seconds so all previous readers/writes can finish.
   * This is to prevent writer starvation
   */
  delay(150);


  create_writer();
  delay(1);
  create_writer();
  delay(1);
  create_reader();
  create_reader();
  create_reader();
  delay(1);
  create_writer();
  delay(1);
  create_reader();
  
  /*------------------------------------------------------- 
  
      Wait until all children terminate
  
  --------------------------------------------------------*/
  for (i=0; i<(readerID+writerID); i++) {
      wait(NULL);
  }
  
  return 0;
}

