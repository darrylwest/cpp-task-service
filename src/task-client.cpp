//
// 2024-12-22 06:29:29 dpw
//


#include <httplib.h>
#include <spdlog/spdlog.h>
#include <cxxopts.hpp>

#include <thread>
#include <iostream>
#include <taskservice/runner.hpp>
#include <taskservice/version.hpp>
#include <taskservice/taskdb.hpp>
#include <vendor/ansi_colors.hpp>

using namespace colors;

struct Config {
    std::string host = "10.0.1.192"; // tiburon.local
    std::string port = "2032";
    int loop_millis = 3000;
    bool verbose = false; // set log to warn

    friend std::ostream& operator<<(std::ostream& os, const Config v) {
        // better to use <format> but it breaks on linux and fmt broken on darwin
        os << "host:  " << v.host << ", "
            << "port:  " << v.port << ", "
            << "timeout:  " << v.loop_millis << ", "
            << "verbose: " << v.verbose << ".";
        return os;
    }

    std::string to_string() const {
        std::ostringstream oss;
        oss << *this;

        return oss.str();
    }
};

Config parse_cli(const int argc, char** argv) {
    auto config = Config();

    // parse the cli
    try {
        cxxopts::Options options("TaskClient", "TLS client to read queue and run jobs.");
        // clang-format off
        options
            .add_options()
                ("p,port", "listening port", cxxopts::value<int>())
                ("H,host", "listening host", cxxopts::value<std::string>())
                ("t,timeout", "loop time in milliseconds", cxxopts::value<int>())
                ("v,verbose", "set the logging to verbose")
                ("V,version", "Show the current version and exit")
                ("h,help", "Show this help")
            ;

        const auto version = taskservice::Version();
        const auto result = options.parse(argc, argv);
        if (result.count("version")) {
            std::cout << "Task Client Version: " << version << std::endl;
            exit(0);
        }

        if (result.count("help")) {
            std::cout << "Task Client Version: " << version << std::endl;
            std::cout << options.help() << std::endl;
            exit(0);
        }

        if (result.count("port")) {
            config.port = result["port"].as<int>();
        }

        if (result.count("host")) {
            config.host = result["host"].as<std::string>();
        }

        if (result.count("timeout")) {
            config.loop_millis = result["timeout"].as<int>();
        }

        if (result.count("verbose")) {
            config.verbose = true;
        }

    } catch (const std::exception& exp) {
        std::cout << "error parsing cli options: " << exp.what() << std::endl;
        exit(1);
    }

    return config;
}

int client_loop(const Config& config) {
    // create the client
    std::string svc = "https://" + config.host + ":" + config.port;
    httplib::Client client(svc);
    client.enable_server_certificate_verification(false);

    // create the db to store launched tasks
    std::vector<taskservice::Task> db = {};

    // lambda to store and run the task
    auto run_task = [&](const taskservice::Task& task) {
        spdlog::info("new task: {}", task.to_string());
        db.push_back(task);
    };

    // loop
    while (true) {
        spdlog::debug("task loop:");
        if (auto res = client.Get("/queue")) {
            if (res->status == 200 && res->body != "0:")  {
                const taskservice::Task task = taskservice::task_from_string(res->body);

                spdlog::debug("parsed task: {}", task.to_string());
                if (task.command != "")  {
                    if (db.empty()) {
                        spdlog::debug("first task: {}", task.to_string());
                        run_task(task);
                    } else {
                        // find it
                        if (db.back().created < task.created) {
                            spdlog::debug("created new task: {}", task.to_string());
                            run_task(task);
                        }
                    }
                }
            } else if (res->status > 299) {
                spdlog::warn("task service at {} is donw..", svc);
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(config.loop_millis));
    }

    return 0;
}

// 
int main(int argc, char** argv) {
    auto config = parse_cli(argc, argv);

    auto vers = taskservice::Version();
    std::cout << cyan << "Task Client, Version "  << vers << reset << std::endl;
    std::cout << yellow << config << reset << std::endl;

    if (config.verbose) {
        spdlog::set_level(spdlog::level::debug);
    } else {
        spdlog::set_level(spdlog::level::warn);
    }

    return client_loop(config);
}
