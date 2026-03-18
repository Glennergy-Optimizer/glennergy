#ifndef WRAPPER_H
#define WRAPPER_H

#include <stdint.h>
#include <stddef.h>

typedef struct timeval timeval;
// Returns: 0 on success, -1 on error
int Connection_SetTimeout(int socket, int seconds);

// return n bytes read, 0 on close, -1 on error
int Connection_Recv(int socket, uint8_t *buffer, size_t length);
// return n bytes written, -1 on error
int Connection_Send(int socket, const char *buffer, size_t length);

#endif