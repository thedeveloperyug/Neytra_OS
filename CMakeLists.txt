cmake_minimum_required(VERSION 3.10)
project(Neytra_os CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# system/logger — first real system/ subsystem; see docs/roadmap.md.
add_library(neytra_logger STATIC
    system/logger/Logger.cpp
)
target_include_directories(neytra_logger PUBLIC system/logger)

# neytra-log — statically-linked CLI wrapper so rootfs/init (running inside the
# minimal BusyBox rootfs, no shared libstdc++ available) can log through the
# real Logger. See scripts/build_system.sh, which copies this into rootfs/usr/bin/.
add_executable(neytra-log system/logger/tools/neytra-log.cpp)
target_link_libraries(neytra-log PRIVATE neytra_logger)
target_link_options(neytra-log PRIVATE -static)

# system/security — UserManager, PermissionManager, Sandbox.
add_library(neytra_security STATIC
    system/security/UserManager.cpp
    system/security/PermissionManager.cpp
    system/security/Sandbox.cpp
)
target_include_directories(neytra_security PUBLIC system/security)
target_link_libraries(neytra_security PUBLIC neytra_logger)

# system/process — ProcessManager, Scheduler, IPC.
add_library(neytra_process STATIC
    system/process/ProcessManager.cpp
    system/process/Scheduler.cpp
    system/process/IPC.cpp
)
target_include_directories(neytra_process PUBLIC system/process)
target_link_libraries(neytra_process PUBLIC neytra_logger)

# system/init — depends on neytra_logger's ILogger interface, and on neytra_process
# so ServiceManager can spawn services (see docs/architecture.md#design-principles
# for the dependency-injection convention).
add_library(neytra_init STATIC
    system/init/InitManager.cpp
    system/init/MountManager.cpp
    system/init/ServiceManager.cpp
    system/init/main.cpp
)
target_include_directories(neytra_init PUBLIC system/init)
target_link_libraries(neytra_init PUBLIC neytra_logger neytra_process)

# system/shell — CommandParser, BuiltinCommands, Shell (depends on neytra_process
# to spawn external commands).
add_library(neytra_shell STATIC
    system/shell/CommandParser.cpp
    system/shell/BuiltinCommands.cpp
    system/shell/Shell.cpp
)
target_include_directories(neytra_shell PUBLIC system/shell)
target_link_libraries(neytra_shell PUBLIC neytra_logger neytra_process)

# neytra-shell — statically-linked interactive CLI so the real Shell can be run
# from inside the booted rootfs (see scripts/build_system.sh, system/README.md).
# main.cpp is deliberately NOT part of the neytra_shell library above: shell_test
# provides its own GTest main(), so the two must never link into the same binary.
add_executable(neytra-shell system/shell/main.cpp)
target_link_libraries(neytra-shell PRIVATE neytra_shell neytra_process neytra_logger)
target_link_options(neytra-shell PRIVATE -static)

# system/network — SocketManager, NetworkManager, DHCPClient, WifiManager.
add_library(neytra_network STATIC
    system/network/SocketManager.cpp
    system/network/NetworkManager.cpp
    system/network/DHCPClient.cpp
    system/network/WifiManager.cpp
)
target_include_directories(neytra_network PUBLIC system/network)
target_link_libraries(neytra_network PUBLIC neytra_logger)

# neytra-netup — statically-linked boot-time network bring-up: brings lo (and any
# other real interface) up, DHCPs it, and applies the lease (see system/README.md).
add_executable(neytra-netup system/network/tools/neytra-netup.cpp)
target_link_libraries(neytra-netup PRIVATE neytra_network neytra_logger)
target_link_options(neytra-netup PRIVATE -static)

# system/package — Repository, Downloader, Installer, PackageManager.
add_library(neytra_package STATIC
    system/package/Repository.cpp
    system/package/Downloader.cpp
    system/package/Installer.cpp
    system/package/PackageManager.cpp
)
target_include_directories(neytra_package PUBLIC system/package)
target_link_libraries(neytra_package PUBLIC neytra_logger neytra_network neytra_process)

# system/drivers — GPIO, I2C, SPI, UART. Raspberry Pi hardware access; only UART
# is meaningfully testable without real target hardware (see tests/drivers/).
add_library(neytra_drivers STATIC
    system/drivers/GPIO.cpp
    system/drivers/I2C.cpp
    system/drivers/SPI.cpp
    system/drivers/UART.cpp
)
target_include_directories(neytra_drivers PUBLIC system/drivers)
target_link_libraries(neytra_drivers PUBLIC neytra_logger)

# neytra-diag — statically-linked interactive CLI that exercises security/network/
# package/drivers/init from inside a booted VM (see system/README.md). Links every
# module library except neytra_shell (which already has its own neytra-shell binary).
add_executable(neytra-diag system/tools/neytra-diag.cpp)
target_link_libraries(neytra-diag PRIVATE
    neytra_security neytra_network neytra_package neytra_drivers neytra_init
    neytra_process neytra_logger
)
target_link_options(neytra-diag PRIVATE -static)

enable_testing()

# All tests/ suites use GoogleTest (apt package libgtest-dev/libgmock-dev) instead of
# plain main()-based asserts -- see docs/development-workflow.md for the convention.
find_package(GTest REQUIRED)

add_executable(logger_test tests/logger/logger_test.cpp)
target_link_libraries(logger_test PRIVATE neytra_logger GTest::gtest)
add_test(NAME logger_test COMMAND logger_test)

add_executable(init_test tests/init/init_test.cpp)
target_link_libraries(init_test PRIVATE neytra_init neytra_process neytra_logger GTest::gtest)
add_test(NAME init_test COMMAND init_test)

add_executable(security_test tests/security/security_test.cpp)
target_link_libraries(security_test PRIVATE neytra_security neytra_logger GTest::gtest)
add_test(NAME security_test COMMAND security_test)

add_executable(process_test tests/process/process_test.cpp)
target_link_libraries(process_test PRIVATE neytra_process neytra_logger GTest::gtest)
add_test(NAME process_test COMMAND process_test)

add_executable(shell_test tests/shell/shell_test.cpp)
target_link_libraries(shell_test PRIVATE neytra_shell neytra_process neytra_logger GTest::gtest)
add_test(NAME shell_test COMMAND shell_test)

add_executable(network_test tests/network/network_test.cpp)
target_link_libraries(network_test PRIVATE neytra_network neytra_logger GTest::gtest)
add_test(NAME network_test COMMAND network_test)

add_executable(package_test tests/package/package_test.cpp)
target_link_libraries(package_test PRIVATE neytra_package neytra_network neytra_process neytra_logger GTest::gtest)
add_test(NAME package_test COMMAND package_test)

add_executable(drivers_test tests/drivers/drivers_test.cpp)
target_link_libraries(drivers_test PRIVATE neytra_drivers neytra_logger GTest::gtest)
add_test(NAME drivers_test COMMAND drivers_test)








