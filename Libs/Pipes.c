/**
 * @file Pipes.c
 * @brief Implementation of binary FIFO and pipe helper functions.
 *
 * @ingroup Pipes
 */

#include "Pipes.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
// Todo - nothing =(

/**
 * @brief Read up to a fixed number of bytes from a file descriptor.
 *
 * Continues reading until the requested size is satisfied, EOF is reached,
 * an interrupt is observed, or a non-blocking read would block.
 *
 * @param _Fd File descriptor to read from.
 * @param _Buf Destination buffer for the read data.
 * @param _Size Maximum number of bytes to read.
 * @return Number of bytes actually read.
 */
ssize_t Pipes_ReadBinary(int _Fd, void *_Buf, size_t _Size)
{
    size_t total = 0;
    ssize_t bytesRead;
    char *buffer = _Buf;

    while (total < _Size)
    {
        bytesRead = read(_Fd, buffer + total, _Size - total);

        if (bytesRead > 0) {
            total += bytesRead;
            continue;
        }

        if (bytesRead == 0) {
            break; // EOF
        }

        if (bytesRead < 0) {
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

/**
 * @brief Write up to a fixed number of bytes to a file descriptor.
 *
 * Continues writing until the requested size is satisfied or a non-blocking
 * write would block.
 *
 * @param _Fd File descriptor to write to.
 * @param _Buf Source buffer for the data to write.
 * @param _Size Number of bytes to write.
 * @return Number of bytes actually written.
 */
ssize_t Pipes_WriteBinary(int _Fd, void *_Buf, size_t _Size)
{
    ssize_t bytesWritten = 0;
    size_t total = 0;
    char *buffer = _Buf;

    while (total < _Size)
    {
        bytesWritten = write(_Fd, buffer + total, _Size - total);

        if (bytesWritten > 0) {
            total += bytesWritten;
        }

        if (bytesWritten < 0)
        {
            if (errno == EAGAIN) {
                return total;
            }
            perror("write");
        }
        else {
            printf("Wrote %zd bytes to the pipe\n", bytesWritten);
        }
    }

    return total;
}
