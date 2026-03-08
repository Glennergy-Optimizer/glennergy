#ifndef CONNECTIONHANDLER_H
#define CONNECTIONHANDLER_H

#include "../../Libs/Utils/smw.h"
#include "../TCPServer.h"
#include "Connection.h"

typedef int (*Callback)(Connection* _Connection);

typedef struct APIHandler_t APIHandler_t;

typedef struct{
    TCPServer* tcp_server;
    Callback client_add;
    APIHandler_t *api_ctx;
}ConnectionHandler;


int ConnectionHandler_Initialize(ConnectionHandler **_ConnectionHandler, int _Port, Callback _Callback, APIHandler_t *api_ctx);


void ConnectionHandler_Dispose(ConnectionHandler** _ConnectionHandler);





#endif