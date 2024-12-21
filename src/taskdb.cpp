//
// 2024-12-21 07:33:17 dpw
//

#include <mutex>
#include <vector>
#include <spdlog/spdlog.h>
#include <thread>

#include <taskservice/taskdb.hpp>

namespace taskservice {
    const auto nulltask = Task{.command = "", .created = 0};
    std::vector<Task> tasks = {};
    std::mutex tasks_mutex;

    Task put_task(std::string cmd) {
        std::lock_guard<std::mutex> lock(tasks_mutex);
        auto now = std::time(nullptr);
        Task t = Task{.command = cmd, .created = now};

        spdlog::info("created: {}, task: {}", t.created, t.command);

        tasks.push_back(t);

        return t;
    }

    // return the last task entered or a null task
    Task get_task() {
        std::lock_guard<std::mutex> lock(tasks_mutex);
        if (not tasks.empty()) {
            Task t = tasks.back();

            spdlog::info("got task: {}, task: {}", t.created, t.command);

            return t;
        } else {
            spdlog::info("return null task");
            return nulltask;
        }
    }

}
