/**
 * bridge.cpp — Emscripten WASM export (C++ analysis engine for the browser)
 */

#include "relation_engine.hpp"
#include <string>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

namespace {

std::string json_escape(const std::string& s) {
    std::string out;
    out.reserve(s.size() + 8);
    for (char c : s) {
        switch (c) {
            case '"':  out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n";  break;
            case '\r': out += "\\r";  break;
            case '\t': out += "\\t";  break;
            default:   out += c;      break;
        }
    }
    return out;
}

std::string error_json(const std::string& msg) {
    return std::string("{\"error\":\"") + json_escape(msg) + "\"}";
}

} // namespace

extern "C" {

#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
#endif
const char* analyse(const char* set_raw, const char* pairs_raw) {
    static thread_local std::string buffer;
    try {
        buffer = math::analyse_json(
            set_raw ? set_raw : "",
            pairs_raw ? pairs_raw : ""
        );
        return buffer.c_str();
    } catch (const std::exception& e) {
        buffer = error_json(e.what());
        return buffer.c_str();
    } catch (...) {
        buffer = error_json("Unknown analysis error");
        return buffer.c_str();
    }
}

} // extern "C"
