//
// 2024-12-20 00:00:33 dpw
//

#pragma once

#include "precompiled.hpp"

namespace taskservice {

    // exec used by the client to run tasks

    auto exec(const char* cmd) {
        std::array<char, 128> buffer;
        std::string result = "";

        std::unique_ptr<FILE, decltype(static_cast<int (*)(FILE*)>(&pclose))> pipe(popen(cmd, "r"), pclose);

        if (!pipe) {
            return "the command " + std::string(cmd) + " failed!";
        }

        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            result += buffer.data();
            std::cout << buffer.data() << std::flush;
        }
        return result;
    }
}  // namespace taskservice
