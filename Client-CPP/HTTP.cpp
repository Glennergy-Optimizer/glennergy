#include "HTTP.hpp"
#include <curl/curl.h>

size_t HTTPClient::HTTPClient_WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    std::string* data = static_cast<std::string*>(userp);
    data->append(static_cast<char *>(contents), size * nmemb);

    return size * nmemb;
}

std::string HTTPClient::HTTPClient_GET(const std::string& request)
{
    CURL *curl;
    CURLcode res;
    std::string response;

    curl = curl_easy_init();
    if (!curl)
    {
        return {};
    }

    curl_easy_setopt(curl, CURLOPT_URL, request.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &HTTPClient::HTTPClient_WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    res = curl_easy_perform(curl);
    if (res != CURLE_OK)
    {
        fprintf(stderr, "Curl_HTTPGet: curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        curl_easy_cleanup(curl);
        return {};
    }

    curl_easy_cleanup(curl);
    return response;
}