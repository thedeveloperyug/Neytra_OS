// main.cpp - composition root: wires the shell's collaborators together via
// interfaces (see docs/architecture.md#design-principles). Built as the
// neytra-shell executable (see CMakeLists.txt) -- run it manually from the
// BusyBox prompt after boot to try the native shell; it doesn't replace
// BusyBox /bin/sh as the automatic login shell yet.
#include "BuiltinCommands.hpp"
#include "CommandParser.hpp"
#include "Logger.hpp"
#include "ProcessManager.hpp"
#include "Shell.hpp"

int main_shell() {
    CommandParser parser;
    BuiltinCommands builtins(Logger::instance());
    ProcessManager processes(Logger::instance());
    Shell shell(parser, builtins, processes, Logger::instance());
    return shell.run();
}

int main() {
    return main_shell();
}

