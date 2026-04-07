// Tu keinem Lebewesen Leid an.

/*
#############################################################################
# Copyright (C) 2026 CrowdWare
#
# This file is part of Forge.
#
# SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-CrowdWare-Commercial
#
# Forge is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Forge is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Forge. If not, see <https://www.gnu.org/licenses/>.
#
# Commercial licensing is available from CrowdWare for proprietary use.
#############################################################################
*/

#include "sms_native.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#if defined(_WIN32)
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#if !defined(_WIN32)
#include <sys/wait.h>
#endif

namespace {
namespace fs = std::filesystem;

struct SmsSessionRuntime;

struct SmsSessionState {
    std::string source;
    std::shared_ptr<SmsSessionRuntime> runtime;
    int invoke_depth = 0;
};

std::mutex g_sessions_mutex;
std::unordered_map<std::int64_t, SmsSessionState> g_sessions;
std::int64_t g_next_session_id = 1;
thread_local std::vector<std::string> g_llvm_string_arena;
sms_native_ui_get_prop_fn g_ui_get_prop = nullptr;
sms_native_ui_set_prop_fn g_ui_set_prop = nullptr;
sms_native_ui_invoke_fn g_ui_invoke = nullptr;
sms_native_ui_get_string_prop_fn g_ui_get_string_prop = nullptr;
sms_native_ui_set_string_prop_fn g_ui_set_string_prop = nullptr;
sms_native_sandbox_path_allow_fn g_sandbox_path_allow = nullptr;
constexpr int kBridgeJsonBufferSize = 65536;
constexpr int kBridgeStringBufferSize = 262144;
constexpr std::size_t kMaxInterpreterCallDepth = 1024;
constexpr int kMaxSessionInvokeDepth = 3;
std::mutex g_aot_mutex;

bool has_parent_traversal_segment(const std::string& path) {
    std::size_t start = 0;
    while (start <= path.size()) {
        std::size_t end = path.find('/', start);
        if (end == std::string::npos) {
            end = path.size();
        }
        if (end > start) {
            const auto segment = path.substr(start, end - start);
            if (segment == "..") {
                return true;
            }
        }
        if (end == path.size()) {
            break;
        }
        start = end + 1;
    }
    return false;
}

void validate_sandbox_uri_path(const std::string& path, const char* owner) {
    if (path.empty()) {
        throw std::runtime_error(std::string(owner) + " path must not be empty.");
    }
    if (path.find("//") != std::string::npos) {
        throw std::runtime_error(std::string(owner) + " path must not contain '//'.");
    }
    const bool allowed_scheme = path.rfind("res:/", 0) == 0
        || path.rfind("appRes:/", 0) == 0
        || path.rfind("user:/", 0) == 0;
    if (!allowed_scheme) {
        throw std::runtime_error(
            std::string(owner) + " path must start with res:/, appRes:/, or user:/");
    }
    const auto slash = path.find('/');
    if (slash != std::string::npos && has_parent_traversal_segment(path.substr(slash + 1))) {
        throw std::runtime_error(std::string(owner) + " path must not contain '..' segments.");
    }
}

void validate_and_authorize_sandbox_uri_path(const std::string& path, const char* owner) {
    const bool is_sandbox_uri = path.rfind("res:/", 0) == 0
        || path.rfind("appRes:/", 0) == 0
        || path.rfind("user:/", 0) == 0;

    if (is_sandbox_uri) {
        validate_sandbox_uri_path(path, owner);
    }

    // Backward-compatible trusted-local default:
    // - no callback registered -> allow host bridge to decide, preserving existing local workflows.
    // - callback registered -> callback enforces trust mode (local-trusted vs remote-sandboxed).
    if (g_sandbox_path_allow != nullptr) {
        char error[1024] = {0};
        const int rc = g_sandbox_path_allow(owner, path.c_str(), error, static_cast<int>(sizeof(error)));
        if (rc != 0) {
            if (error[0] != '\0') {
                throw std::runtime_error(error);
            }
            throw std::runtime_error(std::string(owner) + " path rejected by sandbox policy.");
        }
    }
}

enum class TokenType {
    Eof,
    Identifier,
    Number,
    String,
    Import,
    Export,
    As,
    Var,
    For,
    In,
    If,
    Else,
    When,
    While,
    Fun,
    Data,
    Class,
    On,
    True,
    False,
    Null,
    Break,
    Continue,
    Return,
    LeftParen,
    RightParen,
    LeftBrace,
    RightBrace,
    LeftBracket,
    RightBracket,
    Dot,
    Comma,
    Colon,
    Semicolon,
    Assign,
    Plus,
    Dollar,
    Increment,
    Minus,
    Decrement,
    Star,
    Slash,
    Percent,
    Arrow,
    Less,
    Greater,
    LessEqual,
    GreaterEqual,
    EqualEqual,
    BangEqual,
    Bang,
    AndAnd,
    OrOr
};

struct Token {
    TokenType type;
    std::string text;
    int line = 1;
    int col = 1;
};

class Lexer {
public:
    explicit Lexer(std::string src) : source_(std::move(src)) {}

    std::vector<Token> tokenize() {
        std::vector<Token> out;
        while (true) {
            skip_whitespace();
            if (is_at_end()) {
                out.push_back({TokenType::Eof, "", line_, col_});
                return out;
            }

            const int tok_line = line_;
            const int tok_col = col_;
            const std::size_t prev_size = out.size();
            const char c = advance();
            switch (c) {
                case '(':
                    out.push_back({TokenType::LeftParen, "("});
                    break;
                case ')':
                    out.push_back({TokenType::RightParen, ")"});
                    break;
                case '{':
                    out.push_back({TokenType::LeftBrace, "{"});
                    break;
                case '}':
                    out.push_back({TokenType::RightBrace, "}"});
                    break;
                case '[':
                    out.push_back({TokenType::LeftBracket, "["});
                    break;
                case ']':
                    out.push_back({TokenType::RightBracket, "]"});
                    break;
                case '.':
                    out.push_back({TokenType::Dot, "."});
                    break;
                case ',':
                    out.push_back({TokenType::Comma, ","});
                    break;
                case ':':
                    out.push_back({TokenType::Colon, ":"});
                    break;
                case ';':
                    out.push_back({TokenType::Semicolon, ";"});
                    break;
                case '=':
                    if (!is_at_end() && peek() == '=') {
                        advance();
                        out.push_back({TokenType::EqualEqual, "=="});
                    } else {
                        out.push_back({TokenType::Assign, "="});
                    }
                    break;
                case '+':
                    if (!is_at_end() && peek() == '+') {
                        advance();
                        out.push_back({TokenType::Increment, "++"});
                    } else {
                        out.push_back({TokenType::Plus, "+"});
                    }
                    break;
                case '$':
                    out.push_back({TokenType::Dollar, "$"});
                    break;
                case '-':
                    if (!is_at_end() && peek() == '>') {
                        advance();
                        out.push_back({TokenType::Arrow, "->"});
                    } else if (!is_at_end() && peek() == '-') {
                        advance();
                        out.push_back({TokenType::Decrement, "--"});
                    } else {
                        out.push_back({TokenType::Minus, "-"});
                    }
                    break;
                case '*':
                    out.push_back({TokenType::Star, "*"});
                    break;
                case '/':
                    if (!is_at_end() && peek() == '/') {
                        // line comment
                        advance(); // consume second '/'
                        while (!is_at_end()) {
                            const char ch = advance();
                            if (ch == '\n') {
                                break;
                            }
                        }
                    } else if (!is_at_end() && peek() == '*') {
                        // block comment
                        advance(); // consume '*'
                        bool closed = false;
                        while (!is_at_end()) {
                            const char ch = advance();
                            if (ch == '*' && !is_at_end() && peek() == '/') {
                                advance(); // consume '/'
                                closed = true;
                                break;
                            }
                        }
                        if (!closed) {
                            throw std::runtime_error("Unterminated block comment.");
                        }
                    } else {
                        out.push_back({TokenType::Slash, "/"});
                    }
                    break;
                case '%':
                    out.push_back({TokenType::Percent, "%"});
                    break;
                case '<':
                    if (!is_at_end() && peek() == '=') {
                        advance();
                        out.push_back({TokenType::LessEqual, "<="});
                    } else {
                        out.push_back({TokenType::Less, "<"});
                    }
                    break;
                case '>':
                    if (!is_at_end() && peek() == '=') {
                        advance();
                        out.push_back({TokenType::GreaterEqual, ">="});
                    } else {
                        out.push_back({TokenType::Greater, ">"});
                    }
                    break;
                case '!':
                    if (!is_at_end() && peek() == '=') {
                        advance();
                        out.push_back({TokenType::BangEqual, "!="});
                    } else {
                        out.push_back({TokenType::Bang, "!"});
                    }
                    break;
                case '&':
                    if (!is_at_end() && peek() == '&') {
                        advance();
                        out.push_back({TokenType::AndAnd, "&&"});
                    } else {
                        throw std::runtime_error("Unexpected character in source.");
                    }
                    break;
                case '|':
                    if (!is_at_end() && peek() == '|') {
                        advance();
                        out.push_back({TokenType::OrOr, "||"});
                    } else {
                        throw std::runtime_error("Unexpected character in source.");
                    }
                    break;
                default:
                    if (std::isdigit(static_cast<unsigned char>(c))) {
                        out.push_back(number_token(c, tok_line, tok_col));
                    } else if (c == '"') {
                        out.push_back(string_token(tok_line, tok_col));
                    } else if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
                        out.push_back(identifier_token(c, tok_line, tok_col));
                    } else {
                        throw std::runtime_error("Unexpected character in source.");
                    }
                    break;
            }
            // Stamp start position on any token pushed this iteration.
            if (out.size() > prev_size) {
                out.back().line = tok_line;
                out.back().col = tok_col;
            }
        }
    }

private:
    bool is_at_end() const { return index_ >= source_.size(); }

    char advance() {
        const char c = source_[index_++];
        if (c == '\n') { line_++; col_ = 1; }
        else { col_++; }
        return c;
    }

    char peek() const { return is_at_end() ? '\0' : source_[index_]; }

    void skip_whitespace() {
        static constexpr std::string_view kAhimsa[]  = {
            "Tu keinem Lebewesen Leid an.",
            "Do not harm to living beings.",
            "Ne dama\xc4\xa5u iun ajn vivantan esta\xc4\xb5on.",
        };
        while (!is_at_end()) {
            const char c = peek();
            if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
                advance();
                continue;
            }
            // Easter egg: mantra line in any supported language is a valid no-op.
            bool skipped = false;
            for (const auto& m : kAhimsa) {
                if (source_.compare(index_, m.size(), m) == 0) {
                    while (!is_at_end() && peek() != '\n') advance();
                    skipped = true;
                    break;
                }
            }
            if (skipped) continue;
            break;
        }
    }

    Token number_token(char first, int tok_line, int tok_col) {
        std::string text;
        text.push_back(first);
        while (!is_at_end() && std::isdigit(static_cast<unsigned char>(peek()))) {
            text.push_back(advance());
        }
        return {TokenType::Number, text, tok_line, tok_col};
    }

    Token string_token(int tok_line, int tok_col) {
        std::string text;
        bool escape = false;
        while (!is_at_end()) {
            const char c = advance();
            if (escape) {
                switch (c) {
                    case '"': text.push_back('"'); break;
                    case '\\': text.push_back('\\'); break;
                    case '/': text.push_back('/'); break;
                    case 'b': text.push_back('\b'); break;
                    case 'f': text.push_back('\f'); break;
                    case 'n': text.push_back('\n'); break;
                    case 'r': text.push_back('\r'); break;
                    case 't': text.push_back('\t'); break;
                    default: text.push_back(c); break;
                }
                escape = false;
                continue;
            }

            if (c == '\\') {
                escape = true;
                continue;
            }

            if (c == '"') {
                return {TokenType::String, text, tok_line, tok_col};
            }

            text.push_back(c);
        }

        throw std::runtime_error("Unterminated string literal.");
    }

    Token identifier_token(char first, int tok_line, int tok_col) {
        std::string text;
        text.push_back(first);
        while (!is_at_end()) {
            const char c = peek();
            if (std::isalnum(static_cast<unsigned char>(c)) || c == '_') {
                text.push_back(advance());
                continue;
            }
            break;
        }

        if (text == "import") return {TokenType::Import, text, tok_line, tok_col};
        if (text == "export") return {TokenType::Export, text, tok_line, tok_col};
        if (text == "as") return {TokenType::As, text, tok_line, tok_col};
        if (text == "var") return {TokenType::Var, text, tok_line, tok_col};
        if (text == "for") return {TokenType::For, text, tok_line, tok_col};
        if (text == "in") return {TokenType::In, text, tok_line, tok_col};
        if (text == "if") return {TokenType::If, text, tok_line, tok_col};
        if (text == "else") return {TokenType::Else, text, tok_line, tok_col};
        if (text == "when") return {TokenType::When, text, tok_line, tok_col};
        if (text == "while") return {TokenType::While, text, tok_line, tok_col};
        if (text == "fun") return {TokenType::Fun, text, tok_line, tok_col};
        if (text == "data") return {TokenType::Data, text, tok_line, tok_col};
        if (text == "class") return {TokenType::Class, text, tok_line, tok_col};
        if (text == "on") return {TokenType::On, text, tok_line, tok_col};
        if (text == "true") return {TokenType::True, text, tok_line, tok_col};
        if (text == "false") return {TokenType::False, text, tok_line, tok_col};
        if (text == "null") return {TokenType::Null, text, tok_line, tok_col};
        if (text == "break") return {TokenType::Break, text, tok_line, tok_col};
        if (text == "continue") return {TokenType::Continue, text, tok_line, tok_col};
        if (text == "return") return {TokenType::Return, text, tok_line, tok_col};
        return {TokenType::Identifier, text, tok_line, tok_col};
    }

    std::string source_;
    std::size_t index_ = 0;
    int line_ = 1;
    int col_ = 1;
};

struct FunctionDeclStmt;
struct DataClassDeclStmt;
struct EventHandlerDeclStmt;

struct SourceLoc {
    int line = 0;
    int col = 0;
};

thread_local std::vector<std::string> g_source_lines;

static void init_source_lines(const std::string& source) {
    g_source_lines.clear();
    std::string cur;
    for (const char c : source) {
        if (c == '\n') { g_source_lines.push_back(cur); cur.clear(); }
        else { cur.push_back(c); }
    }
    g_source_lines.push_back(cur);
}

static std::string get_source_line(int line) {
    if (line >= 1 && static_cast<std::size_t>(line) <= g_source_lines.size()) {
        return g_source_lines[static_cast<std::size_t>(line - 1)];
    }
    return "";
}

static std::string value_kind_name(int kind_int) {
    // Avoids circular dependency with Value::Kind - called after Value is defined.
    // 0=Int,1=Bool,2=String,3=Array,4=Object,5=Null,6=Tuple
    switch (kind_int) {
        case 0: return "Int32";
        case 1: return "Bool";
        case 2: return "String";
        case 3: return "Array";
        case 4: return "Object";
        case 5: return "Null";
        case 6: return "Tuple";
    }
    return "Unknown";
}

struct Value {
    enum class Kind { Int, Bool, String, Array, Object, Null, Tuple };

    Kind kind = Kind::Null;
    std::int64_t int_value = 0;
    bool bool_value = false;
    std::string string_value;
    std::shared_ptr<std::vector<Value>> array;
    std::string class_name;
    std::shared_ptr<std::unordered_map<std::string, Value>> object_fields;

    static Value Int(std::int64_t value) {
        Value v;
        v.kind = Kind::Int;
        v.int_value = value;
        return v;
    }

    static Value Bool(bool value) {
        Value v;
        v.kind = Kind::Bool;
        v.bool_value = value;
        return v;
    }

    static Value String(std::string value) {
        Value v;
        v.kind = Kind::String;
        v.string_value = std::move(value);
        return v;
    }

    static Value Array(std::vector<Value> elements) {
        Value v;
        v.kind = Kind::Array;
        v.array = std::make_shared<std::vector<Value>>(std::move(elements));
        return v;
    }

    static Value Null() {
        return Value{};
    }

    static Value Tuple(std::vector<Value> elements) {
        Value v;
        v.kind = Kind::Tuple;
        v.array = std::make_shared<std::vector<Value>>(std::move(elements));
        return v;
    }

    static Value Object(std::string class_name, std::unordered_map<std::string, Value> fields) {
        Value v;
        v.kind = Kind::Object;
        v.class_name = std::move(class_name);
        v.object_fields = std::make_shared<std::unordered_map<std::string, Value>>(std::move(fields));
        return v;
    }

    bool truthy() const {
        if (kind == Kind::Int) return int_value != 0;
        if (kind == Kind::Bool) return bool_value;
        if (kind == Kind::String) return !string_value.empty();
        if (kind == Kind::Array) return array && !array->empty();
        if (kind == Kind::Tuple) return array && !array->empty();
        if (kind == Kind::Object) return object_fields && !object_fields->empty();
        return false;
    }

    std::int64_t as_int(const std::string& where) const {
        if (kind == Kind::Int) {
            return int_value;
        }

        if (kind == Kind::Bool) {
            return bool_value ? 1 : 0;
        }

        throw std::runtime_error(where + " expects integer value.");
    }

    bool operator==(const Value& other) const {
        if (kind != other.kind) {
            return false;
        }

        switch (kind) {
            case Kind::Int:
                return int_value == other.int_value;
            case Kind::Bool:
                return bool_value == other.bool_value;
            case Kind::String:
                return string_value == other.string_value;
            case Kind::Null:
                return true;
            case Kind::Array:
                if (!array || !other.array) {
                    return array == other.array;
                }
                return *array == *other.array;
            case Kind::Tuple:
                if (!array || !other.array) {
                    return array == other.array;
                }
                return *array == *other.array;
            case Kind::Object:
                if (!object_fields || !other.object_fields) {
                    return object_fields == other.object_fields;
                }
                return class_name == other.class_name && *object_fields == *other.object_fields;
        }

        return false;
    }
};

static std::string value_to_string(const Value& value) {
    switch (value.kind) {
        case Value::Kind::Int:
            return std::to_string(value.int_value);
        case Value::Kind::Bool:
            return value.bool_value ? "true" : "false";
        case Value::Kind::String:
            return value.string_value;
        case Value::Kind::Null:
            return "null";
        case Value::Kind::Array:
            return "[array]";
        case Value::Kind::Tuple: {
            std::string out = "(";
            if (value.array) {
                for (std::size_t i = 0; i < value.array->size(); ++i) {
                    if (i > 0) out += ", ";
                    out += value_to_string((*value.array)[i]);
                }
            }
            out += ")";
            return out;
        }
        case Value::Kind::Object:
            return "[object]";
    }
    return "";
}

static std::string json_escape(const std::string& text) {
    std::string out;
    out.reserve(text.size() + 8);
    for (const auto ch : text) {
        switch (ch) {
            case '\\': out += "\\\\"; break;
            case '"': out += "\\\""; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default: out.push_back(ch); break;
        }
    }
    return out;
}

static std::string value_to_json(const Value& value) {
    switch (value.kind) {
        case Value::Kind::Int:
            return std::to_string(value.int_value);
        case Value::Kind::Bool:
            return value.bool_value ? "true" : "false";
        case Value::Kind::String:
            return std::string("\"") + json_escape(value.string_value) + "\"";
        case Value::Kind::Null:
            return "null";
        case Value::Kind::Array:
            return "null";
        case Value::Kind::Tuple: {
            std::string out = "[";
            if (value.array) {
                for (std::size_t i = 0; i < value.array->size(); ++i) {
                    if (i > 0) out += ",";
                    out += value_to_json((*value.array)[i]);
                }
            }
            out += "]";
            return out;
        }
        case Value::Kind::Object:
            if (value.class_name == "__host_ref" && value.object_fields) {
                const auto it = value.object_fields->find("__nativeObjectId");
                if (it != value.object_fields->end() && it->second.kind == Value::Kind::Int) {
                    return std::string("{\"__nativeObjectId\":") + std::to_string(it->second.int_value) + "}";
                }
            }
            return "null";
    }
    return "null";
}

static Value parse_json_scalar(std::string text) {
    auto trim = [](std::string& s) {
        auto start = std::size_t{0};
        while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start])) != 0) {
            start++;
        }
        auto end = s.size();
        while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1])) != 0) {
            end--;
        }
        s = s.substr(start, end - start);
    };

    trim(text);
    if (text.empty() || text == "null") {
        return Value::Null();
    }
    if (text == "true") {
        return Value::Bool(true);
    }
    if (text == "false") {
        return Value::Bool(false);
    }
    if (text.front() == '"' && text.back() == '"' && text.size() >= 2) {
        std::string out;
        out.reserve(text.size() - 2);
        bool escape = false;
        for (std::size_t i = 1; i + 1 < text.size(); i++) {
            const auto c = text[i];
            if (escape) {
                switch (c) {
                    case 'n': out.push_back('\n'); break;
                    case 'r': out.push_back('\r'); break;
                    case 't': out.push_back('\t'); break;
                    case '"': out.push_back('"'); break;
                    case '\\': out.push_back('\\'); break;
                    default: out.push_back(c); break;
                }
                escape = false;
                continue;
            }
            if (c == '\\') {
                escape = true;
                continue;
            }
            out.push_back(c);
        }
        return Value::String(std::move(out));
    }

    try {
        std::size_t consumed = 0;
        const auto number = std::stoll(text, &consumed, 10);
        if (consumed == text.size()) {
            return Value::Int(number);
        }
    } catch (...) {
    }

    return Value::String(text);
}

