#ifndef TCPSERVER_H
#define TCPSERVER_H

#include "../Libs/Utils/smw.h"


typedef int (*TCPServer_OnConnection)(void* context, int socket);

typedef struct {
  smw_task *task;
  TCPServer_OnConnection onConnect;
  void* context;
  int server_socket;
  int backlog;
  int port;
} TCPServer;

int TCPServer_Initialize(TCPServer **_TCPServer, int port, int backlog, TCPServer_OnConnection callback, void *context);

int TCPServer_Listen(TCPServer *_TCPServer);

void TCPServer_Disconnect(int socket);


void TCPServer_Dispose(TCPServer** _TCPServer);


#endif
/*#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#define BUFFER_SIZE 4096

typedef struct {

    int server_socket;
    int port;
    int backlog;
    int running;
} ServerConfig;


int server_Init(ServerConfig *config);
void server_Start(ServerConfig *config);
void server_Stop(ServerConfig *config);

void handle_Connection(int connection_socket);

#endif // TCP_SERVER_H*/
