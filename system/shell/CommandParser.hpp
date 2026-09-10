// CommandParser.hpp - quote-aware whitespace tokenizer.
#pragma once

#include "ICommandParser.hpp"

class CommandParser : public ICommandParser {
public:
    ParsedCommand parse(const std::string& line) const override;
};