static Value parse_json_value(std::string text) {
    auto trim = [](std::string& s) {
        auto start = std::size_t{0};
        while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start])) != 0) {
            start++;
        }
        auto end = s.size();
        while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1])) != 0) {
            end--;
        }
        s = s.substr(start, end - start);
    };

    trim(text);
    if (text.rfind("{\"__nativeObjectId\":", 0) == 0 && !text.empty() && text.back() == '}') {
        const auto prefix = std::string("{\"__nativeObjectId\":");
        const auto number_text = text.substr(prefix.size(), text.size() - prefix.size() - 1);
        try {
            const auto id = std::stoll(number_text);
            std::unordered_map<std::string, Value> fields;
            fields.emplace("__nativeObjectId", Value::Int(id));
            return Value::Object("__host_ref", std::move(fields));
        } catch (...) {
            return Value::Null();
        }
    }

    if (!text.empty() && text.front() == '[' && text.back() == ']') {
        std::vector<Value> items;
        std::size_t i = 1;
        std::size_t token_start = i;
        int nested_braces = 0;
        int nested_brackets = 0;
        bool in_string = false;
        bool escape = false;

        auto push_token = [&](std::size_t start, std::size_t end) {
            if (end < start) {
                return;
            }
            auto raw = text.substr(start, end - start);
            auto trim = [](std::string& s) {
                auto start_ws = std::size_t{0};
                while (start_ws < s.size() && std::isspace(static_cast<unsigned char>(s[start_ws])) != 0) {
                    start_ws++;
                }
                auto end_ws = s.size();
                while (end_ws > start_ws && std::isspace(static_cast<unsigned char>(s[end_ws - 1])) != 0) {
                    end_ws--;
                }
                s = s.substr(start_ws, end_ws - start_ws);
            };
            trim(raw);
            if (raw.empty()) {
                return;
            }
            items.push_back(parse_json_value(std::move(raw)));
        };

        while (i < text.size() - 1) {
            const char c = text[i];
            if (in_string) {
                if (escape) {
                    escape = false;
                } else if (c == '\\') {
                    escape = true;
                } else if (c == '"') {
                    in_string = false;
                }
                i++;
                continue;
            }

            if (c == '"') {
                in_string = true;
                i++;
                continue;
            }
            if (c == '{') {
                nested_braces++;
                i++;
                continue;
            }
            if (c == '}') {
                nested_braces--;
                i++;
                continue;
            }
            if (c == '[') {
                nested_brackets++;
                i++;
                continue;
            }
            if (c == ']') {
                if (nested_brackets > 0) {
                    nested_brackets--;
                }
                i++;
                continue;
            }

            if (c == ',' && nested_braces == 0 && nested_brackets == 0) {
                push_token(token_start, i);
                i++;
                token_start = i;
                continue;
            }

            i++;
        }

        push_token(token_start, text.size() - 1);
        return Value::Array(std::move(items));
    }

    if (!text.empty() && text.front() == '{' && text.back() == '}') {
        std::unordered_map<std::string, Value> fields;
        std::size_t i = 1;

        auto skip_ws = [&]() {
            while (i < text.size() && std::isspace(static_cast<unsigned char>(text[i])) != 0) {
                i++;
            }
        };

        auto parse_string_token = [&]() -> std::string {
            if (i >= text.size() || text[i] != '"') {
                throw std::runtime_error("Expected JSON string token.");
            }
            i++;
            std::string out;
            bool escape = false;
            while (i < text.size()) {
                const char c = text[i++];
                if (escape) {
                    switch (c) {
                        case 'n': out.push_back('\n'); break;
                        case 'r': out.push_back('\r'); break;
                        case 't': out.push_back('\t'); break;
                        case '"': out.push_back('"'); break;
                        case '\\': out.push_back('\\'); break;
                        default: out.push_back(c); break;
                    }
                    escape = false;
                    continue;
                }
                if (c == '\\') {
                    escape = true;
                    continue;
                }
                if (c == '"') {
                    return out;
                }
                out.push_back(c);
            }
            throw std::runtime_error("Unterminated JSON string token.");
        };

        auto parse_scalar_until = [&](char delimiter_a, char delimiter_b) -> Value {
            skip_ws();
            if (i < text.size() && text[i] == '"') {
                return Value::String(parse_string_token());
            }
            const auto start = i;
            while (i < text.size() && text[i] != delimiter_a && text[i] != delimiter_b) {
                i++;
            }
            auto raw = text.substr(start, i - start);
            return parse_json_scalar(std::move(raw));
        };

        skip_ws();
        while (i < text.size() && text[i] != '}') {
            skip_ws();
            const auto key = parse_string_token();
            skip_ws();
            if (i >= text.size() || text[i] != ':') {
                throw std::runtime_error("Expected ':' in JSON object.");
            }
            i++;
            auto value = parse_scalar_until(',', '}');
            fields[key] = std::move(value);
            skip_ws();
            if (i < text.size() && text[i] == ',') {
                i++;
                continue;
            }
            if (i < text.size() && text[i] == '}') {
                break;
            }
        }

        return Value::Object("__json_obj", std::move(fields));
    }

    return parse_json_scalar(std::move(text));
}

struct SmsGuruMeditation final : std::runtime_error {
    explicit SmsGuruMeditation(const std::string& code, const std::string& context = "")
        : std::runtime_error(make_msg(code, context)) {}

    static std::string make_msg(const std::string& code, const std::string& context) {
        // #0000002A = 42 decimal. The answer to life, the universe, and everything.
        std::string msg = "[SMS Guru Meditation] #0000002A." + code + "\n";
        if (!context.empty()) msg += "  in: " + context + "\n";
        msg += "  The interpreter has reached a point of no return.\n";
        msg += "  SMS session terminated. Restart to continue.";
        return msg;
    }
};

class Env {
public:
    explicit Env(Env* parent = nullptr) : parent_(parent) {}

    void define_var(const std::string& name, const Value& value) {
        vars_[name] = value;
        if (value.kind != Value::Kind::Null) {
            var_kinds_[name] = value.kind;
        }
    }

    Value get_var(const std::string& name) const {
        const auto it = vars_.find(name);
        if (it != vars_.end()) {
            return it->second;
        }
        if (parent_ != nullptr) {
            return parent_->get_var(name);
        }
        throw std::runtime_error("Unknown variable: " + name);
    }

    std::optional<Value::Kind> get_var_kind(const std::string& name) const {
        const auto it = var_kinds_.find(name);
        if (it != var_kinds_.end()) return it->second;
        if (parent_ != nullptr) return parent_->get_var_kind(name);
        return std::nullopt;
    }

    bool assign_var(const std::string& name, const Value& value) {
        const auto it = vars_.find(name);
        if (it != vars_.end()) {
            it->second = value;
            if (value.kind != Value::Kind::Null) {
                var_kinds_[name] = value.kind;
            }
            return true;
        }
        if (parent_ != nullptr) {
            return parent_->assign_var(name, value);
        }
        return false;
    }

    void define_function(const std::string& name, const FunctionDeclStmt* decl) {
        root()->functions_[name] = decl;
    }

    const FunctionDeclStmt* get_function(const std::string& name) const {
        const auto* r = root();
        const auto it = r->functions_.find(name);
        return it == r->functions_.end() ? nullptr : it->second;
    }

    void define_data_class(const std::string& name, const DataClassDeclStmt* decl) {
        root()->data_classes_[name] = decl;
    }

    const DataClassDeclStmt* get_data_class(const std::string& name) const {
        const auto* r = root();
        const auto it = r->data_classes_.find(name);
        return it == r->data_classes_.end() ? nullptr : it->second;
    }

    void define_event_handler(const std::string& key, const EventHandlerDeclStmt* decl) {
        root()->event_handlers_[key] = decl;
    }

    const EventHandlerDeclStmt* get_event_handler(const std::string& key) const {
        const auto* r = root();
        const auto it = r->event_handlers_.find(key);
        return it == r->event_handlers_.end() ? nullptr : it->second;
    }

    void enter_call(const std::string& context) {
        auto* r = root();
        if (r->call_depth_ >= kMaxInterpreterCallDepth) {
            throw SmsGuruMeditation("recursion_limit_exceeded", context);
        }
        r->call_depth_++;
    }

    void leave_call() {
        auto* r = root();
        if (r->call_depth_ > 0) {
            r->call_depth_--;
        }
    }

private:
    Env* root() {
        return parent_ == nullptr ? this : parent_->root();
    }

    const Env* root() const {
        return parent_ == nullptr ? this : parent_->root();
    }

    Env* parent_ = nullptr;
    std::unordered_map<std::string, Value> vars_;
    std::unordered_map<std::string, Value::Kind> var_kinds_;
    std::unordered_map<std::string, const FunctionDeclStmt*> functions_;
    std::unordered_map<std::string, const DataClassDeclStmt*> data_classes_;
    std::unordered_map<std::string, const EventHandlerDeclStmt*> event_handlers_;
    std::size_t call_depth_ = 0;
};

struct ReturnSignal final : std::exception {
    explicit ReturnSignal(Value v) : value(std::move(v)) {}
    Value value;
};

struct BreakSignal final : std::exception {};
struct ContinueSignal final : std::exception {};

struct SmsRuntimeError final : std::runtime_error {
    SmsRuntimeError(const std::string& code, SourceLoc loc = {})
        : std::runtime_error(make_msg(code, loc)), code_(code) {}

    static std::string make_msg(const std::string& code, const SourceLoc& loc) {
        std::string msg = "[SMS Runtime Error] " + code + "\n";
        if (loc.line > 0) {
            msg += "  file: <script>, line " + std::to_string(loc.line)
                 + ", col " + std::to_string(loc.col) + "\n";
            const auto src = get_source_line(loc.line);
            if (!src.empty()) msg += "  > " + src + "\n";
        }
        msg += "  SMS thread terminated. App continues.";
        return msg;
    }
    std::string code_;
};

struct Expr {
    virtual ~Expr() = default;
    virtual Value eval(Env& env) const = 0;
    SourceLoc loc;
};

struct NumberExpr final : Expr {
    explicit NumberExpr(std::int64_t v) : value(v) {}
    Value eval(Env&) const override { return Value::Int(value); }
    std::int64_t value;
};

struct StringExpr final : Expr {
    explicit StringExpr(std::string v) : value(std::move(v)) {}
    Value eval(Env&) const override { return Value::String(value); }
    std::string value;
};

struct InterpolatedStringExpr final : Expr {
    struct Part {
        std::string literal;
        std::unique_ptr<Expr> expr;
    };

    explicit InterpolatedStringExpr(std::vector<Part> parts)
        : parts_(std::move(parts)) {}

    Value eval(Env& env) const override {
        std::string out;
        for (const auto& part : parts_) {
            if (part.expr) {
                out += value_to_string(part.expr->eval(env));
            } else {
                out += part.literal;
            }
        }
        return Value::String(std::move(out));
    }

    std::vector<Part> parts_;
};

struct BoolExpr final : Expr {
    explicit BoolExpr(bool v) : value(v) {}
    Value eval(Env&) const override { return Value::Bool(value); }
    bool value;
};

struct NullExpr final : Expr {
    Value eval(Env&) const override { return Value::Null(); }
};

struct VarExpr final : Expr {
    explicit VarExpr(std::string n) : name(std::move(n)) {}
    Value eval(Env& env) const override { return env.get_var(name); }
    std::string name;
};

struct ArrayLiteralExpr final : Expr {
    explicit ArrayLiteralExpr(std::vector<std::unique_ptr<Expr>> elements)
        : elements_(std::move(elements)) {}

    Value eval(Env& env) const override {
        std::vector<Value> out;
        out.reserve(elements_.size());
        for (const auto& expr : elements_) {
            out.push_back(expr->eval(env));
        }
        return Value::Array(std::move(out));
    }

    std::vector<std::unique_ptr<Expr>> elements_;
};

struct TupleLiteralExpr final : Expr {
    explicit TupleLiteralExpr(std::vector<std::unique_ptr<Expr>> elements)
        : elements_(std::move(elements)) {}

    Value eval(Env& env) const override {
        std::vector<Value> out;
        out.reserve(elements_.size());
        for (const auto& expr : elements_) {
            out.push_back(expr->eval(env));
        }
        return Value::Tuple(std::move(out));
    }

    std::vector<std::unique_ptr<Expr>> elements_;
};

struct ArrayAccessExpr final : Expr {
    ArrayAccessExpr(std::unique_ptr<Expr> receiver, std::unique_ptr<Expr> index)
        : receiver_(std::move(receiver)), index_(std::move(index)) {}

    Value eval(Env& env) const override {
        auto receiver = receiver_->eval(env);
        auto index = index_->eval(env).as_int("Array index");
        if (receiver.kind == Value::Kind::Null) {
            throw SmsRuntimeError("null_access", loc);
        }
        if ((receiver.kind != Value::Kind::Array && receiver.kind != Value::Kind::Tuple) || !receiver.array) {
            throw std::runtime_error("Array access expects array receiver.");
        }
        if (index < 0 || static_cast<std::size_t>(index) >= receiver.array->size()) {
            throw SmsRuntimeError("index_out_of_bounds", loc);
        }
        return (*receiver.array)[static_cast<std::size_t>(index)];
    }

    std::unique_ptr<Expr> receiver_;
    std::unique_ptr<Expr> index_;
};

struct MemberAccessExpr final : Expr {
    MemberAccessExpr(std::unique_ptr<Expr> receiver, std::string member)
        : receiver_(std::move(receiver)), member_(std::move(member)) {}

    Value eval(Env& env) const override {
        auto receiver = receiver_->eval(env);
        if (receiver.kind == Value::Kind::Object
            && receiver.class_name == "__ui_ref"
            && receiver.object_fields) {
            const auto id_it = receiver.object_fields->find("id");
            if (id_it == receiver.object_fields->end() || id_it->second.kind != Value::Kind::String) {
                throw std::runtime_error("ui ref missing id.");
            }
            if (g_ui_get_prop == nullptr) {
                throw std::runtime_error("ui bridge unavailable.");
            }
            if (member_ == "text" && g_ui_get_string_prop != nullptr) {
                char out_text[kBridgeStringBufferSize] = {0};
                char error[1024] = {0};
                const auto string_rc = g_ui_get_string_prop(
                    id_it->second.string_value.c_str(),
                    member_.c_str(),
                    out_text,
                    static_cast<int>(sizeof(out_text)),
                    error,
                    static_cast<int>(sizeof(error)));
                if (string_rc != 0) {
                    throw std::runtime_error(error[0] != '\0' ? error : "ui get string failed");
                }
                return Value::String(out_text);
            }
            char out_json[2048] = {0};
            char error[1024] = {0};
            const auto rc = g_ui_get_prop(
                id_it->second.string_value.c_str(),
                member_.c_str(),
                out_json,
                static_cast<int>(sizeof(out_json)),
                error,
                static_cast<int>(sizeof(error)));
            if (rc != 0) {
                throw std::runtime_error(error[0] != '\0' ? error : "ui get failed");
            }
            return parse_json_scalar(out_json);
        }

        if (receiver.kind == Value::Kind::Array && member_ == "size" && receiver.array) {
            return Value::Int(static_cast<std::int64_t>(receiver.array->size()));
        }

        if (receiver.kind != Value::Kind::Object || !receiver.object_fields) {
            throw std::runtime_error("Member access expects object receiver.");
        }

        const auto it = receiver.object_fields->find(member_);
        if (it == receiver.object_fields->end()) {
            return Value::Null();
        }
        return it->second;
    }

    std::unique_ptr<Expr> receiver_;
    std::string member_;
};

struct CallArgumentExpr {
    std::string name;
    std::unique_ptr<Expr> value;
};

struct MethodCallExpr final : Expr {
    MethodCallExpr(std::unique_ptr<Expr> receiver, std::string method, std::vector<CallArgumentExpr> args)
        : receiver_(std::move(receiver)), method_(std::move(method)), args_(std::move(args)) {}

