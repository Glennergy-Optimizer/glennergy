#ifndef CONNECTION_H
#define CONNECTION_H

#include <stdint.h>
#include "../../Libs/Utils/smw.h"

typedef struct{
    int socket;
    uint64_t timeout;
    int bytesReadOut;
}Connection;


int Connection_Initialize(Connection** _Connection, int _Socket);

int Connection_Handle(Connection* _Connection);

void Connection_Dispose(Connection** _Connection);

#endif