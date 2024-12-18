//
// 2024-12-13 15:25:10 dpw
//

#include <httplib.h>
#include <spdlog/spdlog.h>
#include <unistd.h>

#include <atomic>
#include <cassert>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <thread>
#include <vendor/ansi_colors.hpp>
#include <vendor/testlib.hpp>
#include <version.hpp>

using namespace colors;

struct Config {
    std::string host = "localhost";
    std::string port = "22022";
    bool start_server = true;
    std::string logfile = "service.log";
};

// Define the function to start the service
void run_server(std::atomic<bool>& running, const Config& config) {
    running = true;

    // Open a pipe to start the service
    std::string cmd = "./build/cryptor --base html/";
    cmd.append(" --host ");
    cmd.append(config.host);
    cmd.append(" --port ");
    cmd.append(config.port);
    cmd.append(" > " + config.logfile + " 2>&1 & echo $!");

    std::cout << cyan << "Server start command: " << yellow << cmd << reset << std::endl;

    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) {
        std::cerr << red << "Failed to start service." << reset << std::endl;
        running = false;
        return;
    }

    // Read the process ID of the started service
    char buffer[128];
    if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        pid_t pid = std::stoi(buffer);
        std::cout << "\t" << green << "Service started with PID: " << pid << reset << std::endl;
    } else {
        std::cerr << red << "Failed to retrieve PID of the service." << reset << std::endl;
        running = false;
    }

    pclose(pipe);

    // Wait for a brief period to allow the service to initialize
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

// use cxxopts to parse host and port
// if the service is not local (localhost or 127.0.0.1), then skip the startup
int main(int argc, char* argv[]) {
    std::atomic<bool> server_running(false);
    auto config = Config();

    // TODO parse the cli to reconfig...

    std::string msg = "Cryptor Server Integration Tests, Version: ";
    std::cout << "\n" << cyan << msg << yellow << cryptor::Version() << reset << std::endl;

    rcstestlib::Results r = {.name = "Integration Test Summary"};

    // start the server thread
    std::thread server_thread(run_server, std::ref(server_running), config);

    // Wait for the server to start
    auto loop_count = 20;
    while (!server_running && loop_count > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        loop_count--;
    }

    r.equals(server_running, "should be running in background thread now");

    std::cout << std::endl;

    // Create a client for testing
    httplib::Client cli("https://" + config.host + ":" + config.port);
    cli.enable_server_certificate_verification(false);

    // Test 1: Verify version endpoint
    if (auto res = cli.Get("/version")) {
        r.equals(res->status == 200, "the status should be 200");
        std::cout << "\t" << green << "Test passed: Version endpoint returned correct response." << reset << std::endl;
    } else {
        std::cerr << "\t" << red << "Test failed: Unable to reach version endpoint." << reset << std::endl;
    }

    // Verify index page title
    if (auto res = cli.Get("/")) {
        r.equals(res->status == 200);
        r.equals(res->body.find("<title>Cryptor</title>") != std::string::npos, "the title page shoule be Cryptor");
        std::cout << "\t" << green << "Test passed: Index page contains correct title." << reset << std::endl;
    } else {
        std::cerr << "\t" << red << "Test failed: Unable to reach index page." << reset << std::endl;
    }

    // Shut down the server
    if (auto res = cli.Delete("/shutdown")) {
        r.equals(res->status == 200, "return status should be 200");
        r.equals(res->body.find("down") != std::string::npos, "the response should say down");
        std::cout << "\t" << green << "Test passed: Shutdown endpoint had correct response." << reset << std::endl;
    } else {
        std::cerr << "\t" << red << "Test failed: Unable to reach shutdown endpoint." << reset << std::endl;
    }

    // Wait for the server thread to stop
    server_thread.join();

    // give the service time to fully shutdown
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // Verify server has stopped
    // try to Shut down the server
    if (auto res = cli.Delete("/shutdown")) {
        r.equals(res->status != 200, "should be shutdown");
        std::cerr << "\t" << red << "Test failed: Unable to reach shutdown endpoint." << reset << std::endl;
    } else {
        r.equals(true, "shutdown ok");
        std::cout << "\t" << green << "Test passed: the server is down." << reset << std::endl;
        server_running = false;
    }

    r.equals(!server_running, "server should NOT be running.");

    std::cout << "\n" << r << std::endl;
    msg = (r.failed == 0) ? green + "Ok" : "\n" + red + "Tests failed!";

    std::cout << cyan << "\nIntegration Test Results: " << msg << reset << std::endl;

    return 0;
}
