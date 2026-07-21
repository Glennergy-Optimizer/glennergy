#pragma once
#include <string>
#include <vector>

/**
 * @file Response.hpp
 * @brief Public API for the Response module.
 *
 * Defines the parsed data model and the response parsing and formatting
 * interface used by the client.
 *
 * @defgroup ClientCPP ClientCPP
 * @brief C++ client-side response handling.
 *
 * Parses upstream JSON responses into client data and formats the result for
 * terminal output.
 * @{
 */

/**
 * @brief Parsed response entry.
 *
 * Stores one parsed item from the upstream response.
 */
struct ParsedData{
int id;
double percentage;
std::string time;
};

/**
 * @brief Parses and formats the client response payload.
 *
 * Owns the raw response string and the parsed entries created by Parse().
 */
class Response{
private:
std::string raw_response;
std::vector<ParsedData> data;


public:
Response(const std::string& _Response) : raw_response(_Response){}


int Parse();

int FormatResponse();

};

/** @} */
