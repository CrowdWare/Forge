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

#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "cmd_build.h"
#include "sandbox_policy.h"

#if defined(_WIN32)
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace fs = std::filesystem;

using SmlParseFn = int (*)(const char*, std::int64_t*, char*, int);
using SmsSessionCreateFn = int (*)(std::int64_t*, char*, int);
using SmsSessionLoadFn = int (*)(std::int64_t, const char*, char*, int);
using SmsSessionDisposeFn = int (*)(std::int64_t, char*, int);
using SmsSandboxPathAllowFn = int (*)(const char*, const char*, char*, int);
using SmsSetSandboxPathCallbackFn = int (*)(SmsSandboxPathAllowFn, char*, int);

static forgecli::SandboxRoots g_sandbox_roots;

static int sandbox_allow_path(const char* owner, const char* uri_path, char* error, int error_capacity) {
    return forgecli::sandbox_allow_path(g_sandbox_roots, owner, uri_path, error, error_capacity);
}

static void* load_symbol(void* lib, const char* name) {
#if defined(_WIN32)
    return reinterpret_cast<void*>(GetProcAddress(reinterpret_cast<HMODULE>(lib), name));
#else
    return dlsym(lib, name);
#endif
}

static void* load_lib(const std::string& file) {
#if defined(_WIN32)
    return reinterpret_cast<void*>(LoadLibraryA(file.c_str()));
#else
    return dlopen(file.c_str(), RTLD_NOW);
#endif
}

static std::string ext() {
#if defined(_WIN32)
    return ".dll";
#elif defined(__APPLE__)
    return ".dylib";
#else
    return ".so";
#endif
}

