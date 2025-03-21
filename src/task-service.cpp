//
//  taskservice main
//

#include <spdlog/spdlog.h>

#include <taskservice/cli.hpp>
#include <taskservice/service.hpp>
#include <taskservice/version.hpp>
#include <vendor/ansi_colors.hpp>

// this is the server side
int main(const int argc, char **argv) {
    using namespace httplib;

    const std::string ss = std::string(argv[0]);
    std::print("{}{}{}\n", colors::yellow, ss, colors::reset);

    const auto config = taskservice::parse_cli(argc, argv);

    std::string version = "Task Runner Service Version: ";

    if (config.verbose > 0) {
        auto vers = taskservice::Version().to_string();
        spdlog::info("Task Service, Version: {}{}{}", colors::cyan, vers, colors::reset);
        spdlog::info("Server Config : {}", config.to_string());
    }

    auto ok = taskservice::run_service(config);

    spdlog::info("Server shutdown, code: {}...", ok);

    return 0;
}