    Value eval(Env& env) const override {
        auto receiver = receiver_->eval(env);
        std::vector<Value> args;
        args.reserve(args_.size());
        for (const auto& arg : args_) {
            if (!arg.name.empty()) {
                throw std::runtime_error("Named arguments are not supported for method calls.");
            }
            args.push_back(arg.value->eval(env));
        }

        if (receiver.kind == Value::Kind::Object && receiver.class_name == "log") {
            if (method_ == "info" || method_ == "success" || method_ == "warning" || method_ == "warn" || method_ == "error" || method_ == "debug") {
                if (g_ui_invoke == nullptr) {
                    throw std::runtime_error("ui invoke bridge unavailable.");
                }

                const std::string bridge_method = (method_ == "warn") ? "warning" : method_;

                std::string args_json = "[";
                for (std::size_t i = 0; i < args.size(); i++) {
                    if (i > 0) {
                        args_json.push_back(',');
                    }
                    args_json += value_to_json(args[i]);
                }
                args_json.push_back(']');

                char out_json[kBridgeJsonBufferSize] = {0};
                char error[1024] = {0};
                const auto rc = g_ui_invoke(
                    "__log__",
                    bridge_method.c_str(),
                    args_json.c_str(),
                    out_json,
                    static_cast<int>(sizeof(out_json)),
                    error,
                    static_cast<int>(sizeof(error)));
                if (rc != 0) {
                    throw std::runtime_error(error[0] != '\0' ? error : ("Unknown log method: " + method_));
                }
                return parse_json_value(out_json);
            }
            throw std::runtime_error("Unknown log method: " + method_);
        }

        if (receiver.kind == Value::Kind::Object && receiver.class_name == "os") {
            if (method_ == "now" && args.empty()) {
                const auto now = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
                const auto epoch_ms = now.time_since_epoch().count();
                return Value::Int(static_cast<std::int64_t>(epoch_ms));
            }

            if (method_ == "getEnv" && args.size() == 1) {
                const auto key = value_to_string(args[0]);
                if (key.empty()) {
                    return Value::Null();
                }
                const char* value = std::getenv(key.c_str());
                if (value == nullptr) {
                    return Value::Null();
                }
                return Value::String(value);
            }

            if (method_ == "fileExists" && args.size() == 1) {
                validate_and_authorize_sandbox_uri_path(value_to_string(args[0]), "os.fileExists");
            }

            if (g_ui_invoke == nullptr) {
                throw std::runtime_error("ui invoke bridge unavailable.");
            }

            std::string args_json = "[";
            for (std::size_t i = 0; i < args.size(); i++) {
                if (i > 0) {
                    args_json.push_back(',');
                }
                args_json += value_to_json(args[i]);
            }
            args_json.push_back(']');

            char out_json[kBridgeJsonBufferSize] = {0};
            char error[1024] = {0};
            const auto rc = g_ui_invoke(
                "__os__",
                method_.c_str(),
                args_json.c_str(),
                out_json,
                static_cast<int>(sizeof(out_json)),
                error,
                static_cast<int>(sizeof(error)));
            if (rc != 0) {
                throw std::runtime_error(error[0] != '\0' ? error : ("Unknown os method: " + method_));
            }

            return parse_json_value(out_json);
        }

        if (receiver.kind == Value::Kind::Object && receiver.class_name == "benchmark") {
            if (g_ui_invoke == nullptr) {
                throw std::runtime_error("ui invoke bridge unavailable.");
            }

            std::string args_json = "[";
            for (std::size_t i = 0; i < args.size(); i++) {
                if (i > 0) {
                    args_json.push_back(',');
                }
                args_json += value_to_json(args[i]);
            }
            args_json.push_back(']');

            char out_json[kBridgeJsonBufferSize] = {0};
            char error[1024] = {0};
            const auto rc = g_ui_invoke(
                "__benchmark__",
                method_.c_str(),
                args_json.c_str(),
                out_json,
                static_cast<int>(sizeof(out_json)),
                error,
                static_cast<int>(sizeof(error)));
            if (rc != 0) {
                throw std::runtime_error(error[0] != '\0' ? error : ("Unknown benchmark method: " + method_));
            }

            return parse_json_value(out_json);
        }

        if (receiver.kind == Value::Kind::Object && receiver.class_name == "fs") {
            if ((method_ == "list" || method_ == "readText" || method_ == "writeText") && !args.empty()) {
                const std::string owner = "fs." + method_;
                validate_and_authorize_sandbox_uri_path(value_to_string(args[0]), owner.c_str());
            }
            if (g_ui_invoke == nullptr) {
                throw std::runtime_error("ui invoke bridge unavailable.");
            }

            std::string args_json = "[";
            for (std::size_t i = 0; i < args.size(); i++) {
                if (i > 0) {
                    args_json.push_back(',');
                }
                args_json += value_to_json(args[i]);
            }
            args_json.push_back(']');

            char out_json[kBridgeJsonBufferSize] = {0};
            char error[1024] = {0};
            const auto rc = g_ui_invoke(
                "__fs__",
                method_.c_str(),
                args_json.c_str(),
                out_json,
                static_cast<int>(sizeof(out_json)),
                error,
                static_cast<int>(sizeof(error)));
            if (rc != 0) {
                throw std::runtime_error(error[0] != '\0' ? error : ("Unknown fs method: " + method_));
            }

            return parse_json_value(out_json);
        }

        if (receiver.kind == Value::Kind::Object && receiver.class_name == "i18n") {
            if (g_ui_invoke == nullptr) {
                throw std::runtime_error("ui invoke bridge unavailable.");
            }

            std::string args_json = "[";
            for (std::size_t i = 0; i < args.size(); i++) {
                if (i > 0) {
                    args_json.push_back(',');
                }
                args_json += value_to_json(args[i]);
            }
            args_json.push_back(']');

            char out_json[kBridgeJsonBufferSize] = {0};
            char error[1024] = {0};
            const auto rc = g_ui_invoke(
                "__i18n__",
                method_.c_str(),
                args_json.c_str(),
                out_json,
                static_cast<int>(sizeof(out_json)),
                error,
                static_cast<int>(sizeof(error)));
            if (rc != 0) {
                throw std::runtime_error(error[0] != '\0' ? error : ("Unknown i18n method: " + method_));
            }

            return parse_json_value(out_json);
        }

        if (receiver.kind == Value::Kind::Object && receiver.class_name == "ai") {
            if (g_ui_invoke == nullptr) {
                throw std::runtime_error("ui invoke bridge unavailable.");
            }

            std::string args_json = "[";
            for (std::size_t i = 0; i < args.size(); i++) {
                if (i > 0) {
                    args_json.push_back(',');
                }
                args_json += value_to_json(args[i]);
            }
            args_json.push_back(']');

            char out_json[kBridgeJsonBufferSize] = {0};
            char error[1024] = {0};
            const auto rc = g_ui_invoke(
                "__ai__",
                method_.c_str(),
                args_json.c_str(),
                out_json,
                static_cast<int>(sizeof(out_json)),
                error,
                static_cast<int>(sizeof(error)));
            if (rc != 0) {
                throw std::runtime_error(error[0] != '\0' ? error : ("Unknown ai method: " + method_));
            }

            return parse_json_value(out_json);
        }

        if (receiver.kind == Value::Kind::Object && receiver.class_name == "ui") {
            if (method_ == "getObject" && args.size() == 1) {
                if (g_ui_get_prop == nullptr) {
                    throw std::runtime_error("ui bridge unavailable.");
                }
                const auto id = value_to_string(args[0]);
                char out_json[256] = {0};
                char error[1024] = {0};
                const auto rc = g_ui_get_prop(
                    id.c_str(),
                    "__exists",
                    out_json,
                    static_cast<int>(sizeof(out_json)),
                    error,
                    static_cast<int>(sizeof(error)));
                if (rc != 0) {
                    throw std::runtime_error(error[0] != '\0' ? error : "ui exists probe failed");
                }
                const auto exists = parse_json_scalar(out_json).truthy();
                if (!exists) {
                    return Value::Null();
                }
                std::unordered_map<std::string, Value> fields;
                fields.emplace("id", Value::String(id));
                return Value::Object("__ui_ref", std::move(fields));
            }

            if (g_ui_invoke == nullptr) {
                throw std::runtime_error("ui invoke bridge unavailable.");
            }

            std::string args_json = "[";
            for (std::size_t i = 0; i < args.size(); i++) {
                if (i > 0) {
                    args_json.push_back(',');
                }
                args_json += value_to_json(args[i]);
            }
            args_json.push_back(']');

            char out_json[kBridgeJsonBufferSize] = {0};
            char error[1024] = {0};
            const auto rc = g_ui_invoke(
                "__ui__",
                method_.c_str(),
                args_json.c_str(),
                out_json,
                static_cast<int>(sizeof(out_json)),
                error,
                static_cast<int>(sizeof(error)));
            if (rc != 0) {
                throw std::runtime_error(error[0] != '\0' ? error : ("Unknown ui method: " + method_));
            }

            return parse_json_value(out_json);
        }

        if (receiver.kind == Value::Kind::Object
            && receiver.class_name == "__ui_ref"
            && receiver.object_fields) {
            const auto id_it = receiver.object_fields->find("id");
            if (id_it == receiver.object_fields->end() || id_it->second.kind != Value::Kind::String) {
                throw std::runtime_error("ui ref missing id.");
            }
            if (g_ui_invoke == nullptr) {
                throw std::runtime_error("ui invoke bridge unavailable.");
            }

            std::string args_json = "[";
            for (std::size_t i = 0; i < args.size(); i++) {
                if (i > 0) {
                    args_json.push_back(',');
                }
                args_json += value_to_json(args[i]);
            }
            args_json.push_back(']');

            char out_json[kBridgeJsonBufferSize] = {0};
            char error[1024] = {0};
            const auto rc = g_ui_invoke(
                id_it->second.string_value.c_str(),
                method_.c_str(),
                args_json.c_str(),
                out_json,
                static_cast<int>(sizeof(out_json)),
                error,
                static_cast<int>(sizeof(error)));
            if (rc != 0) {
                throw std::runtime_error(error[0] != '\0' ? error : "ui invoke failed");
            }

            return parse_json_value(out_json);
        }

        if (receiver.kind == Value::Kind::Array && receiver.array) {
            if (method_ == "add" && args.size() == 1) {
                receiver.array->push_back(args[0]);
                return Value::Null();
            }

            if (method_ == "remove" && args.size() == 1) {
                const auto it = std::find(receiver.array->begin(), receiver.array->end(), args[0]);
                if (it == receiver.array->end()) {
                    return Value::Int(0);
                }
                receiver.array->erase(it);
                return Value::Int(1);
            }

            if (method_ == "removeAt" && args.size() == 1) {
                const auto index = args[0].as_int("removeAt index");
                if (index < 0 || static_cast<std::size_t>(index) >= receiver.array->size()) {
                    return Value::Null();
                }
                auto value = (*receiver.array)[static_cast<std::size_t>(index)];
                receiver.array->erase(receiver.array->begin() + static_cast<std::ptrdiff_t>(index));
                return value;
            }

            if (method_ == "contains" && args.size() == 1) {
                const auto it = std::find(receiver.array->begin(), receiver.array->end(), args[0]);
                return Value::Int(it != receiver.array->end() ? 1 : 0);
            }

            throw std::runtime_error("Unknown array method: " + method_);
        }

        throw std::runtime_error("Method call expects supported receiver.");
    }

    std::unique_ptr<Expr> receiver_;
    std::string method_;
    std::vector<CallArgumentExpr> args_;
};

struct BinaryExpr final : Expr {
    BinaryExpr(TokenType op, std::unique_ptr<Expr> left, std::unique_ptr<Expr> right)
        : oper(op), left_(std::move(left)), right_(std::move(right)) {}

    Value eval(Env& env) const override {
        const auto left_value = left_->eval(env);
        const auto right_value = right_->eval(env);
        switch (oper) {
            case TokenType::Plus:
                if (left_value.kind == Value::Kind::String || right_value.kind == Value::Kind::String) {
                    return Value::String(value_to_string(left_value) + value_to_string(right_value));
                }
                return Value::Int(left_value.as_int("Binary expression") + right_value.as_int("Binary expression"));
            case TokenType::Dollar:
                return Value::String(value_to_string(left_value) + value_to_string(right_value));
            case TokenType::Minus:
                return Value::Int(left_value.as_int("Binary expression") - right_value.as_int("Binary expression"));
            case TokenType::Star:
                return Value::Int(left_value.as_int("Binary expression") * right_value.as_int("Binary expression"));
            case TokenType::Slash:
                if (right_value.as_int("Binary expression") == 0) {
                    throw SmsRuntimeError("division_by_zero", loc);
                }
                return Value::Int(left_value.as_int("Binary expression") / right_value.as_int("Binary expression"));
            case TokenType::Percent:
                if (right_value.as_int("Binary expression") == 0) {
                    throw SmsRuntimeError("modulo_by_zero", loc);
                }
                return Value::Int(left_value.as_int("Binary expression") % right_value.as_int("Binary expression"));
            case TokenType::Less:
                return Value::Int(left_value.as_int("Binary expression") < right_value.as_int("Binary expression") ? 1 : 0);
            case TokenType::LessEqual:
                return Value::Int(left_value.as_int("Binary expression") <= right_value.as_int("Binary expression") ? 1 : 0);
            case TokenType::Greater:
                return Value::Int(left_value.as_int("Binary expression") > right_value.as_int("Binary expression") ? 1 : 0);
            case TokenType::GreaterEqual:
                return Value::Int(left_value.as_int("Binary expression") >= right_value.as_int("Binary expression") ? 1 : 0);
            case TokenType::EqualEqual:
                return Value::Int(left_value == right_value ? 1 : 0);
            case TokenType::BangEqual:
                return Value::Int(left_value == right_value ? 0 : 1);
            default:
                throw std::runtime_error("Unsupported operator.");
        }
    }

    TokenType oper;
    std::unique_ptr<Expr> left_;
    std::unique_ptr<Expr> right_;
};

struct LogicalExpr final : Expr {
    LogicalExpr(TokenType op, std::unique_ptr<Expr> left, std::unique_ptr<Expr> right)
        : oper(op), left_(std::move(left)), right_(std::move(right)) {}

    Value eval(Env& env) const override {
        const auto left = left_->eval(env);
        const auto right = right_->eval(env);
        if (oper == TokenType::AndAnd) {
            return Value::Int(left.truthy() && right.truthy() ? 1 : 0);
        }

        if (oper == TokenType::OrOr) {
            return Value::Int(left.truthy() || right.truthy() ? 1 : 0);
        }

        throw std::runtime_error("Unsupported logical operator.");
    }

    TokenType oper;
    std::unique_ptr<Expr> left_;
    std::unique_ptr<Expr> right_;
};

struct UnaryExpr final : Expr {
    UnaryExpr(TokenType op, std::unique_ptr<Expr> operand)
        : oper(op), operand_(std::move(operand)) {}

    Value eval(Env& env) const override {
        const auto value = operand_->eval(env);
        if (oper == TokenType::Bang) {
            return Value::Int(value.truthy() ? 0 : 1);
        }
        if (oper == TokenType::Minus) {
            return Value::Int(-value.as_int("Unary '-'"));
        }
        throw std::runtime_error("Unsupported unary operator.");
    }

    TokenType oper;
    std::unique_ptr<Expr> operand_;
};

struct PostfixExpr final : Expr {
    PostfixExpr(TokenType op, std::unique_ptr<Expr> operand)
        : oper(op), operand_(std::move(operand)) {}

    Value eval(Env& env) const override {
        const auto* variable = dynamic_cast<const VarExpr*>(operand_.get());
        if (variable == nullptr) {
            throw std::runtime_error("Postfix operators only work on variables.");
        }

        auto current = env.get_var(variable->name);
        if (current.kind != Value::Kind::Int) {
            throw std::runtime_error("Postfix operators only work on integer variables.");
        }

        Value next = current;
        if (oper == TokenType::Increment) {
            next.int_value = next.int_value + 1;
        } else if (oper == TokenType::Decrement) {
            next.int_value = next.int_value - 1;
        } else {
            throw std::runtime_error("Unsupported postfix operator.");
        }

        if (!env.assign_var(variable->name, next)) {
            throw std::runtime_error("Assignment to unknown variable: " + variable->name);
        }

        // postfix returns old value
        return current;
    }

    TokenType oper;
    std::unique_ptr<Expr> operand_;
};

struct WhenBranch {
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Expr> result;
    bool is_else = false;
};

struct ParameterDecl {
    std::string name;
    std::string type_name;
    std::unique_ptr<Expr> default_value;
};

struct WhenExpr final : Expr {
    explicit WhenExpr(std::unique_ptr<Expr> subject, std::vector<WhenBranch> branches)
        : subject_(std::move(subject)), branches_(std::move(branches)) {}

    Value eval(Env& env) const override {
        const auto subject = subject_ ? subject_->eval(env) : Value::Null();
        const auto has_subject = static_cast<bool>(subject_);
        for (const auto& branch : branches_) {
            if (branch.is_else) {
                return branch.result->eval(env);
            }

            if (!branch.condition) {
                throw std::runtime_error("Invalid when branch.");
            }

            const auto condition = branch.condition->eval(env);
            const auto matches = has_subject ? (condition == subject) : condition.truthy();
            if (matches) {
                return branch.result->eval(env);
            }
        }
        return Value::Null();
    }

    std::unique_ptr<Expr> subject_;
    std::vector<WhenBranch> branches_;
};

struct Stmt {
    virtual ~Stmt() = default;
    virtual void execute(Env& env, Value& last) const = 0;
    SourceLoc loc;
};

static void execute_statements(const std::vector<std::unique_ptr<Stmt>>& body, Env& env, Value& last) {
    for (const auto& stmt : body) {
        stmt->execute(env, last);
    }
}

struct FunctionDeclStmt final : Stmt {
    std::string name;
    std::vector<ParameterDecl> params;
    std::string return_type;
    std::vector<std::unique_ptr<Stmt>> body;

    void execute(Env& env, Value&) const override {
        env.define_function(name, this);
    }
};

struct DataClassDeclStmt final : Stmt {
    std::string name;
    std::vector<ParameterDecl> fields;

    void execute(Env& env, Value&) const override {
        env.define_data_class(name, this);
    }
};

struct EventHandlerDeclStmt final : Stmt {
    std::string target_id;
    std::string event_name;
    std::vector<std::string> params;
    std::vector<std::unique_ptr<Stmt>> body;

    std::string key() const {
        return target_id + "." + event_name;
    }

    void execute(Env& env, Value&) const override {
        env.define_event_handler(key(), this);
    }
};

struct ImportDeclStmt final : Stmt {
    std::string path;
    std::string alias;

    void execute(Env&, Value&) const override {
        // Module linking is handled by higher-level runtime integration.
        // Native parser/runtime keeps import declarations as validated no-ops for now.
    }
};

struct CallExpr final : Expr {
    CallExpr(std::string n, std::vector<CallArgumentExpr> args)
        : name(std::move(n)), args_(std::move(args)) {}

    Value eval(Env& env) const override {
        if (name == "size") {
            if (args_.size() != 1) {
                throw std::runtime_error("size expects 1 argument.");
            }
            if (!args_[0].name.empty()) {
                throw std::runtime_error("Named arguments are not supported for size.");
            }
            auto value = args_[0].value->eval(env);
            if (value.kind != Value::Kind::Array || !value.array) {
                return Value::Int(0);
            }
            return Value::Int(static_cast<std::int64_t>(value.array->size()));
        }

        struct CallDepthGuard {
            Env& env_ref;
            bool armed = false;
            CallDepthGuard(Env& env, const std::string& context) : env_ref(env) {
                env_ref.enter_call(context);
                armed = true;
            }
            ~CallDepthGuard() {
                if (armed) {
                    env_ref.leave_call();
                }
            }
        } call_depth_guard(env, name);

        const auto* fn = env.get_function(name);
        if (fn == nullptr) {
            const auto* data_class = env.get_data_class(name);
            if (data_class == nullptr) {
                throw std::runtime_error("Unknown function: " + name);
            }

            std::unordered_map<std::string, Value> fields;
            fields.reserve(data_class->fields.size());
            std::vector<Value> bound_args(data_class->fields.size(), Value::Null());
            std::vector<bool> assigned(data_class->fields.size(), false);
            std::size_t next_positional = 0;

            for (const auto& arg : args_) {
                if (arg.name.empty()) {
                    while (next_positional < data_class->fields.size() && assigned[next_positional]) {
                        next_positional++;
                    }
                    if (next_positional >= data_class->fields.size()) {
                        throw std::runtime_error("Data class '" + name + "' expects at most "
                            + std::to_string(data_class->fields.size()) + " arg(s), got "
                            + std::to_string(args_.size()) + ".");
                    }
                    bound_args[next_positional] = arg.value->eval(env);
                    assigned[next_positional] = true;
                    next_positional++;
                    continue;
                }

                const auto it = std::find_if(data_class->fields.begin(), data_class->fields.end(),
                    [&](const ParameterDecl& field) { return field.name == arg.name; });
                if (it == data_class->fields.end()) {
                    throw std::runtime_error("Unknown named argument '" + arg.name + "' for data class '" + name + "'.");
                }

                const auto idx = static_cast<std::size_t>(std::distance(data_class->fields.begin(), it));
                if (assigned[idx]) {
                    throw std::runtime_error("Duplicate named argument '" + arg.name + "' for data class '" + name + "'.");
                }
                bound_args[idx] = arg.value->eval(env);
                assigned[idx] = true;
            }

            for (std::size_t i = 0; i < data_class->fields.size(); i++) {
                if (assigned[i]) {
                    fields[data_class->fields[i].name] = bound_args[i];
                    continue;
                }

                if (!data_class->fields[i].default_value) {
                    throw std::runtime_error("Missing required argument '" + data_class->fields[i].name
                        + "' for data class '" + name + "'.");
                }
                fields[data_class->fields[i].name] = data_class->fields[i].default_value->eval(env);
            }
            return Value::Object(data_class->name, std::move(fields));
        }

        Env local(&env);
        std::vector<Value> bound_args(fn->params.size(), Value::Null());
        std::vector<bool> assigned(fn->params.size(), false);
        std::size_t next_positional = 0;

        for (const auto& arg : args_) {
            if (arg.name.empty()) {
                while (next_positional < fn->params.size() && assigned[next_positional]) {
                    next_positional++;
                }
                if (next_positional >= fn->params.size()) {
                    throw std::runtime_error("Function '" + name + "' expects at most "
                        + std::to_string(fn->params.size()) + " arg(s), got "
                        + std::to_string(args_.size()) + ".");
                }
                bound_args[next_positional] = arg.value->eval(env);
                assigned[next_positional] = true;
                next_positional++;
                continue;
            }

            const auto it = std::find_if(fn->params.begin(), fn->params.end(),
                [&](const ParameterDecl& param) { return param.name == arg.name; });
            if (it == fn->params.end()) {
                throw std::runtime_error("Unknown named argument '" + arg.name + "' for function '" + name + "'.");
            }

            const auto idx = static_cast<std::size_t>(std::distance(fn->params.begin(), it));
            if (assigned[idx]) {
                throw std::runtime_error("Duplicate named argument '" + arg.name + "' for function '" + name + "'.");
            }
            bound_args[idx] = arg.value->eval(env);
            assigned[idx] = true;
        }

        for (std::size_t i = 0; i < fn->params.size(); i++) {
            if (!assigned[i]) {
                if (!fn->params[i].default_value) {
                    throw std::runtime_error("Missing required argument '" + fn->params[i].name
                        + "' for function '" + name + "'.");
                }
                bound_args[i] = fn->params[i].default_value->eval(local);
                assigned[i] = true;
            }
            local.define_var(fn->params[i].name, bound_args[i]);
        }

        Value last = Value::Null();
        try {
            execute_statements(fn->body, local, last);
            return last;
        } catch (const ReturnSignal& ret) {
            return ret.value;
        }
    }

    std::string name;
    std::vector<CallArgumentExpr> args_;
};

struct VarDeclStmt final : Stmt {
    VarDeclStmt(std::string n, std::unique_ptr<Expr> v) : name(std::move(n)), value(std::move(v)) {}
    void execute(Env& env, Value&) const override {
        env.define_var(name, value->eval(env));
    }

    std::string name;
    std::unique_ptr<Expr> value;
};

struct AssignStmt final : Stmt {
    AssignStmt(std::unique_ptr<Expr> target, std::unique_ptr<Expr> value)
        : target_expr(std::move(target)), value_expr(std::move(value)) {}

    void execute(Env& env, Value&) const override {
        const auto value = value_expr->eval(env);
        if (const auto* variable = dynamic_cast<const VarExpr*>(target_expr.get())) {
            const auto existing_kind = env.get_var_kind(variable->name);
            if (existing_kind.has_value() && value.kind != *existing_kind) {
                const auto expected = value_kind_name(static_cast<int>(*existing_kind));
                const auto got = value_kind_name(static_cast<int>(value.kind));
                std::string msg = "[SMS Compiler Error] type_mismatch\n";
                if (loc.line > 0) {
                    msg += "  file: <script>, line " + std::to_string(loc.line)
                         + ", col " + std::to_string(loc.col) + "\n";
                }
                msg += "  expected: " + expected + "\n";
                msg += "  got: " + got;
                const auto src = get_source_line(loc.line);
                if (!src.empty()) msg += "\n  > " + src;
                throw std::runtime_error(msg);
            }
            if (!env.assign_var(variable->name, value)) {
                throw std::runtime_error("Assignment to unknown variable: " + variable->name);
            }
            return;
        }

        if (const auto* member = dynamic_cast<const MemberAccessExpr*>(target_expr.get())) {
            auto receiver = member->receiver_->eval(env);
            if (receiver.kind == Value::Kind::Object
                && receiver.class_name == "__ui_ref"
                && receiver.object_fields) {
                const auto id_it = receiver.object_fields->find("id");
                if (id_it == receiver.object_fields->end() || id_it->second.kind != Value::Kind::String) {
                    throw std::runtime_error("ui ref missing id.");
                }
                if (g_ui_set_prop == nullptr) {
                    throw std::runtime_error("ui bridge unavailable.");
                }
                if (member->member_ == "text" && g_ui_set_string_prop != nullptr) {
                    char error[1024] = {0};
                    const auto text = value_to_string(value);
                    const auto string_rc = g_ui_set_string_prop(
                        id_it->second.string_value.c_str(),
                        member->member_.c_str(),
                        text.c_str(),
                        error,
                        static_cast<int>(sizeof(error)));
                    if (string_rc != 0) {
                        throw std::runtime_error(error[0] != '\0' ? error : "ui set string failed");
                    }
                    return;
                }
                char error[1024] = {0};
                const auto payload = value_to_json(value);
                const auto rc = g_ui_set_prop(
                    id_it->second.string_value.c_str(),
                    member->member_.c_str(),
                    payload.c_str(),
                    error,
                    static_cast<int>(sizeof(error)));
                if (rc != 0) {
                    throw std::runtime_error(error[0] != '\0' ? error : "ui set failed");
                }
                return;
            }
            if (receiver.kind != Value::Kind::Object || !receiver.object_fields) {
                throw std::runtime_error("Member assignment expects object receiver.");
            }
            (*receiver.object_fields)[member->member_] = value;
            return;
        }

        if (const auto* array_access = dynamic_cast<const ArrayAccessExpr*>(target_expr.get())) {
            auto receiver = array_access->receiver_->eval(env);
            if (receiver.kind != Value::Kind::Array || !receiver.array) {
                throw std::runtime_error("Array assignment expects array receiver.");
            }

            const auto idx = array_access->index_->eval(env).as_int("Array assignment index");
            if (idx < 0) {
                throw std::runtime_error("Array assignment index must be >= 0.");
            }

            const auto target = static_cast<std::size_t>(idx);
            if (target >= receiver.array->size()) {
                receiver.array->resize(target + 1, Value::Null());
            }

            (*receiver.array)[target] = value;
            return;
        }

        throw std::runtime_error("Invalid assignment target.");
    }

