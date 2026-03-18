#include "wrapper.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>


//After timeout, recv() will return -1 with errno = EAGAIN/EWOULDBLOCK
int Connection_SetTimeout(int socket, int seconds)
{
    timeval timeout;
    timeout.tv_sec = seconds;
    timeout.tv_usec = 0;
    
    if (setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        return -1;
    }
    
    return 0;
}

int Connection_Recv(int socket, uint8_t *buffer, size_t length)
{
    if (buffer == NULL || length == 0)
        return -1;

    ssize_t bytes_read = recv(socket, buffer, length, 0);
    if (bytes_read < 0)
        return -1;
    if (bytes_read == 0)
        return 0;
    
    return (int)bytes_read;
}

int Connection_Send(int socket, const char *buffer, size_t length)
{
    if (buffer == NULL || length == 0)
        return -1;

    ssize_t bytes_written = send(socket, buffer, length, MSG_NOSIGNAL);
    if (bytes_written < 0)
        return -1;

    return (int)bytes_written;
}
