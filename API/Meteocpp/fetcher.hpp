#pragma once

#include <memory>
#include <string>

typedef void CURL;

namespace glennergy {

using curl_deleter = void(*)(CURL *);
using curl_ptr = std::unique_ptr<CURL, curl_deleter>;

class fetcher {
public:
    fetcher();
    std::string get(const std::string &url);

private:
    static size_t write_cb(char *data, size_t size, size_t nmemb, std::string *out);
    curl_ptr curl_;
};

} // namespace glennergy