    std::unique_ptr<Expr> target_expr;
    std::unique_ptr<Expr> value_expr;
};

struct ReturnStmt final : Stmt {
    explicit ReturnStmt(std::unique_ptr<Expr> v) : value(std::move(v)) {}
    void execute(Env& env, Value&) const override {
        throw ReturnSignal(value ? value->eval(env) : Value::Null());
    }

    std::unique_ptr<Expr> value;
};

struct BreakStmt final : Stmt {
    void execute(Env&, Value&) const override {
        throw BreakSignal();
    }
};

struct ContinueStmt final : Stmt {
    void execute(Env&, Value&) const override {
        throw ContinueSignal();
    }
};

struct ExprStmt final : Stmt {
    explicit ExprStmt(std::unique_ptr<Expr> v) : value(std::move(v)) {}
    void execute(Env& env, Value& last) const override {
        last = value->eval(env);
    }

    std::unique_ptr<Expr> value;
};

struct ForStmt final : Stmt {
    std::unique_ptr<Stmt> init;
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> update;
    std::vector<std::unique_ptr<Stmt>> body;

    void execute(Env& env, Value& last) const override {
        init->execute(env, last);
        while (condition->eval(env).truthy()) {
            try {
                execute_statements(body, env, last);
            } catch (const ContinueSignal&) {
                update->execute(env, last);
                continue;
            } catch (const BreakSignal&) {
                break;
            }
            update->execute(env, last);
        }
    }
};

struct ForInStmt final : Stmt {
    std::string variable;
    std::unique_ptr<Expr> iterable;
    std::vector<std::unique_ptr<Stmt>> body;

    void execute(Env& env, Value& last) const override {
        auto value = iterable->eval(env);
        if (value.kind != Value::Kind::Array || !value.array) {
            throw std::runtime_error("for-in requires an array");
        }

        for (const auto& element : *value.array) {
            if (!env.assign_var(variable, element)) {
                env.define_var(variable, element);
            }
            try {
                execute_statements(body, env, last);
            } catch (const ContinueSignal&) {
                continue;
            } catch (const BreakSignal&) {
                break;
            }
        }
    }
};

struct WhileStmt final : Stmt {
    std::unique_ptr<Expr> condition;
    std::vector<std::unique_ptr<Stmt>> body;

    void execute(Env& env, Value& last) const override {
        while (condition->eval(env).truthy()) {
            try {
                execute_statements(body, env, last);
            } catch (const ContinueSignal&) {
                continue;
            } catch (const BreakSignal&) {
                break;
            }
        }
    }
};

struct IfStmt final : Stmt {
    std::unique_ptr<Expr> condition;
    std::vector<std::unique_ptr<Stmt>> then_body;
    std::vector<std::unique_ptr<Stmt>> else_body;

    void execute(Env& env, Value& last) const override {
        if (condition->eval(env).truthy()) {
            execute_statements(then_body, env, last);
        } else {
            execute_statements(else_body, env, last);
        }
    }
};


class Parser {
public:
    explicit Parser(std::vector<Token> tokens) : tokens_(std::move(tokens)) {}

    std::vector<std::unique_ptr<Stmt>> parse_program() {
        std::vector<std::unique_ptr<Stmt>> out;
        std::unordered_set<std::string> event_keys;
        while (!check(TokenType::Eof)) {
            auto stmt = parse_statement();
            if (const auto* handler = dynamic_cast<const EventHandlerDeclStmt*>(stmt.get())) {
                const auto key = handler->key();
                if (!event_keys.insert(key).second) {
                    throw std::runtime_error("Duplicate event handler '" + key + "'");
                }
            }
            out.push_back(std::move(stmt));
            match(TokenType::Semicolon);
        }
        return out;
    }

private:
    std::unique_ptr<Stmt> parse_statement() {
        if (match(TokenType::Import)) return parse_import_decl();
        if (match(TokenType::Export)) return parse_export_decl();
        if (match(TokenType::Var)) return parse_var_decl(false);
        if (match(TokenType::Data)) return parse_data_class_decl();
        if (match(TokenType::For)) return parse_for();
        if (match(TokenType::While)) return parse_while();
        if (match(TokenType::If)) return parse_if();
        if (match(TokenType::Fun)) return parse_function_decl();
        if (match(TokenType::On)) return parse_event_handler_decl();
        if (match(TokenType::Break)) return parse_break();
        if (match(TokenType::Continue)) return parse_continue();
        if (match(TokenType::Return)) return parse_return();

        if (check(TokenType::Identifier)) {
            const auto checkpoint = current_;
            try {
                return parse_assignment(false);
            } catch (const std::runtime_error&) {
                current_ = checkpoint;
            }
        }

        return parse_expr_stmt();
    }

    std::unique_ptr<Stmt> parse_var_decl(bool inside_for_clause) {
        const auto name = consume(TokenType::Identifier, "Expected variable name").text;
        if (match(TokenType::Colon)) {
            consume(TokenType::Identifier, "Expected type name after ':'");
        }
        consume(TokenType::Assign, "Expected '=' after variable name");
        auto value = parse_expression();
        if (!inside_for_clause) {
            match(TokenType::Semicolon);
        }
        return std::make_unique<VarDeclStmt>(name, std::move(value));
    }

    std::unique_ptr<Stmt> parse_data_class_decl() {
        consume(TokenType::Class, "Expected 'class' after 'data'");
        const auto name = consume(TokenType::Identifier, "Expected data class name").text;
        consume(TokenType::LeftParen, "Expected '(' after data class name");

        std::vector<ParameterDecl> fields;
        if (!check(TokenType::RightParen)) {
            do {
                fields.push_back(parse_parameter_decl());
            } while (match(TokenType::Comma));
        }

        consume(TokenType::RightParen, "Expected ')' after data class fields");
        match(TokenType::Semicolon);

        auto data = std::make_unique<DataClassDeclStmt>();
        data->name = name;
        data->fields = std::move(fields);
        return data;
    }

    std::unique_ptr<Stmt> parse_assignment(bool inside_for_clause) {
        auto target = parse_assignment_target();

        const auto eq_tok = consume(TokenType::Assign, "Expected '=' in assignment");
        auto value = parse_expression();
        if (!inside_for_clause) {
            match(TokenType::Semicolon);
        }
        auto stmt = std::make_unique<AssignStmt>(std::move(target), std::move(value));
        stmt->loc = {eq_tok.line, eq_tok.col};
        return stmt;
    }

    std::unique_ptr<Expr> parse_assignment_target() {
        std::unique_ptr<Expr> target =
            std::make_unique<VarExpr>(consume(TokenType::Identifier, "Expected assignment target").text);

        while (true) {
            if (match(TokenType::Dot)) {
                const auto member = consume(TokenType::Identifier, "Expected property name after '.'").text;
                if (match(TokenType::LeftParen)) {
                    auto args = parse_arguments();
                    target = std::make_unique<MethodCallExpr>(std::move(target), member, std::move(args));
                } else {
                    target = std::make_unique<MemberAccessExpr>(std::move(target), member);
                }
                continue;
            }

            if (match(TokenType::LeftBracket)) {
                auto index = parse_expression();
                consume(TokenType::RightBracket, "Expected ']' after assignment index");
                target = std::make_unique<ArrayAccessExpr>(std::move(target), std::move(index));
                continue;
            }

            break;
        }

        if (!dynamic_cast<VarExpr*>(target.get())
            && !dynamic_cast<MemberAccessExpr*>(target.get())
            && !dynamic_cast<ArrayAccessExpr*>(target.get())) {
            throw std::runtime_error("Invalid assignment target.");
        }

        return target;
    }

    std::unique_ptr<Stmt> parse_expr_stmt() {
        auto value = parse_expression();
        match(TokenType::Semicolon);
        return std::make_unique<ExprStmt>(std::move(value));
    }

    std::unique_ptr<Stmt> parse_return() {
        std::unique_ptr<Expr> value;
        if (!check(TokenType::Semicolon)
            && !check(TokenType::RightBrace)
            && !check(TokenType::Eof)) {
            value = parse_expression();
        }
        match(TokenType::Semicolon);
        return std::make_unique<ReturnStmt>(std::move(value));
    }

    std::unique_ptr<Stmt> parse_break() {
        match(TokenType::Semicolon);
        return std::make_unique<BreakStmt>();
    }

    std::unique_ptr<Stmt> parse_continue() {
        match(TokenType::Semicolon);
        return std::make_unique<ContinueStmt>();
    }

    std::unique_ptr<Stmt> parse_function_decl() {
        const auto name = consume(TokenType::Identifier, "Expected function name").text;
        consume(TokenType::LeftParen, "Expected '(' after function name");

        std::vector<ParameterDecl> params;
        if (!check(TokenType::RightParen)) {
            do {
                params.push_back(parse_parameter_decl());
            } while (match(TokenType::Comma));
        }

        consume(TokenType::RightParen, "Expected ')' after function parameters");
        std::string return_type;
        if (match(TokenType::Colon)) {
            if (match(TokenType::LeftParen)) {
                // Tuple return type: (Type1, Type2, Type3)
                std::string tuple_type = "(";
                bool first_type = true;
                while (!check(TokenType::RightParen) && !check(TokenType::Eof)) {
                    if (!first_type) tuple_type += ", ";
                    first_type = false;
                    tuple_type += consume(TokenType::Identifier, "Expected type name in tuple return type").text;
                    if (!check(TokenType::RightParen)) {
                        consume(TokenType::Comma, "Expected ',' between tuple return types");
                    }
                }
                consume(TokenType::RightParen, "Expected ')' after tuple return types");
                tuple_type += ")";
                return_type = std::move(tuple_type);
            } else {
                return_type = consume(TokenType::Identifier, "Expected return type name").text;
            }
        }
        auto body = parse_block();

        auto fn = std::make_unique<FunctionDeclStmt>();
        fn->name = name;
        fn->params = std::move(params);
        fn->return_type = std::move(return_type);
        fn->body = std::move(body);
        return fn;
    }

    std::unique_ptr<Stmt> parse_event_handler_decl() {
        const auto target_id = consume(TokenType::Identifier, "Expected target id after on").text;
        consume(TokenType::Dot, "Expected '.' after event target id");
        const auto event_name = consume(TokenType::Identifier, "Expected event name after '.'").text;
        consume(TokenType::LeftParen, "Expected '(' after event name");

        std::vector<std::string> params;
        if (!check(TokenType::RightParen)) {
            do {
                auto param = parse_parameter_decl();
                params.push_back(std::move(param.name));
            } while (match(TokenType::Comma));
        }
        consume(TokenType::RightParen, "Expected ')' after event parameters");
        auto body = parse_block();

        auto handler = std::make_unique<EventHandlerDeclStmt>();
        handler->target_id = target_id;
        handler->event_name = event_name;
        handler->params = std::move(params);
        handler->body = std::move(body);
        return handler;
    }

    std::unique_ptr<Stmt> parse_for() {
        consume(TokenType::LeftParen, "Expected '(' after for");

        if (check(TokenType::Identifier)) {
            const auto checkpoint = current_;
            const auto variable = advance().text;
            if (match(TokenType::Colon)) {
                consume(TokenType::Identifier, "Expected type name after ':' in for-in variable");
            }
            if (match(TokenType::In)) {
                auto iterable = parse_expression();
                consume(TokenType::RightParen, "Expected ')' after for-in");
                auto body = parse_block();

                auto stmt = std::make_unique<ForInStmt>();
                stmt->variable = variable;
                stmt->iterable = std::move(iterable);
                stmt->body = std::move(body);
                return stmt;
            }
            current_ = checkpoint;
        }

        std::unique_ptr<Stmt> init;
        if (match(TokenType::Var)) {
            init = parse_var_decl(true);
        } else {
            init = parse_assignment(true);
        }

        consume(TokenType::Semicolon, "Expected ';' after for init");
        auto condition = parse_expression();
        consume(TokenType::Semicolon, "Expected ';' after for condition");
        auto update = parse_assignment(true);
        consume(TokenType::RightParen, "Expected ')' after for clauses");

        auto body = parse_block();

        auto for_stmt = std::make_unique<ForStmt>();
        for_stmt->init = std::move(init);
        for_stmt->condition = std::move(condition);
        for_stmt->update = std::move(update);
        for_stmt->body = std::move(body);
        return for_stmt;
    }

    std::unique_ptr<Stmt> parse_while() {
        consume(TokenType::LeftParen, "Expected '(' after while");
        auto condition = parse_expression();
        consume(TokenType::RightParen, "Expected ')' after while condition");
        auto body = parse_block();

        auto while_stmt = std::make_unique<WhileStmt>();
        while_stmt->condition = std::move(condition);
        while_stmt->body = std::move(body);
        return while_stmt;
    }

    std::unique_ptr<Stmt> parse_if() {
        consume(TokenType::LeftParen, "Expected '(' after if");
        auto condition = parse_expression();
        consume(TokenType::RightParen, "Expected ')' after if condition");

        auto then_body = parse_block();
        std::vector<std::unique_ptr<Stmt>> else_body;
        if (match(TokenType::Else)) {
            if (match(TokenType::If)) {
                // Preserve else-if semantics by nesting the chained if in else_body.
                else_body.push_back(parse_if());
            } else {
                else_body = parse_block();
            }
        }

        auto if_stmt = std::make_unique<IfStmt>();
        if_stmt->condition = std::move(condition);
        if_stmt->then_body = std::move(then_body);
        if_stmt->else_body = std::move(else_body);
        return if_stmt;
    }

    std::vector<std::unique_ptr<Stmt>> parse_block() {
        consume(TokenType::LeftBrace, "Expected '{' before block");
        std::vector<std::unique_ptr<Stmt>> body;
        while (!check(TokenType::RightBrace) && !check(TokenType::Eof)) {
            body.push_back(parse_statement());
            match(TokenType::Semicolon);
        }
        consume(TokenType::RightBrace, "Expected '}' after block");
        return body;
    }

    std::unique_ptr<Expr> parse_expression() { return parse_or(); }

    std::unique_ptr<Expr> parse_or() {
        auto expr = parse_and();
        while (match(TokenType::OrOr)) {
            auto rhs = parse_and();
            expr = std::make_unique<LogicalExpr>(TokenType::OrOr, std::move(expr), std::move(rhs));
        }
        return expr;
    }

    std::unique_ptr<Expr> parse_and() {
        auto expr = parse_equality();
        while (match(TokenType::AndAnd)) {
            auto rhs = parse_equality();
            expr = std::make_unique<LogicalExpr>(TokenType::AndAnd, std::move(expr), std::move(rhs));
        }
        return expr;
    }

    std::unique_ptr<Expr> parse_equality() {
        auto expr = parse_comparison();
        while (true) {
            if (match(TokenType::EqualEqual)) {
                auto rhs = parse_comparison();
                expr = std::make_unique<BinaryExpr>(TokenType::EqualEqual, std::move(expr), std::move(rhs));
                continue;
            }
            if (match(TokenType::BangEqual)) {
                auto rhs = parse_comparison();
                expr = std::make_unique<BinaryExpr>(TokenType::BangEqual, std::move(expr), std::move(rhs));
                continue;
            }
            break;
        }
        return expr;
    }

    std::unique_ptr<Expr> parse_comparison() {
        auto expr = parse_term();
        while (true) {
            if (match(TokenType::Less)) {
                auto rhs = parse_term();
                expr = std::make_unique<BinaryExpr>(TokenType::Less, std::move(expr), std::move(rhs));
                continue;
            }
            if (match(TokenType::LessEqual)) {
                auto rhs = parse_term();
                expr = std::make_unique<BinaryExpr>(TokenType::LessEqual, std::move(expr), std::move(rhs));
                continue;
            }
            if (match(TokenType::Greater)) {
                auto rhs = parse_term();
                expr = std::make_unique<BinaryExpr>(TokenType::Greater, std::move(expr), std::move(rhs));
                continue;
            }
            if (match(TokenType::GreaterEqual)) {
                auto rhs = parse_term();
                expr = std::make_unique<BinaryExpr>(TokenType::GreaterEqual, std::move(expr), std::move(rhs));
                continue;
            }
            break;
        }
        return expr;
    }

    std::unique_ptr<Expr> parse_term() {
        auto expr = parse_factor();
        while (true) {
            if (match(TokenType::Plus)) {
                auto rhs = parse_factor();
                expr = std::make_unique<BinaryExpr>(TokenType::Plus, std::move(expr), std::move(rhs));
                continue;
            }
            if (match(TokenType::Dollar)) {
                auto rhs = parse_factor();
                expr = std::make_unique<BinaryExpr>(TokenType::Dollar, std::move(expr), std::move(rhs));
                continue;
            }
            if (match(TokenType::Minus)) {
                auto rhs = parse_factor();
                expr = std::make_unique<BinaryExpr>(TokenType::Minus, std::move(expr), std::move(rhs));
                continue;
            }
            break;
        }
        return expr;
    }

    std::unique_ptr<Expr> parse_factor() {
        auto expr = parse_unary();
        while (true) {
            if (match(TokenType::Star)) {
                auto rhs = parse_unary();
                expr = std::make_unique<BinaryExpr>(TokenType::Star, std::move(expr), std::move(rhs));
                continue;
            }
            if (match(TokenType::Slash)) {
                const auto op_loc = SourceLoc{previous().line, previous().col};
                auto rhs = parse_unary();
                auto node = std::make_unique<BinaryExpr>(TokenType::Slash, std::move(expr), std::move(rhs));
                node->loc = op_loc;
                expr = std::move(node);
                continue;
            }
            if (match(TokenType::Percent)) {
                const auto op_loc = SourceLoc{previous().line, previous().col};
                auto rhs = parse_unary();
                auto node = std::make_unique<BinaryExpr>(TokenType::Percent, std::move(expr), std::move(rhs));
                node->loc = op_loc;
                expr = std::move(node);
                continue;
            }
            break;
        }
        return expr;
    }

    std::unique_ptr<Expr> parse_unary() {
        if (match(TokenType::Bang)) {
            return std::make_unique<UnaryExpr>(TokenType::Bang, parse_unary());
        }
        if (match(TokenType::Minus)) {
            return std::make_unique<UnaryExpr>(TokenType::Minus, parse_unary());
        }
        return parse_postfix();
    }

    std::unique_ptr<Expr> parse_postfix() {
        auto expr = parse_primary();

        while (true) {
            if (match(TokenType::LeftParen)) {
                auto args = parse_arguments();
                if (auto* var = dynamic_cast<VarExpr*>(expr.get())) {
                    expr = std::make_unique<CallExpr>(var->name, std::move(args));
                } else {
                    throw std::runtime_error("Invalid call target.");
                }
                continue;
            }

            if (match(TokenType::Dot)) {
                const auto member = consume(TokenType::Identifier, "Expected property name after '.'").text;
                if (match(TokenType::LeftParen)) {
                    auto args = parse_arguments();
                    expr = std::make_unique<MethodCallExpr>(std::move(expr), member, std::move(args));
                } else {
                    expr = std::make_unique<MemberAccessExpr>(std::move(expr), member);
                }
                continue;
            }

            if (match(TokenType::LeftBracket)) {
                const auto bracket_loc = SourceLoc{previous().line, previous().col};
                auto index = parse_expression();
                consume(TokenType::RightBracket, "Expected ']' after array index");
                auto node = std::make_unique<ArrayAccessExpr>(std::move(expr), std::move(index));
                node->loc = bracket_loc;
                expr = std::move(node);
                continue;
            }

            break;
        }

        if (match(TokenType::Increment)) {
            return std::make_unique<PostfixExpr>(TokenType::Increment, std::move(expr));
        }
        if (match(TokenType::Decrement)) {
            return std::make_unique<PostfixExpr>(TokenType::Decrement, std::move(expr));
        }

        return expr;
    }

    std::vector<CallArgumentExpr> parse_arguments() {
        std::vector<CallArgumentExpr> args;
        bool named_args_started = false;
        if (!check(TokenType::RightParen)) {
            do {
                if (check(TokenType::Identifier) && check_next(TokenType::Assign)) {
                    named_args_started = true;
                    auto name = consume(TokenType::Identifier, "Expected argument name").text;
                    consume(TokenType::Assign, "Expected '=' after named argument name");
                    CallArgumentExpr arg;
                    arg.name = std::move(name);
                    arg.value = parse_expression();
                    args.push_back(std::move(arg));
                } else {
                    if (named_args_started) {
                        throw std::runtime_error("Positional argument is not allowed after named argument.");
                    }
                    CallArgumentExpr arg;
                    arg.value = parse_expression();
                    args.push_back(std::move(arg));
                }
            } while (match(TokenType::Comma));
        }
        consume(TokenType::RightParen, "Expected ')' after arguments");
        return args;
    }

