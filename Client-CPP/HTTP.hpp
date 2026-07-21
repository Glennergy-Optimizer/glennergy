/**
 * @file HTTP.hpp
 * @brief Public API for the HTTP client helper.
 *
 * Provides a small wrapper around libcurl for performing GET requests and
 * collecting the response in memory.
 *
 * @ingroup Client-CPP
 *
 * @note Requests perform blocking network I/O.
 */
#pragma once
#include <string>

/**
 * @brief HTTP client helper that stores the latest response in memory.
 *
 * The class is a lightweight wrapper around a GET request helper and a
 * libcurl write callback.
 */
class HTTPClient
{
private:
    std::string data;

    /**
     * @brief Appends received response data to the destination string.
     *
     * @param contents Pointer to the received buffer.
     * @param size Size of one element in the buffer.
     * @param nmemb Number of elements in the buffer.
     * @param userp Pointer to the destination std::string.
     *
     * @return Number of bytes consumed from the callback input.
     */
    static size_t HTTPClient_WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);
public:

    /**
     * @brief Performs an HTTP GET request and returns the response body.
     *
     * @param request Request URL.
     *
     * @return Response body on success, or an empty string on failure.
     */
    std::string HTTPClient_GET(const std::string& request);


};
