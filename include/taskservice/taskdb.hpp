//
// 2024-12-21 07:33:17 dpw
//

#pragma once

#include <ctime>
#include <iostream>
#include <mutex>
#include <sstream>
#include <vector>

namespace taskservice {
    struct Task {
        std::string command = "";
        std::time_t created = std::time(nullptr);

        friend std::ostream& operator<<(std::ostream& os, const Task v) {
            // better to use <format> but it breaks on linux and fmt broken on darwin
            os << v.created << ":" << v.command;

            return os;
        }

        std::string to_string() const {
            std::ostringstream oss;
            oss << *this;

            return oss.str();
        }
    };

    Task task_from_string(const std::string task_str);
    Task put_task(const std::string cmd);
    Task get_task();
}  // namespace taskservice