    std::unique_ptr<Stmt> parse_import_decl() {
        const auto path = consume(TokenType::String, "Expected import path string").text;
        validate_import_path(path);
        std::string alias;
        if (match(TokenType::As)) {
            alias = consume(TokenType::Identifier, "Expected alias after 'as'").text;
        }

        auto stmt = std::make_unique<ImportDeclStmt>();
        stmt->path = path;
        stmt->alias = alias;
        return stmt;
    }

    std::unique_ptr<Stmt> parse_export_decl() {
        if (match(TokenType::Fun)) {
            return parse_function_decl();
        }
        if (match(TokenType::Var)) {
            return parse_var_decl(false);
        }
        throw std::runtime_error("Expected 'fun' or 'var' after 'export'.");
    }

    ParameterDecl parse_parameter_decl() {
        ParameterDecl param;
        param.name = consume(TokenType::Identifier, "Expected parameter name").text;
        if (match(TokenType::Colon)) {
            param.type_name = consume(TokenType::Identifier, "Expected type name after ':'").text;
        }
        if (match(TokenType::Assign)) {
            param.default_value = parse_expression();
        }
        return param;
    }

    void validate_import_path(const std::string& path) const {
        if (path.find("//") != std::string::npos) {
            throw std::runtime_error("Import paths must not contain '//'.");
        }
        if (path.rfind("./", 0) == 0 || path.rfind("../", 0) == 0) {
            throw std::runtime_error("Relative import paths are not allowed.");
        }
        if (path.rfind("res:/", 0) != 0
            && path.rfind("appRes:/", 0) != 0
            && path.rfind("user:/", 0) != 0) {
            throw std::runtime_error("Import path must start with res:/, appRes:/, or user:/");
        }
        const auto slash = path.find('/');
        if (slash != std::string::npos && has_parent_traversal_segment(path.substr(slash + 1))) {
            throw std::runtime_error("Import path must not contain '..' segments.");
        }
    }

    std::unique_ptr<Expr> parse_primary() {
        if (match(TokenType::When)) {
            return parse_when_expression();
        }

        if (match(TokenType::String)) {
            return parse_string_literal_expr(previous().text);
        }

        if (match(TokenType::True)) {
            return std::make_unique<BoolExpr>(true);
        }

        if (match(TokenType::False)) {
            return std::make_unique<BoolExpr>(false);
        }

        if (match(TokenType::Null)) {
            return std::make_unique<NullExpr>();
        }

        if (match(TokenType::Number)) {
            return std::make_unique<NumberExpr>(std::stoll(previous().text));
        }

        if (match(TokenType::Identifier)) {
            return std::make_unique<VarExpr>(previous().text);
        }

        if (match(TokenType::LeftParen)) {
            auto first = parse_expression();
            if (match(TokenType::Comma)) {
                // Tuple literal: (expr, expr, ...)
                std::vector<std::unique_ptr<Expr>> elements;
                elements.push_back(std::move(first));
                do {
                    elements.push_back(parse_expression());
                } while (match(TokenType::Comma));
                consume(TokenType::RightParen, "Expected ')' after tuple elements");
                return std::make_unique<TupleLiteralExpr>(std::move(elements));
            }
            consume(TokenType::RightParen, "Expected ')' after expression");
            return first;
        }

        if (match(TokenType::LeftBracket)) {
            std::vector<std::unique_ptr<Expr>> elements;
            if (!check(TokenType::RightBracket)) {
                do {
                    elements.push_back(parse_expression());
                } while (match(TokenType::Comma));
            }
            consume(TokenType::RightBracket, "Expected ']' after array elements");
            return std::make_unique<ArrayLiteralExpr>(std::move(elements));
        }

        throw std::runtime_error("Expected expression.");
    }

    std::unique_ptr<Expr> parse_when_expression() {
        std::unique_ptr<Expr> subject;
        if (match(TokenType::LeftParen)) {
            subject = parse_expression();
            consume(TokenType::RightParen, "Expected ')' after when subject");
        }

        consume(TokenType::LeftBrace, "Expected '{' after when");
        std::vector<WhenBranch> branches;
        bool else_seen = false;
        while (!check(TokenType::RightBrace) && !check(TokenType::Eof)) {
            WhenBranch branch;
            if (match(TokenType::Else)) {
                if (else_seen) {
                    throw std::runtime_error("Multiple else branches in when.");
                }
                else_seen = true;
                branch.is_else = true;
            } else {
                branch.condition = parse_expression();
            }

            consume(TokenType::Arrow, "Expected '->' after when branch condition");
            branch.result = parse_expression();
            branches.push_back(std::move(branch));
            match(TokenType::Semicolon);
        }
        consume(TokenType::RightBrace, "Expected '}' after when branches");
        return std::make_unique<WhenExpr>(std::move(subject), std::move(branches));
    }

    std::unique_ptr<Expr> parse_embedded_expression(const std::string& expr_source) {
        Lexer embedded_lexer(expr_source);
        auto embedded_tokens = embedded_lexer.tokenize();
        Parser embedded_parser(std::move(embedded_tokens));
        auto expr = embedded_parser.parse_expression();
        if (!embedded_parser.check(TokenType::Eof)) {
            throw std::runtime_error("String interpolation expression must end at '}'.");
        }
        return expr;
    }

    std::unique_ptr<Expr> parse_string_literal_expr(const std::string& text) {
        if (text.find("${") == std::string::npos) {
            return std::make_unique<StringExpr>(text);
        }

        std::vector<InterpolatedStringExpr::Part> parts;
        std::size_t i = 0;
        std::size_t literal_start = 0;
        while (i < text.size()) {
            if (i + 1 < text.size() && text[i] == '$' && text[i + 1] == '{') {
                if (i > literal_start) {
                    InterpolatedStringExpr::Part literal_part;
                    literal_part.literal = text.substr(literal_start, i - literal_start);
                    parts.push_back(std::move(literal_part));
                }

                i += 2; // skip ${
                const std::size_t expr_start = i;
                int brace_depth = 1;
                bool in_string = false;
                bool escaped = false;
                while (i < text.size()) {
                    const char ch = text[i];
                    if (in_string) {
                        if (escaped) {
                            escaped = false;
                        } else if (ch == '\\') {
                            escaped = true;
                        } else if (ch == '"') {
                            in_string = false;
                        }
                        i++;
                        continue;
                    }

                    if (ch == '"') {
                        in_string = true;
                        i++;
                        continue;
                    }
                    if (ch == '{') {
                        brace_depth++;
                        i++;
                        continue;
                    }
                    if (ch == '}') {
                        brace_depth--;
                        if (brace_depth == 0) {
                            break;
                        }
                        i++;
                        continue;
                    }
                    i++;
                }

                if (i >= text.size() || text[i] != '}') {
                    throw std::runtime_error("Unterminated string interpolation '${...}'.");
                }

                auto expr_text = text.substr(expr_start, i - expr_start);
                auto trim = [](const std::string& in) {
                    std::size_t start = 0;
                    while (start < in.size() && std::isspace(static_cast<unsigned char>(in[start])) != 0) {
                        start++;
                    }
                    std::size_t end = in.size();
                    while (end > start && std::isspace(static_cast<unsigned char>(in[end - 1])) != 0) {
                        end--;
                    }
                    return in.substr(start, end - start);
                };
                expr_text = trim(expr_text);
                if (expr_text.empty()) {
                    throw std::runtime_error("Empty expression in string interpolation.");
                }

                InterpolatedStringExpr::Part expr_part;
                expr_part.expr = parse_embedded_expression(expr_text);
                parts.push_back(std::move(expr_part));

                i++; // skip closing }
                literal_start = i;
                continue;
            }
            i++;
        }

        if (literal_start < text.size()) {
            InterpolatedStringExpr::Part tail_literal;
            tail_literal.literal = text.substr(literal_start);
            parts.push_back(std::move(tail_literal));
        }

        return std::make_unique<InterpolatedStringExpr>(std::move(parts));
    }

    bool match(TokenType type) {
        if (!check(type)) {
            return false;
        }
        advance();
        return true;
    }

    bool check(TokenType type) const {
        if (is_at_end()) {
            return type == TokenType::Eof;
        }
        return tokens_[current_].type == type;
    }

    bool check_next(TokenType type) const {
        if (current_ + 1 >= tokens_.size()) {
            return false;
        }
        return tokens_[current_ + 1].type == type;
    }

    const Token& advance() {
        if (!is_at_end()) {
            current_++;
        }
        return previous();
    }

    bool is_at_end() const { return tokens_[current_].type == TokenType::Eof; }
    const Token& previous() const { return tokens_[current_ - 1]; }

    Token consume(TokenType type, const char* message) {
        if (check(type)) {
            return advance();
        }
        throw std::runtime_error(message);
    }

    std::vector<Token> tokens_;
    std::size_t current_ = 0;
};

void write_error(char* error, int capacity, const std::string& message) {
    if (error == nullptr || capacity <= 0) {
        return;
    }

    const auto count = static_cast<int>(std::min<std::size_t>(message.size(), static_cast<std::size_t>(capacity - 1)));
    std::memcpy(error, message.data(), static_cast<std::size_t>(count));
    error[count] = '\0';
}

inline std::uint32_t rotr32(std::uint32_t x, std::uint32_t n) {
    return (x >> n) | (x << (32u - n));
}

std::string sha256_hex(const std::string& text) {
    static constexpr std::array<std::uint32_t, 64> k = {{
        0x428a2f98u, 0x71374491u, 0xb5c0fbcfu, 0xe9b5dba5u, 0x3956c25bu, 0x59f111f1u, 0x923f82a4u, 0xab1c5ed5u,
        0xd807aa98u, 0x12835b01u, 0x243185beu, 0x550c7dc3u, 0x72be5d74u, 0x80deb1feu, 0x9bdc06a7u, 0xc19bf174u,
        0xe49b69c1u, 0xefbe4786u, 0x0fc19dc6u, 0x240ca1ccu, 0x2de92c6fu, 0x4a7484aau, 0x5cb0a9dcu, 0x76f988dau,
        0x983e5152u, 0xa831c66du, 0xb00327c8u, 0xbf597fc7u, 0xc6e00bf3u, 0xd5a79147u, 0x06ca6351u, 0x14292967u,
        0x27b70a85u, 0x2e1b2138u, 0x4d2c6dfcu, 0x53380d13u, 0x650a7354u, 0x766a0abbu, 0x81c2c92eu, 0x92722c85u,
        0xa2bfe8a1u, 0xa81a664bu, 0xc24b8b70u, 0xc76c51a3u, 0xd192e819u, 0xd6990624u, 0xf40e3585u, 0x106aa070u,
        0x19a4c116u, 0x1e376c08u, 0x2748774cu, 0x34b0bcb5u, 0x391c0cb3u, 0x4ed8aa4au, 0x5b9cca4fu, 0x682e6ff3u,
        0x748f82eeu, 0x78a5636fu, 0x84c87814u, 0x8cc70208u, 0x90befffau, 0xa4506cebu, 0xbef9a3f7u, 0xc67178f2u
    }};

    std::array<std::uint32_t, 8> h = {{
        0x6a09e667u, 0xbb67ae85u, 0x3c6ef372u, 0xa54ff53au,
        0x510e527fu, 0x9b05688cu, 0x1f83d9abu, 0x5be0cd19u
    }};

    std::vector<std::uint8_t> msg(text.begin(), text.end());
    const std::uint64_t bit_len = static_cast<std::uint64_t>(msg.size()) * 8ull;
    msg.push_back(0x80u);
    while ((msg.size() % 64u) != 56u) {
        msg.push_back(0x00u);
    }
    for (int i = 7; i >= 0; --i) {
        msg.push_back(static_cast<std::uint8_t>((bit_len >> (i * 8)) & 0xffu));
    }

    std::array<std::uint32_t, 64> w{};
    for (std::size_t offset = 0; offset < msg.size(); offset += 64) {
        for (int i = 0; i < 16; ++i) {
            const std::size_t base = offset + static_cast<std::size_t>(i) * 4u;
            w[static_cast<std::size_t>(i)] =
                (static_cast<std::uint32_t>(msg[base]) << 24u) |
                (static_cast<std::uint32_t>(msg[base + 1]) << 16u) |
                (static_cast<std::uint32_t>(msg[base + 2]) << 8u) |
                static_cast<std::uint32_t>(msg[base + 3]);
        }
        for (int i = 16; i < 64; ++i) {
            const std::uint32_t s0 = rotr32(w[static_cast<std::size_t>(i - 15)], 7u) ^
                                     rotr32(w[static_cast<std::size_t>(i - 15)], 18u) ^
                                     (w[static_cast<std::size_t>(i - 15)] >> 3u);
            const std::uint32_t s1 = rotr32(w[static_cast<std::size_t>(i - 2)], 17u) ^
                                     rotr32(w[static_cast<std::size_t>(i - 2)], 19u) ^
                                     (w[static_cast<std::size_t>(i - 2)] >> 10u);
            w[static_cast<std::size_t>(i)] =
                w[static_cast<std::size_t>(i - 16)] + s0 + w[static_cast<std::size_t>(i - 7)] + s1;
        }

        std::uint32_t a = h[0], b = h[1], c = h[2], d = h[3];
        std::uint32_t e = h[4], f = h[5], g = h[6], hv = h[7];
        for (int i = 0; i < 64; ++i) {
            const std::uint32_t s1 = rotr32(e, 6u) ^ rotr32(e, 11u) ^ rotr32(e, 25u);
            const std::uint32_t ch = (e & f) ^ ((~e) & g);
            const std::uint32_t temp1 = hv + s1 + ch + k[static_cast<std::size_t>(i)] + w[static_cast<std::size_t>(i)];
            const std::uint32_t s0 = rotr32(a, 2u) ^ rotr32(a, 13u) ^ rotr32(a, 22u);
            const std::uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
            const std::uint32_t temp2 = s0 + maj;

            hv = g;
            g = f;
            f = e;
            e = d + temp1;
            d = c;
            c = b;
            b = a;
            a = temp1 + temp2;
        }

        h[0] += a; h[1] += b; h[2] += c; h[3] += d;
        h[4] += e; h[5] += f; h[6] += g; h[7] += hv;
    }

    std::ostringstream out;
    out << std::hex << std::nouppercase;
    for (std::uint32_t word : h) {
        for (int shift = 24; shift >= 0; shift -= 8) {
            const auto byte = static_cast<unsigned>((word >> shift) & 0xffu);
            if (byte < 16u) {
                out << '0';
            }
            out << byte;
        }
    }
    return out.str();
}

std::string shell_quote_arg(const std::string& arg) {
    std::string out = "\"";
    for (char ch : arg) {
        if (ch == '"' || ch == '\\') {
            out.push_back('\\');
        }
        out.push_back(ch);
    }
    out.push_back('"');
    return out;
}

std::string capture_command_output(const std::string& command, int* out_exit_code) {
    if (out_exit_code != nullptr) {
        *out_exit_code = -1;
    }
    std::string output;
#if defined(_WIN32)
    FILE* pipe = _popen(command.c_str(), "r");
#else
    FILE* pipe = popen(command.c_str(), "r");
#endif
    if (pipe == nullptr) {
        return output;
    }

    char buffer[1024];
    while (std::fgets(buffer, static_cast<int>(sizeof(buffer)), pipe) != nullptr) {
        output += buffer;
    }

#if defined(_WIN32)
    const int rc = _pclose(pipe);
    if (out_exit_code != nullptr) {
        *out_exit_code = rc;
    }
#else
    const int rc = pclose(pipe);
    if (out_exit_code != nullptr) {
        if (WIFEXITED(rc)) {
            *out_exit_code = WEXITSTATUS(rc);
        } else {
            *out_exit_code = rc;
        }
    }
#endif
    return output;
}

std::string resolve_sms_aot_cache_dir() {
    const char* env_override = std::getenv("SMS_NATIVE_AOT_CACHE_DIR");
    if (env_override != nullptr && env_override[0] != '\0') {
        return std::string(env_override);
    }
#if defined(_WIN32)
    const char* local_app_data = std::getenv("LOCALAPPDATA");
    if (local_app_data != nullptr && local_app_data[0] != '\0') {
        return (fs::path(local_app_data) / "forge-runner" / "sms_aot").string();
    }
#endif
    const char* home = std::getenv("HOME");
    if (home != nullptr && home[0] != '\0') {
        return (fs::path(home) / ".cache" / "forge-runner" / "sms_aot").string();
    }
    return (fs::temp_directory_path() / "forge-runner" / "sms_aot").string();
}

std::string detect_clang_command() {
    const char* env_clang = std::getenv("SMS_NATIVE_CLANG");
    if (env_clang != nullptr && env_clang[0] != '\0') {
        return std::string(env_clang);
    }
    return "clang";
}

std::string shared_lib_extension() {
#if defined(_WIN32)
    return ".dll";
#elif defined(__APPLE__)
    return ".dylib";
#else
    return ".so";
#endif
}

void* load_shared_lib(const std::string& path) {
#if defined(_WIN32)
    return reinterpret_cast<void*>(LoadLibraryA(path.c_str()));
#else
    return dlopen(path.c_str(), RTLD_NOW);
#endif
}

void* load_shared_symbol(void* handle, const char* name) {
#if defined(_WIN32)
    if (handle == nullptr) return nullptr;
    return reinterpret_cast<void*>(GetProcAddress(reinterpret_cast<HMODULE>(handle), name));
#else
    return handle != nullptr ? dlsym(handle, name) : nullptr;
#endif
}

void close_shared_lib(void* handle) {
    if (handle == nullptr) {
        return;
    }
#if defined(_WIN32)
    FreeLibrary(reinterpret_cast<HMODULE>(handle));
#else
    dlclose(handle);
#endif
}

std::int64_t count_sml_nodes(const char* source) {
    enum class Mode { Normal, String, LineComment };

    const std::string text(source);
    Mode mode = Mode::Normal;
    bool escape = false;
    bool pending_identifier = false;
    std::int64_t nodes = 0;

    auto is_identifier_start = [](char c) {
        return std::isalpha(static_cast<unsigned char>(c)) || c == '_';
    };
    auto is_identifier_char = [](char c) {
        return std::isalnum(static_cast<unsigned char>(c)) || c == '_' || c == '.';
    };

    for (std::size_t i = 0; i < text.size(); i++) {
        const char c = text[i];
        const char next = (i + 1 < text.size()) ? text[i + 1] : '\0';

        if (mode == Mode::LineComment) {
            if (c == '\n') {
                mode = Mode::Normal;
            }
            continue;
        }

        if (mode == Mode::String) {
            if (!escape && c == '"') {
                mode = Mode::Normal;
                continue;
            }
            if (c == '\\' && !escape) {
                escape = true;
            } else {
                escape = false;
            }
            continue;
        }

        if (c == '/' && next == '/') {
            mode = Mode::LineComment;
            i++;
            continue;
        }

        if (c == '"') {
            mode = Mode::String;
            escape = false;
            continue;
        }

        if (is_identifier_start(c)) {
            std::size_t j = i + 1;
            while (j < text.size() && is_identifier_char(text[j])) {
                j++;
            }
            pending_identifier = true;
            i = j - 1;
            continue;
        }

        if (c == '{') {
            if (pending_identifier) {
                nodes++;
            }
            pending_identifier = false;
            continue;
        }

        if (c == ':' || c == '=' || c == '}' || c == '(' || c == ')' || c == ';') {
            pending_identifier = false;
            continue;
        }
    }

    return nodes;
}

// ── Code Generator: SMS → C++ (Phase 1: int64_t subset) ─────────────────────

static std::string cg_indent(int depth) {
    return std::string(static_cast<std::size_t>(depth * 4), ' ');
}

static std::string cg_op(TokenType op) {
    switch (op) {
        case TokenType::Plus:          return "+";
        case TokenType::Minus:         return "-";
        case TokenType::Star:          return "*";
        case TokenType::Slash:         return "/";
        case TokenType::Percent:       return "%";
        case TokenType::Less:          return "<";
        case TokenType::LessEqual:     return "<=";
        case TokenType::Greater:       return ">";
        case TokenType::GreaterEqual:  return ">=";
        case TokenType::EqualEqual:    return "==";
        case TokenType::BangEqual:     return "!=";
        case TokenType::AndAnd:        return "&&";
        case TokenType::OrOr:          return "||";
        case TokenType::Increment:     return "++";
        case TokenType::Decrement:     return "--";
        default: throw std::runtime_error("codegen: unsupported operator");
    }
}

static std::string cg_expr(const Expr* expr);

static std::string cg_call_args(const std::vector<CallArgumentExpr>& args) {
    std::string out;
    for (std::size_t i = 0; i < args.size(); ++i) {
        if (i > 0) out += ", ";
        out += cg_expr(args[i].value.get());
    }
    return out;
}

