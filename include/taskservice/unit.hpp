//
// 2024-12-10 18:27:44 dpw
//

#pragma once

#include <taskservice/cli.hpp>
#include <taskservice/logging.hpp>
#include <taskservice/service.hpp>
#include <taskservice/taskdb.hpp>
#include <taskservice/version.hpp>
#include <vendor/ansi_colors.hpp>
#include <vendor/testlib.hpp>

// the actual tests
using namespace rcstestlib;

Results test_version() {
    Results r = {.name = "Version Tests"};

    auto vers = taskservice::Version();
    r.equals(vers.major == 24);
    r.equals(vers.minor == 12);
    r.equals(vers.patch == 18);
    r.equals(vers.build > 99);

    return r;
}

//
// cli tests and helper funcs
//
std::pair<int, char**> build_args(const std::vector<std::string>& vargs) {
    int argc = vargs.size();

    // Allocate memory for argv
    char** argv = new char*[argc];

    // Fill the argv array with C-style strings
    for (int i = 0; i < argc; ++i) {
        argv[i] = const_cast<char*>(vargs[i].c_str());  // Unsafe cast for illustration purposes
    }

    return {argc, argv};
}

void test_default_config(Results& r) {
    const std::vector<std::string> args = {"test"};
    auto [argc, argv] = build_args(args);
    auto cfg = taskservice::parse_cli(argc, argv);

    r.equals(cfg.port == 2032, "the default port assignment");
    r.equals(cfg.host == "0.0.0.0", "the default host assignment");
    r.equals(cfg.verbose == 1, "the default verbose assignment");
    r.equals(cfg.cert_file.ends_with("/cert.pem"), "the default cert file assignment");
    r.equals(cfg.key_file.ends_with("/key.pem"), "the default key file assignment");
}

void test_port(Results& r) {
    const std::vector<std::string> args = {"test", "-p", "2500"};
    auto [argc, argv] = build_args(args);
    auto cfg = taskservice::parse_cli(argc, argv);

    r.equals(cfg.port == 2500, "the port assignment");

    r.equals(cfg.host == "0.0.0.0", "the default host assignment");
    r.equals(cfg.verbose == 1, "the default verbose assignment");
    r.equals(cfg.cert_file.ends_with("/cert.pem"), "the default cert file assignment");
    r.equals(cfg.key_file.ends_with("/key.pem"), "the default key file assignment");
}

void test_host(Results& r) {
    const std::vector<std::string> args = {"test", "--host", "1.1.1.1"};
    auto [argc, argv] = build_args(args);
    auto cfg = taskservice::parse_cli(argc, argv);

    r.equals(cfg.host == "1.1.1.1", "the host assignment");
    r.equals(cfg.port == 2032, "the default port assignment");
    r.equals(cfg.verbose == 1, "the default verbose assignment");
    r.equals(cfg.cert_file.ends_with("/cert.pem"), "the default cert file assignment");
    r.equals(cfg.key_file.ends_with("/key.pem"), "the default key file assignment");
}

void test_cert_key(Results& r) {
    auto cert = "my-cert.pem";
    auto key = "my-key.pem";

    const std::vector<std::string> args = {"test", "--cert", cert, "--key", key};
    auto [argc, argv] = build_args(args);
    auto cfg = taskservice::parse_cli(argc, argv);

    r.equals(cfg.cert_file == cert, "the cert file assignment");
    r.equals(cfg.key_file == key, "the key file assignment");
    r.equals(cfg.port == 2032, "the default port assignment");
    r.equals(cfg.host == "0.0.0.0", "the default host assignment");
    r.equals(cfg.verbose == 1, "the default verbose assignment");
}

Results test_cli() {
    Results r = {.name = "CLI Tests"};

    test_default_config(r);
    test_port(r);
    test_host(r);
    test_cert_key(r);

    return r;
}

void test_default_service(Results& r) {
    auto config = taskservice::Config();
    httplib::SSLServer svr(config.cert_file.c_str(), config.key_file.c_str());
    auto ok = taskservice::setup_service(svr, config);

    r.equals(ok == true, "should create the default server");
}

