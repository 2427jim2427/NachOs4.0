/**************************************************************
 *
 * userprog/ksyscall.h
 *
 * Kernel interface for systemcalls 
 *
 * by Marcus Voelp  (c) Universitaet Karlsruhe
 *
 **************************************************************/

#ifndef __USERPROG_KSYSCALL_H__ 
#define __USERPROG_KSYSCALL_H__ 

#include "kernel.h"

#include "synchconsole.h"


void SysHalt()
{
  kernel->interrupt->Halt();
}

int SysAdd(int op1, int op2)
{
  return op1 + op2;
}


int SysCreate(char *filename)
{
	// return value
	// 1: success
	// 0: failed
	// cout << filename << endl;
	return kernel->interrupt->CreateFile(filename);
	// return 0;
}
// bool SysCreate(char* fileName) {
//     bool success;
//     int fileNameLength = strlen(fileName);

//     if (fileNameLength == 0) {
//         DEBUG(dbgSys, "\nFile name can't be empty");
//         success = false;

//     } else if (fileName == NULL) {
//         DEBUG(dbgSys, "\nNot enough memory in system");
//         success = false;

//     } else {
//         DEBUG(dbgSys, "\nFile's name read successfully");
//         if (!kernel->fileSystem->Create(fileName)) {
//             DEBUG(dbgSys, "\nError creating file");
//             success = false;
//         } else {
//             success = true;
//         }
//     }

//     return success;
// }
OpenFileId sysOpen(char *name)
{
	
	OpenFile *openfile = kernel->interrupt->Open(name);
	return openfile->getID();
}

void SysPrintInt(int value)
{
	kernel->interrupt->PrintInt(value);
}

int Write(char *buffer, int size, OpenFileId id)
{
	OpenFile* openFile = kernel->fileSystem->fileDescriptorTable[id];
	// OpenFile* openFile = kernel->fileSystem->OpenFile;
	if(openFile != nullptr){
		return openFile->Write(buffer, size);
	}
	return -1;
}
int Read(char *buffer, int size, OpenFileId id)
{
	OpenFile* openFile = kernel->fileSystem->fileDescriptorTable[id];
	// OpenFile* openFile = kernel->fileSystem->OpenFile;
	if(openFile != nullptr){
		return openFile->Read(buffer, size);
	}
	return -1;	
}
int sysClose(OpenFileId id)
{
	OpenFile* file = kernel->fileSystem->fileDescriptorTable[id];
    if (file != NULL) {
        delete file;  
        kernel->fileSystem->fileDescriptorTable[id] = NULL; 
		// cout << "close id" << id << endl;
		return 1;
	}	
	else 
		return 0;
	
}
#endif /* ! __USERPROG_KSYSCALL_H__ */