static std::string cg_expr(const Expr* expr) {
    if (expr == nullptr) return "0LL";
    if (const auto* e = dynamic_cast<const NumberExpr*>(expr)) {
        return std::to_string(e->value) + "LL";
    }
    if (const auto* e = dynamic_cast<const BoolExpr*>(expr)) {
        return e->value ? "1LL" : "0LL";
    }
    if (dynamic_cast<const NullExpr*>(expr)) {
        return "0LL";
    }
    if (const auto* e = dynamic_cast<const VarExpr*>(expr)) {
        return e->name;
    }
    if (const auto* e = dynamic_cast<const BinaryExpr*>(expr)) {
        return "(" + cg_expr(e->left_.get()) + " " + cg_op(e->oper) + " " + cg_expr(e->right_.get()) + ")";
    }
    if (const auto* e = dynamic_cast<const LogicalExpr*>(expr)) {
        return "(" + cg_expr(e->left_.get()) + " " + cg_op(e->oper) + " " + cg_expr(e->right_.get()) + ")";
    }
    if (const auto* e = dynamic_cast<const UnaryExpr*>(expr)) {
        if (e->oper == TokenType::Bang)  return "(!(" + cg_expr(e->operand_.get()) + ") ? 1LL : 0LL)";
        if (e->oper == TokenType::Minus) return "(-" + cg_expr(e->operand_.get()) + ")";
        throw std::runtime_error("codegen: unsupported unary op");
    }
    if (const auto* e = dynamic_cast<const PostfixExpr*>(expr)) {
        const auto* var = dynamic_cast<const VarExpr*>(e->operand_.get());
        if (!var) throw std::runtime_error("codegen: postfix on non-variable");
        return "(" + var->name + cg_op(e->oper) + ")";
    }
    if (const auto* e = dynamic_cast<const CallExpr*>(expr)) {
        return "sms_" + e->name + "(" + cg_call_args(e->args_) + ")";
    }
    throw std::runtime_error("codegen: unsupported expression type");
}

static std::string cg_stmts(const std::vector<std::unique_ptr<Stmt>>& stmts, int depth);

static std::string cg_stmt(const Stmt* stmt, int depth) {
    const std::string ind = cg_indent(depth);
    if (const auto* s = dynamic_cast<const VarDeclStmt*>(stmt)) {
        return ind + "int64_t " + s->name + " = " + cg_expr(s->value.get()) + ";\n";
    }
    if (const auto* s = dynamic_cast<const AssignStmt*>(stmt)) {
        const auto* var = dynamic_cast<const VarExpr*>(s->target_expr.get());
        if (!var) throw std::runtime_error("codegen: non-variable assignment not supported in Phase 1");
        return ind + var->name + " = " + cg_expr(s->value_expr.get()) + ";\n";
    }
    if (const auto* s = dynamic_cast<const ReturnStmt*>(stmt)) {
        return ind + "return " + (s->value ? cg_expr(s->value.get()) : "0LL") + ";\n";
    }
    if (dynamic_cast<const BreakStmt*>(stmt)) {
        return ind + "break;\n";
    }
    if (dynamic_cast<const ContinueStmt*>(stmt)) {
        return ind + "continue;\n";
    }
    if (const auto* s = dynamic_cast<const ExprStmt*>(stmt)) {
        return ind + cg_expr(s->value.get()) + ";\n";
    }
    if (const auto* s = dynamic_cast<const WhileStmt*>(stmt)) {
        return ind + "while (" + cg_expr(s->condition.get()) + ") {\n"
             + cg_stmts(s->body, depth + 1)
             + ind + "}\n";
    }
    if (const auto* s = dynamic_cast<const IfStmt*>(stmt)) {
        std::string out = ind + "if (" + cg_expr(s->condition.get()) + ") {\n"
             + cg_stmts(s->then_body, depth + 1)
             + ind + "}";
        if (!s->else_body.empty()) {
            out += " else {\n" + cg_stmts(s->else_body, depth + 1) + ind + "}";
        }
        return out + "\n";
    }
    if (const auto* s = dynamic_cast<const FunctionDeclStmt*>(stmt)) {
        std::string params_str;
        for (std::size_t i = 0; i < s->params.size(); ++i) {
            if (i > 0) params_str += ", ";
            params_str += "int64_t " + s->params[i].name;
        }
        return "static int64_t sms_" + s->name + "(" + params_str + ") {\n"
             + cg_stmts(s->body, 1)
             + "}\n";
    }
    // ImportDeclStmt, DataClassDeclStmt, EventHandlerDeclStmt: ignore in Phase 1
    return "";
}

static std::string cg_stmts(const std::vector<std::unique_ptr<Stmt>>& stmts, int depth) {
    std::string out;
    for (const auto& s : stmts) {
        out += cg_stmt(s.get(), depth);
    }
    return out;
}

static std::string codegen_program_cpp(const std::vector<std::unique_ptr<Stmt>>& program) {
    std::string fn_defs;
    std::string entry_call;

    for (const auto& stmt : program) {
        if (dynamic_cast<const FunctionDeclStmt*>(stmt.get())) {
            fn_defs += cg_stmt(stmt.get(), 0) + "\n";
        } else if (const auto* es = dynamic_cast<const ExprStmt*>(stmt.get())) {
            if (const auto* call = dynamic_cast<const CallExpr*>(es->value.get())) {
                entry_call = "sms_" + call->name + "(" + cg_call_args(call->args_) + ")";
            }
        }
    }

    if (entry_call.empty()) {
        throw std::runtime_error("codegen: no top-level function call found as entry point");
    }

    std::string out;
    out += "#include <cstdint>\n";
    out += "#include <chrono>\n";
    out += "#include <cstdio>\n";
    out += "\n";
    out += fn_defs;
    out += "int main() {\n";
    out += "    auto t0 = std::chrono::high_resolution_clock::now();\n";
    out += "    int64_t result = " + entry_call + ";\n";
    out += "    auto t1 = std::chrono::high_resolution_clock::now();\n";
    out += "    double us = std::chrono::duration<double, std::micro>(t1 - t0).count();\n";
    out += "    printf(\"RESULT:%lld\\n\", (long long)result);\n";
    out += "    printf(\"TIME_US:%.1f\\n\", us);\n";
    out += "    return 0;\n";
    out += "}\n";
    return out;
}

// ── LLVM IR Code Generator (SMS → .ll, Phase 1) ──────────────────────────────

enum class LlvmValueKind {
    Unknown = 0,
    Integer = 1,
    String = 2
};

struct LlvmCtx {
    struct ProgramState {
        int global_string_counter = 0;
        std::vector<std::string> global_defs;
    };

    ProgramState* program = nullptr;
    int temp_cnt = 0;
    int label_cnt = 0;
    bool terminated = false;
    std::string code;
    std::vector<std::pair<std::string, std::string>> loops; // {cond_label, end_label}
    std::unordered_map<std::string, LlvmValueKind> var_kinds;
};

static std::string llvm_t(LlvmCtx& c) { return "%t" + std::to_string(c.temp_cnt++); }
static std::string llvm_lbl_name(LlvmCtx& c, const std::string& p) { return p + std::to_string(c.label_cnt++); }
static void llvm_i(LlvmCtx& c, const std::string& s) { c.code += "    " + s + "\n"; c.terminated = false; }
static void llvm_l(LlvmCtx& c, const std::string& l) { c.code += l + ":\n"; c.terminated = false; }
static void llvm_term(LlvmCtx& c, const std::string& s) {
    if (!c.terminated) { c.code += "    " + s + "\n"; c.terminated = true; }
}

struct LlvmGlobalStringRef {
    std::string symbol;
    std::size_t size = 0;
};

static std::string llvm_escape_c_string(const std::string& input) {
    static const char* kHex = "0123456789ABCDEF";
    std::string out;
    out.reserve(input.size() + 8);
    for (unsigned char ch : input) {
        switch (ch) {
            case '\\': out += "\\5C"; break;
            case '"':  out += "\\22"; break;
            case '\n': out += "\\0A"; break;
            case '\r': out += "\\0D"; break;
            case '\t': out += "\\09"; break;
            default:
                if (ch >= 32 && ch <= 126) {
                    out.push_back(static_cast<char>(ch));
                } else {
                    out.push_back('\\');
                    out.push_back(kHex[(ch >> 4) & 0x0F]);
                    out.push_back(kHex[ch & 0x0F]);
                }
                break;
        }
    }
    return out;
}

static std::string llvm_escape_printf_literal(const std::string& input) {
    std::string out;
    out.reserve(input.size() + 8);
    for (char ch : input) {
        if (ch == '%') {
            out += "%%";
        } else {
            out.push_back(ch);
        }
    }
    return out;
}

static LlvmGlobalStringRef llvm_add_global_c_string(LlvmCtx& c, const std::string& text) {
    if (c.program == nullptr) {
        throw std::runtime_error("llvm codegen: missing program context for global string");
    }
    LlvmGlobalStringRef ref;
    ref.symbol = "@.str" + std::to_string(c.program->global_string_counter++);
    ref.size = text.size() + 1;
    const auto escaped = llvm_escape_c_string(text);
    c.program->global_defs.push_back(
        ref.symbol + " = private unnamed_addr constant ["
        + std::to_string(ref.size) + " x i8] c\"" + escaped + "\\00\"");
    return ref;
}

static void llvm_emit_printf(LlvmCtx& c, const std::string& format, const std::vector<std::string>& i64_args) {
    const auto global = llvm_add_global_c_string(c, format);
    const auto fmt_ptr = llvm_t(c);
    llvm_i(c, fmt_ptr + " = getelementptr inbounds [" + std::to_string(global.size) + " x i8], ["
        + std::to_string(global.size) + " x i8]* " + global.symbol + ", i64 0, i64 0");
    std::string call = "call i32 (i8*, ...) @printf(i8* " + fmt_ptr;
    for (const auto& arg : i64_args) {
        call += ", i64 " + arg;
    }
    call += ")";
    llvm_i(c, call);
}

static bool llvm_is_log_method(const std::string& method) {
    return method == "info" || method == "success" || method == "warning"
        || method == "warn" || method == "error" || method == "debug";
}

static bool llvm_expr_has_log_call(const Expr* e) {
    if (e == nullptr) return false;
    if (const auto* m = dynamic_cast<const MethodCallExpr*>(e)) {
        const auto* receiver = dynamic_cast<const VarExpr*>(m->receiver_.get());
        if (receiver != nullptr && receiver->name == "log" && llvm_is_log_method(m->method_)) {
            return true;
        }
        for (const auto& arg : m->args_) {
            if (llvm_expr_has_log_call(arg.value.get())) return true;
        }
        return llvm_expr_has_log_call(m->receiver_.get());
    }
    if (const auto* b = dynamic_cast<const BinaryExpr*>(e)) {
        return llvm_expr_has_log_call(b->left_.get()) || llvm_expr_has_log_call(b->right_.get());
    }
    if (const auto* l = dynamic_cast<const LogicalExpr*>(e)) {
        return llvm_expr_has_log_call(l->left_.get()) || llvm_expr_has_log_call(l->right_.get());
    }
    if (const auto* u = dynamic_cast<const UnaryExpr*>(e)) {
        return llvm_expr_has_log_call(u->operand_.get());
    }
    if (const auto* p = dynamic_cast<const PostfixExpr*>(e)) {
        return llvm_expr_has_log_call(p->operand_.get());
    }
    if (const auto* c = dynamic_cast<const CallExpr*>(e)) {
        for (const auto& arg : c->args_) {
            if (llvm_expr_has_log_call(arg.value.get())) return true;
        }
        return false;
    }
    if (const auto* a = dynamic_cast<const ArrayLiteralExpr*>(e)) {
        for (const auto& item : a->elements_) {
            if (llvm_expr_has_log_call(item.get())) return true;
        }
        return false;
    }
    if (const auto* a = dynamic_cast<const ArrayAccessExpr*>(e)) {
        return llvm_expr_has_log_call(a->receiver_.get()) || llvm_expr_has_log_call(a->index_.get());
    }
    if (const auto* m = dynamic_cast<const MemberAccessExpr*>(e)) {
        return llvm_expr_has_log_call(m->receiver_.get());
    }
    if (const auto* w = dynamic_cast<const WhenExpr*>(e)) {
        if (llvm_expr_has_log_call(w->subject_.get())) return true;
        for (const auto& branch : w->branches_) {
            if (llvm_expr_has_log_call(branch.condition.get()) || llvm_expr_has_log_call(branch.result.get())) return true;
        }
    }
    if (const auto* s = dynamic_cast<const InterpolatedStringExpr*>(e)) {
        for (const auto& part : s->parts_) {
            if (llvm_expr_has_log_call(part.expr.get())) return true;
        }
    }
    return false;
}

static bool llvm_stmt_has_log_call(const Stmt* s) {
    if (s == nullptr) return false;
    if (const auto* v = dynamic_cast<const VarDeclStmt*>(s)) return llvm_expr_has_log_call(v->value.get());
    if (const auto* a = dynamic_cast<const AssignStmt*>(s)) {
        return llvm_expr_has_log_call(a->target_expr.get()) || llvm_expr_has_log_call(a->value_expr.get());
    }
    if (const auto* r = dynamic_cast<const ReturnStmt*>(s)) return llvm_expr_has_log_call(r->value.get());
    if (const auto* e = dynamic_cast<const ExprStmt*>(s)) return llvm_expr_has_log_call(e->value.get());
    if (const auto* w = dynamic_cast<const WhileStmt*>(s)) {
        if (llvm_expr_has_log_call(w->condition.get())) return true;
        for (const auto& stmt : w->body) if (llvm_stmt_has_log_call(stmt.get())) return true;
        return false;
    }
    if (const auto* i = dynamic_cast<const IfStmt*>(s)) {
        if (llvm_expr_has_log_call(i->condition.get())) return true;
        for (const auto& stmt : i->then_body) if (llvm_stmt_has_log_call(stmt.get())) return true;
        for (const auto& stmt : i->else_body) if (llvm_stmt_has_log_call(stmt.get())) return true;
        return false;
    }
    if (const auto* f = dynamic_cast<const ForStmt*>(s)) {
        if (llvm_stmt_has_log_call(f->init.get())) return true;
        if (llvm_expr_has_log_call(f->condition.get())) return true;
        if (llvm_stmt_has_log_call(f->update.get())) return true;
        for (const auto& stmt : f->body) if (llvm_stmt_has_log_call(stmt.get())) return true;
        return false;
    }
    if (const auto* fi = dynamic_cast<const ForInStmt*>(s)) {
        if (llvm_expr_has_log_call(fi->iterable.get())) return true;
        for (const auto& stmt : fi->body) if (llvm_stmt_has_log_call(stmt.get())) return true;
        return false;
    }
    if (const auto* fn = dynamic_cast<const FunctionDeclStmt*>(s)) {
        for (const auto& stmt : fn->body) if (llvm_stmt_has_log_call(stmt.get())) return true;
        return false;
    }
    return false;
}

static std::string llvm_expr(LlvmCtx& c, const Expr* e);
static std::string llvm_cond(LlvmCtx& c, const Expr* e);
static void llvm_stmt(LlvmCtx& c, const Stmt* s);
static void llvm_stmts(LlvmCtx& c, const std::vector<std::unique_ptr<Stmt>>& ss);

static std::optional<std::string> llvm_try_const_string_expr(const Expr* e) {
    if (const auto* s = dynamic_cast<const StringExpr*>(e)) {
        return s->value;
    }
    if (const auto* is = dynamic_cast<const InterpolatedStringExpr*>(e)) {
        std::string out;
        for (const auto& part : is->parts_) {
            if (part.expr != nullptr) {
                return std::nullopt;
            }
            out += part.literal;
        }
        return out;
    }
    return std::nullopt;
}

static std::string llvm_emit_const_string_ptr(LlvmCtx& c, const std::string& text) {
    const auto global = llvm_add_global_c_string(c, text);
    const auto ptr = llvm_t(c);
    llvm_i(c, ptr + " = getelementptr inbounds [" + std::to_string(global.size) + " x i8], ["
        + std::to_string(global.size) + " x i8]* " + global.symbol + ", i64 0, i64 0");
    return ptr;
}

static std::string llvm_expr_as_string_ptr(LlvmCtx& c, const Expr* e) {
    const auto const_text = llvm_try_const_string_expr(e);
    if (const_text.has_value()) {
        return llvm_emit_const_string_ptr(c, const_text.value());
    }
    const auto value_i64 = llvm_expr(c, e);
    const auto value_ptr = llvm_t(c);
    llvm_i(c, value_ptr + " = inttoptr i64 " + value_i64 + " to i8*");
    return value_ptr;
}

static LlvmValueKind llvm_expr_kind(const LlvmCtx& c, const Expr* e) {
    if (e == nullptr) return LlvmValueKind::Unknown;
    if (dynamic_cast<const NumberExpr*>(e) || dynamic_cast<const BoolExpr*>(e) || dynamic_cast<const NullExpr*>(e)) {
        return LlvmValueKind::Integer;
    }
    if (dynamic_cast<const StringExpr*>(e) || dynamic_cast<const InterpolatedStringExpr*>(e)) {
        return LlvmValueKind::String;
    }
    if (const auto* v = dynamic_cast<const VarExpr*>(e)) {
        const auto it = c.var_kinds.find(v->name);
        return it != c.var_kinds.end() ? it->second : LlvmValueKind::Unknown;
    }
    if (const auto* b = dynamic_cast<const BinaryExpr*>(e)) {
        const auto lk = llvm_expr_kind(c, b->left_.get());
        const auto rk = llvm_expr_kind(c, b->right_.get());
        if (b->oper == TokenType::Plus && (lk == LlvmValueKind::String || rk == LlvmValueKind::String)) {
            return LlvmValueKind::String;
        }
        return LlvmValueKind::Integer;
    }
    if (const auto* m = dynamic_cast<const MethodCallExpr*>(e)) {
        const auto* receiver = dynamic_cast<const VarExpr*>(m->receiver_.get());
        if (receiver != nullptr && receiver->name == "ui" && m->method_ == "getObject") {
            return LlvmValueKind::String;
        }
    }
    return LlvmValueKind::Unknown;
}

