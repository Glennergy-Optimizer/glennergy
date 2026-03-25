#pragma once
#include <string>

class Client{
private:
int id;
std::string property;

public:
Client(int client_id, const std::string& property_name) : id(client_id), property(property_name){}

int GetID()
{
    return id;
}


};