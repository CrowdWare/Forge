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

#include "forge_markdown.h"

#include <sstream>

namespace forge {

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static std::string normalize_newlines(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (std::size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '\r') {
            out += '\n';
            if (i + 1 < s.size() && s[i + 1] == '\n') ++i;
        } else {
            out += s[i];
        }
    }
    return out;
}

static bool starts_with(const std::string& s, const char* prefix) {
    std::size_t n = 0;
    while (prefix[n]) {
        if (n >= s.size() || s[n] != prefix[n]) return false;
        ++n;
    }
    return true;
}

static std::string ltrim(const std::string& s) {
    std::size_t i = 0;
    while (i < s.size() && (s[i] == ' ' || s[i] == '\t')) ++i;
    return s.substr(i);
}

static std::string rtrim(const std::string& s) {
    std::size_t i = s.size();
    while (i > 0 && (s[i - 1] == ' ' || s[i - 1] == '\t')) --i;
    return s.substr(0, i);
}

static bool is_blank(const std::string& s) {
    for (char c : s) if (c != ' ' && c != '\t' && c != '\r') return false;
    return true;
}

// ---------------------------------------------------------------------------
// Block parsers
// ---------------------------------------------------------------------------

static bool try_heading(const std::string& line, MarkdownBlock& out) {
    const std::string t = ltrim(line);
    int level = 0;
    while (level < (int)t.size() && level < 6 && t[level] == '#') ++level;
    if (level == 0 || level >= (int)t.size() || t[level] != ' ') return false;
    out.kind  = BlockKind::Heading;
    out.level = level;
    out.text  = rtrim(t.substr(level + 1));
    return true;
}

static bool try_hrule(const std::string& line) {
    const std::string t = ltrim(line);
    if (t.size() < 3) return false;
    char c = t[0];
    if (c != '-' && c != '*' && c != '_') return false;
    for (char x : t) if (x != c && x != ' ') return false;
    return true;
}

static bool try_image(const std::string& line, MarkdownBlock& out) {
    const std::string t = rtrim(ltrim(line));
    if (!starts_with(t, "![") || t.back() != ')') return false;
    std::size_t alt_close = t.find(']');
    if (alt_close == std::string::npos || alt_close < 2) return false;
    std::size_t paren = t.find('(', alt_close + 1);
    if (paren == std::string::npos) return false;
    out.kind = BlockKind::Image;
    out.alt  = t.substr(2, alt_close - 2);
    out.src  = rtrim(ltrim(t.substr(paren + 1, t.size() - paren - 2)));
    return !out.src.empty();
}

static bool try_list_item(const std::string& line, MarkdownBlock& out) {
    const std::string t = ltrim(line);
    if (!starts_with(t, "- ") && !starts_with(t, "* ")) return false;
    out.kind = BlockKind::ListItem;
    out.text = t.substr(2);
    return true;
}

static bool try_code_fence_open(const std::string& line, std::string& lang_out) {
    const std::string t = ltrim(line);
    if (!starts_with(t, "```")) return false;
    lang_out = rtrim(ltrim(t.substr(3)));
    return true;
}

// ---------------------------------------------------------------------------
// parse_markdown
// ---------------------------------------------------------------------------

std::vector<MarkdownBlock> parse_markdown(const std::string& md) {
    const std::string norm = normalize_newlines(md);

    std::vector<std::string> lines;
    {
        std::istringstream ss(norm);
        std::string ln;
        while (std::getline(ss, ln)) lines.push_back(ln);
    }

    std::vector<MarkdownBlock> blocks;
    int i = 0;
    const int N = (int)lines.size();

    while (i < N) {
        const std::string& line = lines[i];

        if (is_blank(line)) { ++i; continue; }

        // Fenced code block
        std::string lang;
        if (try_code_fence_open(line, lang)) {
            ++i;
            std::string code_text;
            while (i < N) {
                if (starts_with(ltrim(lines[i]), "```")) { ++i; break; }
                if (!code_text.empty()) code_text += '\n';
                code_text += lines[i];
                ++i;
            }
            MarkdownBlock b;
            b.kind = BlockKind::CodeBlock;
            b.lang = lang;
            b.text = code_text;
            blocks.push_back(std::move(b));
            continue;
        }

        // Horizontal rule
        if (try_hrule(line)) {
            MarkdownBlock b; b.kind = BlockKind::HRule;
            blocks.push_back(b);
            ++i;
            continue;
        }

        // Heading
        {
            MarkdownBlock b;
            if (try_heading(line, b)) { blocks.push_back(b); ++i; continue; }
        }

        // Image
        {
            MarkdownBlock b;
            if (try_image(line, b)) { blocks.push_back(b); ++i; continue; }
        }

        // List item
        {
            MarkdownBlock b;
            if (try_list_item(line, b)) { blocks.push_back(b); ++i; continue; }
        }

        // Paragraph: accumulate until blank line or next block marker
        {
            std::string para;
            bool last_hard_break = false;
            while (i < N) {
                const std::string& ln = lines[i];
                if (is_blank(ln)) break;
                const std::string t = ltrim(ln);
                if (starts_with(t, "#")   ||
                    starts_with(t, "- ")  ||
                    starts_with(t, "* ")  ||
                    starts_with(t, "![")  ||
                    starts_with(t, "```") ||
                    try_hrule(t))
                    break;
                if (!para.empty())
                    para += last_hard_break ? '\n' : ' ';
                bool hard_break = ln.size() >= 2 &&
                                  ln[ln.size()-1] == ' ' &&
                                  ln[ln.size()-2] == ' ';
                para += rtrim(ln);
                last_hard_break = hard_break;
                ++i;
            }
            if (!para.empty()) {
                MarkdownBlock b;
                b.kind = BlockKind::Paragraph;
                b.text = para;
                blocks.push_back(std::move(b));
            }
        }
    }

    return blocks;
}

