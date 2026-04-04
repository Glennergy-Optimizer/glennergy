/**
 * @file Pipes.c
 * @brief Binary pipe read and write helpers.
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
 * @brief Reads up to the requested number of bytes from a file descriptor.
 *
 * Continues until the buffer is filled, EOF is reached, or a non-retryable
 * error occurs. Returns the number of bytes actually read.
 *
 * @param _Fd File descriptor to read from.
 * @param _Buf Destination buffer.
 * @param _Size Maximum number of bytes to read.
 * @return Number of bytes stored in the buffer.
 *
 * @note Returns early on `EAGAIN` with the number of bytes read so far.
 * @note Retries automatically when interrupted by a signal.
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
 * @brief Writes up to the requested number of bytes to a file descriptor.
 *
 * Continues until the buffer is fully written or a non-retryable error occurs.
 * Returns the number of bytes actually written.
 *
 * @param _Fd File descriptor to write to.
 * @param _Buf Source buffer.
 * @param _Size Number of bytes to write.
 * @return Number of bytes written to the file descriptor.
 *
 * @note Returns early on `EAGAIN` with the number of bytes written so far.
 * @note Reports write errors through `perror`.
 * @note Preserves the existing debug print for each successful write call.
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
