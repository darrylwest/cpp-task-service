//
// 2024-12-09 10:28:16 dpw
//

#include "precompiled.hpp"

#include <spdlog/spdlog.h>

#include <cxxopts.hpp>
#include <taskservice/cli.hpp>
#include <taskservice/version.hpp>

namespace taskservice {
    namespace fs = std::filesystem;

    /*
     * parse the command line
     */
    Config parse_cli(const int argc, char** argv) {
        bool skip_cert_check = false;
        auto config = Config();

        try {
            cxxopts::Options options("TaskService", "TLS server to queue jobs.");
            // clang-format off
            options
                .add_options()
                    ("p,port", "listening port", cxxopts::value<int>())
                    ("H,host", "listening host", cxxopts::value<std::string>())
                    ("C,cert", "the cert pem file", cxxopts::value<std::string>())
                    ("K,key", "the key pem file", cxxopts::value<std::string>())
                    ("l,level", "verbose level 0--4", cxxopts::value<int>())
                    ("V,version", "Show the current version and exit")
                    ("h,help", "Show this help")
                ;

            // clang-format on
            const auto version = taskservice::Version();
            const auto result = options.parse(argc, argv);
            if (result.count("version")) {
                std::cout << "Server Version: " << version << std::endl;
                exit(0);
            }

            if (result.count("help")) {
                std::cout << "Server Version: " << version << std::endl;
                std::cout << options.help() << std::endl;
                exit(0);
            }

            if (result.count("port")) {
                config.port = result["port"].as<int>();
            }

            if (result.count("host")) {
                config.host = result["host"].as<std::string>();
            }

            if (result.count("cert")) {
                skip_cert_check = true;
                config.cert_file = result["cert"].as<std::string>();
            }

            if (result.count("key")) {
                skip_cert_check = true;
                config.key_file = result["key"].as<std::string>();
            }

            if (result.count("level")) {
                auto level = result["level"].as<int>();
                config.verbose = level;
                switch (level) {
                    case 0:
                        spdlog::set_level(spdlog::level::debug);
                        break;
                    case 2:
                        spdlog::set_level(spdlog::level::warn);
                        break;
                    case 3:
                        spdlog::set_level(spdlog::level::err);
                        break;
                    case 4:
                        spdlog::set_level(spdlog::level::critical);
                        break;
                    case 5:
                        spdlog::set_level(spdlog::level::off);
                        break;
                    default:
                        config.verbose = 1;
                        spdlog::set_level(spdlog::level::info);
                        break;
                }
            }

        } catch (const std::exception& exp) {
            std::cout << "error parsing cli options: " << exp.what() << std::endl;
            exit(1);
        }

        if (!skip_cert_check) {
            if (!ensureCertFiles(config)) {
                std::cerr << "Failed to create cert/key in $HOME/.task-service, bailing out." << std::endl;
                exit(1);
            }

            spdlog::debug("config: {}", config.to_string());
        }

        return config;
    }

    // ensure the defaults are in place if the CLI doesn't specify
    bool ensureCertFiles(Config& config) {
        const char* home = std::getenv("HOME");

        fs::path dir = fs::path(home) / ".task-service";

        if (!fs::exists(dir)) {
            std::error_code ec;
            if (!fs::create_directory(dir, ec)) {
                std::cerr << "Failed to create folder " << dir << ", Error: " << ec.message() << std::endl;
                exit(1);
            }
        }

        fs::path cert = dir / "cert.pem";
        fs::path key = dir / "key.pem";

        if (!fs::exists(cert) || !fs::exists(key)) {
            std::string cmd = "openssl req -x509 -newkey rsa:4096 -nodes -keyout ";
            cmd.append(key.c_str());
            cmd.append(" -out ");
            cmd.append(cert.c_str());
            cmd.append(" -days 365 -subj \"");
            cmd.append("/C=US/ST=California/L=Berkeley/O=RainCitySoftware");
            cmd.append("/CN=raincitysoftware.com\"");

            // std::cout << "run this: " << cmd << std::endl;
            int code = std::system(cmd.c_str());
            if (code != 0) {
                std::cerr << "Failed to create pem files. Consider creating them and place them in " << dir.c_str()
                          << std::endl;
                exit(1);
            }
        }

        config.cert_file = cert.c_str();
        config.key_file = key.c_str();

        return true;
    }
}  // namespace taskservice
