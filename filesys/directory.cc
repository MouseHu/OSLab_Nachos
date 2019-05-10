// directory.cc 
//	Routines to manage a directory of file names.
//
//	The directory is a table of fixed length entries; each
//	entry represents a single file, and contains the file name,
//	and the location of the file header on disk.  The fixed size
//	of each directory entry means that we have the restriction
//	of a fixed maximum size for file names.
//
//	The constructor initializes an empty directory of a certain size;
//	we use ReadFrom/WriteBack to fetch the contents of the directory
//	from disk, and to write back any modifications back to disk.
//
//	Also, this implementation has the restriction that the size
//	of the directory cannot expand.  In other words, once all the
//	entries in the directory are used, no more files can be created.
//	Fixing this is one of the parts to the assignment.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "utility.h"
#include "filehdr.h"
#include "directory.h"
#include "system.h"
//----------------------------------------------------------------------
// Directory::Directory
// 	Initialize a directory; initially, the directory is completely
//	empty.  If the disk is being formatted, an empty directory
//	is all we need, but otherwise, we need to call FetchFrom in order
//	to initialize it from disk.
//
//	"size" is the number of entries in the directory
//----------------------------------------------------------------------

Directory::Directory(int size)
{
    root = new std::vector<DirectoryEntry*>();
    tableSize = 0;//size;
}

//----------------------------------------------------------------------
// Directory::~Directory
// 	De-allocate directory data structure.
//----------------------------------------------------------------------

Directory::~Directory()
{ 
    for(int i =0;i<root->size();i++){
        delete &(*root)[i];
    }
} 
void PrintSector(int sector){
    printf("Printing Sector:%d\n",sector);
    char data[SectorSize];
    synchDisk->ReadSector(sector, data);
    for (int j = 0; (j < SectorSize) ; j++) {
	    if ('\040' <= data[j] && data[j] <= '\176')   // isprint(data[j])
		printf("%c", data[j]);
            else
		printf("\\%x", (unsigned char)data[j]);
	}
    printf("\n"); 
    
}
//----------------------------------------------------------------------
// Directory::FetchFrom
// 	Read the contents of the directory from disk.
//
//	"file" -- file containing the directory contents
//----------------------------------------------------------------------

void
Directory::FetchFrom(OpenFile *file)
{
    //printf("Directory Fetch From.\n");
    //std::vector<int>* root;
    (void) file->ReadAt((char *)&tableSize, sizeof(int),0);
    root = new std::vector<DirectoryEntry*>();
    DirectoryEntry* de;
    for(int i=0;i<tableSize;i++){
        de = new DirectoryEntry();
        int deSector=0;
        (void) file->ReadAt((char *)&deSector, sizeof(int),i*sizeof(int)+sizeof(int));//, sizeof(int));
        //printf("De sector:%d\n",deSector);
        de->FetchFrom(new OpenFile(deSector));
        de->parent = NULL;
        root->push_back(de);
    }
    //printf("Fetched root:%d\n",(int)root);
}

void
DirectoryEntry::FetchFrom(OpenFile *file)
{
    //printf("Directory Entry Fetch From.\n");
    
    file->Seek(0);
    (void) file->Read((char *)&selfSector, sizeof(int));
    //PrintSector(selfSector);
    (void) file->Read((char *)&fileSector, sizeof(int));
    (void) file->Read((char *)&childSize, sizeof(int));
    (void) file->Read((char *)type, TypeNameMaxLen+1);
    (void) file->Read((char *)name, FileNameMaxLen+1);
    //printf("Read File Name:%s\n",name);
    (void) file->Read((char *)directory, DirNameMaxLen+1);
    //(void) file->Read((char *)parentSector, sizeof(int));
    child = new std::vector<DirectoryEntry*>();
    DirectoryEntry* de;
    for(int i=0;i<childSize;i++){
        de = new DirectoryEntry();
        int deSector=0;
        //, sizeof(int));
        (void) file->Read((char *)&deSector, sizeof(int));//, sizeof(int));
        de->selfSector = deSector;
        de->parent = this;
        de->FetchFrom(new OpenFile(deSector));
        child->push_back(de);
    }
    //prin

}
//----------------------------------------------------------------------
// Directory::WriteBack
// 	Write any modifications to the directory back to disk
//
//	"file" -- file to contain the new directory contents
//----------------------------------------------------------------------


