#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define INODE_SIZE 16

int fd;
int freeblocklist[128];

struct inode {
   char  name[8];
   int size;
   int blockPointers[8];
   int used;
} inode;

struct inode inodes[INODE_SIZE];

int writeSuperblock() {
    //Set write position to start of file
    lseek(fd, 0, SEEK_SET);

    //Create write buffer
    char *buf = (char *) calloc(1024,sizeof(char));

    //Add free block list to buffer
    for(int i = 0; i < sizeof(freeblocklist);i++){
        buf[i] = freeblocklist[i];
    }

    //Add inodes to buffer
    for(int i = 0; i < INODE_SIZE;i++){
        buf[127 + (i * 48)] = (int) &inodes[i];
    }
    //write buffer to file
    write(fd,buf, 1024);

    return 0;
}

int myFileSystem(char diskName[8]){
    // Open the file with name diskName
    fd = open(diskName, O_RDWR);

    // Read the first 1KB and parse it to structs/objecs representing the super block
    // 	An easy way to work with the 1KB memory chunk is to move a pointer to a
    //	position where a struct/object begins. You can use the sizeof operator to help
    //	cleanly determine the position. Next, cast the pointer to a pointer of the
    //	struct/object type.
    char *buf = (char *) calloc(1024,sizeof(char));
    if(read(fd, buf, 1024) < 0){
        printf("read failed");
    }

    //Current inode
    int inodecounter = 0;
    //Current inode byte
    int bytecounter = 0;

    //Temporary inode variables
    char name[8];
    char sizebinary[4];
    char blockpointer[32];
    char usedbinary[4];

    //Loop through all bytes
    for(int i=0;i<896;i++){
        printf("buf: %s", buf[i]);
        //First 128 bytes are the free block list
        if(i < 128){
            freeblocklist[i] = buf[i];
        }
        //Other 768 bytes are the 16 inodes (48 bytes each)
        else{
            //First 8 bytes are the inode name
            if(bytecounter < 8) name[bytecounter] = buf[i];
            //Next 4 bytes are the inode size
            else if(bytecounter >= 8 && bytecounter < 12) sizebinary[bytecounter-4] = buf[i];
            //Next 32 bytes are the blockpointers
            else if(bytecounter >= 12 && bytecounter < 44) blockpointer[bytecounter-12] = buf[i];
            //The final 4 bytes is the inode used value
            else if(bytecounter >= 44 && bytecounter < 48){
                usedbinary[bytecounter-44] = buf[i];
                //On the last byte add the inode to the inodes array
                if(bytecounter == 47){
                    //Reset bytecounter for next inode
                    bytecounter = 0;
                    //Set inode name
                    strcpy(inodes[inodecounter].name,name);
                    //set inode size
                    inodes[inodecounter].size = (unsigned int) sizebinary;

                    //Temp variables for inode blockpointers
                    int blockcounter = 0;
                    char block[4];

                    //Iterate through the blockpointer bytes
                    for(int j = 0; j < 32; j++){

                        //For each 4 bytes increase write the block to the inode blockpointers array
                        //Then increase block counter
                        if(j != 0 && j % 4 == 0){
                            block[j-(blockcounter*4)] = blockpointer[j];
                            inodes[inodecounter].blockPointers[blockcounter] = (int) block;
                            blockcounter++;
                        }else{
                            block[j-(blockcounter*4)] = blockpointer[j];
                        }
                    }
                    //Set inode used
                    inodes[inodecounter].used = (unsigned int) usedbinary;

                    //Increase inode counter
                    inodecounter++;
                    continue;
                }
            }
            //Increase byte counter
            bytecounter++;
        }
    }

    // Be sure to close the file in a destructor or otherwise before
    // the process exits.


    return 0;
}


int createFile(char name[8], int size){
    printf("Creating: %s with size: %d\n",name,size);
 // Step 1: Look for a free inode by searching the collection of objects
    // representing inodes within the super block object.
    // If none exist, then return an error.
    // Also make sure that no other file in use with the same name exists.
    int freeinode = 32;
    //Loop through all inodes
    for(int i = 0; i < INODE_SIZE;i++){
        //Checks for duplicate names, returns if found.
        if(inodes[i].name == name) {
                printf("error: inode with same name found.");
                return 0;
        }
        printf("USED: %d\n",inodes[i].used);
        //If inode is not used, if all are used freeinode will stay 32
        if(inodes[i].used == 0) {
            freeinode = i;
        }
    }
    if(freeinode == 32) {
        printf("error: no free inodes\n");
        return 0;
    }

    // Step 2: Look for a number of free blocks equal to the size variable
    // passed to this method. If not enough free blocks exist, then return an error.
    int freeblocks = 0;
    //Loop through the free block list
    for (int i=0; i < sizeof(freeblocklist); i++) {
        //If block is 0, add to freeblocks
        freeblocks += (freeblocklist[i] == '0');
    }
    if(freeblocks < size) {
        printf("error: not enough free blocks\n");
        return 0;
    }

    // Step 3: Now we know we have an inode and free blocks necessary to
    // create the file. So mark the inode and blocks as used and update the rest of
    // the information in the inode.
    struct inode tempNode;

    int blockcounter = 0;
    //Loop through all free blocks
    for (int i=0; i < sizeof(freeblocklist); i++) {
        if(blockcounter >= size){
            //break once enough blocks have been allocated
            break;
        }
        //get the free blocks
        if(freeblocklist[i] == '0') {
            //set the block to be used
            freeblocklist[i] = 1;
            //update the block pointer
            tempNode.blockPointers[blockcounter++] = i;
        }
    }

    //Update inode values
    tempNode.used = 1;
    strcpy(tempNode.name,name);
    tempNode.size = size;

    //Add inode to the master list
    inodes[freeinode] = tempNode;

    // Step 4: Write the entire super block back to disk.
    //	An easy way to do this is to seek to the beginning of the disk
    //	and write the 1KB memory chunk.
    writeSuperblock();
  return 0;
} // End Create



