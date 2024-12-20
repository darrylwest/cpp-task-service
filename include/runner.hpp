//
// 2024-12-20 00:00:33 dpw
//

#pragma once

#include <iostream>
#include <memory>
#include <array>

namespace taskservice {

    // TODO create a wrap method that wraps the response in html, body, pre, etc.

    auto exec(const char* cmd) {
        std::array<char, 128> buffer;
        std::string result = "<html><body><pre>\n";
        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);

        if (!pipe) {
            return "the command " + std::string(cmd) + " failed!</pre></body></html>";
        }

        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            result += buffer.data();
        }
        result += "</pre></body></html>\n";

        return result;
    }

    auto build() {
        const auto cmd = "./mk build test";
        return exec(cmd);
    }
}
