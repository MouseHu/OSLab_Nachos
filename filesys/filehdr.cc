// filehdr.cc 
//	Routines for managing the disk file header (in UNIX, this
//	would be called the i-node).
//
//	The file header is used to locate where on disk the 
//	file's data is stored.  We implement this as a fixed size
//	table of pointers -- each entry in the table points to the 
//	disk sector containing that portion of the file data
//	(in other words, there are no indirect or doubly indirect 
//	blocks). The table size is chosen so that the file header
//	will be just big enough to fit in one disk sector, 
//
//      Unlike in a real system, we do not keep track of file permissions, 
//	ownership, last modification date, etc., in the file header. 
//
//	A file header can be initialized in two ways:
//	   for a new file, by modifying the in-memory data structure
//	     to point to the newly allocated data blocks
//	   for a file already on disk, by reading the file header from disk
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"

#include "system.h"
#include "filehdr.h"
#include <time.h>
//----------------------------------------------------------------------
// FileHeader::Allocate
// 	Initialize a fresh file header for a newly created file.
//	Allocate data blocks for the file out of the map of free disk blocks.
//	Return FALSE if there are not enough free blocks to accomodate
//	the new file.
//
//	"freeMap" is the bit map of free disk sectors
//	"fileSize" is the bit map of free disk sectors
//----------------------------------------------------------------------

bool
FileHeader::Allocate(BitMap *freeMap, int fileSize)
{
    DEBUG('f',"Fdr allocate:%d\n",fileSize); 
    time_t t;
    time(&t);
    numBytes = fileSize;
    numSectors  = divRoundUp(fileSize, SectorSize);
    int SectorBuffer[SectorSize/sizeof(int)];
    int IndexBuffer[SectorSize/sizeof(int)];
    int max_l1index = SectorSize/sizeof(int);
    int max_l2index = SectorSize/sizeof(int)*SectorSize/sizeof(int);
    //int num_l2index2 = MAX(0,divRoundUp(numSectors-(NumDirect+num_l1index),SectorSize/sizeof(int)));
    if(numSectors<NumDirect){
        if (freeMap->NumClear() < numSectors)
	        return FALSE;		// not enough space
        //all direct index
        l1Index = -1;
        l2Index = -1;
        for (int i = 0; i < numSectors; i++){
            dataSectors[i] = freeMap->Find();
            //printf("Allocate:%d->%d\n",i,dataSectors[i]);
        }
	        
    }
    else if(numSectors<(NumDirect+max_l1index)){
        //only l1 index
        if (freeMap->NumClear() < numSectors+1)
            return FALSE;
        l1Index = freeMap->Find();
        l2Index = -1;
        for (int i = 0; i < NumDirect; i++)
	        dataSectors[i] = freeMap->Find();
        for(int i=NumDirect;i<numSectors;i++){
            IndexBuffer[i-NumDirect]=freeMap->Find();
            //printf("Allocate:%d->%d\n",i,IndexBuffer[i-NumDirect]);
        }
        synchDisk->WriteSector(l1Index, (char *)IndexBuffer ); 
    }
    else if(numSectors<(NumDirect+max_l1index+max_l2index)){
        //l1 index and l2 index
        int num_l2index = divRoundUp(numSectors-(NumDirect+max_l1index),SectorSize/sizeof(int));
        if (freeMap->NumClear() < numSectors+2+num_l2index)
            return FALSE;
        l1Index = freeMap->Find();
        l2Index = freeMap->Find();
        for (int i = 0; i < NumDirect; i++){
            dataSectors[i] = freeMap->Find();
            //printf("Allocate:%d->%d\n",i,dataSectors[i]);
        }
        for(int i=NumDirect;i<NumDirect+max_l1index;i++){
            IndexBuffer[i-NumDirect]=freeMap->Find();
            //printf("Allocate:%d->%d\n",i,IndexBuffer[i-NumDirect]);
        }
        synchDisk->WriteSector(l1Index, (char *)IndexBuffer );
        int j = -1; 
        for(int i =NumDirect+max_l1index;i<numSectors;i++){
            int pos = (i-NumDirect-max_l1index)%(SectorSize/sizeof(int));
            if(pos==0){
                if(j>=0){
                    synchDisk->WriteSector(IndexBuffer[j], (char *)SectorBuffer );
                }
                IndexBuffer[++j]=freeMap->Find();
            }
            SectorBuffer[pos] = freeMap->Find();
            //printf("Allocate:%d->%d\n",i,SectorBuffer[pos]);
        }
        synchDisk->WriteSector(IndexBuffer[j], (char *)SectorBuffer );
        synchDisk->WriteSector(l2Index, (char *)IndexBuffer );
    }
    else{
        DEBUG('f',"Too big! %d %d\n",numSectors,NumDirect+max_l1index+max_l2index);
        return FALSE;
    }
    

    createTime = int(t);
    modifyTime = int(t);
    visitTime = int(t);
    return TRUE;
}

