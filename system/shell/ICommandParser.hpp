// ICommandParser.hpp - command-line tokenizing interface (depend on this, not CommandParser).
#pragma once

#include <string>
#include <vector>

struct ParsedCommand {
    std::string command;
    std::vector<std::string> args;
    bool valid = false;  // false for blank/comment-only lines
};

class ICommandParser {
public:
    virtual ~ICommandParser() = default;

    // Splits a raw input line into a command and its arguments, honoring single
    // and double quotes (e.g. `echo "hello world"` -> {"echo", {"hello world"}}).
    virtual ParsedCommand parse(const std::string& line) const = 0;
};
