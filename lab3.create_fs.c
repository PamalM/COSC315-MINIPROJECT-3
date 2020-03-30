/* create a file  to act as a disk  and format the file system residing on the disk */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int
main(int argc, char *argv[])
{
  int i, fd;
  char *buf;

  if (argc == 1 ){
    fprintf(stderr,"usage: %s <diskFileName> \n",argv[0]);
    exit(0);
  }

  printf( "Creating  a 128KB  file in %s\n",argv[1]);
  printf("This file will act as a dummy disk and will hold your filesystem\n");

  /* open the disk file for writing */
  fd = open(argv[1],O_WRONLY | O_CREAT | O_TRUNC,  S_IRUSR | S_IWUSR);

  printf("Formatting your filesystem...\n");

  buf = (char *) calloc(1024,sizeof(char));
  
  /* write super block */
  buf[0]=1;  /* mark superblock as allocated in the free block list 
                         all other blocks are free, all inodes are zeroed out */

  /* write out the super block */
  if(write(fd,buf, 1024)<0)
    printf("error: write failed \n");  

  buf[0]=0;
  /* write out the remaining 127 data blocks, all zeroed out */
  for(i=0;i<127;i++){
    if(write(fd,buf,1024)<0)
      printf("error: write failed \n");  
  }

  close(fd);
  exit(1);

}