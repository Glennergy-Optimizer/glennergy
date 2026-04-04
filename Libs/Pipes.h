#ifndef __PIPES_H__
#define __PIPES_H__

/**
 * @file Pipes.h
 * @brief Public API for binary pipe and FIFO helper functions.
 *
 * Provides small wrappers for creating FIFOs and reading or writing binary
 * data through pipe file descriptors.
 *
 * @defgroup Libs Libs
 * @brief Shared helper utilities used across the project.
 *
 * The Libs module contains reusable infrastructure such as sockets, threads,
 * pipes, shared memory, pipes, fetchers, and cache helpers.
 * @{
 */

// Todo - nothing =)

#include <unistd.h>

/**
 * @brief Creates a FIFO at the requested path.
 *
 * @param _Path Filesystem path where the FIFO should be created.
 * @return Result of the FIFO creation routine.
 */
int Pipes_CreateFifo(const char* _Path);

/**
 * @brief Reads binary data from a pipe file descriptor.
 *
 * @param _Fd File descriptor to read from.
 * @param _Buf Destination buffer for the read data.
 * @param _Size Number of bytes to read.
 * @return Number of bytes actually read.
 */
ssize_t Pipes_ReadBinary(int _Fd, void* _Buf, size_t _Size);

/**
 * @brief Writes binary data to a pipe file descriptor.
 *
 * @param _Fd File descriptor to write to.
 * @param _Buf Source buffer containing the data to write.
 * @param _Size Number of bytes to write.
 * @return Number of bytes actually written.
 */
ssize_t Pipes_WriteBinary(int _Fd, void* _Buf, size_t _Size);

/** @} */

#endif /* PIPES_H */
