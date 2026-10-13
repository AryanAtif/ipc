#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>  // For shm_open() and shm_unlink()
#include <fcntl.h>    // For oflags
#include <errno.h>

#define ARR_LENGTH 4
#define ARR_SIZE (ARR_LENGTH * sizeof(int)) // sizeof(int) = 4 bytes

int create_buf(); // returns the buf name
int fd;
char* buffer_path = "/buffer"; // the path to the buffer; will be used by the shm_open() and shm_unlink()

int main ()
{ 
  if (create_buf() == -1) return -1;
  printf("Program exited successfully\n");

  /*
  int pid = fork();
  if (pid == 0) // child proc
  { 
    // create memory buffer
    char* buffer = create_buf();
    
    // write into the memory buffer
    
  }
  else if (pid == 1) // parent proc
  {
    // wait for the proc to finish 
    // read from the buffer
  }*/

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