void
Directory::WriteBack(OpenFile *file)
{
    (void) file->WriteAt((char *)&tableSize, sizeof(int),0);
    for(int i=0;i<tableSize;i++){
        DirectoryEntry* de = (*root)[i];
        (void) file->WriteAt((char *)&(de->selfSector), sizeof(int),i*sizeof(int)+sizeof(int));//, sizeof(int));
        de->WriteBack(new OpenFile(de->selfSector));
    }
}

void
DirectoryEntry::WriteBack(OpenFile *file)
{
    file->Seek(0);
    (void) file->Write((char *)&selfSector, sizeof(int));
    (void) file->Write((char *)&fileSector, sizeof(int));
    (void) file->Write((char *)&childSize, sizeof(int));
    (void) file->Write((char *)type, TypeNameMaxLen+1);
    (void) file->Write((char *)name, FileNameMaxLen+1);
    (void) file->Write((char *)directory, DirNameMaxLen+1);

    //(void) file->Write((char *)parentSector, sizeof(int));
    for(int i=0;i<childSize;i++){
        DirectoryEntry* de = (*child)[i];
        (void) file->Write((char *)&(de->selfSector), sizeof(int));//, sizeof(int));
        de->WriteBack(new OpenFile(de->selfSector));
    }
    //PrintSector(selfSector);

}
//----------------------------------------------------------------------
// Directory::FindIndex
// 	Look up file name in directory, and return its location in the table of
//	directory entries.  Return -1 if the name isn't in the directory.
//
//	"name" -- the file name to look up
//----------------------------------------------------------------------

// int
// Directory::FindIndex(char *name)
// {
//     for (int i = 0; i < tableSize; i++)
//         if (table[i].inUse && !strncmp(table[i].name, name, FileNameMaxLen))
// 	    return i;
//     return -1;		// name not in directory
// }

//----------------------------------------------------------------------
// Directory::Find
// 	Look up file name in directory, and return the disk sector number
//	where the file's header is stored. Return -1 if the name isn't 
//	in the directory.
//
//	"name" -- the file name to look up
//----------------------------------------------------------------------

// int
// Directory::Find(char *name,char *dir)
// {
//     int i = FindIndex(name);

//     if (i != -1)
// 	    return table[i].sector;
//     return -1;
// }
std::string Join(std::vector<std::string> vec, char join)
{
    std::stringstream joined;
    for(int i=0;i<vec.size();i++){
        joined<<vec[i]<<join;
    }
    return joined.str();
}

std::vector<std::string> Split(const std::string& s, char delimiter)
{
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter))
    {
        tokens.push_back(token);
    }
    return tokens;
}
void StrCopy(char* src,char* dst){
    if(src==NULL){
        dst = NULL;
        return;
    }
    int size = 0;
    while(src[size++]!='\0');
    //printf("copy size:%d\n",size);
    //dst = new char[size];
    for(int i=0;i<size;i++){
        dst[i] = src[i];
    }
}
//----------------------------------------------------------------------
// Directory::Add
// 	Add a file into the directory.  Return TRUE if successful;
//	return FALSE if the file name is already in the directory, or if
//	the directory is completely full, and has no more space for
//	additional file names.
//
//	"name" -- the name of the file being added
//	"newSector" -- the disk sector containing the added file's header
//----------------------------------------------------------------------

bool
Directory::Add(char *name,char* directory,char* type, int fileSector, int selfSector)
{ 

    if (FindEntry(name,directory)!=NULL)
	    return FALSE;
    FileHeader* hdr = new FileHeader;

    DirectoryEntry* entry = new DirectoryEntry();
    //entry->inUse = TRUE;
    StrCopy(name,entry->name);
    //printf("String Copy:%s %s\n",name,entry->name);
    StrCopy(type,entry->type);
    StrCopy(directory,entry->directory);
    entry->fileSector = fileSector;
    entry->parent = NULL;
    entry->selfSector = selfSector;
    if(directory==NULL){
        root->push_back(entry);
        //printf("Pushing Entry %s\n",entry->name);
        tableSize++;
    }
    else{
        std::vector<std::string> split_vector = Split(directory,'\\');
        const char* parentName = split_vector[split_vector.size()-1].c_str();
        split_vector.pop_back();
        DirectoryEntry* parent = FindEntry(parentName,Join(split_vector,'\\').c_str());
        if(parent->selfSector==-1)
            return false;
        entry->parent = parent;
        parent->child->push_back(entry);
        parent->childSize++;
    }
    return TRUE;	// no space.  Fix when we have extensible files.
}

