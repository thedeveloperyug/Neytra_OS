// Installer.cpp - extracts archives by spawning `tar` through IProcessManager.
#include "Installer.hpp"

#include <sys/stat.h>

Installer::Installer(IProcessManager& processes, ILogger& logger) : processes_(processes), logger_(logger) {}

bool Installer::install(const std::string& archivePath, const std::string& destDir) {
    mkdir(destDir.c_str(), 0755);  // ignore EEXIST; fail loudly later if it's really unusable

    const ProcessId pid = processes_.spawn("tar", {"-xf", archivePath, "-C", destDir});
    if (pid < 0) {
        logger_.error("Installer", "failed to spawn tar for " + archivePath);
        return false;
    }

    int exitCode = -1;
    if (!processes_.wait(pid, exitCode) || exitCode != 0) {
        logger_.error("Installer", "tar extraction failed for " + archivePath +
                     " (exit " + std::to_string(exitCode) + ")");
        return false;
    }

    logger_.info("Installer", "installed " + archivePath + " into " + destDir);
    return true;
}

