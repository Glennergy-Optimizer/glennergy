#include "fetcher.hpp"
#include <curl/curl.h>

namespace glennergy {

//curl_guard runs only at creation and deletion
struct curl_guard {
    curl_guard()  { curl_global_init(CURL_GLOBAL_DEFAULT); }
    ~curl_guard() { curl_global_cleanup(); }
};

static curl_guard global_curl;


fetcher::fetcher()
    : curl_(curl_easy_init(), curl_easy_cleanup) {}

size_t fetcher::write_cb(char *data, size_t size, size_t nmemb, std::string *out) {
    out->append(data, size * nmemb);
    return size * nmemb;
}

std::string fetcher::get(const std::string &url) {
    std::string response;

    if (curl_) {
        curl_easy_setopt(curl_.get(), CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl_.get(), CURLOPT_WRITEFUNCTION, write_cb);
        curl_easy_setopt(curl_.get(), CURLOPT_WRITEDATA, &response);
        curl_easy_perform(curl_.get());
    }

    return response;
}

} // namespace glennergy
