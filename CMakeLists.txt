cmake_minimum_required(VERSION 3.10)
project(Neytra_os CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# system/logger — first real system/ subsystem; see docs/roadmap.md.
add_library(neytra_logger STATIC
    system/logger/Logger.cpp
)
target_include_directories(neytra_logger PUBLIC system/logger)

# system/init — depends on neytra_logger's ILogger interface only (see
# docs/architecture.md#design-principles for the dependency-injection convention).
add_library(neytra_init STATIC
    system/init/InitManager.cpp
    system/init/MountManager.cpp
    system/init/ServiceManager.cpp
    system/init/main.cpp
)
target_include_directories(neytra_init PUBLIC system/init)
target_link_libraries(neytra_init PUBLIC neytra_logger)

enable_testing()
add_executable(logger_test tests/logger/logger_test.cpp)
target_link_libraries(logger_test PRIVATE neytra_logger)
add_test(NAME logger_test COMMAND logger_test)

add_executable(init_test tests/init/init_test.cpp)
target_link_libraries(init_test PRIVATE neytra_init neytra_logger)
add_test(NAME init_test COMMAND init_test)