//----------------------------------------------------------------------
// FileHeader::Deallocate
// 	De-allocate all the space allocated for data blocks for this file.
//
//	"freeMap" is the bit map of free disk sectors
//----------------------------------------------------------------------

void 
FileHeader::Deallocate(BitMap *freeMap)
{
    // printf("problem here?%d\n",this->numSectors);
    int num_l1index = SectorSize/sizeof(int);
    int num_l2index = MAX(0,divRoundUp(numSectors-(NumDirect+num_l1index),SectorSize/sizeof(int)));
    int SectorBuffer[SectorSize/sizeof(int)];
    int IndexBuffer[SectorSize/sizeof(int)];
    if(numSectors<NumDirect){
        for (int i = 0; i < numSectors; i++) {
	        ASSERT(freeMap->Test((int) dataSectors[i]));  // ought to be marked!
	        freeMap->Clear((int) dataSectors[i]);
        }
    }
    else if(numSectors<NumDirect+num_l1index){
        for (int i = 0; i < NumDirect; i++) {
	        ASSERT(freeMap->Test((int) dataSectors[i]));  // ought to be marked!
	        freeMap->Clear((int) dataSectors[i]);
        }
        synchDisk->ReadSector(l1Index, (char*)IndexBuffer);
        for(int i =NumDirect;i<numSectors;i++){
            ASSERT(freeMap->Test((int) IndexBuffer[i-NumDirect]));
            freeMap->Clear((int) IndexBuffer[i-NumDirect]);
        }
        ASSERT(freeMap->Test((int) l1Index));
        freeMap->Clear((int) l1Index);
    }
    else{
        for (int i = 0; i < NumDirect; i++) {
	        ASSERT(freeMap->Test((int) dataSectors[i]));  // ought to be marked!
	        freeMap->Clear((int) dataSectors[i]);
        }
        synchDisk->ReadSector(l1Index, (char*)IndexBuffer);
        for(int i =0;i<num_l1index;i++){
            ASSERT(freeMap->Test((int) IndexBuffer[i]));
            freeMap->Clear((int) IndexBuffer[i]);
        }
        ASSERT(freeMap->Test((int) l1Index));
        freeMap->Clear((int) l1Index);
        synchDisk->ReadSector(l2Index, (char*)IndexBuffer);
        int j = 0; 
        for(int i =NumDirect+num_l1index;i<numSectors;i++){
            int pos = (i-NumDirect-num_l1index)%(SectorSize/sizeof(int));
            if(pos==0){
                synchDisk->ReadSector(IndexBuffer[j++], (char *)SectorBuffer );
            }
            ASSERT(freeMap->Test((int) SectorBuffer[pos]));
            freeMap->Clear((int) SectorBuffer[pos]);
        }
        for(int i =0;i<num_l2index;i++){
            ASSERT(freeMap->Test((int) IndexBuffer[i]));
            freeMap->Clear((int) IndexBuffer[i]);
        }
        ASSERT(freeMap->Test((int) l2Index));
        freeMap->Clear((int) l2Index);
    }
    numBytes=0;
    numSectors=0;
}

