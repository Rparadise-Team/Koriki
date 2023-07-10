#include "config.h"

#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <unordered_map>

#include <SDL.h>

#include "utf8.h"

namespace {

inline const char *skipLeadingWhitespace(const char *begin, const char *end)
{
    while (begin != end && (*begin == ' ' || *begin == '\t')) ++begin;
    return begin;
}

inline const char *skipTrailingWhitespace(const char *begin, const char *end)
{
    while (begin != end && (*(end - 1) == ' ' || *(end - 1) == '\t')) --end;
    return end;
}

bool parseConfigLine(
    int line_num, const std::string &line, std::string *key, std::string *value)
{
    const char *line_begin = line.data();
    const char *line_end = line.data() + line.size();
    const char *key_begin = skipLeadingWhitespace(line_begin, line_end);
    if (key_begin == line_end || *key_begin == '#') return false;

    const char *eq_pos = reinterpret_cast<const char *>(std::memchr(
        reinterpret_cast<const void *>(key_begin), '=', line_end - key_begin));
    if (eq_pos == nullptr) {
        std::fprintf(stderr,
            "Config file parse error: key '%.*s' has no value on line %d\n",
            (int)(line_end - key_begin), key_begin, line_num + 1);
        return false;
    }
    const char *key_end = skipTrailingWhitespace(key_begin, eq_pos);
    if (key_end - key_begin == 0) {
        std::cerr << "Config file parse error: empty key on line "
                  << line_num + 1 << "\n";
        return false;
    }

    const char *value_begin = skipLeadingWhitespace(eq_pos + 1, line_end);
    const char *value_end = line_end;
    if (value_end > value_begin && (*(value_end - 1) == '\r')) --value_end;

    key->assign(key_begin, key_end - key_begin);
    value->assign(value_begin, value_end - value_begin);
    return true;
}

std::map<std::string, std::string> loadConfigFile(const std::string &path)
{
    std::map<std::string, std::string> result;
    std::ifstream input_file(path.c_str(), std::ios::binary);
    if (!input_file.is_open()) {
        std::cerr << path << " " << std::strerror(errno) << "\n";
        return result;
    }
    std::cerr << "Reading settings from " << path << "\n";
    std::string line, key, value;
    int line_i = 0;
    while (!input_file.eof()) {
        std::getline(input_file, line);
        if (line_i == 0) utf8::removeBom(&line);
        if (parseConfigLine(line_i, line, &key, &value)) result[key] = value;
        ++line_i;
    }
    return result;
}

void processEnvValue(std::string *value)
{
    if (!value->empty() && value->front() == '$')
        value->assign(std::getenv(value->c_str() + 1));
}

bool parseBool(const std::string &value)
{
    return value == "1" || value == "true";
}

SDLC_Keycode parseKeycode(const std::string &value)
{
    // We support a few common SDLK codes as config values.
    // We then allow single-char keys and ints for the rest.
    static const std::unordered_map<std::string, SDLC_Keycode> kSdlkToInt {
        { "BACKSPACE", SDLK_BACKSPACE },
        { "DOWN", SDLK_DOWN },
        { "ESCAPE", SDLK_ESCAPE },
        { "INSERT", SDLK_INSERT },
        { "LALT", SDLK_LALT },
        { "LCTRL", SDLK_LCTRL },
        { "LEFT", SDLK_LEFT },
        { "LSHIFT", SDLK_LSHIFT },
        { "PAGEDOWN", SDLK_PAGEDOWN },
        { "PAGEUP", SDLK_PAGEUP },
        { "RETURN", SDLK_RETURN },
        { "RIGHT", SDLK_RIGHT },
        { "SPACE", SDLK_SPACE },
        { "TAB", SDLK_TAB },
        { "UP", SDLK_UP },
    };

    const auto sdlk_it = kSdlkToInt.find(value);
    if (sdlk_it != kSdlkToInt.end()) return sdlk_it->second;

    if (value.size() == 1 && value[0] >= 'a' && value[0] <= 'z')
        return static_cast<SDLC_Keycode>(SDLK_a + (value[0] - 'a'));

    return static_cast<SDLC_Keycode>(std::stoi(value));
}

} // namespace

Config &config()
{
    static Config global_config;
    return global_config;
}

#define CFG_INT(KEY)                                                           \
    if ((it = m.find(#KEY)) != m.end()) {                                      \
        this->KEY = std::stoi(it->second);                                     \
        m.erase(it);                                                           \
    }
#define CFG_FLOAT(KEY)                                                         \
    if ((it = m.find(#KEY)) != m.end()) {                                      \
        this->KEY = std::stof(it->second);                                     \
        m.erase(it);                                                           \
    }
#define CFG_BOOL(KEY)                                                          \
    if ((it = m.find(#KEY)) != m.end()) {                                      \
        this->KEY = parseBool(it->second);                                     \
        m.erase(it);                                                           \
    }
#define CFG_SDLK(KEY)                                                          \
    if ((it = m.find(#KEY)) != m.end()) {                                      \
        this->KEY = parseKeycode(it->second);                                  \
        m.erase(it);                                                           \
    }
#define CFG_STR(KEY)                                                           \
    if ((it = m.find(#KEY)) != m.end()) {                                      \
        this->KEY = it->second;                                                \
        m.erase(it);                                                           \
    }

void Config::Load(const std::string &path)
{
    auto m = loadConfigFile(path);
    decltype(m.find("")) it;

    CFG_INT(disp_width)
    CFG_INT(disp_height)
    CFG_INT(disp_bpp)
    CFG_FLOAT(disp_ppu_x)
    CFG_FLOAT(disp_ppu_y)
    CFG_BOOL(disp_autoscale)
    CFG_BOOL(disp_autoscale_dpi)

    CFG_STR(path_default)
    processEnvValue(&path_default);
    CFG_STR(path_default_right)
    processEnvValue(&path_default_right);
    CFG_STR(path_default_right_fallback)
    processEnvValue(&path_default_right_fallback);
    CFG_STR(file_system)
    processEnvValue(&file_system);
    CFG_STR(res_dir)
    processEnvValue(&res_dir);

    CFG_BOOL(osk_key_system_is_backspace)

    CFG_SDLK(key_down)
    CFG_SDLK(key_left)
    CFG_SDLK(key_open)
    CFG_SDLK(key_operation)
    CFG_SDLK(key_pagedown)
    CFG_SDLK(key_pageup)
    CFG_SDLK(key_parent)
    CFG_SDLK(key_right)
    CFG_SDLK(key_select)
    CFG_SDLK(key_system)
    CFG_SDLK(key_transfer)
    CFG_SDLK(key_up)

    if (!m.empty()) {
        std::cerr << "  Unknown settings:\n";
        for (const auto &it : m) {
            std::cerr << "    " << it.first << " = " << it.second << "\n";
        }
    }
}
