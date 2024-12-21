//
// 2024-12-10 18:27:44 dpw
//

#include <httplib.h>
#include <spdlog/spdlog.h>

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <taskservice/logging.hpp>
#include <taskservice/runner.hpp>
#include <taskservice/service.hpp>
#include <taskservice/taskdb.hpp>
#include <thread>

using namespace httplib;

namespace taskservice {

    // Function to set up the server and apply configurations
    bool setup_service(SSLServer& svr, const Config& config) {
        if (svr.is_valid() == 0) {
            spdlog::error("ERROR! Server is not valid. Check the cert/key files? Exiting...");
            return false;
        }

        // Error handler
        svr.set_error_handler([](const Request& req, Response& res) {
            spdlog::error("ERROR! bad request {} {}", req.method.c_str(), req.path.c_str());

            const char* fmt = "Error Status: %d\n";
            char buf[BUFSIZ];
            snprintf(buf, sizeof(buf), fmt, res.status);
            res.set_content(buf, "text/plain");
        });

        // Logger
        svr.set_logger([](const Request& req, const Response& res) {
            taskservice::log_request(req, res);
        });

        svr.Get("/", [](const Request& req, Response& res) {
            spdlog::info("home request");

            res.set_content("Task Service Home Page\n", "text/plain");
        });

        svr.Post("/queue", [](const Request& req, Response& res) {
            std::string cmd = "";
            for (auto it = req.params.begin(); it != req.params.end(); ++it) {
                const auto& x = *it;
                cmd.append(x.first.c_str());
            }

            spdlog::info("task request: {}", cmd.c_str());

            const taskservice::Task t = taskservice::put_task(cmd);

            res.set_content(t.to_string(), "text/plain");
        });

        svr.Get("/queue", [](const Request& req, Response& res) {
            spdlog::info("long poll");

            // std::this_thread::sleep_for(std::chrono::milliseconds(15000));
            const taskservice::Task t = taskservice::get_task();

            res.set_content(t.to_string(), "text/plain");
        });

        svr.Get("/version", [](const Request&, Response& res) {
            auto vers = taskservice::Version().to_string();
            res.set_content(vers, "text/plain");
            spdlog::warn("Version Request: {}", vers);
        });

        // Shutdown hook
        svr.Delete("/shutdown", [&](const Request&, Response& res) {
            res.set_content("ok, shutting down...", "text/plain");
            spdlog::warn("Shutting down...");
            svr.stop();
        });

        return true;
    }

    // Function to run the server
    bool run_service(const Config& config) {
        SSLServer svr(config.cert_file.c_str(), config.key_file.c_str());

        // Set up the server
        if (!taskservice::setup_service(svr, config)) {
            return false;
        }

        spdlog::info("Server starting at https://{}:{}", config.host, config.port);

        // Start the server
        return svr.listen(config.host, config.port);
    }
}  // namespace taskservice
