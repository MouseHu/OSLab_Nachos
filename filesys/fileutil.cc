#include "fileutil.h"
#include <string>
#include <vector>
#include <sstream>
#include "directory.h"
const char* Join(std::vector<std::string> vec, char join)
{
    if(vec.size()==0)
        return NULL;
    std::stringstream joined;
    for(int i=0;i<vec.size()-1;i++){
        joined<<vec[i]<<join;
    }
    joined<<vec[vec.size()-1];
    return joined.str().c_str();
}

std::vector<std::string> Split(const std::string& s, char delimiter)
{
    std::vector<std::string> tokens = *(new std::vector<std::string>());
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter))
    {
        if(token[token.length()-1]==delimiter)
            token = token.substr(0,token.length()-1);
        // printf("???%s\n",token.c_str());
        tokens.push_back(token);
    }
    return tokens;
}
void StrCopy(const char* src,char* dst){
    if(src==NULL){
        dst = NULL;
        return;
    }
    int size = 0;
    while(src[size++]!='\0');
    // size--;
    // printf("copy size:%d\n",size);
    //dst = new char[size];
    for(int i=0;i<size;i++){
        dst[i] = src[i];
    }
    // printf("copy finished.\n");
}

bool StrCmp(char* a,char* b ){
    if(a==NULL && b==NULL){
        return 0;
    }
    if(a==NULL || b == NULL){
        return 1;
    }
    return strcmp(a,b);
}

bool VectorRemove(std::vector<DirectoryEntry*>* vector,DirectoryEntry *entry){
    std::vector<DirectoryEntry*>::iterator iter = vector->begin();
    for(;iter!=vector->end();){
        if(*iter == entry){
            iter = vector->erase(iter);
            return TRUE;
        }else if((**iter) == *entry){
            // printf("Vector Remove:%s %s\n",entry->name,(*iter)->name);
            iter = vector->erase(iter);
            return TRUE;
        }
        else{
            iter++;
        }
    }
    return FALSE;
}