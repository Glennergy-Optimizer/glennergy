#pragma once
#include <string>
#include <vector>

struct ParsedData{
int id;
double percentage;
std::string time;
};

class Response{
private:
std::string raw_response;
std::vector<ParsedData> data;


public:
Response(const std::string& _Response) : raw_response(_Response){}


int Parse();

int FormatResponse();

};