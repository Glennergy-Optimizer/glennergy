#ifndef TCPSERVER_H
#define TCPSERVER_H

#include "../Libs/Utils/smw.h"

/**
 * @file TCPServer.h
 * @brief Public API for the TCP server module.
 */

/**
 * @defgroup TCPServer TCPServer
 * @brief TCP server interface for accepting incoming connections.
 *
 * The module creates a listening socket, accepts clients, and dispatches each
 * accepted socket to the configured callback.
 * @{
 */

/**
 * @brief Callback function type for handling new TCP connections.
 *
 * @param context User-defined context passed during initialization.
 * @param socket Accepted client socket descriptor.
 *
 * @return 0 on success, negative value on failure.
 */
typedef int (*TCPServer_OnConnection)(void* context, int socket);

/**
 * @brief TCP server state.
 *
 * @note Allocated by TCPServer_Initialize() and released by TCPServer_Dispose().
 * @note `server_socket` is opened by TCPServer_Listen() and closed during dispose.
 */
typedef struct {
    smw_task *task;                  /**< Task handling the server work loop. */
    TCPServer_OnConnection onConnect;/**< Callback invoked on new connection. */
    void* context;                   /**< User-defined context pointer. */
    int server_socket;               /**< Listening socket descriptor. */
    int backlog;                     /**< Listen backlog size. */
    int port;                        /**< Listening port. */
} TCPServer;

/**
 * @brief Allocates and initializes a TCPServer instance.
 *
 * @param _TCPServer Output pointer for the allocated server instance.
 * @param port Port number to listen on.
 * @param backlog Maximum pending connections in the listen queue.
 * @param callback Function invoked for each accepted connection.
 * @param context User-defined pointer passed to the callback.
 *
 * @return 0 on success, negative value on allocation failure.
 *
 * @warning Caller must release the instance with TCPServer_Dispose().
 */
int TCPServer_Initialize(TCPServer **_TCPServer, int port, int backlog, TCPServer_OnConnection callback, void *context);

/**
 * @brief Starts listening for incoming TCP connections.
 *
 * @param _TCPServer Pointer to an initialized TCPServer instance.
 *
 * @return 0 on success, negative value on error.
 *
 * @warning Creates the internal server task with smw_create_task().
 */
int TCPServer_Listen(TCPServer *_TCPServer);

/**
 * @brief Accepts a single incoming connection.
 *
 * @param _TCPServer Pointer to an initialized TCPServer instance.
 *
 * @return 0 on success, negative value on error or when no connection is available.
 */
int TCPServer_Accept(TCPServer *_TCPServer);

/**
 * @brief Closes a client socket.
 *
 * @param socket Socket descriptor to close.
 */
void TCPServer_Disconnect(int socket);

/**
 * @brief Releases all resources associated with a TCPServer instance.
 *
 * @param _TCPServer Pointer to a TCPServer pointer.
 *
 * @warning Closes the listening socket and destroys the internal task.
 */
void TCPServer_Dispose(TCPServer** _TCPServer);

/** @} */ // end of TCPServer group

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
#endif
