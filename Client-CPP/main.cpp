/**
 * @file main.cpp
 * @brief Test entry point for the Client-CPP application.
 *
 * @ingroup Client-CPP
 *
 * Runs the client polling loop, sends requests to the local server, and parses
 * the returned response.
 *
 * @note This program runs an infinite loop until interrupted by the user.
 */

#include <iostream>
#include <chrono>
#include <ctime>
#include <thread>
#include "Client.hpp"
#include "Request.hpp"
#include "HTTP.hpp"
#include "Response.hpp"

/**
 * @brief Runs the client request loop.
 *
 * Builds the request URL, polls the local server periodically, and processes
 * the received response.
 *
 * @return Always returns 0 if the loop exits normally.
 *
 * @note The main loop is intended to run indefinitely.
 */
int main()
{
    HTTPClient http;
    Client client(2, "Stockholm");

    std::string req = "http://localhost:8080/id=" + std::to_string(client.GetID());

    bool once = false;

    int fetched_minute = -1;

    while (true)
    {

        auto now = std::chrono::system_clock::now();
        std::time_t now_time = std::chrono::system_clock::to_time_t(now);

        std::tm *local_time = std::localtime(&now_time);

        int minutes = local_time->tm_min;

        if (!once || ((minutes % 15 == 0) && minutes != fetched_minute))
        {

            fetched_minute = minutes;

            Request request(req, http);

            std::string res = request.SendRequest();

            Response response(res);
            response.Parse();

            response.FormatResponse();
            once = true;
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    return 0;
}
