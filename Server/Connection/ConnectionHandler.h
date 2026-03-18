#ifndef CONNECTIONHANDLER_H
#define CONNECTIONHANDLER_H

#include "../../Libs/Utils/smw.h"
#include "../TCPServer.h"
#include "Connection.h"

typedef int (*Callback)(Connection* _Connection);


typedef struct{
    TCPServer* tcp_server;
    Callback client_add;
}ConnectionHandler;


int ConnectionHandler_Initialize(ConnectionHandler **_ConnectionHandler, int _Port, Callback _Callback);


void ConnectionHandler_Dispose(ConnectionHandler** _ConnectionHandler);





#endif