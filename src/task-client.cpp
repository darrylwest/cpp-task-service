//
// 2024-12-22 06:29:29 dpw
//


#include <httplib.h>
#include <spdlog/spdlog.h>

#include <thread>
#include <iostream>
#include <taskservice/version.hpp>
#include <taskservice/taskdb.hpp>
#include <vendor/ansi_colors.hpp>

using namespace colors;

struct Config {
    std::string host = "localhost";
    std::string port = "2032";
};

Config parse_cli(const int argc, char** argv) {
    auto config = Config();

    return config;
}

int client_loop(const Config& config) {
    // create the client
    std::string svc = "https://" + config.host + ":" + config.port;
    httplib::Client client(svc);
    client.enable_server_certificate_verification(false);

    // create the db to store launched tasks
    std::vector<taskservice::Task> db = {};

    // loop
    while (true) {
        if (auto res = client.Get("/queue")) {
            if (res->status == 200) {
                spdlog::info("task: {}", res->body);
                // now parse and compare time
                // run the task if it's new
            } else {
                spdlog::warn("task service at {} is donw..", svc);
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    }

    return 0;
}

// 
int main(int argc, char** argv) {
    auto config = parse_cli(argc, argv);

    auto vers = taskservice::Version();
    std::cout << cyan << "Task Client, Version "  << vers << reset << std::endl;

    return client_loop(config);
}
