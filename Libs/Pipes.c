/**
 * @file Pipes.c
 * @brief Implementation of binary pipe read and write helpers.
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
 * @brief Read a binary payload from a file descriptor into a buffer.
 *
 * Repeatedly calls read() until the requested number of bytes has been read,
 * end-of-file is reached, or a non-recoverable error occurs.
 *
 * @param[in] _Fd File descriptor to read from.
 * @param[out] _Buf Destination buffer that receives the read bytes.
 * @param[in] _Size Number of bytes requested.
 * @return Total number of bytes actually read, which may be less than _Size on
 *         EOF, EAGAIN, or error.
 *
 * @pre _Buf points to a writable memory region of at least _Size bytes.
 * @pre _Fd is a valid file descriptor open for reading.
 * @post The first returned-byte-count bytes of _Buf contain data read from _Fd.
 * @post The file offset of _Fd may advance by the number of bytes read.
 *
 * @warning This function performs raw binary reads and does not append a
 *          terminator. On perror() logging, stderr output may be emitted.
 * @note Short reads are returned as-is. EINTR is retried. EAGAIN returns the
 *       number of bytes already read.
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
 * @brief Write a binary payload from a buffer to a file descriptor.
 *
 * Repeatedly calls write() until the requested number of bytes has been
 * written, the call would block, or a non-recoverable error occurs.
 *
 * @param[in] _Fd File descriptor to write to.
 * @param[in] _Buf Source buffer that provides the bytes to write.
 * @param[in] _Size Number of bytes requested to write.
 * @return Total number of bytes actually written, which may be less than _Size
 *         on EAGAIN or error.
 *
 * @pre _Buf points to a readable memory region of at least _Size bytes.
 * @pre _Fd is a valid file descriptor open for writing.
 * @post The first returned-byte-count bytes from _Buf have been submitted to
 *       _Fd.
 * @post The file offset of _Fd may advance by the number of bytes written.
 *
 * @warning This function performs raw binary writes and may emit debug output
 *          with printf() for each successful write call. On perror() logging,
 *          stderr output may be emitted.
 * @note EAGAIN returns the number of bytes already written. The debug print
 *       reports the bytes written by the most recent write() call, including 0
 *       if write() returns 0.
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
