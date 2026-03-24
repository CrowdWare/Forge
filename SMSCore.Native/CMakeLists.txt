cmake_minimum_required(VERSION 3.16)
project(sms_native LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(sms_native SHARED
    src/sms_native.cpp
)

target_include_directories(sms_native PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(BUILD_BENCHMARK)
    add_executable(sms_bench
        benchmark/sms_bench.cpp
    )
    target_link_libraries(sms_bench PRIVATE sms_native)
endif()

include(CTest)
if(BUILD_TESTING)
    add_executable(sms_native_spec_tests
        tests/spec_2026_tests.cpp
    )
    target_link_libraries(sms_native_spec_tests PRIVATE sms_native)
    set(SMS_SPEC_CASES
        import_res_scheme
        import_appres_scheme
        import_user_scheme
        import_forbid_double_slash
        import_forbid_relative
        import_forbid_parent_traversal_after_scheme
        import_forbid_deep_parent_traversal_after_scheme
        try_catch_handles_runtime_error
        try_catch_skips_catch_when_no_error
        try_catch_does_not_swallow_return
        export_function_named_args_and_defaults
        data_class_named_args_and_defaults
        for_in_typed_variable
        named_then_positional_rejected
        unknown_named_function_argument
        duplicate_named_function_argument
        missing_required_function_argument
        unknown_named_dataclass_argument
        duplicate_named_dataclass_argument
        missing_required_dataclass_argument
        invalid_import_scheme
        os_file_exists_rejects_non_sandbox_path
        fs_read_text_rejects_non_sandbox_path
        os_file_exists_trusted_fallback_without_sandbox_policy_callback
    )
    foreach(CASE_NAME IN LISTS SMS_SPEC_CASES)
        add_test(NAME sms_native_spec_${CASE_NAME} COMMAND sms_native_spec_tests ${CASE_NAME})
    endforeach()

    add_executable(sms_native_parser_events_tests
        tests/parser_events_tests.cpp
    )
    target_link_libraries(sms_native_parser_events_tests PRIVATE sms_native)
    set(SMS_EVENTS_PARSER_CASES
        sms_events_parse_with_event_handlers_parses_top_level_handlers
        sms_events_parse_with_invalid_event_syntax_rejected_empty_param_comma
        sms_events_parse_with_invalid_event_syntax_rejected_missing_comma
        sms_events_parse_with_duplicate_event_handlers_throws
        sms_events_runtime_invoke_event_with_no_handler_returns_error
        sms_events_runtime_invoke_event_binds_args_and_executes_body
        sms_events_runtime_invoke_event_with_arg_count_mismatch_throws
        sms_parser_multiline_global_function_call_parses
        sms_parser_multiline_method_call_parses
        sms_parser_multiline_nested_calls_parses
        sms_parser_integer_literal_uses_integer_runtime_value
        sms_parser_double_literals_are_rejected_in_native_subset
        sms_parser_exponentials_are_rejected
    )
    foreach(CASE_NAME IN LISTS SMS_EVENTS_PARSER_CASES)
        add_test(NAME ${CASE_NAME} COMMAND sms_native_parser_events_tests ${CASE_NAME})
    endforeach()

    add_executable(sms_native_ui_string_bridge_tests
        tests/ui_string_bridge_tests.cpp
    )
    target_link_libraries(sms_native_ui_string_bridge_tests PRIVATE sms_native)
    set(SMS_UI_STRING_BRIDGE_CASES
        ui_string_bridge_text_roundtrip_uses_raw_callbacks
        ui_string_bridge_text_roundtrip_falls_back_to_json_without_raw_callbacks
    )
    foreach(CASE_NAME IN LISTS SMS_UI_STRING_BRIDGE_CASES)
        add_test(NAME sms_native_${CASE_NAME} COMMAND sms_native_ui_string_bridge_tests ${CASE_NAME})
    endforeach()
endif()
