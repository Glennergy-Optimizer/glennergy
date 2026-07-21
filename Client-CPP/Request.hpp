/**
 * @file Request.hpp
 * @brief Public API for the Request class.
 *
 * Declares a small wrapper around HTTPClient for sending a stored request
 * string and returning the response.
 *
 * @ingroup ClientCPP
 */

/**
 * @defgroup ClientCPP ClientCPP
 * @brief C++ client-side request helpers.
 *
 * Contains the Request wrapper used by the C++ client code to send HTTP
 * requests through the shared HTTPClient implementation.
 * @{
 */

#pragma once
#include <string>
#include "HTTP.hpp"

/**
 * @brief Wraps a request string and the HTTP client used to send it.
 *
 * The request text is stored locally. The HTTP client is referenced and must
 * remain valid for the lifetime of the Request instance.
 */
class Request{
private:
std::string request;
HTTPClient& http_client;

public:
/**
 * @brief Creates a Request instance.
 *
 * @param _Request Request string to store.
 * @param _HTTPClient HTTP client used for sending the request.
 */
Request(std::string& _Request, HTTPClient& _HTTPClient) : request(_Request), http_client(_HTTPClient){}

/**
 * @brief Sends the stored request through the configured HTTP client.
 *
 * @return Response body returned by the HTTP client.
 */
std::string SendRequest();




};

/** @} */
