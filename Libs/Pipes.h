#ifndef PIPES_H
#define PIPES_H

#include <unistd.h>



int Pipes_CreateFifo(const char* _Path);


ssize_t Pipes_ReadBinary(int _Fd, void* _Buf, size_t _Size);

ssize_t Pipes_WriteBinary(int _Fd, void* _Buf, size_t _Size);



#endif