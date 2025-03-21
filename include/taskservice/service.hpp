//
// 2024-12-10 18:27:44 dpw
//

#pragma once

#include <vendor/httplib.h>

#include "cli.hpp"

namespace taskservice {

    // Function to set up the server and apply configurations
    bool setup_service(httplib::SSLServer &svr, const Config &config);

    // Function to run the server
    bool run_service(const Config &config);

}  // namespace taskservice
