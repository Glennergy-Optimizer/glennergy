/**
 * @file Client.hpp
 * @brief Public declaration of the Client class.
 *
 * Defines a small value-like client object with an integer identifier and a
 * string property name.
 *
 * @ingroup CLIENT_CPP
 */
#pragma once
#include <string>

/**
 * @brief Represents a client identifier and associated property name.
 */
class Client{
private:
int id;
std::string property;

public:
Client(int client_id, const std::string& property_name) : id(client_id), property(property_name){}

/**
 * @brief Returns the client identifier.
 *
 * @return The stored client id.
 */
int GetID()
{
    return id;
}


};
