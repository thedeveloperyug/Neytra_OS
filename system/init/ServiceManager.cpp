// ServiceManager.cpp - reads a simple service config and spawns each entry.
#include "ServiceManager.hpp"

#include <fstream>
#include <sstream>

ServiceManager::ServiceManager(IProcessManager& processes, ILogger& logger, std::string configPath)
    : processes_(processes), logger_(logger), configPath_(std::move(configPath)) {}

std::vector<ServiceSpec> ServiceManager::parseConfig(const std::string& text) {
    std::vector<ServiceSpec> specs;
    std::istringstream lines(text);
    std::string line;

    while (std::getline(lines, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }

        std::istringstream fields(line);
        std::string name, command, argsField;
        if (!std::getline(fields, name, '|') || !std::getline(fields, command, '|')) {
            continue;
        }
        std::getline(fields, argsField);  // optional; may be empty

        ServiceSpec spec;
        spec.name = name;
        spec.command = command;
        if (!argsField.empty()) {
            std::istringstream argStream(argsField);
            std::string arg;
            while (std::getline(argStream, arg, ',')) {
                spec.args.push_back(arg);
            }
        }
        specs.push_back(std::move(spec));
    }
    return specs;
}

bool ServiceManager::startAll() {
    if (configPath_.empty()) {
        logger_.info("ServiceManager", "no service config configured; nothing to start");
        return true;
    }

    std::ifstream in(configPath_);
    if (!in.is_open()) {
        logger_.warn("ServiceManager", "cannot open service config: " + configPath_);
        return true;  // nothing configured is not a failure
    }

    std::ostringstream buffer;
    buffer << in.rdbuf();
    const std::vector<ServiceSpec> specs = parseConfig(buffer.str());

    bool allOk = true;
    for (const auto& spec : specs) {
        const ProcessId pid = processes_.spawn(spec.command, spec.args);
        if (pid < 0) {
            logger_.error("ServiceManager", "failed to start service: " + spec.name);
            allOk = false;
            continue;
        }
        logger_.info("ServiceManager", "started service " + spec.name + " (pid " + std::to_string(pid) + ")");
    }
    return allOk;
}


