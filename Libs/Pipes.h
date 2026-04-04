#ifndef __PIPES_H__
#define __PIPES_H__

/**
 * @file Pipes.h
 * @brief Public API for pipe and FIFO helper functions.
 *
 * Provides helpers for FIFO creation and for binary reads and writes that
 * retry on short transfers and handle common non-fatal errors.
 *
 * @ingroup Pipes
 */

/**
 * @defgroup Pipes Pipes
 * @brief Pipe and FIFO helper functions.
 *
 * This module wraps low-level pipe and FIFO operations used by the project.
 * The helpers focus on binary data transfer and basic FIFO creation while
 * preserving the caller's buffer ownership.
 *
 * @{
 */

#include <unistd.h>

/**
 * @brief Create a FIFO at the given path.
 *
 * @param[in] _Path Filesystem path where the FIFO should be created.
 *
 * @return Implementation-specific success or failure status.
 *
 * @note The function operates on the filesystem path provided by the caller.
 */
int Pipes_CreateFifo(const char* _Path);

/**
 * @brief Read binary data from a file descriptor.
 *
 * @param[in] _Fd File descriptor to read from.
 * @param[out] _Buf Destination buffer for the read data.
 * @param[in] _Size Maximum number of bytes to read.
 *
 * @return Number of bytes read, which may be smaller than @_Size on EOF or
 * transient unavailability.
 *
 * @note The caller owns the destination buffer and must ensure it is valid for
 * writes of at least @_Size bytes.
 */
ssize_t Pipes_ReadBinary(int _Fd, void* _Buf, size_t _Size);

/**
 * @brief Write binary data to a file descriptor.
 *
 * @param[in] _Fd File descriptor to write to.
 * @param[in] _Buf Source buffer containing the bytes to write.
 * @param[in] _Size Number of bytes to write.
 *
 * @return Number of bytes written, which may be smaller than @_Size on
 * transient unavailability.
 *
 * @note The caller remains responsible for the source buffer lifetime during
 * the call.
 */
ssize_t Pipes_WriteBinary(int _Fd, void* _Buf, size_t _Size);

/** @} */

#endif /* PIPES_H */