//----------------------------------------------------------------------
// FileHeader::FetchFrom
// 	Fetch contents of file header from disk. 
//
//	"sector" is the disk sector containing the file header
//----------------------------------------------------------------------

void
FileHeader::FetchFrom(int sector)
{

    synchDisk->ReadSector(sector, (char *)this);
}

//----------------------------------------------------------------------
// FileHeader::WriteBack
// 	Write the modified contents of the file header back to disk. 
//
//	"sector" is the disk sector to contain the file header
//----------------------------------------------------------------------

void
FileHeader::WriteBack(int sector)
{
    // time_t t;
    // time(&t);
    // modifyTime = int(t);
    synchDisk->WriteSector(sector, (char *)this); 
}

//----------------------------------------------------------------------
// FileHeader::ByteToSector
// 	Return which disk sector is storing a particular byte within the file.
//      This is essentially a translation from a virtual address (the
//	offset in the file) to a physical address (the sector where the
//	data at the offset is stored).
//
//	"offset" is the location within the file of the byte in question
//----------------------------------------------------------------------

int
FileHeader::ByteToSector(int offset)
{
    int numSector = offset/SectorSize;
    int num_l1index = SectorSize/sizeof(int);
    int num_l2index = MAX(0,divRoundUp(numSectors-(NumDirect+num_l1index),SectorSize/sizeof(int)));
    int SectorBuffer[SectorSize/sizeof(int)];
    int IndexBuffer[SectorSize/sizeof(int)];
    if(numSector<NumDirect){
        // printf("Byte to sector:%d -> %d\n",offset,dataSectors[offset / SectorSize]);
        return(dataSectors[numSector]);
    }
        
    else if(numSector<NumDirect+num_l1index){
        synchDisk->ReadSector(l1Index,(char*)IndexBuffer);
        // printf("Byte to sector:%d -> %d\n",offset,IndexBuffer[offset / SectorSize - NumDirect]);
        return IndexBuffer[numSector - NumDirect];
    }
    else{
        //printf("l2 INDEX buffer:%d\n",l2Index);
        // printf("Here.%d\n",l2Index);
        synchDisk->ReadSector(l2Index,(char*)IndexBuffer);
        int j = divRoundDown(numSector-(NumDirect+num_l1index),SectorSize/sizeof(int));
        int pos = (numSector-(NumDirect+num_l1index))%(SectorSize/sizeof(int));
        // printf("Sector buffer:%d %d %d %d\n",IndexBuffer[j],pos,j,offset);
        synchDisk->ReadSector(IndexBuffer[j],(char*)SectorBuffer);
        // printf("Byte to sector:%d -> %d\n",offset,SectorBuffer[pos]);
        return SectorBuffer[pos];
    }
}

//----------------------------------------------------------------------
// FileHeader::FileLength
// 	Return the number of bytes in the file.
//----------------------------------------------------------------------

int
FileHeader::FileLength()
{
    return numBytes;
}

char* GetTime(int time){
    time_t t = (time_t)time;
    struct tm* timeBlock;
    timeBlock = localtime(&t);
    return(asctime(timeBlock));
}
//----------------------------------------------------------------------
// FileHeader::Print
// 	Print the contents of the file header, and the contents of all
//	the data blocks pointed to by the file header.
//----------------------------------------------------------------------

void
FileHeader::Print()
{
    int i, j, k;
    char *data = new char[SectorSize];

    printf("FileHeader contents.  File size: %d.  File blocks:\n", numBytes);
    printf("Create time: %s",GetTime(createTime));
    printf("Modify time: %s",GetTime(modifyTime));
    printf("Visit time: %s",GetTime(visitTime));
    for (i = 0; i < numSectors; i++)
	printf("%d ", dataSectors[i]);
    printf("\nFile contents:\n");
    for (i = k = 0; i < numSectors; i++) {
	synchDisk->ReadSector(dataSectors[i], data);
        for (j = 0; (j < SectorSize) && (k < numBytes); j++, k++) {
	    if ('\040' <= data[j] && data[j] <= '\176')   // isprint(data[j])
		printf("%c", data[j]);
            else
		printf("\\%x", (unsigned char)data[j]);
	}
        printf("\n"); 
    }
    delete [] data;
}


