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
        runtime_error_division_by_zero
        runtime_error_division_by_zero_thread_message
        runtime_error_division_by_zero_has_location
        runtime_error_modulo_by_zero
        runtime_error_index_out_of_bounds
        runtime_error_tuple_index_out_of_bounds
        runtime_error_null_access
        runtime_error_has_source_line_context
        tuple_return_replaces_try_catch
        tuple_literal_indexing
        tuple_literal_first_element
        tuple_return_from_function
        tuple_return_error_case
        tuple_return_success_flag
        tuple_return_failure_flag
        type_mismatch_reassign_int_to_string
        type_mismatch_reassign_string_to_int
        type_mismatch_error_contains_expected_and_got
        type_mismatch_has_source_location
        type_mismatch_has_source_line_context
        type_consistent_reassign_allowed
        guru_meditation_recursion_limit
        guru_meditation_contains_42
        guru_meditation_point_of_no_return
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
        sms_runtime_recursive_function_hits_recursion_limit
        sms_parser_string_concat_dollar_operator_with_call_parses_and_executes
        sms_parser_string_interpolation_braces_with_call_parses_and_executes
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

    add_executable(sms_native_compiler_codegen_tests
        tests/compiler_codegen_tests.cpp
    )
    target_link_libraries(sms_native_compiler_codegen_tests PRIVATE sms_native)
    set(SMS_COMPILER_CODEGEN_CASES
        sms_compiler_codegen_llvm_ir_emits_main_for_top_level_call
        sms_compiler_codegen_llvm_ir_uses_main_function_fallback
        sms_compiler_codegen_llvm_ir_reports_buffer_too_small
        sms_compiler_codegen_llvm_ir_reports_unsupported_expression
        sms_compiler_aot_invoke_validates_required_arguments
        sms_compiler_aot_invoke_reports_clang_failure
        sms_compiler_aot_invoke_succeeds_when_clang_available_or_skips
        sms_compiler_aot_invoke_reuses_cached_binary_without_clang_or_skips
        sms_compiler_aot_invoke_emits_shared_library_artifact_or_skips
    )
    foreach(CASE_NAME IN LISTS SMS_COMPILER_CODEGEN_CASES)
        add_test(NAME sms_native_${CASE_NAME} COMMAND sms_native_compiler_codegen_tests ${CASE_NAME})
    endforeach()
endif()
