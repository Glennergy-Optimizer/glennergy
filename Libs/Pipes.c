#include "Pipes.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

ssize_t Pipes_ReadBinary(int _Fd, void *_Buf, size_t _Size)
{
    size_t total = 0;
    ssize_t bytesRead;
    char *buffer = _Buf;

    while (total < _Size)
    {
        bytesRead = read(_Fd, buffer + total, _Size - total);

        if (bytesRead > 0)
        {
            total += bytesRead;
            continue;
        }

        if(bytesRead == 0)
        {
            break;
        }

        if (bytesRead < 0)
        {
            if (errno == EINTR)
                continue; // Retry if interrupted by signal
        
            if (errno == EAGAIN)
            {
                return total;
            }
            perror("read");
        }
    }

    return total;
}

ssize_t Pipes_WriteBinary(int _Fd, void *_Buf, size_t _Size)
{
    ssize_t bytesWritten = 0;
    size_t total = 0;
    char *buffer = _Buf;

    while (total < _Size)
    {
        bytesWritten = write(_Fd, buffer + total, _Size - total);

        if (bytesWritten > 0)
        {
            total += bytesWritten;
        }

        if (bytesWritten < 0)
        {

            if (errno == EAGAIN)
            {
                return total;
            }
            perror("write");
        }
        else
        {
            printf("Wrote %zd bytes to the pipe\n", bytesWritten);
        }
    }
    return total;
}
