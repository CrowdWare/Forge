cmake_minimum_required(VERSION 3.22.1)
project(bench_sms_bridge LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# app/src/main/cpp -> ... -> repo root
set(FORGE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../")
set(SMSCORE_DIR "${FORGE_ROOT}/SMSCore.Native")

add_library(
    bench_sms_bridge
    SHARED
    sms_bridge_jni.cpp
    bench_sms_aot.cpp
    ${SMSCORE_DIR}/src/sms_native.cpp
)

target_include_directories(
    bench_sms_bridge
    PRIVATE
    ${SMSCORE_DIR}/include
)

find_library(log-lib log)

target_link_libraries(
    bench_sms_bridge
    ${log-lib}
)
