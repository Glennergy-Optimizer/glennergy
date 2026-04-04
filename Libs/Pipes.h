#ifndef __PIPES_H__
#define __PIPES_H__
// Todo - nothing =)

/**
 * @file Pipes.h
 * @brief Public API for binary pipe and FIFO helpers.
 *
 * Provides helper functions for creating FIFOs and performing blocking-safe
 * binary reads and writes on file descriptors.
 */

/**
 * @defgroup Pipes Pipes
 * @brief Binary pipe and FIFO helpers
 *
 * Utilities for creating named pipes and transferring raw byte buffers
 * through file descriptors.
 *
 * @note These helpers operate directly on file descriptors and may block
 * depending on the descriptor state and kernel behavior.
 * @{
 */

#include <unistd.h>

/**
 * @brief Creates a FIFO at the given path.
 *
 * @param _Path Filesystem path where the FIFO should be created.
 *
 * @return Implementation-defined status code from the FIFO creation helper.
 *
 * @note See the implementation for the exact return-value contract.
 */
int Pipes_CreateFifo(const char* _Path);

/**
 * @brief Reads binary data from a file descriptor into a buffer.
 *
 * @param _Fd File descriptor to read from.
 * @param _Buf Destination buffer.
 * @param _Size Maximum number of bytes to read.
 *
 * @return Number of bytes actually read.
 *
 * @note See the implementation for retry and short-read behavior.
 */
ssize_t Pipes_ReadBinary(int _Fd, void* _Buf, size_t _Size);

/**
 * @brief Writes binary data from a buffer to a file descriptor.
 *
 * @param _Fd File descriptor to write to.
 * @param _Buf Source buffer.
 * @param _Size Number of bytes to write.
 *
 * @return Number of bytes actually written.
 *
 * @note See the implementation for retry and short-write behavior.
 */
ssize_t Pipes_WriteBinary(int _Fd, void* _Buf, size_t _Size);

/** @} */

#endif /* PIPES_H */
