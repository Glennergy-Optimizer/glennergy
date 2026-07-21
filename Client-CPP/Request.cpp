/**
 * @file Request.cpp
 * @brief Implementation of the Request class.
 *
 * @ingroup ClientCPP
 */

#include "Request.hpp"
#include <iostream>

/**
 * @brief Implementation of Request::SendRequest.
 *
 * See header for full contract documentation.
 */
std::string Request::SendRequest()
{
    std::string response = this->http_client.HTTPClient_GET(this->request);


    if(response.empty())
    {
        std::cout << "Failed to fetch data" << std::endl;
    }

    return response;
}