// ---------------------------------------------------------------------------
// replace_emoji
// ---------------------------------------------------------------------------

static std::string replace_emoji(const std::string& src) {
    struct Entry { const char* key; const char* utf8; };
    static const Entry map[] = {
        { "smile",   "\xF0\x9F\x98\x84" },   // 😄
        { "warning", "\xE2\x9A\xA0\xEF\xB8\x8F" }, // ⚠️
        { "rocket",  "\xF0\x9F\x9A\x80" },   // 🚀
        { "heart",   "\xE2\x9D\xA4\xEF\xB8\x8F" }, // ❤️
    };
    std::string out;
    std::size_t i = 0;
    const std::size_t n = src.size();
    while (i < n) {
        if (src[i] == ':') {
            std::size_t end = src.find(':', i + 1);
            if (end != std::string::npos && end > i + 1) {
                std::string key = src.substr(i + 1, end - i - 1);
                bool found = false;
                for (const auto& e : map) {
                    if (key == e.key) {
                        out += e.utf8;
                        i = end + 1;
                        found = true;
                        break;
                    }
                }
                if (found) continue;
            }
        }
        out += src[i++];
    }
    return out;
}

// ---------------------------------------------------------------------------
// inline_to_bbcode
// ---------------------------------------------------------------------------

std::string inline_to_bbcode(const std::string& raw) {
    const std::string src = replace_emoji(raw);
    std::string out;
    out.reserve(src.size() * 2);
    std::size_t i = 0;
    const std::size_t n = src.size();

    while (i < n) {
        // Inline code: `...` (no further parsing inside)
        if (src[i] == '`') {
            std::size_t end = src.find('`', i + 1);
            if (end != std::string::npos) {
                out += "[code]";
                for (std::size_t j = i + 1; j < end; ++j) out += src[j];
                out += "[/code]";
                i = end + 1;
                continue;
            }
        }

        // Bold+Italic: ***...***  (must check before ** and *)
        if (i + 2 < n && src[i] == '*' && src[i+1] == '*' && src[i+2] == '*') {
            std::size_t end = src.find("***", i + 3);
            if (end != std::string::npos) {
                out += "[b][i]";
                out += inline_to_bbcode(src.substr(i + 3, end - i - 3));
                out += "[/i][/b]";
                i = end + 3;
                continue;
            }
        }

        // Bold: **...**
        if (i + 1 < n && src[i] == '*' && src[i+1] == '*') {
            std::size_t end = src.find("**", i + 2);
            if (end != std::string::npos) {
                out += "[b]";
                out += inline_to_bbcode(src.substr(i + 2, end - i - 2));
                out += "[/b]";
                i = end + 2;
                continue;
            }
        }

        // Italic: *...* (single asterisk)
        if (src[i] == '*') {
            std::size_t end = src.find('*', i + 1);
            if (end != std::string::npos) {
                out += "[i]";
                out += inline_to_bbcode(src.substr(i + 1, end - i - 1));
                out += "[/i]";
                i = end + 1;
                continue;
            }
        }

        // Italic: _..._
        if (src[i] == '_') {
            std::size_t end = src.find('_', i + 1);
            if (end != std::string::npos) {
                out += "[i]";
                out += inline_to_bbcode(src.substr(i + 1, end - i - 1));
                out += "[/i]";
                i = end + 1;
                continue;
            }
        }

        // Link: [label](url)
        if (src[i] == '[') {
            std::size_t label_end = src.find(']', i + 1);
            if (label_end != std::string::npos &&
                label_end + 1 < n && src[label_end + 1] == '(') {
                std::size_t url_end = src.find(')', label_end + 2);
                if (url_end != std::string::npos) {
                    std::string label = src.substr(i + 1, label_end - i - 1);
                    std::string url   = src.substr(label_end + 2, url_end - label_end - 2);
                    out += "[url=" + url + "]" + label + "[/url]";
                    i = url_end + 1;
                    continue;
                }
            }
            // Not a link - escape bracket
            out += "\\[";
            ++i;
            continue;
        }

        if (src[i] == ']') {
            out += "\\]";
            ++i;
            continue;
        }

        out += src[i++];
    }

    return out;
}

} // namespace forge
