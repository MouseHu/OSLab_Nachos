#ifndef FILESYS_UTIL_H
#define FILESYS_UTIL_H
#include <string>
#include <vector>
#include <sstream>
#include "directory.h"
const char* Join(std::vector<std::string> vec, char join);

std::vector<std::string> Split(const std::string& s, char delimiter);
void StrCopy(const char* src,char* dst);

bool StrCmp(char* a,char* b );

bool VectorRemove(std::vector<DirectoryEntry*>* vector,DirectoryEntry *entry);
#endif