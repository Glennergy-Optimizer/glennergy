/**
 * @file Pipes.c
 * @brief Binary read and write helpers for pipe file descriptors.
 *
 * @ingroup Libs
 */

#include "Pipes.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
// Todo - nothing =(

/**
 * @brief Reads binary data from a file descriptor until the buffer is filled or the stream stops.
 *
 * Continues reading until the requested number of bytes has been collected, EOF is reached,
 * or a non-retryable error occurs.
 *
 * @param _Fd File descriptor to read from.
 * @param _Buf Destination buffer for the read data.
 * @param _Size Number of bytes requested.
 * @return Number of bytes actually read.
 *
 * @note Returns early with the number of bytes read so far when errno is EAGAIN.
 * @note Interrupted reads are retried automatically when errno is EINTR.
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
 * @brief Writes binary data to a file descriptor until the buffer is exhausted or the stream stops.
 *
 * Continues writing until the requested number of bytes has been sent or a non-retryable error occurs.
 *
 * @param _Fd File descriptor to write to.
 * @param _Buf Source buffer containing the data to write.
 * @param _Size Number of bytes to write.
 * @return Number of bytes actually written.
 *
 * @note Returns early with the number of bytes written so far when errno is EAGAIN.
 * @note Successful writes are logged with a printf message.
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
