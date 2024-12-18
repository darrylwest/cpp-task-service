//
// 2024-12-09 10:28:16 dpw
//

#pragma once

#include <spdlog/spdlog.h>

#include <cxxopts.hpp>
#include <iostream>

#include "version.hpp"

namespace cryptor {

    struct Config {
        std::string host = "0.0.0.0";
        int port = 2022;
        std::string base_dir = "./";
        int verbose = 1;
        std::string cert_file = "./.ssh/cert.pem";
        std::string key_file = "./.ssh/key.pem";

        friend std::ostream& operator<<(std::ostream& os, const Config v) {
            // better to use <format> but it breaks on linux and fmt broken on darwin
            os << "host: " << v.host << ", "
               << "port: " << v.port << ", "
               << "base: " << v.base_dir << ", "
               << "cert: " << v.cert_file << ", "
               << "key : " << v.key_file << ", "
               << "verbose: " << v.verbose << ".";
            return os;
        }

        std::string to_string() const {
            std::ostringstream oss;
            oss << *this;

            return oss.str();
        }

        Config parse_cli(const int argc, char** argv);
    };

    Config parse_cli(const int argc, char** argv);
}  // namespace cryptor