static std::string llvm_expr(LlvmCtx& c, const Expr* e) {
    if (const auto* n = dynamic_cast<const NumberExpr*>(e))
        return std::to_string(n->value);
    if (const auto* b = dynamic_cast<const BoolExpr*>(e))
        return b->value ? "1" : "0";
    if (dynamic_cast<const NullExpr*>(e))
        return "0";
    if (const auto* s = dynamic_cast<const StringExpr*>(e)) {
        const auto ptr = llvm_emit_const_string_ptr(c, s->value);
        const auto raw = llvm_t(c);
        llvm_i(c, raw + " = ptrtoint i8* " + ptr + " to i64");
        return raw;
    }
    if (const auto* is = dynamic_cast<const InterpolatedStringExpr*>(e)) {
        const auto value = llvm_try_const_string_expr(is);
        if (!value.has_value()) {
            throw std::runtime_error("llvm codegen: interpolated string with expressions is not supported yet");
        }
        const auto ptr = llvm_emit_const_string_ptr(c, value.value());
        const auto raw = llvm_t(c);
        llvm_i(c, raw + " = ptrtoint i8* " + ptr + " to i64");
        return raw;
    }
    if (const auto* v = dynamic_cast<const VarExpr*>(e)) {
        const auto t = llvm_t(c);
        llvm_i(c, t + " = load i64, i64* %" + v->name);
        return t;
    }
    if (const auto* b = dynamic_cast<const BinaryExpr*>(e)) {
        const auto left_kind = llvm_expr_kind(c, b->left_.get());
        const auto right_kind = llvm_expr_kind(c, b->right_.get());
        const bool string_op = (left_kind == LlvmValueKind::String || right_kind == LlvmValueKind::String);
        if (b->oper == TokenType::Plus && string_op) {
            const auto left_ptr = llvm_expr_as_string_ptr(c, b->left_.get());
            const auto right_ptr = llvm_expr_as_string_ptr(c, b->right_.get());
            const auto concat_ptr = llvm_t(c);
            llvm_i(c, concat_ptr + " = call i8* @sms_native_llvm_string_concat(i8* " + left_ptr + ", i8* " + right_ptr + ")");
            const auto concat_i64 = llvm_t(c);
            llvm_i(c, concat_i64 + " = ptrtoint i8* " + concat_ptr + " to i64");
            return concat_i64;
        }
        if ((b->oper == TokenType::EqualEqual || b->oper == TokenType::BangEqual) && string_op) {
            const auto left_ptr = llvm_expr_as_string_ptr(c, b->left_.get());
            const auto right_ptr = llvm_expr_as_string_ptr(c, b->right_.get());
            const auto eq_i64 = llvm_t(c);
            llvm_i(c, eq_i64 + " = call i64 @sms_native_llvm_string_eq(i8* " + left_ptr + ", i8* " + right_ptr + ")");
            if (b->oper == TokenType::EqualEqual) {
                return eq_i64;
            }
            const auto neq_i64 = llvm_t(c);
            llvm_i(c, neq_i64 + " = sub i64 1, " + eq_i64);
            return neq_i64;
        }
        const auto lv = llvm_expr(c, b->left_.get());
        const auto rv = llvm_expr(c, b->right_.get());
        const auto t = llvm_t(c);
        switch (b->oper) {
            case TokenType::Plus:    llvm_i(c, t + " = add i64 " + lv + ", " + rv); break;
            case TokenType::Minus:   llvm_i(c, t + " = sub i64 " + lv + ", " + rv); break;
            case TokenType::Star:    llvm_i(c, t + " = mul i64 " + lv + ", " + rv); break;
            case TokenType::Slash:   llvm_i(c, t + " = sdiv i64 " + lv + ", " + rv); break;
            case TokenType::Percent: llvm_i(c, t + " = srem i64 " + lv + ", " + rv); break;
            default: {
                const char* op = nullptr;
                switch (b->oper) {
                    case TokenType::Less:         op = "slt"; break;
                    case TokenType::LessEqual:    op = "sle"; break;
                    case TokenType::Greater:      op = "sgt"; break;
                    case TokenType::GreaterEqual: op = "sge"; break;
                    case TokenType::EqualEqual:   op = "eq";  break;
                    case TokenType::BangEqual:    op = "ne";  break;
                    default: throw std::runtime_error("llvm codegen: unsupported binary op");
                }
                const auto cmp = llvm_t(c);
                llvm_i(c, cmp + " = icmp " + op + " i64 " + lv + ", " + rv);
                llvm_i(c, t + " = zext i1 " + cmp + " to i64");
                break;
            }
        }
        return t;
    }
    if (const auto* l = dynamic_cast<const LogicalExpr*>(e)) {
        const auto lv = llvm_expr(c, l->left_.get());
        const auto rv = llvm_expr(c, l->right_.get());
        const auto cl = llvm_t(c); llvm_i(c, cl + " = icmp ne i64 " + lv + ", 0");
        const auto cr = llvm_t(c); llvm_i(c, cr + " = icmp ne i64 " + rv + ", 0");
        const auto tmp = llvm_t(c);
        if (l->oper == TokenType::AndAnd) llvm_i(c, tmp + " = and i1 " + cl + ", " + cr);
        else                              llvm_i(c, tmp + " = or i1 "  + cl + ", " + cr);
        const auto t = llvm_t(c);
        llvm_i(c, t + " = zext i1 " + tmp + " to i64");
        return t;
    }
    if (const auto* u = dynamic_cast<const UnaryExpr*>(e)) {
        const auto v = llvm_expr(c, u->operand_.get());
        const auto t = llvm_t(c);
        if (u->oper == TokenType::Minus) {
            llvm_i(c, t + " = sub i64 0, " + v);
        } else if (u->oper == TokenType::Bang) {
            const auto cmp = llvm_t(c);
            llvm_i(c, cmp + " = icmp eq i64 " + v + ", 0");
            llvm_i(c, t + " = zext i1 " + cmp + " to i64");
        } else {
            throw std::runtime_error("llvm codegen: unsupported unary op");
        }
        return t;
    }
    if (const auto* p = dynamic_cast<const PostfixExpr*>(e)) {
        const auto* var = dynamic_cast<const VarExpr*>(p->operand_.get());
        if (!var) throw std::runtime_error("llvm codegen: postfix on non-variable");
        const auto old_t = llvm_t(c);
        llvm_i(c, old_t + " = load i64, i64* %" + var->name);
        const auto new_t = llvm_t(c);
        llvm_i(c, new_t + " = " + (p->oper == TokenType::Increment ? "add" : "sub") + " i64 " + old_t + ", 1");
        llvm_i(c, "store i64 " + new_t + ", i64* %" + var->name);
        return old_t;
    }
    if (const auto* call = dynamic_cast<const CallExpr*>(e)) {
        std::string args_str;
        for (std::size_t i = 0; i < call->args_.size(); ++i) {
            if (i > 0) args_str += ", ";
            args_str += "i64 " + llvm_expr(c, call->args_[i].value.get());
        }
        const auto t = llvm_t(c);
        llvm_i(c, t + " = call i64 @sms_" + call->name + "(" + args_str + ")");
        return t;
    }
    if (const auto* m = dynamic_cast<const MethodCallExpr*>(e)) {
        const auto* receiver = dynamic_cast<const VarExpr*>(m->receiver_.get());
        if (receiver != nullptr && receiver->name == "ui") {
            if (m->method_ == "getObject") {
                if (m->args_.size() != 1) {
                    throw std::runtime_error("llvm codegen: ui.getObject expects exactly one argument");
                }
                const auto object_ptr = llvm_expr_as_string_ptr(c, m->args_[0].value.get());
                const auto ui_ref_ptr = llvm_t(c);
                llvm_i(c, ui_ref_ptr + " = call i8* @sms_native_llvm_ui_get_object(i8* " + object_ptr + ")");
                const auto ui_ref_i64 = llvm_t(c);
                llvm_i(c, ui_ref_i64 + " = ptrtoint i8* " + ui_ref_ptr + " to i64");
                return ui_ref_i64;
            }
            throw std::runtime_error("llvm codegen: unsupported ui method '" + m->method_ + "'");
        }
        if (receiver != nullptr && receiver->name == "os") {
            if (m->method_ == "now") {
                if (!m->args_.empty()) {
                    throw std::runtime_error("llvm codegen: os.now expects no arguments");
                }
                const auto now_ms = llvm_t(c);
                llvm_i(c, now_ms + " = call i64 @sms_native_llvm_os_now_ms()");
                return now_ms;
            }
            throw std::runtime_error("llvm codegen: unsupported os method '" + m->method_ + "'");
        }
        if (receiver != nullptr && receiver->name == "log") {
            if (!llvm_is_log_method(m->method_)) {
                throw std::runtime_error("llvm codegen: unknown log method");
            }
            if (m->args_.size() != 1) {
                throw std::runtime_error("llvm codegen: log methods expect exactly one argument");
            }

            const bool green = (m->method_ == "success");
            const std::string color_prefix = green ? "\x1b[32m" : "";
            const std::string color_suffix = green ? "\x1b[0m" : "";
            const Expr* arg_expr = m->args_[0].value.get();
            if (const auto* s = dynamic_cast<const StringExpr*>(arg_expr)) {
                llvm_emit_printf(c, color_prefix + llvm_escape_printf_literal(s->value) + color_suffix + "\n", {});
                return "0";
            }
            if (const auto* is = dynamic_cast<const InterpolatedStringExpr*>(arg_expr)) {
                std::string format = color_prefix;
                std::vector<std::string> values;
                for (const auto& part : is->parts_) {
                    if (part.expr) {
                        format += "%lld";
                        values.push_back(llvm_expr(c, part.expr.get()));
                    } else {
                        format += llvm_escape_printf_literal(part.literal);
                    }
                }
                format += color_suffix + "\n";
                llvm_emit_printf(c, format, values);
                return "0";
            }

            const auto value = llvm_expr(c, arg_expr);
            llvm_emit_printf(c, color_prefix + std::string("%lld") + color_suffix + "\n", {value});
            return "0";
        }
    }
    throw std::runtime_error("llvm codegen: unsupported expression type");
}

static std::string llvm_cond(LlvmCtx& c, const Expr* e) {
    if (const auto* b = dynamic_cast<const BinaryExpr*>(e)) {
        const char* op = nullptr;
        switch (b->oper) {
            case TokenType::Less:         op = "slt"; break;
            case TokenType::LessEqual:    op = "sle"; break;
            case TokenType::Greater:      op = "sgt"; break;
            case TokenType::GreaterEqual: op = "sge"; break;
            case TokenType::EqualEqual:   op = "eq";  break;
            case TokenType::BangEqual:    op = "ne";  break;
            default: break;
        }
        if (op) {
            const auto lv = llvm_expr(c, b->left_.get());
            const auto rv = llvm_expr(c, b->right_.get());
            const auto t = llvm_t(c);
            llvm_i(c, t + " = icmp " + op + " i64 " + lv + ", " + rv);
            return t;
        }
    }
    const auto v = llvm_expr(c, e);
    const auto t = llvm_t(c);
    llvm_i(c, t + " = icmp ne i64 " + v + ", 0");
    return t;
}

static void llvm_stmts(LlvmCtx& c, const std::vector<std::unique_ptr<Stmt>>& ss) {
    for (const auto& s : ss) llvm_stmt(c, s.get());
}

static void llvm_stmt(LlvmCtx& c, const Stmt* s) {
    if (const auto* v = dynamic_cast<const VarDeclStmt*>(s)) {
        llvm_i(c, "%" + v->name + " = alloca i64");
        const auto init = llvm_expr(c, v->value.get());
        llvm_i(c, "store i64 " + init + ", i64* %" + v->name);
        c.var_kinds[v->name] = llvm_expr_kind(c, v->value.get());
        return;
    }
    if (const auto* a = dynamic_cast<const AssignStmt*>(s)) {
        if (const auto* member = dynamic_cast<const MemberAccessExpr*>(a->target_expr.get())) {
            const auto value_kind = llvm_expr_kind(c, a->value_expr.get());
            if (value_kind == LlvmValueKind::String) {
                const auto object_ref_i64 = llvm_expr(c, member->receiver_.get());
                const auto object_ref_ptr = llvm_t(c);
                llvm_i(c, object_ref_ptr + " = inttoptr i64 " + object_ref_i64 + " to i8*");
                const auto member_name_ptr = llvm_emit_const_string_ptr(c, member->member_);
                const auto text_ptr = llvm_expr_as_string_ptr(c, a->value_expr.get());
                llvm_i(c, "call i64 @sms_native_llvm_set_ui_string_prop(i8* " + object_ref_ptr + ", i8* " + member_name_ptr + ", i8* " + text_ptr + ")");
                return;
            }
            if (value_kind == LlvmValueKind::Integer || value_kind == LlvmValueKind::Unknown) {
                const auto object_ref_i64 = llvm_expr(c, member->receiver_.get());
                const auto object_ref_ptr = llvm_t(c);
                llvm_i(c, object_ref_ptr + " = inttoptr i64 " + object_ref_i64 + " to i8*");
                const auto member_name_ptr = llvm_emit_const_string_ptr(c, member->member_);
                const auto val = llvm_expr(c, a->value_expr.get());
                llvm_i(c, "call i64 @sms_native_llvm_set_ui_int_prop(i8* " + object_ref_ptr + ", i8* " + member_name_ptr + ", i64 " + val + ")");
                return;
            }
            throw std::runtime_error("llvm codegen: unsupported member assignment target '" + member->member_ + "'");
        }
        const auto* var = dynamic_cast<const VarExpr*>(a->target_expr.get());
        if (!var) throw std::runtime_error("llvm codegen: non-variable assignment not supported in Phase 1");
        const auto val = llvm_expr(c, a->value_expr.get());
        llvm_i(c, "store i64 " + val + ", i64* %" + var->name);
        c.var_kinds[var->name] = llvm_expr_kind(c, a->value_expr.get());
        return;
    }
    if (const auto* r = dynamic_cast<const ReturnStmt*>(s)) {
        const auto val = r->value ? llvm_expr(c, r->value.get()) : std::string("0");
        llvm_term(c, "ret i64 " + val);
        return;
    }
    if (dynamic_cast<const BreakStmt*>(s)) {
        if (c.loops.empty()) throw std::runtime_error("llvm codegen: break outside loop");
        llvm_term(c, "br label %" + c.loops.back().second);
        return;
    }
    if (dynamic_cast<const ContinueStmt*>(s)) {
        if (c.loops.empty()) throw std::runtime_error("llvm codegen: continue outside loop");
        llvm_term(c, "br label %" + c.loops.back().first);
        return;
    }
    if (const auto* es = dynamic_cast<const ExprStmt*>(s)) {
        llvm_expr(c, es->value.get());
        return;
    }
    if (const auto* w = dynamic_cast<const WhileStmt*>(s)) {
        const auto cond_l = llvm_lbl_name(c, "while_cond");
        const auto body_l = llvm_lbl_name(c, "while_body");
        const auto end_l  = llvm_lbl_name(c, "while_end");
        llvm_term(c, "br label %" + cond_l);
        llvm_l(c, cond_l);
        const auto cond = llvm_cond(c, w->condition.get());
        llvm_term(c, "br i1 " + cond + ", label %" + body_l + ", label %" + end_l);
        llvm_l(c, body_l);
        c.loops.push_back({cond_l, end_l});
        llvm_stmts(c, w->body);
        c.loops.pop_back();
        llvm_term(c, "br label %" + cond_l);
        llvm_l(c, end_l);
        return;
    }
    if (const auto* i = dynamic_cast<const IfStmt*>(s)) {
        const auto then_l = llvm_lbl_name(c, "if_then");
        const auto else_l = llvm_lbl_name(c, "if_else");
        const auto end_l  = llvm_lbl_name(c, "if_end");
        const bool has_else = !i->else_body.empty();
        const auto cond = llvm_cond(c, i->condition.get());
        llvm_term(c, "br i1 " + cond + ", label %" + then_l + ", label %" + (has_else ? else_l : end_l));
        llvm_l(c, then_l);
        llvm_stmts(c, i->then_body);
        llvm_term(c, "br label %" + end_l);
        if (has_else) {
            llvm_l(c, else_l);
            llvm_stmts(c, i->else_body);
            llvm_term(c, "br label %" + end_l);
        }
        llvm_l(c, end_l);
        return;
    }
    // FunctionDeclStmt, ImportDeclStmt, DataClassDeclStmt, EventHandlerDeclStmt: handled at top level
}

static std::string codegen_program_llvm_ir(const std::vector<std::unique_ptr<Stmt>>& program, bool emit_exe_entry) {
    std::vector<const FunctionDeclStmt*> fns;
    std::string entry_fn;
    bool has_log_calls = false;

    for (const auto& stmt : program) {
        if (llvm_stmt_has_log_call(stmt.get())) {
            has_log_calls = true;
        }
        if (const auto* fn = dynamic_cast<const FunctionDeclStmt*>(stmt.get())) {
            fns.push_back(fn);
        } else if (const auto* es = dynamic_cast<const ExprStmt*>(stmt.get())) {
            if (const auto* call = dynamic_cast<const CallExpr*>(es->value.get())) {
                entry_fn = call->name;
            }
        }
    }

    if (entry_fn.empty()) {
        for (const auto* fn : fns) {
            if (fn->name == "main") {
                entry_fn = "main";
                break;
            }
        }
    }
    if (emit_exe_entry && entry_fn.empty()) {
        throw std::runtime_error("llvm codegen: no top-level function call and no 'main' function fallback");
    }

    LlvmCtx::ProgramState program_state;
    std::string function_defs;

    for (const auto* fn : fns) {
        std::string params_str;
        for (std::size_t i = 0; i < fn->params.size(); ++i) {
            if (i > 0) params_str += ", ";
            params_str += "i64 %" + fn->params[i].name + "_arg";
        }

        LlvmCtx ctx;
        ctx.program = &program_state;
        for (const auto& param : fn->params) {
            ctx.code += "    %" + param.name + " = alloca i64\n";
            ctx.code += "    store i64 %" + param.name + "_arg, i64* %" + param.name + "\n";
        }
        llvm_stmts(ctx, fn->body);
        if (!ctx.terminated) ctx.code += "    ret i64 0\n";

        function_defs += "define i64 @sms_" + fn->name + "(" + params_str + ") {\n";
        function_defs += "entry:\n";
        function_defs += ctx.code;
        function_defs += "}\n\n";
    }

    std::string out;
    out += "; SMS -> LLVM IR  |  Phase 1  |  CrowdWare Forge 2026\n\n";
    out += "declare i64 @clock()\n";
    out += "declare i32 @printf(i8*, ...)\n";
    out += "declare i8* @sms_native_llvm_ui_get_object(i8*)\n";
    out += "declare i64 @sms_native_llvm_set_ui_text(i8*, i8*)\n\n";
    out += "declare i64 @sms_native_llvm_set_ui_string_prop(i8*, i8*, i8*)\n";
    out += "declare i64 @sms_native_llvm_set_ui_int_prop(i8*, i8*, i64)\n";
    out += "declare i64 @sms_native_llvm_os_now_ms()\n";
    out += "declare i8* @sms_native_llvm_string_concat(i8*, i8*)\n";
    out += "declare i64 @sms_native_llvm_string_eq(i8*, i8*)\n\n";
    if (!has_log_calls) {
        // "RESULT:%lld\n\0" = 13 bytes,  "TIME_US:%.1f\n\0" = 14 bytes
        out += "@.rfmt = private unnamed_addr constant [13 x i8] c\"RESULT:%lld\\0A\\00\"\n";
        out += "@.tfmt = private unnamed_addr constant [14 x i8] c\"TIME_US:%.1f\\0A\\00\"\n";
    }
    for (const auto& global_def : program_state.global_defs) {
        out += global_def + "\n";
    }
    out += "\n";
    out += function_defs;

    if (emit_exe_entry) {
        // main() — uses clock() for timing (CLOCKS_PER_SEC=1000000 on macOS → µs directly)
        out += "define i32 @main() {\n";
        out += "entry:\n";
        out += "    %t_start = call i64 @clock()\n";
        out += "    %result = call i64 @sms_" + entry_fn + "()\n";
        out += "    %t_end = call i64 @clock()\n";
        out += "    %elapsed = sub i64 %t_end, %t_start\n";
        out += "    %elapsed_f = sitofp i64 %elapsed to double\n";
        if (!has_log_calls) {
            out += "    %rfmt = getelementptr inbounds [13 x i8], [13 x i8]* @.rfmt, i64 0, i64 0\n";
            out += "    call i32 (i8*, ...) @printf(i8* %rfmt, i64 %result)\n";
            out += "    %tfmt = getelementptr inbounds [14 x i8], [14 x i8]* @.tfmt, i64 0, i64 0\n";
            out += "    call i32 (i8*, ...) @printf(i8* %tfmt, double %elapsed_f)\n";
        }
        out += "    %result_i32 = trunc i64 %result to i32\n";
        out += "    ret i32 %result_i32\n";
        out += "}\n";
    }

    return out;
}

// ─────────────────────────────────────────────────────────────────────────────

std::vector<Value> parse_event_args_json(const char* args_json);

struct SmsSessionRuntime {
    std::vector<std::unique_ptr<Stmt>> program;
    Env env;
    Value top_level_last = Value::Null();
    bool has_ahimsa = false;
    // Re-entrant because UI callbacks can synchronously trigger nested SMS events
    // while an outer event invoke is still on the stack (e.g. loadProject -> scenePropAdded).
    std::recursive_mutex mutex;
    std::size_t invoke_depth = 0;
};

constexpr std::size_t kMaxInvokeDepth = 256;

static bool source_has_ahimsa_comment(std::string_view src) {
    static constexpr std::string_view kComments[] = {
        "// Do not harm any living being.",
        "// Tu keinem Lebewesen Leid an.",
        "// Ne dama\xc4\x9du iun ajn vivantan esta\xc4\xb5on.",
    };
    for (const auto& c : kComments) {
        if (src.substr(0, c.size()) == c) return true;
    }
    return false;
}

static std::shared_ptr<SmsSessionRuntime> build_session_runtime_or_throw(const char* source) {
    const auto src = std::string_view(source);
    const bool hasAhimsa = source_has_ahimsa_comment(src);
    init_source_lines(source);
    Lexer lexer(source);
    auto tokens = lexer.tokenize();
    Parser parser(std::move(tokens));
    auto program = parser.parse_program();

    auto runtime = std::make_shared<SmsSessionRuntime>();
    runtime->program = std::move(program);
    runtime->has_ahimsa = hasAhimsa;
    runtime->env.define_var("log", Value::Object("log", {}));
    runtime->env.define_var("os", Value::Object("os", {}));
    runtime->env.define_var("fs", Value::Object("fs", {}));
    runtime->env.define_var("i18n", Value::Object("i18n", {}));
    runtime->env.define_var("ai", Value::Object("ai", {}));
    runtime->env.define_var("ui", Value::Object("ui", {}));
    runtime->env.define_var("benchmark", Value::Object("benchmark", {}));

    for (const auto& stmt : runtime->program) {
        if (const auto* fn = dynamic_cast<const FunctionDeclStmt*>(stmt.get())) {
            runtime->env.define_function(fn->name, fn);
            continue;
        }
        if (const auto* data = dynamic_cast<const DataClassDeclStmt*>(stmt.get())) {
            runtime->env.define_data_class(data->name, data);
            continue;
        }
        if (const auto* handler = dynamic_cast<const EventHandlerDeclStmt*>(stmt.get())) {
            runtime->env.define_event_handler(handler->key(), handler);
        }
    }

    Value last = Value::Null();
    for (const auto& stmt : runtime->program) {
        if (dynamic_cast<const FunctionDeclStmt*>(stmt.get()) != nullptr
            || dynamic_cast<const DataClassDeclStmt*>(stmt.get()) != nullptr
            || dynamic_cast<const EventHandlerDeclStmt*>(stmt.get()) != nullptr) {
            continue;
        }
        stmt->execute(runtime->env, last);
    }

    runtime->top_level_last = last;
    return runtime;
}

static int invoke_event_on_runtime(
    SmsSessionRuntime& runtime,
    const char* target_id,
    const char* event_name,
    const char* args_json,
    std::int64_t* out_result,
    char* error,
    int error_capacity) {
    if (target_id == nullptr || event_name == nullptr || out_result == nullptr) {
        write_error(error, error_capacity, "target_id/event_name/out_result must not be null");
        return 2;
    }

    try {
        auto args = parse_event_args_json(args_json);
        std::lock_guard<std::recursive_mutex> lock(runtime.mutex);

        const std::string key = std::string(target_id) + "." + std::string(event_name);
        struct InvokeDepthGuard {
            SmsSessionRuntime& runtime;
            bool armed = false;
            explicit InvokeDepthGuard(SmsSessionRuntime& runtime_ref, const std::string& invoke_key) : runtime(runtime_ref) {
                if (runtime.invoke_depth >= kMaxInvokeDepth) {
                    throw SmsGuruMeditation("recursion_limit_exceeded", invoke_key);
                }
                runtime.invoke_depth++;
                armed = true;
            }
            ~InvokeDepthGuard() {
                if (armed && runtime.invoke_depth > 0) {
                    runtime.invoke_depth--;
                }
            }
        } depth_guard(runtime, key);

        const auto* handler = runtime.env.get_event_handler(key);
        if (handler == nullptr) {
            write_error(error, error_capacity, "No SMS event handler found for '" + key + "'.");
            return 1;
        }

        if (args.size() != handler->params.size()) {
            write_error(error, error_capacity, "Event handler '" + key + "' expects "
                + std::to_string(handler->params.size()) + " arg(s), got "
                + std::to_string(args.size()) + ".");
            return 1;
        }

        Env local(&runtime.env);
        for (std::size_t idx = 0; idx < args.size(); idx++) {
            local.define_var(handler->params[idx], args[idx]);
        }

        Value handler_last = Value::Null();
        try {
            execute_statements(handler->body, local, handler_last);
        } catch (const ReturnSignal& ret) {
            handler_last = ret.value;
        }

        *out_result = handler_last.kind == Value::Kind::Int
            ? handler_last.int_value
            : (handler_last.kind == Value::Kind::Bool ? (handler_last.bool_value ? 1 : 0) : 0);
        return 0;
    } catch (const std::exception& ex) {
        write_error(error, error_capacity, ex.what());
        return 1;
    } catch (...) {
        write_error(error, error_capacity, SmsGuruMeditation("unknown_error").what());
        return 1;
    }
}