void FileHeader::ClearSector( int sectorNum ){
	char allZero[SectorSize] ={0};
	synchDisk->WriteSector(sectorNum, (char *)allZero);
}

bool FileHeader::ChangeSize( BitMap* freeMap,int newSize ){
    
    ASSERT(newSize>=numBytes)
    int newNumSector = divRoundUp(newSize,SectorSize);
    int num_l1index = SectorSize/sizeof(int);
    int num_l2index = MAX(0,divRoundUp(numSectors-(NumDirect+num_l1index),SectorSize/sizeof(int)));
    int SectorBuffer[SectorSize/sizeof(int)];
    int IndexBuffer[SectorSize/sizeof(int)];
    if(newNumSector==numSectors){
        numBytes = newSize;
        return TRUE;
    }
    printf("Extending %d sectors.\n",newNumSector-numSectors);
    while(numSectors<MIN(NumDirect,newNumSector))
    {
        dataSectors[numSectors++]= freeMap->Find();
        // printf("Allocate:%d\n",dataSectors[numSectors-1]);
    }
    if(newNumSector>NumDirect){
        if(l1Index<0){
            for(int i=0;i<num_l1index;i++)
                IndexBuffer[i] = 0;
            l1Index=freeMap->Find();
        }
        else
            synchDisk->ReadSector(l1Index,(char*)IndexBuffer);

        while(numSectors<MIN(NumDirect+num_l1index,newNumSector)){
            IndexBuffer[numSectors-NumDirect] = freeMap->Find();
            numSectors++;
        }
        synchDisk->WriteSector(l1Index, (char *)IndexBuffer );
    }
    if(newNumSector>NumDirect+num_l1index){
        if(l2Index<0){
            for(int i=0;i<num_l1index;i++)
                IndexBuffer[i] = 0;
            
            l2Index=freeMap->Find();
            // printf("l2 index:%d\n");
        }
        else{
            synchDisk->ReadSector(l2Index,(char*)IndexBuffer);
            int jj = divRoundDown((numSectors-NumDirect-num_l1index),(SectorSize/sizeof(int)));
            bool flag2 = (numSectors-NumDirect-num_l1index)%(SectorSize/sizeof(int))==0;
            if(flag2)jj--;
            synchDisk->ReadSector(IndexBuffer[jj],(char*)SectorBuffer);
        }
        int j = divRoundDown((numSectors-NumDirect-num_l1index),(SectorSize/sizeof(int))); 
        bool flag = (numSectors-NumDirect-num_l1index)%(SectorSize/sizeof(int))==0;
        if(flag)j--;
        while(numSectors<newNumSector){
            int pos = (numSectors-NumDirect-num_l1index)%(SectorSize/sizeof(int));
            if(pos==0){
                if(!flag){
                    synchDisk->WriteSector(IndexBuffer[j], (char *)SectorBuffer );
                }
                flag = FALSE;
                IndexBuffer[++j]=freeMap->Find();
            }
            SectorBuffer[pos] = freeMap->Find();
            //printf("Allocate:%d->%d\n",i,SectorBuffer[pos]);
            numSectors++;
        }
        synchDisk->WriteSector(IndexBuffer[j], (char *)SectorBuffer );
        synchDisk->WriteSector(l2Index, (char *)IndexBuffer );
           
    }
    numBytes = newSize;
    // printf("??%d\n",numBytes);
    numSectors = divRoundUp(newSize,SectorSize);
    return TRUE;
}

void FileHeader::Modify(){
    time_t t;
    time(&t);
    modifyTime = int(t);
}

void FileHeader::Visit(){
    time_t t;
    time(&t);
    visitTime = int(t);
}