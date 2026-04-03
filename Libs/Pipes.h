#ifndef __PIPES_H__
#define __PIPES_H__
// Todo - nothing =)

#include <unistd.h>

/**
 * @file Pipes.h
 * @brief Public API for FIFO and binary pipe helper functions.
 *
 * Provides a small interface for creating FIFOs and reading or writing binary
 * data through file descriptors.
 */

/**
 * @defgroup Pipes Pipes
 * @brief FIFO and binary pipe helper utilities.
 *
 * This module wraps basic FIFO creation and binary read/write operations used
 * by other parts of the repository.
 *
 * @note The helpers operate on existing file descriptors and paths supplied by
 * the caller.
 * @note The functions do not manage the lifetime of caller-owned buffers or
 * file descriptors.
 * @{
 */

/**
 * @brief Create a FIFO at the specified filesystem path.
 *
 * @param[in] _Path Path where the FIFO should be created.
 *
 * @return
 * - 0 on success
 * - -1 on error
 *
 * @pre _Path must point to a valid, null-terminated string.
 * @post A FIFO exists at the requested path when the call succeeds.
 * @warning The function may fail due to filesystem permissions, existing
 * paths, or invalid input.
 * @note The path string is owned by the caller and is not modified.
 */
int Pipes_CreateFifo(const char* _Path);

/**
 * @brief Read binary data from a file descriptor.
 *
 * @param[in] _Fd File descriptor to read from.
 * @param[out] _Buf Destination buffer for the data.
 * @param[in] _Size Number of bytes requested from the descriptor.
 *
 * @return Number of bytes read on success, or a negative value on error.
 *
 * @pre _Fd must refer to a readable file descriptor.
 * @pre _Buf must point to a writable buffer of at least _Size bytes.
 * @post The buffer contains the bytes read from the file descriptor.
 * @warning Partial reads may occur depending on the underlying pipe or FIFO
 * state.
 * @note The buffer is owned by the caller.
 */
ssize_t Pipes_ReadBinary(int _Fd, void* _Buf, size_t _Size);

/**
 * @brief Write binary data to a file descriptor.
 *
 * @param[in] _Fd File descriptor to write to.
 * @param[in] _Buf Source buffer containing the data to send.
 * @param[in] _Size Number of bytes to write.
 *
 * @return Number of bytes written on success, or a negative value on error.
 *
 * @pre _Fd must refer to a writable file descriptor.
 * @pre _Buf must point to a readable buffer of at least _Size bytes.
 * @post The requested data has been submitted to the descriptor when the call
 * succeeds.
 * @warning Partial writes may occur depending on the underlying pipe or FIFO
 * state.
 * @note The buffer is owned by the caller and is not modified.
 */
ssize_t Pipes_WriteBinary(int _Fd, void* _Buf, size_t _Size);

/** @} */

#endif /* PIPES_H */