void test_bad_cert(Results& r) {
    auto config = taskservice::Config();
    config.cert_file = "./no-file-here.pem";
    httplib::SSLServer svr(config.cert_file.c_str(), config.key_file.c_str());
    auto ok = taskservice::setup_service(svr, config);

    r.equals(ok == false, "should fail with bad cert file server");
}

void test_bad_key(Results& r) {
    auto config = taskservice::Config();
    config.key_file = "./no-file-here.pem";
    httplib::SSLServer svr(config.cert_file.c_str(), config.key_file.c_str());
    auto ok = taskservice::setup_service(svr, config);

    r.equals(ok == false, "should fail with bad cert file server");
}

Results test_service() {
    Results r = {.name = "HTTPS Service Tests"};

    test_default_service(r);
    test_bad_cert(r);
    test_bad_key(r);

    return r;
}

//
// tasks tests
//
void test_get_nulltask(Results& r) {
    const auto t = taskservice::get_task();

    r.equals(t.command == "", "the command should be an empty string");
    r.equals(t.created == 0, "created should be zero");
}

void test_put(Results& r) {
    const auto cmd = std::string("mk clobber init build test");
    const auto t = taskservice::put_task(cmd);

    r.equals(t.command == t.command, "the command should be what was passed in");
    r.equals(t.created > 0, "created should not be zero");
}

void test_get_task(Results& r) {
    const auto cmd = std::string("mk build test");
    const auto t1 = taskservice::put_task(cmd);

    r.equals(t1.command == t1.command, "the command should be what was passed in");
    r.equals(t1.created > 0, "created should not be zero");

    const auto t2 = taskservice::get_task();

    r.equals(t2.command == t1.command, "the command should equal what was put");
    r.equals(t2.created == t1.created, "created should be equal what was put");
}

void test_task_from_string(Results& r) {
    const std::string tstr1 = "1734821177:mk build test";

    taskservice::Task task = taskservice::task_from_string(tstr1);
    r.equals(task.created == 1734821177, "should parse the correct task time stamp");
    r.equals(task.command == "mk build test", "should parse the correct task time stamp");

    const std::string tstr2 = "1734821177 mk build test";

    task = taskservice::task_from_string(tstr2);
    r.equals(task.created == 0, "should parse the correct task time stamp");
    r.equals(task.command == "", "should parse the correct task time stamp");

    const std::string tstr3 = "mk build test";

    task = taskservice::task_from_string(tstr3);
    r.equals(task.created == 0, "should parse the correct task time stamp");
    r.equals(task.command == "", "should parse the correct task time stamp");
}

Results test_tasks() {
    Results r = {.name = "Task Tests"};

    test_get_nulltask(r);
    test_put(r);
    test_get_task(r);
    test_task_from_string(r);

    return r;
}

Results test_taskdb() {
    Results r = {.name = "Task DB Tests"};

    for (int i = 0; i < 15; i++) {
        const auto cmd = std::string("mk clobber init build test");
        const auto t = taskservice::put_task(cmd);

        r.equals(t.command == t.command, "the command should be what was passed in");
        r.equals(t.created > 0, "created should not be zero");
        r.equals(taskservice::get_db_size() <= 12, "should not grow larger than the max size");
    }

    return r;
}

// replaces main
int run_unit_tests(int argc, char* argv[]) {
    using namespace colors;
    spdlog::set_level(spdlog::level::off);

    auto msg = std::string("Cryptor Server Unit Tests, Version: ");
    std::cout << cyan << msg << yellow << taskservice::Version() << reset << "\n" << std::endl;
    // std::vector<std::string> args(argv, argv + argc);

    Results summary = Results{.name = "Unit Test Summary"};

    // lambda to run a test and add its result to the summary
    auto run_test = [&summary](auto test_func) {
        auto result = test_func();
        std::cout << result << std::endl;
        summary.add(result);
    };

    run_test(test_version);
    run_test(test_cli);
    run_test(test_tasks);
    run_test(test_taskdb);
    run_test(test_service);

    std::cout << "\n" << summary << std::endl;
    msg = (summary.failed == 0) ? green + "Ok" : "\n" + red + "Tests failed!";
    std::cout << cyan << "\nUnit Test Results: " << msg << reset << std::endl;

    return 0;
}
