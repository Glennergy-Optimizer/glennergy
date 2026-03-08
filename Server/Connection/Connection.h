#ifndef CONNECTION_H
#define CONNECTION_H

#include <stdint.h>
#include "../../Libs/Utils/smw.h"

typedef struct APIHandler_t APIHandler_t;

typedef struct{
    int socket;
    uint64_t timeout;
    APIHandler_t *api_ctx;
}Connection;


int Connection_Initialize(Connection** _Connection, int _Socket, APIHandler_t *api_ctx);

int Connection_Handle(Connection* _Connection);

void Connection_Dispose(Connection** _Connection);

#endif