/**
 * @file Pipes.c
 * @brief Implementation of the Pipes module.
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
 * @brief Read up to a requested number of bytes from a file descriptor.
 *
 * Continues reading until the requested size is satisfied, EOF is reached,
 * a non-recoverable error occurs, or the descriptor reports EAGAIN.
 *
 * @param[in] _Fd File descriptor to read from.
 * @param[out] _Buf Destination buffer for the read data.
 * @param[in] _Size Maximum number of bytes to read.
 * @return Number of bytes read so far, which may be smaller than @_Size on EOF
 * or EAGAIN.
 *
 * @note EINTR is treated as a retry condition.
 * @warning On read errors other than EAGAIN, the function logs the error and
 * returns the bytes read so far without changing the buffer contract.
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
 * @brief Write up to a requested number of bytes to a file descriptor.
 *
 * Continues writing until the requested size is satisfied or the descriptor
 * reports EAGAIN.
 *
 * @param[in] _Fd File descriptor to write to.
 * @param[in] _Buf Source buffer containing the bytes to write.
 * @param[in] _Size Number of bytes to write.
 * @return Number of bytes written so far, which may be smaller than @_Size on
 * EAGAIN.
 *
 * @note The function retries implicit short writes by continuing the loop until
 * all data is written or an error stops progress.
 * @warning On write errors other than EAGAIN, the function logs the error and
 * keeps the current total unchanged for that iteration.
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
