cmake_minimum_required(VERSION 3.20)
project(ForgeRunnerNative LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(CTest)

# ---------------------------------------------------------------------------
# godot-cpp
# ---------------------------------------------------------------------------
if(NOT DEFINED GODOT_CPP_DIR OR GODOT_CPP_DIR STREQUAL "")
    set(GODOT_CPP_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../third_party/godot-cpp")
    message(STATUS "GODOT_CPP_DIR not set - using bundled: ${GODOT_CPP_DIR}")
endif()

set(GODOT_CPP_GEN_DIR     "${GODOT_CPP_DIR}/gen")
set(GODOT_CPP_INCLUDE_DIR "${GODOT_CPP_DIR}/include")
set(GODOT_CPP_BINDING_DIR "${GODOT_CPP_DIR}/gdextension")

if(NOT EXISTS "${GODOT_CPP_INCLUDE_DIR}/godot_cpp/godot.hpp")
    message(FATAL_ERROR "godot-cpp headers not found in GODOT_CPP_DIR='${GODOT_CPP_DIR}'.")
endif()

if(ANDROID)
    if(ANDROID_ABI STREQUAL "arm64-v8a")
        set(FRN_GODOT_CPP_ARCH_SUFFIX "arm64")
    elseif(ANDROID_ABI STREQUAL "armeabi-v7a")
        set(FRN_GODOT_CPP_ARCH_SUFFIX "arm32")
    elseif(ANDROID_ABI STREQUAL "x86_64")
        set(FRN_GODOT_CPP_ARCH_SUFFIX "x86_64")
    else()
        message(FATAL_ERROR "Unsupported ANDROID_ABI='${ANDROID_ABI}' for godot-cpp selection.")
    endif()
    file(GLOB GODOT_CPP_LIB_CANDIDATES
        "${GODOT_CPP_DIR}/bin/libgodot-cpp.android.*.${FRN_GODOT_CPP_ARCH_SUFFIX}.a")
elseif(APPLE)
    file(GLOB GODOT_CPP_LIB_CANDIDATES
        "${GODOT_CPP_DIR}/bin/libgodot-cpp.macos.*.a")
elseif(WIN32)
    file(GLOB GODOT_CPP_LIB_CANDIDATES
        "${GODOT_CPP_DIR}/bin/libgodot-cpp.windows.*.lib")
else()
    file(GLOB GODOT_CPP_LIB_CANDIDATES
        "${GODOT_CPP_DIR}/bin/libgodot-cpp.linux.*.a")
endif()

list(LENGTH GODOT_CPP_LIB_CANDIDATES GODOT_CPP_LIB_COUNT)
if(GODOT_CPP_LIB_COUNT EQUAL 0)
    message(FATAL_ERROR "No matching godot-cpp library found in ${GODOT_CPP_DIR}/bin")
endif()
list(SORT GODOT_CPP_LIB_CANDIDATES)
list(GET GODOT_CPP_LIB_CANDIDATES 0 GODOT_CPP_LIB)
message(STATUS "Using godot-cpp library: ${GODOT_CPP_LIB}")

# ---------------------------------------------------------------------------
# SMLCore.Native (parser - compiled directly into the extension)
# ---------------------------------------------------------------------------
set(SMLCORE_NATIVE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../SMLCore.Native")
set(SMSCORE_NATIVE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../SMSCore.Native")

if(NOT EXISTS "${SMLCORE_NATIVE_DIR}/include/sml_document.h")
    message(FATAL_ERROR "SMLCore.Native not found at '${SMLCORE_NATIVE_DIR}'.")
endif()
if(NOT EXISTS "${SMSCORE_NATIVE_DIR}/include/sms_native.h")
    message(FATAL_ERROR "SMSCore.Native not found at '${SMSCORE_NATIVE_DIR}'.")
endif()

# ---------------------------------------------------------------------------
# forge_runner_native shared library (GDExtension)
# ---------------------------------------------------------------------------
add_library(forge_runner_native SHARED
    src/forge_asset_cache.cpp
    src/forge_json_string.cpp
    src/forge_markdown.cpp
    src/forge_runner_extension.cpp
    src/forge_runner_main.cpp
    src/forge_sms_error_policy.cpp
    src/forge_sms_bridge.cpp
    src/forge_ui_builder.cpp
    "${SMLCORE_NATIVE_DIR}/src/smlcore_native.cpp"
)

if(ANDROID)
    target_sources(forge_runner_native PRIVATE
        "${SMSCORE_NATIVE_DIR}/src/sms_native.cpp"
    )
endif()

target_include_directories(forge_runner_native PRIVATE
    "${GODOT_CPP_INCLUDE_DIR}"
    "${GODOT_CPP_GEN_DIR}/include"
    "${GODOT_CPP_BINDING_DIR}"
    "${SMLCORE_NATIVE_DIR}/include"
    "${SMSCORE_NATIVE_DIR}/include"
    "src"
)

target_link_libraries(forge_runner_native PRIVATE "${GODOT_CPP_LIB}")

if(APPLE)
    target_link_options(forge_runner_native PRIVATE "-Wl,-undefined,dynamic_lookup")
    # Use absolute install name so macOS dyld doesn't confuse this with other
    # libraries sharing the same @rpath-relative install name.
    set_target_properties(forge_runner_native PROPERTIES
        MACOSX_RPATH OFF
        BUILD_WITH_INSTALL_RPATH OFF
    )
endif()

# ---------------------------------------------------------------------------
# Standalone executable (thin launcher - optional)
# ---------------------------------------------------------------------------
add_executable(forge-runner-native
    src/main.cpp
)

if(BUILD_TESTING)
    add_executable(forge_runner_native_sms_text_codec_tests
        tests/sms_text_codec_tests.cpp
        src/forge_json_string.cpp
    )

    add_test(NAME forge_runner_native_sms_text_codec_tests
        COMMAND forge_runner_native_sms_text_codec_tests)

    set(FRN_SMS_TEXT_CODEC_CASES
        sms_text_codec_decodes_multiline_html
        sms_text_codec_real_fixture_qml_payload_roundtrip
        sms_text_codec_real_fixture_generated_html_contains_expected_markup
        sms_text_codec_decodes_unicode_escape
        sms_text_codec_invalid_escape_falls_back
        sms_text_codec_non_string_falls_back
        sms_text_codec_perf_large_payload
    )
    foreach(CASE_NAME IN LISTS FRN_SMS_TEXT_CODEC_CASES)
        add_test(NAME forge_runner_native_${CASE_NAME}
            COMMAND forge_runner_native_sms_text_codec_tests ${CASE_NAME})
    endforeach()

    add_executable(forge_runner_native_sms_error_policy_tests
        tests/sms_error_policy_tests.cpp
        src/forge_sms_error_policy.cpp
    )

    add_test(NAME forge_runner_native_sms_error_policy_tests
        COMMAND forge_runner_native_sms_error_policy_tests)

    set(FRN_SMS_ERROR_POLICY_CASES
        sms_error_policy_missing_handler_does_not_require_exit
        sms_error_policy_runtime_error_requires_exit
        sms_error_policy_non_runtime_error_does_not_require_exit
        sms_error_policy_plain_stack_overflow_requires_exit
    )
    foreach(CASE_NAME IN LISTS FRN_SMS_ERROR_POLICY_CASES)
        add_test(NAME forge_runner_native_${CASE_NAME}
            COMMAND forge_runner_native_sms_error_policy_tests ${CASE_NAME})
    endforeach()

    # ---------------------------------------------------------------------------
    # SMS Runner Integration Tests
    # Exercises SMS script execution + mock UI callbacks + dispatch depth guard.
    # Reproduces stack overflow scenarios (e.g. setKeyframe cycle in ForgePoser).
    # No Godot dependency - links sms_native sources directly.
    # ---------------------------------------------------------------------------
    set(SMS_NATIVE_DIR "${SMSCORE_NATIVE_DIR}")

    if(EXISTS "${SMS_NATIVE_DIR}/src/sms_native.cpp")
        add_executable(forge_runner_native_sms_integration_tests
            tests/sms_runner_integration_tests.cpp
            src/forge_sms_error_policy.cpp
            "${SMS_NATIVE_DIR}/src/sms_native.cpp"
            "${SMLCORE_NATIVE_DIR}/src/smlcore_native.cpp"
        )

        target_include_directories(forge_runner_native_sms_integration_tests PRIVATE
            "${SMS_NATIVE_DIR}/include"
            "${SMLCORE_NATIVE_DIR}/include"
            "src"
        )

        add_test(NAME forge_runner_native_sms_integration_tests
            COMMAND forge_runner_native_sms_integration_tests)

        set(FRN_INTEGRATION_CASES
            runner_integration_basic_dispatch
            runner_integration_no_handler_is_silent
            runner_integration_direct_script_recursion_caught
            runner_integration_dispatch_depth_guard_fires
            runner_integration_setKeyframe_cycle_caught
            runner_integration_setKeyframe_cycle_unprotected
            runner_integration_sequential_dispatches_ok
            runner_integration_error_policy_on_overflow
            runner_integration_property_get_set_round_trip
            runner_sml_known_id_accessible
            runner_sml_unknown_id_is_null
            runner_sml_initial_attribute_readable
            runner_sml_button_click_updates_label
        )
        foreach(CASE_NAME IN LISTS FRN_INTEGRATION_CASES)
            add_test(NAME forge_runner_native_${CASE_NAME}
                COMMAND forge_runner_native_sms_integration_tests ${CASE_NAME})
        endforeach()
    else()
        message(WARNING "SMSCore.Native not found - skipping SMS integration tests.")
    endif()
endif()
