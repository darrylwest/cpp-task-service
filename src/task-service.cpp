//
//  taskservice main
//

#include <spdlog/spdlog.h>

#include <taskservice/cli.hpp>
#include <taskservice/logging.hpp>
#include <taskservice/service.hpp>
#include <taskservice/taskdb.hpp>
#include <taskservice/unit.hpp>
#include <taskservice/version.hpp>

// this is the server side
int main(const int argc, char **argv) {
    using namespace httplib;

    std::cout << argv[0] << std::endl;
    const std::string ss = std::string(argv[0]);

    // run the unit tests
    if (ss.substr(ss.size() - 4) == "unit") {
        int code = run_unit_tests(argc, argv);
        return code;
    }

    const auto config = taskservice::parse_cli(argc, argv);

    std::string version = "Task Runner Service Version: ";
    auto vers = taskservice::Version();

    version.append(vers.to_string());

    if (config.verbose > 0) {
        spdlog::info("{}", version);
        spdlog::info("Server Config : {}", config.to_string());
    }

    auto ok = taskservice::run_service(config);

    spdlog::info("Server shutdown, code: {}...", ok);

    return 0;
}
