//
//  cryptor main
//

#include <spdlog/spdlog.h>

#include <cli.hpp>
#include <logging.hpp>
#include <service.hpp>
#include <unit.hpp>
#include <version.hpp>

int main(const int argc, char **argv) {
    using namespace httplib;

    std::cout << argv[0] << std::endl;
    const std::string ss = std::string(argv[0]);

    // run the unit tests
    if (ss.substr(ss.size() - 4) == "unit") {
        int code = run_unit_tests(argc, argv);
        return code;
    }

    const auto config = cryptor::parse_cli(argc, argv);

    std::string version = "Server Version: ";
    auto vers = cryptor::Version();

    version.append(vers.to_string());

    if (config.verbose > 0) {
        spdlog::info("{}", version);
        spdlog::info("Server Config : {}", config.to_string());
    }

    auto ok = cryptor::run_service(config);

    spdlog::info("Server shutdown, code: {}...", ok);

    return 0;
}