int execute_sms_source(const char* source, std::int64_t* out_result, char* error, int error_capacity) {
    if (source == nullptr || out_result == nullptr) {
        write_error(error, error_capacity, "source/out_result must not be null");
        return 2;
    }

    try {
        auto runtime = build_session_runtime_or_throw(source);
        const auto& last = runtime->top_level_last;
        *out_result = last.kind == Value::Kind::Int
            ? last.int_value
            : (last.kind == Value::Kind::Bool ? (last.bool_value ? 1 : 0) : 0);
        return 0;
    } catch (const ReturnSignal&) {
        write_error(error, error_capacity, "return outside function");
        return 1;
    } catch (const BreakSignal&) {
        write_error(error, error_capacity, "break outside loop");
        return 1;
    } catch (const ContinueSignal&) {
        write_error(error, error_capacity, "continue outside loop");
        return 1;
    } catch (const std::exception& ex) {
        write_error(error, error_capacity, ex.what());
        return 1;
    } catch (...) {
        write_error(error, error_capacity, SmsGuruMeditation("unknown_error").what());
        return 1;
    }
}

std::vector<Value> parse_event_args_json(const char* args_json) {
    std::vector<Value> args;
    if (args_json == nullptr) {
        return args;
    }

    const std::string text(args_json);
    std::size_t i = 0;
    auto skip_ws = [&]() {
        while (i < text.size() && std::isspace(static_cast<unsigned char>(text[i]))) {
            i++;
        }
    };
    auto starts_with = [&](const char* literal) {
        const auto len = std::strlen(literal);
        return i + len <= text.size() && text.compare(i, len, literal) == 0;
    };
    auto parse_int = [&]() -> Value {
        const auto start = i;
        if (i < text.size() && (text[i] == '-' || text[i] == '+')) {
            i++;
        }
        bool has_digit = false;
        while (i < text.size() && std::isdigit(static_cast<unsigned char>(text[i]))) {
            has_digit = true;
            i++;
        }
        if (!has_digit) {
            throw std::runtime_error("Invalid numeric argument in args_json.");
        }
        return Value::Int(std::stoll(text.substr(start, i - start)));
    };
    auto parse_string = [&]() -> Value {
        i++; // opening quote
        std::string out;
        bool escape = false;
        while (i < text.size()) {
            const char c = text[i++];
            if (escape) {
                switch (c) {
                    case '"': out.push_back('"'); break;
                    case '\\': out.push_back('\\'); break;
                    case '/': out.push_back('/'); break;
                    case 'b': out.push_back('\b'); break;
                    case 'f': out.push_back('\f'); break;
                    case 'n': out.push_back('\n'); break;
                    case 'r': out.push_back('\r'); break;
                    case 't': out.push_back('\t'); break;
                    default: out.push_back(c); break;
                }
                escape = false;
                continue;
            }
            if (c == '\\') {
                escape = true;
                continue;
            }
            if (c == '"') {
                return Value::String(std::move(out));
            }
            out.push_back(c);
        }
        throw std::runtime_error("Unterminated string in args_json.");
    };

    skip_ws();
    if (i >= text.size()) {
        return args;
    }
    if (text[i] != '[') {
        throw std::runtime_error("args_json must be a JSON array.");
    }
    i++;

    while (true) {
        skip_ws();
        if (i >= text.size()) {
            throw std::runtime_error("Unterminated args_json array.");
        }
        if (text[i] == ']') {
            i++;
            break;
        }

        Value value = Value::Null();
        if (text[i] == '"') {
            value = parse_string();
        } else if (starts_with("true")) {
            i += 4;
            value = Value::Bool(true);
        } else if (starts_with("false")) {
            i += 5;
            value = Value::Bool(false);
        } else if (starts_with("null")) {
            i += 4;
            value = Value::Null();
        } else {
            value = parse_int();
        }

        args.push_back(value);
        skip_ws();
        if (i < text.size() && text[i] == ',') {
            i++;
            continue;
        }
        if (i < text.size() && text[i] == ']') {
            i++;
            break;
        }
        if (i >= text.size()) {
            throw std::runtime_error("Unterminated args_json array.");
        }
        throw std::runtime_error("Expected ',' or ']' in args_json array.");
    }

    return args;
}

int invoke_sms_event(
    const char* source,
    const char* target_id,
    const char* event_name,
    const char* args_json,
    std::int64_t* out_result,
    char* error,
    int error_capacity) {
    if (source == nullptr || out_result == nullptr || target_id == nullptr || event_name == nullptr) {
        write_error(error, error_capacity, "source/target_id/event_name/out_result must not be null");
        return 2;
    }

    try {
        auto runtime = build_session_runtime_or_throw(source);
        return invoke_event_on_runtime(*runtime, target_id, event_name, args_json, out_result, error, error_capacity);
    } catch (const std::exception& ex) {
        write_error(error, error_capacity, ex.what());
        return 1;
    } catch (...) {
        write_error(error, error_capacity, SmsGuruMeditation("unknown_error").what());
        return 1;
    }
}

} // namespace

extern "C" int sms_native_execute(const char* source, std::int64_t* out_result, char* error, int error_capacity) {
    return execute_sms_source(source, out_result, error, error_capacity);
}

extern "C" int sms_native_session_create(std::int64_t* out_session, char* error, int error_capacity) {
    if (out_session == nullptr) {
        write_error(error, error_capacity, "out_session must not be null");
        return 2;
    }

    std::lock_guard<std::mutex> lock(g_sessions_mutex);
    const auto session_id = g_next_session_id++;
    g_sessions.emplace(session_id, SmsSessionState{});
    *out_session = session_id;
    return 0;
}

extern "C" int sms_native_session_load(std::int64_t session, const char* source, char* error, int error_capacity) {
    if (source == nullptr) {
        write_error(error, error_capacity, "source must not be null");
        return 2;
    }

    try {
        auto runtime = build_session_runtime_or_throw(source);

        std::lock_guard<std::mutex> lock(g_sessions_mutex);
        const auto it = g_sessions.find(session);
        if (it == g_sessions.end()) {
            write_error(error, error_capacity, "invalid session");
            return 2;
        }

        it->second.source = source;
        it->second.runtime = std::move(runtime);
        return 0;
    } catch (const ReturnSignal&) {
        write_error(error, error_capacity, "return outside function");
        return 1;
    } catch (const BreakSignal&) {
        write_error(error, error_capacity, "break outside loop");
        return 1;
    } catch (const ContinueSignal&) {
        write_error(error, error_capacity, "continue outside loop");
        return 1;
    } catch (const std::exception& ex) {
        write_error(error, error_capacity, ex.what());
        return 1;
    } catch (...) {
        write_error(error, error_capacity, SmsGuruMeditation("unknown_error").what());
        return 1;
    }
}

extern "C" int sms_native_session_invoke(
    std::int64_t session,
    const char* target_id,
    const char* event_name,
    const char* args_json,
    std::int64_t* out_result,
    char* error,
    int error_capacity) {
    std::shared_ptr<SmsSessionRuntime> runtime;

    struct SessionInvokeGuard {
        std::int64_t session_id;
        bool armed = false;

        explicit SessionInvokeGuard(std::int64_t id) : session_id(id) {}

        void arm() { armed = true; }

        ~SessionInvokeGuard() {
            if (!armed) {
                return;
            }
            std::lock_guard<std::mutex> lock(g_sessions_mutex);
            const auto it = g_sessions.find(session_id);
            if (it != g_sessions.end()) {
                if (it->second.invoke_depth > 0) {
                    --it->second.invoke_depth;
                }
                if (it->second.invoke_depth == 0) {
                    g_llvm_string_arena.clear();
                }
            }
        }
    } invoke_guard(session);

    {
        std::lock_guard<std::mutex> lock(g_sessions_mutex);
        const auto it = g_sessions.find(session);
        if (it == g_sessions.end()) {
            write_error(error, error_capacity, "invalid session");
            return 2;
        }
        if (it->second.invoke_depth >= kMaxSessionInvokeDepth) {
            const std::string reentrant_error =
                "RuntimeError: sms_native_session_invoke recursion limit exceeded for session "
                + std::to_string(session)
                + " (possible infinite loop in event handler; nested invoke depth "
                + std::to_string(it->second.invoke_depth) + ").";
            write_error(
                error,
                error_capacity,
                reentrant_error.c_str());
            return 1;
        }
        ++it->second.invoke_depth;
        invoke_guard.arm();
        runtime = it->second.runtime;
    }

    if (!runtime) {
        write_error(error, error_capacity, "session has no loaded source");
        return 2;
    }

    return invoke_event_on_runtime(*runtime, target_id, event_name, args_json, out_result, error, error_capacity);
}

extern "C" int sms_native_session_dispose(std::int64_t session, char* error, int error_capacity) {
    std::lock_guard<std::mutex> lock(g_sessions_mutex);
    if (g_sessions.erase(session) == 0) {
        write_error(error, error_capacity, "invalid session");
        return 2;
    }
    return 0;
}

extern "C" int sms_native_session_has_ahimsa(std::int64_t session, char* error, int error_capacity) {
    std::lock_guard<std::mutex> lock(g_sessions_mutex);
    const auto it = g_sessions.find(session);
    if (it == g_sessions.end()) {
        write_error(error, error_capacity, "invalid session");
        return -1;
    }
    if (!it->second.runtime) {
        write_error(error, error_capacity, "session has no loaded source");
        return -1;
    }
    return it->second.runtime->has_ahimsa ? 1 : 0;
}

extern "C" int sms_native_set_ui_callbacks(
    sms_native_ui_get_prop_fn get_prop,
    sms_native_ui_set_prop_fn set_prop,
    sms_native_ui_invoke_fn invoke,
    char*,
    int) {
    g_ui_get_prop = get_prop;
    g_ui_set_prop = set_prop;
    g_ui_invoke = invoke;
    if (get_prop == nullptr && set_prop == nullptr && invoke == nullptr) {
        g_ui_get_string_prop = nullptr;
        g_ui_set_string_prop = nullptr;
    }
    return 0;
}

extern "C" int sms_native_set_ui_string_callbacks(
    sms_native_ui_get_string_prop_fn get_string_prop,
    sms_native_ui_set_string_prop_fn set_string_prop,
    char*,
    int) {
    g_ui_get_string_prop = get_string_prop;
    g_ui_set_string_prop = set_string_prop;
    return 0;
}

extern "C" int sms_native_set_sandbox_path_callback(
    sms_native_sandbox_path_allow_fn allow_path,
    char*,
    int) {
    g_sandbox_path_allow = allow_path;
    return 0;
}

extern "C" SMS_EXPORT const char* sms_native_llvm_ui_get_object(const char* object_id) {
    return object_id != nullptr ? object_id : "";
}

extern "C" SMS_EXPORT std::int64_t sms_native_llvm_os_now_ms() {
    const auto now = std::chrono::steady_clock::now().time_since_epoch();
    return static_cast<std::int64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(now).count());
}

extern "C" SMS_EXPORT const char* sms_native_llvm_string_concat(const char* left, const char* right) {
    const std::string l = left != nullptr ? left : "";
    const std::string r = right != nullptr ? right : "";
    g_llvm_string_arena.push_back(l + r);
    return g_llvm_string_arena.back().c_str();
}

extern "C" SMS_EXPORT std::int64_t sms_native_llvm_string_eq(const char* left, const char* right) {
    const char* l = left != nullptr ? left : "";
    const char* r = right != nullptr ? right : "";
    return std::strcmp(l, r) == 0 ? 1 : 0;
}

extern "C" SMS_EXPORT int sms_native_llvm_set_ui_string_prop(
    const char* object_id,
    const char* property_name,
    const char* value_text) {
    if (object_id == nullptr || property_name == nullptr || value_text == nullptr) {
        return 2;
    }
    char error[1024] = {0};
    if (g_ui_set_string_prop != nullptr) {
        const auto rc = g_ui_set_string_prop(
            object_id,
            property_name,
            value_text,
            error,
            static_cast<int>(sizeof(error)));
        return rc == 0 ? 0 : 1;
    }
    if (g_ui_set_prop != nullptr) {
        const auto payload = value_to_json(Value::String(value_text));
        const auto rc = g_ui_set_prop(
            object_id,
            property_name,
            payload.c_str(),
            error,
            static_cast<int>(sizeof(error)));
        return rc == 0 ? 0 : 1;
    }
    return 2;
}

extern "C" SMS_EXPORT int sms_native_llvm_set_ui_text(const char* object_id, const char* value_text) {
    return sms_native_llvm_set_ui_string_prop(object_id, "text", value_text);
}

extern "C" SMS_EXPORT int sms_native_llvm_set_ui_int_prop(
    const char* object_id,
    const char* property_name,
    int64_t value) {
    if (object_id == nullptr || property_name == nullptr) {
        return 2;
    }
    char error[1024] = {0};
    if (g_ui_set_prop != nullptr) {
        const std::string payload = std::to_string(value);
        const auto rc = g_ui_set_prop(
            object_id,
            property_name,
            payload.c_str(),
            error,
            static_cast<int>(sizeof(error)));
        return rc == 0 ? 0 : 1;
    }
    return 2;
}

extern "C" int sms_native_codegen_llvm_ir(
    const char* source,
    char* out_ir,
    int out_ir_capacity,
    char* error,
    int error_capacity) {
    if (source == nullptr || out_ir == nullptr) {
        write_error(error, error_capacity, "source/out_ir must not be null");
        return 2;
    }
    try {
        Lexer lexer(source);
        auto tokens = lexer.tokenize();
        Parser parser(std::move(tokens));
        auto program = parser.parse_program();
        const auto ir = codegen_program_llvm_ir(program, true);
        if (static_cast<int>(ir.size()) >= out_ir_capacity) {
            write_error(error, error_capacity, "output buffer too small for generated IR");
            return 1;
        }
        std::memcpy(out_ir, ir.data(), ir.size());
        out_ir[ir.size()] = '\0';
        return 0;
    } catch (const std::exception& ex) {
        write_error(error, error_capacity, ex.what());
        return 1;
    } catch (...) {
        write_error(error, error_capacity, "unknown llvm codegen exception");
        return 1;
    }
}

extern "C" int sms_native_codegen_llvm_ir_mode(
    const char* source,
    const char* mode,
    char* out_ir,
    int out_ir_capacity,
    char* error,
    int error_capacity) {
    if (source == nullptr || out_ir == nullptr || mode == nullptr) {
        write_error(error, error_capacity, "source/mode/out_ir must not be null");
        return 2;
    }
    const std::string mode_value(mode);
    const bool emit_exe_entry = mode_value == "exe";
    const bool emit_lib_module = mode_value == "lib";
    if (!emit_exe_entry && !emit_lib_module) {
        write_error(error, error_capacity, "unsupported llvm codegen mode (expected 'exe' or 'lib')");
        return 2;
    }
    try {
        Lexer lexer(source);
        auto tokens = lexer.tokenize();
        Parser parser(std::move(tokens));
        auto program = parser.parse_program();
        const auto ir = codegen_program_llvm_ir(program, emit_exe_entry);
        if (static_cast<int>(ir.size()) >= out_ir_capacity) {
            write_error(error, error_capacity, "output buffer too small for generated IR");
            return 1;
        }
        std::memcpy(out_ir, ir.data(), ir.size());
        out_ir[ir.size()] = '\0';
        return 0;
    } catch (const std::exception& ex) {
        write_error(error, error_capacity, ex.what());
        return 1;
    } catch (...) {
        write_error(error, error_capacity, "unknown llvm codegen exception");
        return 1;
    }
}

extern "C" int sms_native_aot_invoke(
    const char* source,
    const char* target_id,
    const char* event_name,
    const char* args_json,
    std::int64_t* out_result,
    char* error,
    int error_capacity) {
    if (source == nullptr || target_id == nullptr || event_name == nullptr || out_result == nullptr) {
        write_error(error, error_capacity, "source/target_id/event_name/out_result must not be null");
        return 2;
    }

    const std::string invoke_key = std::string(target_id) + "." + std::string(event_name);
    (void)args_json;

    struct AotArenaGuard {
        ~AotArenaGuard() { g_llvm_string_arena.clear(); }
    } arena_guard;

    std::lock_guard<std::mutex> lock(g_aot_mutex);
    try {
        Lexer lexer(source);
        auto tokens = lexer.tokenize();
        Parser parser(std::move(tokens));
        auto program = parser.parse_program();
        const auto ir = codegen_program_llvm_ir(program, true);

        const fs::path cache_dir = resolve_sms_aot_cache_dir();
        std::error_code ec;
        fs::create_directories(cache_dir, ec);
        if (ec) {
            write_error(error, error_capacity, "failed to create sms aot cache dir: " + cache_dir.string());
            return 1;
        }

        const std::string key = sha256_hex(std::string(source));
        const fs::path ir_path = cache_dir / (key + ".ll");
        const fs::path lib_path = cache_dir / (key + shared_lib_extension());

        if (!fs::exists(lib_path)) {
            {
                std::ofstream out(ir_path, std::ios::binary);
                if (!out.is_open()) {
                    write_error(error, error_capacity, "failed to write IR file: " + ir_path.string());
                    return 1;
                }
                out << ir;
            }

            const std::string clang = detect_clang_command();
#if defined(_WIN32)
            const std::string compile_cmd =
                clang + " -O2 -shared -o " + shell_quote_arg(lib_path.string()) + " "
                + shell_quote_arg(ir_path.string()) + " 2>&1";
#else
            const std::string compile_cmd =
                clang + " -O2 -shared -fPIC -o " + shell_quote_arg(lib_path.string()) + " "
                + shell_quote_arg(ir_path.string()) + " 2>&1";
#endif
            int compile_rc = -1;
            const std::string compile_out = capture_command_output(compile_cmd, &compile_rc);
            if (compile_rc != 0) {
                write_error(error, error_capacity,
                    "aot clang build failed for '" + invoke_key + "' ("
                    + std::to_string(compile_rc) + "): " + compile_out);
                return 1;
            }
        }

        void* lib_handle = load_shared_lib(lib_path.string());
        if (lib_handle == nullptr) {
            write_error(error, error_capacity, "aot failed to load shared library: " + lib_path.string());
            return 1;
        }
        struct ScopedLib {
            void* handle = nullptr;
            explicit ScopedLib(void* h) : handle(h) {}
            ~ScopedLib() { close_shared_lib(handle); }
        } scoped_lib(lib_handle);

        using SmsCompiledMainFn = int (*)();
        auto compiled_main = reinterpret_cast<SmsCompiledMainFn>(load_shared_symbol(lib_handle, "main"));
        if (compiled_main == nullptr) {
            write_error(error, error_capacity, "aot shared library missing symbol 'main'");
            return 1;
        }

        *out_result = static_cast<std::int64_t>(compiled_main());
        return 0;
    } catch (const std::exception& ex) {
        write_error(error, error_capacity, ex.what());
        return 1;
    } catch (...) {
        write_error(error, error_capacity, "unknown aot invoke exception");
        return 1;
    }
}

extern "C" int sms_native_codegen_cpp(
    const char* source,
    char* out_cpp,
    int out_cpp_capacity,
    char* error,
    int error_capacity) {
    if (source == nullptr || out_cpp == nullptr) {
        write_error(error, error_capacity, "source/out_cpp must not be null");
        return 2;
    }
    try {
        Lexer lexer(source);
        auto tokens = lexer.tokenize();
        Parser parser(std::move(tokens));
        auto program = parser.parse_program();
        const auto cpp = codegen_program_cpp(program);
        if (static_cast<int>(cpp.size()) >= out_cpp_capacity) {
            write_error(error, error_capacity, "output buffer too small for generated C++");
            return 1;
        }
        std::memcpy(out_cpp, cpp.data(), cpp.size());
        out_cpp[cpp.size()] = '\0';
        return 0;
    } catch (const std::exception& ex) {
        write_error(error, error_capacity, ex.what());
        return 1;
    } catch (...) {
        write_error(error, error_capacity, "unknown codegen exception");
        return 1;
    }
}

extern "C" int sms_native_sml_parse(const char* source, std::int64_t* out_node_count, char* error, int error_capacity) {
    if (source == nullptr || out_node_count == nullptr) {
        write_error(error, error_capacity, "source/out_node_count must not be null");
        return 2;
    }

    try {
        *out_node_count = count_sml_nodes(source);
        return 0;
    } catch (const std::exception& ex) {
        write_error(error, error_capacity, ex.what());
        return 1;
    } catch (...) {
        write_error(error, error_capacity, SmsGuruMeditation("unknown_error").what());
        return 1;
    }
}
