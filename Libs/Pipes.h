#ifndef __PIPES_H__
#define __PIPES_H__

/**
 * @file Pipes.h
 * @brief Public API for binary pipe and FIFO helpers.
 *
 * Provides helper functions for creating FIFOs and performing partial-safe
 * binary reads and writes on file descriptors.
 */

/**
 * @defgroup Pipes Pipes
 * @brief Binary pipe and FIFO helper utilities.
 *
 * This module contains small helpers for FIFO creation and for reading or
 * writing binary data through file descriptors without assuming a single
 * system call will transfer the entire buffer.
 * @{
 */
// Todo - nothing =)

#include <unistd.h>

/**
 * @brief Create a FIFO at the given path.
 *
 * @param[in] _Path Filesystem path where the FIFO should be created.
 *
 * @return 0 on success, or a non-zero error code on failure.
 *
 * @note The FIFO is created in the filesystem and is not opened by this call.
 */
int Pipes_CreateFifo(const char* _Path);

/**
 * @brief Read up to the requested number of bytes from a file descriptor.
 *
 * @param[in] _Fd File descriptor to read from.
 * @param[out] _Buf Destination buffer for the received bytes.
 * @param[in] _Size Maximum number of bytes to read.
 *
 * @return Number of bytes actually read. Returns less than @_Size on EOF or
 * when a non-blocking read would block.
 *
 * @note This helper may return after a partial read.
 * @warning The caller must provide a valid writable buffer of at least
 * @_Size bytes.
 */
ssize_t Pipes_ReadBinary(int _Fd, void* _Buf, size_t _Size);

/**
 * @brief Write up to the requested number of bytes to a file descriptor.
 *
 * @param[in] _Fd File descriptor to write to.
 * @param[in] _Buf Source buffer containing the bytes to write.
 * @param[in] _Size Number of bytes to write from the buffer.
 *
 * @return Number of bytes actually written. Returns less than @_Size when a
 * non-blocking write would block.
 *
 * @note This helper may return after a partial write.
 * @warning The caller must provide a valid readable buffer of at least
 * @_Size bytes.
 */
ssize_t Pipes_WriteBinary(int _Fd, void* _Buf, size_t _Size);

/** @} */

#endif /* PIPES_H */
