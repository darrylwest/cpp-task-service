//
// 2024-12-21 07:33:17 dpw
//

#include <spdlog/spdlog.h>

#include <mutex>
#include <taskservice/taskdb.hpp>
#include <thread>
#include <vector>

namespace taskservice {
    const auto max_db_size = 10;
    const auto nulltask = Task{.command = "", .created = 0};
    std::vector<Task> tasks = {};
    std::mutex tasks_mutex;

    Task put_task(const std::string cmd) {
        std::lock_guard<std::mutex> lock(tasks_mutex);
        auto now = std::time(nullptr);
        Task t = Task{.command = cmd, .created = now};

        tasks.insert(tasks.begin(), t);
        spdlog::info("created: {}, task: {} : db size: {}", t.created, t.command, tasks.size());

        if (tasks.size() > max_db_size) {
            spdlog::info("pop back to keep size to {}.", max_db_size);
            tasks.pop_back();
        }

        return t;
    }

    // return the last task entered or a null task
    Task get_task() {
        std::lock_guard<std::mutex> lock(tasks_mutex);
        if (not tasks.empty()) {
            Task t = tasks.front();

            spdlog::info("got task: {}, task: {}", t.created, t.command);

            return t;
        } else {
            spdlog::info("return null task");
            return nulltask;
        }
    }

    // parse the Task from the formatted string "123:task"
    Task task_from_string(const std::string task_str) {
        std::istringstream iss(task_str);
        std::string timestamp_str;

        if (std::getline(iss, timestamp_str, ':')) {
            std::string task;
            if (std::getline(iss, task)) {
                try {
                    std::time_t timestamp = std::stoi(timestamp_str);
                    return Task{.command = task, .created = timestamp};
                } catch (const std::invalid_argument& e) {
                    // Handle invalid timestamp (e.g., not a number)
                    return nulltask;
                } catch (const std::out_of_range& e) {
                    // Handle timestamp out of range for std::time_t
                    return nulltask;
                }
            }
        }

        return nulltask;
    }

}  // namespace taskservice
