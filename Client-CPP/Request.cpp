#include "Request.hpp"
#include <iostream>


std::string Request::SendRequest()
{
    std::string response = this->http_client.HTTPClient_GET(this->request);


    if(response.empty())
    {
        std::cout << "Failed to fetch data" << std::endl;
    }

    return response;
}