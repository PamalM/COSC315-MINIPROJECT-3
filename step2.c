#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int fd;

void myFileSystem(char diskName[8]){
    // Open the file with name diskName
    fd = open(diskName, O_WRONLY | O_CREAT | O_TRUNC,  S_IRUSR | S_IWUSR);

    // Read the first 1KB and parse it to structs/objecs representing the super block
    // 	An easy way to work with the 1KB memory chunk is to move a pointer to a
    //	position where a struct/object begins. You can use the sizeof operator to help
    //	cleanly determine the position. Next, cast the pointer to a pointer of the
    //	struct/object type.
    char *buf = (char *) calloc(1024,sizeof(char));


    // Be sure to close the file in a destructor or otherwise before
    // the process exits.

}


int createFile(char name[8], int size){
    printf("Creating: %s with size: %d",name,size);
    //create a file with this name and this size

  // high level pseudo code for creating a new file

  // Step 1: Look for a free inode by searching the collection of objects
  // representing inodes within the super block object.
  // If none exist, then return an error.
  // Also make sure that no other file in use with the same name exists.

  // Step 2: Look for a number of free blocks equal to the size variable
  // passed to this method. If not enough free blocks exist, then return an error.

  // Step 3: Now we know we have an inode and free blocks necessary to
  // create the file. So mark the inode and blocks as used and update the rest of
  // the information in the inode.

  // Step 4: Write the entire super block back to disk.
  //	An easy way to do this is to seek to the beginning of the disk
  //	and write the 1KB memory chunk.
} // End Create



int deleteFile(char name[8]){
    printf("DELET: %s",name);
  // Delete the file with this name

  // Step 1: Look for an inode that is in use with given name
  // by searching the collection of objects
  // representing inodes within the super block object.
  // If it does not exist, then return an error.

  // Step 2: Free blocks of the file being deleted by updating
  // the object representing the free block list in the super block object.

  // Step 3: Mark inode as free.

  // Step 4: Write the entire super block back to disk.

} // End Delete


int listDisk(void){
    printf("LISTING THE BOIS");
  // List names of all files on disk

  // Step 1: Print the name and size fields of all used inodes.

} // End ls

int readBlock(char name[8], int blockNum, char buf[1024]){
    printf("Reading: %s with blockNum: %d",name,blockNum);
   // read this block from this file
   // Return an error if and when appropriate. For instance, make sure
   // blockNum does not exceed size-1.

   // Step 1: Locate the inode for this file as in Step 1 of delete.

   // Step 2: Seek to blockPointers[blockNum] and read the block
   // from disk to buf.

} // End read


int writeBlock(char name[8], int blockNum, char buf[1024]){
    printf("Wirtigng: %s with blockNum: %d",name,blockNum);
   // write this block to this file
   // Return an error if and when appropriate.

   // Step 1: Locate the inode for this file as in Step 1 of delete.

   // Step 2: Seek to blockPointers[blockNum] and write buf to disk.

} // end write

int main(int argc, char *argv[]){
    readInput(argv[0]);

    //Close the file
    close(fd);
    return 0;
}

void readInput(char inputfile[]){
    FILE* file = fopen(inputfile,"r");

    char line[256];
    char delim[] = " ";

    int i = 0;
    while (fgets(line, sizeof(line), file)) {

        if(i == 0){
            //Read disk name
            myFileSystem(line);
            continue;
        }

        char *operation = strtok(line, delim);
        char *filename = strtok(NULL, delim);
        int *size = atoi(strtok(NULL, line));

        char *buffer = (char *) calloc(1024,sizeof(char));

        switch (*operation)
        {
        case 'C':
            createFile(&filename, &size);
            break;
        case 'L':
            listDisk();
            break;
        case 'W':
            writeBlock(&filename, &size, &buffer);
            break;
        case 'R':
            readBlock(&filename, &size, &buffer);
            break;
        case 'D':
            deleteFile(&filename);
            break;
        default: //Do nothing
            break;
        }

        i++;
    }

    fclose(file);
}

