//
// 2024-12-21 07:33:17 dpw
//

#pragma once

#include <mutex>
#include <vector>

namespace taskservice {
    struct Task {
        std::string command = "";
        std::time_t created = std::time(nullptr);
    };

    Task put_task(std::string cmd);
    Task get_task();
}