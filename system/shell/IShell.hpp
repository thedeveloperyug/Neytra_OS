// IShell.hpp - interactive shell interface (depend on this, not Shell).
#pragma once

class IShell {
public:
    virtual ~IShell() = default;

    // Runs the read-eval-print loop until "exit" is invoked or stdin is closed.
    // Returns the exit code the shell should terminate with.
    virtual int run() = 0;
};