static bool parse_sml(SmlParseFn fn, const fs::path& p, std::string& err) {
    if (!fs::exists(p)) {
        err = "File not found.";
        return false;
    }
    std::ifstream in(p, std::ios::binary);
    std::string src((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    char e[2048] = {0};
    std::int64_t nodes = 0;
    if (fn(src.c_str(), &nodes, e, static_cast<int>(sizeof(e))) == 0) {
        return true;
    }
    err = e[0] ? std::string(e) : "SML parse failed";
    return false;
}

static bool parse_sms(SmsSessionCreateFn create_fn, SmsSessionLoadFn load_fn, SmsSessionDisposeFn dispose_fn, const fs::path& p, std::string& err) {
    if (!fs::exists(p)) {
        err = "File not found.";
        return false;
    }
    std::ifstream in(p, std::ios::binary);
    std::string src((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    char e[2048] = {0};
    std::int64_t session = 0;
    if (create_fn(&session, e, static_cast<int>(sizeof(e))) != 0) {
        err = e[0] ? std::string(e) : "sms session create failed";
        return false;
    }
    int rc = load_fn(session, src.c_str(), e, static_cast<int>(sizeof(e)));
    dispose_fn(session, e, static_cast<int>(sizeof(e)));
    if (rc == 0) {
        return true;
    }
    err = e[0] ? std::string(e) : "SMS parse failed";
    return false;
}

static int cmd_new(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cerr << "Missing project name.\n";
        return 1;
    }
    fs::path out = fs::current_path() / args[0];
    bool force = false;
    for (std::size_t i = 1; i < args.size(); i++) {
        if (args[i] == "--force") {
            force = true;
        } else if (args[i] == "--output" && i + 1 < args.size()) {
            out = args[++i];
        } else {
            std::cerr << "Unknown option: " << args[i] << "\n";
            return 1;
        }
    }
    std::error_code ec;
    if (fs::exists(out) && !fs::is_empty(out, ec) && !force) {
        std::cerr << "Directory not empty. Use --force.\n";
        return 1;
    }
    fs::create_directories(out / "assets", ec);

    std::ofstream(out / "app.sml")
        << "SplashScreen {\n"
           "    id: splashScreen\n"
           "    size: 640, 480\n"
           "    duration: 500\n"
           "    loadOnReady: \"main.sml\"\n"
           "\n"
           "    VBoxContainer {\n"
           "        anchors: left | top | right | bottom\n"
           "        padding: 20, 20, 20, 20\n"
           "\n"
           "        Control { sizeFlagsVertical: expandFill }\n"
           "\n"
           "        Label {\n"
           "            text: \"Loading...\"\n"
           "            sizeFlagsHorizontal: shrinkCenter\n"
           "        }\n"
           "\n"
           "        Control { sizeFlagsVertical: expandFill }\n"
           "    }\n"
           "}\n";

    std::ofstream(out / "main.sml")
        << "Window {\n"
           "    id: mainWindow\n"
           "    title: @Strings.windowTitle, \"" << args[0] << "\"\n"
           "    minSize: 900, 600\n"
           "    size: 1200, 800\n"
           "\n"
           "    DockingHost {\n"
           "        id: mainDockHost\n"
           "        anchors: left | top | right | bottom\n"
           "\n"
           "        DockingContainer {\n"
           "            id: centerDock\n"
           "            dockSide: center\n"
           "            flex: true\n"
           "            closeable: false\n"
           "\n"
           "            Viewport3D {\n"
           "                id: viewport\n"
           "                centerDock.title: \"Viewport\"\n"
           "                anchors: left | top | right | bottom\n"
           "            }\n"
           "        }\n"
           "    }\n"
           "}\n";

    std::ofstream(out / "main.sms")
        << "fun ready() {\n"
           "    log.info(\"Forge app ready\")\n"
           "}\n";

    std::ofstream(out / "theme.sml")
        << "Colors {\n"
           "    accent: \"#28A9E0\"\n"
           "}\n";

    std::ofstream(out / "strings.sml")
        << "Strings {\n"
           "    windowTitle: \"" << args[0] << "\"\n"
           "}\n";

    std::ofstream(out / "README.md")
        << "# " << args[0] << "\n"
           "\n"
           "Generated by ForgeCli.\n"
           "\n"
           "## Files\n"
           "\n"
           "- app.sml (startup splash)\n"
           "- main.sml (main UI)\n"
           "- main.sms (event logic)\n"
           "- theme.sml (theme overrides)\n"
           "- strings.sml (localized strings)\n"
           "\n"
           "## Validate\n"
           "\n"
           "```bash\n"
           "forgecli validate --project .\n"
           "```\n"
           "\n"
           "## Generate from AI prompt\n"
           "\n"
           "```bash\n"
           "forgecli generate --project . --provider mock --prompt \"create a window with docking and centered viewport3d\"\n"
           "```\n";

    std::cout << "Created scaffold at '" << fs::absolute(out).string() << "'.\n";
    return 0;
}

static int cmd_validate(const std::vector<std::string>& args) {
    fs::path project = fs::current_path();
    for (std::size_t i = 0; i < args.size(); i++) {
        if (args[i] == "--project" && i + 1 < args.size()) {
            project = args[++i];
        } else if (args[i] != "--verbose") {
            std::cerr << "Unknown option: " << args[i] << "\n";
            return 1;
        }
    }

    const char* sml_dir = std::getenv("SML_NATIVE_LIB_DIR");
    const char* sms_dir = std::getenv("SMS_NATIVE_LIB_DIR");
    if (!sml_dir || !sms_dir) {
        std::cerr << "Set SML_NATIVE_LIB_DIR and SMS_NATIVE_LIB_DIR.\n";
        return 1;
    }

    const std::string sml_lib_path = (fs::path(sml_dir) / ("libsmlcore_native" + ext())).string();
    const std::string sms_lib_path = (fs::path(sms_dir) / ("libsms_native" + ext())).string();
    void* sml_lib = load_lib(sml_lib_path);
    void* sms_lib = load_lib(sms_lib_path);
    if (!sml_lib || !sms_lib) {
        std::cerr << "Failed to load native libraries.\n";
        return 1;
    }

    auto sml_parse = reinterpret_cast<SmlParseFn>(load_symbol(sml_lib, "smlcore_native_parse"));
    auto sms_create = reinterpret_cast<SmsSessionCreateFn>(load_symbol(sms_lib, "sms_native_session_create"));
    auto sms_load = reinterpret_cast<SmsSessionLoadFn>(load_symbol(sms_lib, "sms_native_session_load"));
    auto sms_dispose = reinterpret_cast<SmsSessionDisposeFn>(load_symbol(sms_lib, "sms_native_session_dispose"));
    auto sms_set_sandbox = reinterpret_cast<SmsSetSandboxPathCallbackFn>(load_symbol(sms_lib, "sms_native_set_sandbox_path_callback"));
    if (!sml_parse || !sms_create || !sms_load || !sms_dispose || !sms_set_sandbox) {
        std::cerr << "Missing native symbol(s).\n";
        return 1;
    }

    std::string sandbox_init_error;
    if (!forgecli::initialize_sandbox_roots(project, g_sandbox_roots, sandbox_init_error)) {
        std::cerr << sandbox_init_error << "\n";
        return 1;
    }

    char sandbox_error[2048] = {0};
    if (sms_set_sandbox(&sandbox_allow_path, sandbox_error, static_cast<int>(sizeof(sandbox_error))) != 0) {
        std::cerr << (sandbox_error[0] ? sandbox_error : "Failed to register SMS sandbox callback.") << "\n";
        return 1;
    }

    bool ok = true;
    std::string err;
    const fs::path app = project / "app.sml";
    const fs::path main_sml = project / "main.sml";
    const fs::path main_sms = project / "main.sms";

    if (parse_sml(sml_parse, app, err)) std::cout << "[OK]   " << app.string() << "\n";
    else { std::cout << "[FAIL] " << app.string() << "\n  error: " << err << "\n"; ok = false; }
    err.clear();
    if (parse_sml(sml_parse, main_sml, err)) std::cout << "[OK]   " << main_sml.string() << "\n";
    else { std::cout << "[FAIL] " << main_sml.string() << "\n  error: " << err << "\n"; ok = false; }
    err.clear();
    if (parse_sms(sms_create, sms_load, sms_dispose, main_sms, err)) std::cout << "[OK]   " << main_sms.string() << "\n";
    else { std::cout << "[FAIL] " << main_sms.string() << "\n  error: " << err << "\n"; ok = false; }

    std::cout << "\n" << (ok ? "Validation passed." : "Validation failed.") << "\n";
    return ok ? 0 : 2;
}

static void help() {
    std::cout << "forgecli-native\n\n";
    std::cout << "Usage:\n";
    std::cout << "  forgecli-native new <name> [--output <dir>] [--force]\n";
    std::cout << "  forgecli-native validate [--project <dir>]\n";
    std::cout << "  forgecli-native build mac     --project <dir> [--output <path>] [--godot-version <ver>]\n";
    std::cout << "  forgecli-native build android --project <dir> [--output <path>] [--godot-version <ver>]\n";
}

int main(int argc, char** argv) {
    if (argc < 2) {
        help();
        return 0;
    }
    std::string cmd = argv[1];
    std::vector<std::string> args;
    for (int i = 2; i < argc; i++) args.emplace_back(argv[i]);

    if (cmd == "new") return cmd_new(args);
    if (cmd == "validate") return cmd_validate(args);
    if (cmd == "build") return cmd_build(args);
    if (cmd == "help" || cmd == "--help" || cmd == "-h") {
        help();
        return 0;
    }
    std::cerr << "Unknown command: " << cmd << "\n";
    return 1;
}
