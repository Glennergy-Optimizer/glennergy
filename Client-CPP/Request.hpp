#pragma once
#include <string>
#include "HTTP.hpp"

class Request{
private:
std::string request;
HTTPClient& http_client;

public:
Request(std::string& _Request, HTTPClient& _HTTPClient) : request(_Request), http_client(_HTTPClient){}

std::string SendRequest();




};