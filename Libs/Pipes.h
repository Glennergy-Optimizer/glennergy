#ifndef __PIPES_H__
#define __PIPES_H__
// Todo - nothing =)

/**
 * @file Pipes.h
 * @brief Public API for pipe and FIFO helpers.
 *
 * Provides helper functions for FIFO creation and binary pipe I/O.
 *
 * @defgroup PIPES PIPES
 * @brief Pipe and FIFO helper functions.
 *
 * Helpers for creating FIFOs and reading or writing binary data through
 * file descriptors.
 * @{
 */

#include <unistd.h>

/**
 * @brief Creates a FIFO at the given path.
 *
 * @param _Path Path where the FIFO should be created.
 *
 * @return Result from the FIFO creation routine.
 */
int Pipes_CreateFifo(const char* _Path);

/**
 * @brief Reads binary data from a file descriptor.
 *
 * @param _Fd File descriptor to read from.
 * @param _Buf Buffer that receives the data.
 * @param _Size Number of bytes to read.
 *
 * @return Number of bytes read.
 */
ssize_t Pipes_ReadBinary(int _Fd, void* _Buf, size_t _Size);

/**
 * @brief Writes binary data to a file descriptor.
 *
 * @param _Fd File descriptor to write to.
 * @param _Buf Buffer containing the data to write.
 * @param _Size Number of bytes to write.
 *
 * @return Number of bytes written.
 */
ssize_t Pipes_WriteBinary(int _Fd, void* _Buf, size_t _Size);

/** @} */

#endif /* PIPES_H */
