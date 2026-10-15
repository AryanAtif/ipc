#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>  // For shm_open() and shm_unlink()
#include <sys/wait.h>  // For waitpid()
#include <stdlib.h>    // For exit()
#include <fcntl.h>     // For oflags
#include <errno.h>

#define ARR_LENGTH 4
#define ARR_SIZE (ARR_LENGTH * sizeof(int)) // sizeof(int) = 4 bytes

int fd;
int *ptr = NULL;
char* object_path = "/mem_obj"; // the path to the object; used by shm_open() and shm_unlink()

int create_mem_obj();

int main ()
{
  // create and map the memory object 
  printf ("Creating the memory object.\n");
  if (create_mem_obj() == -1) return 1;

  int wstatus;
  int pid = fork();
  if (pid == -1)                        // fork failed
  {
    fprintf (stderr, "Fork failed with the error: %d\n", errno);
    shm_unlink(object_path);
    return 1;
  }
  else if (pid == 0)                    // Child Process 
  {
    printf ("In the Producer Process.\n");

    // write into the shared memory through the mapping
    int integers[ARR_LENGTH] = {1, 2, 3, 4};
    for (int i = 0; i < ARR_LENGTH; i++)
    {
      ptr[i] = integers[i];
    }

    printf ("Integers written into the memory object\n");
    exit(EXIT_SUCCESS);
  }
  else                                  // Parent Process
  {
    // wait for the child to finish
    wait(NULL);

    printf ("In the Consumer Process.\n");

    // read from the memory object
    for (int i = 0; i < ARR_LENGTH; i++)
    {
      printf ("%d ", ptr[i]);
    }
    printf ("Integers read from the memory object\n");

    // close the connection
    munmap(ptr, ARR_SIZE);
    shm_unlink(object_path);
  }
  printf("Program exited successfully\n");
  return 0;
}

int create_mem_obj()
{
  // open a new shm connection
  fd = shm_open (object_path,          // Connection path
      O_RDWR | O_CREAT | O_TRUNC,      // Create a new memory object for read/write; truncate it if it already exists
      0666                             // Give Read and Write Perms
      );

  if (fd == -1)
  {
    fprintf (stderr, "shm_open returned an error: %d\n", errno); return -1;
  }

  if (ftruncate(fd, ARR_SIZE) == -1)   // size the memory object to 4 * sizeof(int)
  {
    fprintf (stderr, "ftruncate returned an error: %d\n", errno); return -1;
  }

  /*
   * mmap maps the memory object into this process's address space with READ and WRITE permissions.
   * With MAP_SHARED, i've made the mapping visible to the other processes so that the consumer process can read from this memory.
   */
  if ((ptr = mmap(NULL, ARR_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED)
  {
    fprintf (stderr, "mmap returned an error: %d\n", errno); return -1;
  }

  return 0;
}
