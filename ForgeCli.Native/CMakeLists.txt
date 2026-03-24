cmake_minimum_required(VERSION 3.16)
project(ForgeCliNative LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(forgecli-native
    src/main.cpp
    src/sandbox_policy.cpp
    src/cmd_build.cpp
    src/godot_sdk.cpp
)

target_include_directories(forgecli-native PRIVATE
    src
    ../SMLCore.Native/include
    ../SMSCore.Native/include
)

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