bool VectorRemove(std::vector<DirectoryEntry*>* vector,DirectoryEntry *entry){
    std::vector<DirectoryEntry*>::iterator iter = vector->begin();
    for(;iter!=vector->end();iter++){
        if((**iter) == *entry){
            iter = vector->erase(iter);
            return TRUE;
        }
        else{
            iter++;
        }
    }
    return FALSE;
}

bool MyStrCmp(char* a,char* b ){
    if(a==NULL && b==NULL){
        return 0;
    }
    if(a==NULL || b == NULL){
        return 1;
    }
    return strcmp(a,b);
}
bool DirectoryEntry::operator==(DirectoryEntry &temp){
    return !(MyStrCmp(name,temp.name)&&MyStrCmp(type,temp.type)&&MyStrCmp(directory,temp.directory));
}
//----------------------------------------------------------------------
// Directory::Remove
// 	Remove a file name from the directory.  Return TRUE if successful;
//	return FALSE if the file isn't in the directory. 
//
//	"name" -- the file name to be removed
//----------------------------------------------------------------------

bool
Directory::Remove(char *name,char* dir)
{ 
    DEBUG('f',"Remove Directory\n");
    DirectoryEntry* entry= FindEntry(name,dir);
    if(entry==NULL){
        return FALSE;
    }
    DirectoryEntry* parent= entry->parent;
    if (parent==NULL){
        VectorRemove(root,entry);
        tableSize--;
    }
	else{
        VectorRemove((parent->child),entry);
        parent->childSize--;
    }
    delete entry;
    return TRUE;	
}
void PrintEntry(DirectoryEntry* entry,int indent){
    for(int i=0;i<indent;i++)printf("-");
    printf("%s\n",entry->name);
    for(int i =0;i<entry->child->size();i++){
        PrintEntry((*(entry->child))[i],indent+1);
    }
}
//----------------------------------------------------------------------
// Directory::List
// 	List all the file names in the directory. 
//----------------------------------------------------------------------

void
Directory::List()
{
    for(int i=0;i<root->size();i++){
        PrintEntry((*root)[i],0);
    }
	    
}

//----------------------------------------------------------------------
// Directory::Print
// 	List all the file names in the directory, their FileHeader locations,
//	and the contents of each file.  For debugging.
//----------------------------------------------------------------------


void
Directory::Print()
{ 
    FileHeader *hdr = new FileHeader;

    
    printf("Directory contents:\n");
    for (int i = 0; i < tableSize; i++){
	    printf("Name: %s, Sector: %d\n", (*root)[i]->name, (*root)[i]->fileSector);
        hdr->FetchFrom((*root)[i]->fileSector);
	    hdr->Print();
	}
    printf("\n");
    delete hdr;

}




DirectoryEntry *
Directory::Find(std::vector<DirectoryEntry*>* vector,const char* name){
    for(int i=0;i<vector->size();i++){
        if(!strcmp((*vector)[i]->name,name)){
            return ((*vector)[i]);
        }
    }
    
    return NULL;
}

DirectoryEntry *
Directory::FindEntry(const char* name,const char *dir){
    std::vector<DirectoryEntry*>* entryDir = root;
    if(dir==NULL){
        DirectoryEntry* tmp =  Find(entryDir,name);
        //printf("Wtf\n");
        return tmp;
    }
    else{
        std::vector<std::string> dirs = Split(std::string(dir),'\\');
        
        for(int i=0;i<dirs.size();i++){
            entryDir=Find(entryDir,dirs[i].c_str())->child;
        }
        return Find(entryDir,name);
    }

    return NULL;
}


