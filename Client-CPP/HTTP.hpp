#pragma once
#include <string>


class HTTPClient
{
private:
    std::string data;

    static size_t HTTPClient_WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);
public:

    std::string HTTPClient_GET(const std::string& request);


};