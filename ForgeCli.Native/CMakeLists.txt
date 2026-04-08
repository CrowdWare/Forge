cmake_minimum_required(VERSION 3.16)
project(ForgeCliNative LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Collect all shared libraries (from FetchContent/add_subdirectory too) in
# the build root so that SMS_NATIVE_LIB_DIR / SML_NATIVE_LIB_DIR can point
# to a single, predictable directory.
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

# ---------------------------------------------------------------------------
# sml-cpp and sms-cpp (via FetchContent or local override)
# Local development: cmake -DSML_CPP_DIR=/path/to/sml-cpp -DSMS_CPP_DIR=/path/to/sms-cpp ..
# ---------------------------------------------------------------------------
include(FetchContent)

if(DEFINED SML_CPP_DIR AND EXISTS "${SML_CPP_DIR}")
    message(STATUS "sml-cpp: using local override at ${SML_CPP_DIR}")
    add_subdirectory("${SML_CPP_DIR}" "${CMAKE_BINARY_DIR}/sml-cpp-build")
else()
    FetchContent_Declare(sml-cpp
        GIT_REPOSITORY https://codeberg.org/CrowdWare/sml-cpp.git
        GIT_TAG main
    )
    FetchContent_MakeAvailable(sml-cpp)
endif()

if(DEFINED SMS_CPP_DIR AND EXISTS "${SMS_CPP_DIR}")
    message(STATUS "sms-cpp: using local override at ${SMS_CPP_DIR}")
    add_subdirectory("${SMS_CPP_DIR}" "${CMAKE_BINARY_DIR}/sms-cpp-build")
else()
    FetchContent_Declare(sms-cpp
        GIT_REPOSITORY https://codeberg.org/CrowdWare/sms-cpp.git
        GIT_TAG main
    )
    FetchContent_MakeAvailable(sms-cpp)
endif()

add_executable(forgecli-native
    src/main.cpp
    src/sandbox_policy.cpp
    src/cmd_build.cpp
    src/godot_sdk.cpp
)

target_include_directories(forgecli-native PRIVATE src)
target_link_libraries(forgecli-native PRIVATE smlcore_native_static sms_native_static)

include(CTest)
if(BUILD_TESTING)
    add_executable(forgecli_sandbox_policy_tests
        tests/sandbox_policy_tests.cpp
        src/sandbox_policy.cpp
    )
    target_include_directories(forgecli_sandbox_policy_tests PRIVATE src)
    set(FORGECLI_SANDBOX_CASES
        traversal_rejected
        symlink_component_rejected
        safe_path_allowed
    )
    foreach(CASE_NAME IN LISTS FORGECLI_SANDBOX_CASES)
        add_test(NAME forgecli_sandbox_${CASE_NAME} COMMAND forgecli_sandbox_policy_tests ${CASE_NAME})
    endforeach()
endif()
