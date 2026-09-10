// CommandParser.cpp - quote-aware whitespace tokenizer.
#include "CommandParser.hpp"

namespace {
std::vector<std::string> tokenize(const std::string& line) {
    std::vector<std::string> tokens;
    std::string current;
    bool inToken = false;
    char quoteChar = '\0';

    for (size_t i = 0; i < line.size(); ++i) {
        const char c = line[i];

        if (quoteChar != '\0') {
            if (c == quoteChar) {
                quoteChar = '\0';
            } else {
                current += c;
            }
            continue;
        }

        if (c == '"' || c == '\'') {
            quoteChar = c;
            inToken = true;
            continue;
        }

        if (c == ' ' || c == '\t') {
            if (inToken) {
                tokens.push_back(current);
                current.clear();
                inToken = false;
            }
            continue;
        }

        current += c;
        inToken = true;
    }

    if (inToken) {
        tokens.push_back(current);
    }
    return tokens;
}
}  // namespace

ParsedCommand CommandParser::parse(const std::string& line) const {
    ParsedCommand result;

    const size_t start = line.find_first_not_of(" \t");
    if (start == std::string::npos || line[start] == '#') {
        return result;  // blank line or comment: valid stays false
    }

    const std::vector<std::string> tokens = tokenize(line);
    if (tokens.empty()) {
        return result;
    }

    result.command = tokens.front();
    result.args.assign(tokens.begin() + 1, tokens.end());
    result.valid = true;
    return result;
}

