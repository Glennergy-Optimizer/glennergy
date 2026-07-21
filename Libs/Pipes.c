/**
 * @file Pipes.c
 * @brief Implementation of the Pipes module.
 *
 * @ingroup PIPES
 */

#include "Pipes.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
// Todo - nothing =(

/**
 * @brief Reads binary data from a file descriptor.
 *
 * Continues until the requested byte count is reached, EOF is encountered,
 * or a non-blocking read returns `EAGAIN`.
 *
 * @param _Fd File descriptor to read from.
 * @param _Buf Buffer that receives the data.
 * @param _Size Number of bytes to read.
 *
 * @return Number of bytes read so far.
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
 * @brief Writes binary data to a file descriptor.
 *
 * Continues until the requested byte count is written or a non-blocking write
 * returns `EAGAIN`.
 *
 * @param _Fd File descriptor to write to.
 * @param _Buf Buffer containing the data to write.
 * @param _Size Number of bytes to write.
 *
 * @return Number of bytes written so far.
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
