#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>  // For shm_open() and shm_unlink()
#include <sys/wait.h>  // For wait()
#include <stdlib.h>    // For exit()
#include <fcntl.h>     // For oflags
#include <errno.h>

#define ARR_LENGTH 4
#define ARR_SIZE (ARR_LENGTH * sizeof(int)) // sizeof(int) = 4 bytes

int fd;
char* buffer_path = "/buffer"; // the path to the buffer; will be used by the shm_open() and shm_unlink()

int create_buf(); 

int main ()
{ 
  int pid = fork();
  int wstatus;
  if (pid == -1)                        // fork failed 
  {
    fprintf (stderr, "Fork failed with the error: %d\n", errno); return 1;
  }
  else if (pid == 0)                    // Child Process
  { 
    printf ("In the Producer Process.\n");
    // create memory buffer
    printf ("Creating the memory object.\n");
    if (create_buf() == -1) return -1;
    
    // write into the memory buffer
    int integers [ARR_LENGTH] = {1, 2, 3, 4};

    if (write(fd, integers, ARR_LENGTH) == -1)
    {
      fprintf (stderr, "Write returned an error: %d\n", errno); return -1;
    }
    printf ("Integers written into the memory object\n");
    exit(EXIT_SUCCESS);
  }
  else                                  // Parent Process
  {
    do // wait for the proc to finish
    {
      wait(NULL);
    }
    while (!WIFEXITED(wstatus) && !WIFSIGNALED(wstatus));

    printf ("In the Consumer Process.\n");
    // read from the buffer
    printf ("reading...\n");
  }
  printf("Program exited successfully\n");
  return 0;
}

int create_buf()
{
  // open a new shm connection
  fd = shm_open (buffer_path,          // Connection path 
      O_RDWR | O_CREAT | O_TRUNC,      // Create a new memory object for read/write and if there already exists one, truncate it to 0 bytes 
      0400 | 0200                      // Give Read and Write Perms 
      );
  
  if (ftruncate(fd,  ARR_SIZE) == -1) // truncates the memory object to only 4 * sizeof(int)
  {
    fprintf (stderr, "ftruncate returned an error: %d\n", errno); return -1;
  }

  /* 
   * The below mmap call allocates memory to the memory object pointed to by fd with permissions to READ and WRITE.
   * With MAP_SHARED, i've made the mapping visible to the other processes so that the consumer process can read from this memory.
   */

  if (mmap(NULL, ARR_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)  == (void *)-1)
  {
    fprintf (stderr, "mmap returned an error: %d\n", errno); return -1;
  }

}