int deleteFile(char name[8]){
    printf("Deleting file: %s\n",name);
    // Delete the file with this name

    // Step 1: Look for an inode that is in use with given name
    // by searching the collection of objects
    // representing inodes within the super block object.
    // If it does not exist, then return an error.

    //Check if file exists, if not print an error message and return
    //otherwise store the index of the inode
    int iindex = 32;

    for(int i = 0; i < INODE_SIZE; i++){
        if(inodes[i].used == 1 && inodes[i].name == name){
            iindex = i;
        }
    }
    if(iindex == 32){
        printf("File doesn't exist\n");
        return 0;
    }

    // Step 2: Free blocks of the file being deleted by updating
    // the object representing the free block list in the super block object.

    //Creates a tempNode and sets all the block pointers to 0
    struct inode tempNode = inodes[iindex];
    for(int i = 0; i < 8; i++){
        tempNode.blockPointers[i] = 0;
    }
    //sets inodes array at the index to the tempNode
    inodes[iindex] = tempNode;

    // Step 3: Mark inode as free.
    inodes[iindex].used = 0;
    // Step 4: Write the entire super block back to disk.

    writeSuperblock();

    return 0;
}


int listDisk(void){
    printf("Listing files:\n");
  // List names of all files on disk

  // Step 1: Print the name and size fields of all used inodes.
    for(int i = 0; i < INODE_SIZE; i++){
        printf("File name: %s\nsize: %d\n\n",inodes[i].name,inodes[i].size);
    }

    return 0;
} // End ls

int readBlock(char name[8], int blockNum, char buf[1024]){
    printf("Reading: %s with blockNum: %d\n",name,blockNum);
    // read this block from this file
    // Return an error if and when appropriate. For instance, make sure
    // blockNum does not exceed size-1.

    //Check if file exists, if not print an error message and return
    //otherwise store the index of the inode
    int iindex = 32;

    for(int i = 0; i < INODE_SIZE; i++){
        if(inodes[i].used == 1 && inodes[i].name == name){
            iindex = i;
        }
    }
    if(iindex == 32){
        printf("File doesn't exist");
        return 0;
    }

    struct inode tempNode = inodes[iindex];

    // Step 2: Seek to blockPointers[blockNum] and read the block
    // from disk to buf.
    int offset = 1024 + (1024 * tempNode.blockPointers[blockNum]);
    //Seek the offset
    lseek(fd, offset, SEEK_SET);
    //Read next 1KB to buffer
    read(fd, buf, 1024);
    return 0;
}


int writeBlock(char name[8], int blockNum, char buf[1024]){
    printf("Writing: %s with blockNum: %d\n",name,blockNum);

    //Check if file exists, if not print an error message and return
    //otherwise store the index of the inode
    int iindex = 32;

    for(int i = 0; i < INODE_SIZE; i++){
        if(inodes[i].used == 1 && inodes[i].name == name){
            iindex = i;
        }
    }
    if(iindex == 32){
        printf("File doesn't exist");
        return 0;
    }

    struct inode tempNode = inodes[iindex];

    // Step 2: Seek to blockPointers[blockNum] and write buf to disk.
    int offset = 1024 + (1024 * tempNode.blockPointers[blockNum]);
    //Seek the offset
    lseek(fd, offset, SEEK_SET);
    //write next 1KB from buffer
    write(fd, buf, 1024);
    return 0;
}

int readInput(char inputfile[]){
    FILE* file = fopen(inputfile,"r");

    char line[256];
    char delim[] = " ";

    int i = 0;
    while (fgets(line, sizeof(line), file)) {

        if(i == 0){
             //Read disk name
             myFileSystem(line);

         }
         else{

            char *operation = strtok(line, delim);
            if(*operation == 'L'){
                listDisk();
                continue;
            }

            char *filename = strtok(NULL, delim);
            if(*operation == 'D'){
                deleteFile(filename);
                continue;
            }

            char *sizepointer = strtok(NULL, line);
            int size = atoi(sizepointer);

            char *buffer = (char *) calloc(1024,sizeof(char));
            //buffer[0] = 1;
            switch (*operation)
            {
            case 'C':
                createFile(filename, size);
                break;
            case 'W':
                writeBlock(filename, size, buffer);
                break;
            case 'R':
                readBlock(filename, size, buffer);
                break;
            default: //Do nothing
                break;
            }
         }

         i++;
    }

    fclose(file);
    return 0;
}

int main(int argc, char *argv[]){

    for(int i = 0;i<INODE_SIZE; i++){
        strcpy(inodes[i].name, "noname");
        inodes[i].size = 0;
        for(int j = 0; j < 8; j++){
            inodes[i].blockPointers[j] = 0;
        }
        inodes[i].used = 0;
    }

    readInput(argv[1]);

    //Close the file
    close(fd);
    return 0;
}

