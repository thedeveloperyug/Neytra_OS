// Sandbox.hpp - fork/exec-based process confinement (rlimits, optional chroot+setuid).
#pragma once

#include "ISandbox.hpp"
#include "ILogger.hpp"

class Sandbox : public ISandbox {
public:
    explicit Sandbox(ILogger& logger);

    int run(const std::string& path, const std::vector<std::string>& args,
            unsigned int uid, const std::string& chrootDir,
            const SandboxLimits& limits) override;

private:
    ILogger& logger_;
};
