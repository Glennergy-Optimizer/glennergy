#ifndef __PIPES_H__
#define __PIPES_H__
// Todo - nothing =)

#include <unistd.h>

/**
 * @file Pipes.h
 * @brief Public API for pipe and FIFO helper functions.
 *
 * Provides helpers for creating FIFOs and for reading or writing binary data
 * through file descriptors.
 */

/**
 * @defgroup Pipes Pipes
 * @brief Pipe and FIFO helper functions.
 *
 * This module wraps common pipe/FIFO operations used by the repository.
 * The helpers work with file descriptors directly and are intended for
 * binary-safe I/O.
 * @{
 */

/**
 * @brief Create a FIFO at the specified path.
 *
 * @param[in] _Path Filesystem path where the FIFO should be created.
 *
 * @return Implementation-specific status value from the FIFO creation helper.
 */
int Pipes_CreateFifo(const char* _Path);

/**
 * @brief Read binary data from a file descriptor.
 *
 * Attempts to read up to the requested number of bytes and returns the
 * number of bytes actually read.
 *
 * @param[in] _Fd File descriptor to read from.
 * @param[out] _Buf Destination buffer for the data.
 * @param[in] _Size Maximum number of bytes to read.
 *
 * @return Number of bytes read.
 */
ssize_t Pipes_ReadBinary(int _Fd, void* _Buf, size_t _Size);

/**
 * @brief Write binary data to a file descriptor.
 *
 * Attempts to write up to the requested number of bytes and returns the
 * number of bytes actually written.
 *
 * @param[in] _Fd File descriptor to write to.
 * @param[in] _Buf Source buffer for the data.
 * @param[in] _Size Number of bytes to write.
 *
 * @return Number of bytes written.
 */
ssize_t Pipes_WriteBinary(int _Fd, void* _Buf, size_t _Size);

/** @} */

#endif /* PIPES_H */
