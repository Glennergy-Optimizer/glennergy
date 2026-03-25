#include <iostream>
#include "Response.hpp"
#include "json.hpp"

int Response::Parse()
{
    nlohmann::json arr = nlohmann::json::parse(this->raw_response);

    if (arr.is_array())
    {
        for (auto &item : arr)
        {
            ParsedData temp;
            temp.id = item["id"];
            temp.percentage = item["type"];
            temp.time = item["timestamp"];
            this->data.push_back(temp);
        }
    }
    else if (arr.is_object())
    {
        return -1;
    }

    return 0;
}

int Response::FormatResponse()
{
    system("clear");
    const std::string RESET = "\033[0m";
    const std::string GREEN = "\033[32m";
    const std::string RED = "\033[31m";
    const std::string YELLOW = "\033[33m";
    const std::string CYAN = "\033[36m";
    const std::string BOLD = "\033[1m";

    std::cout << BOLD << CYAN;
    std::cout << "=============================================\n";
    std::cout << "               Glennergy Optimizer       \n";
    std::cout << "=============================================\n";
    std::cout << RESET;

    std::cout << std::left
              << std::setw(26) << "Time"
              << std::setw(8) << "Percent"
              << std::setw(10) << "Action"
              << "\n";

    std::cout << "---------------------------------------------\n";

    for (auto &item : this->data)
    {
        std::string action;
        std::string color;

        if (item.percentage < 0.45)
        {
            action = "BUY";
            color = GREEN;
        }
        else if (item.percentage > 0.55)
        {
            action = "SELL";
            color = RED;
        }
        else
        {
            action = "IDLE";
            color = YELLOW;
        }

        std::cout << std::left
                  << std::setw(12) << item.time
                  << std::setw(2) << std::fixed << " | " << std::setprecision(2) << item.percentage
                  << " | " << color << std::setw(10) << action << RESET
                  << "\n";
    }

    std::cout << "=============================================\n";

    return 0;
}