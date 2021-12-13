//
// Created by yuyuan on 2021/6/25.
//

#include "AssUtils.h"
#include "frame_work_log.h"
#include <algorithm>
#include <climits>
#include <cstdlib>
#include <cstring>


using namespace Cicada;

typedef enum { PST_UNKNOWN = 0, PST_INFO, PST_STYLES, PST_EVENTS, PST_FONTS } ParserState;

static void skip_spaces(char **str)
{
    char *p = *str;
    while ((*p == ' ') || (*p == '\t')) ++p;
    *str = p;
}

void rskip_spaces(char **str, char *limit)
{
    char *p = *str;
    while ((p > limit) && ((p[-1] == ' ') || (p[-1] == '\t'))) --p;
    *str = p;
}
#ifdef _MSC_VER
#define strcasecmp  stricmp
#define strncasecmp  strnicmp
#endif

static char parse_bool(char *str)
{
    skip_spaces(&str);
    return !strncasecmp(str, "yes", 3) || std::strtol(str, nullptr, 10) > 0;
}

static int process_info_line(AssHeader &header, ParserState &state, char *str)
{
    if (!strncmp(str, "PlayResX:", 9)) {
        header.PlayResX = atoi(str + 9);
    } else if (!strncmp(str, "PlayResY:", 9)) {
        header.PlayResY = atoi(str + 9);
    } else if (!strncmp(str, "Timer:", 6)) {
        header.Timer = std::strtod(str + 6, nullptr);
    } else if (!strncmp(str, "WrapStyle:", 10)) {
        header.WrapStyle = atoi(str + 10);
    } else if (!strncmp(str, "ScaledBorderAndShadow:", 22)) {
        header.ScaledBorderAndShadow = parse_bool(str + 22);
    }
    return 0;
}

static inline void advance_token_pos(const char **const str, const char **const start, const char **const end)
{
    *start = *str;
    *end = *start;
    while (**end != '\0' && **end != ',') ++*end;
    *str = *end + (**end == ',');
    rskip_spaces((char **) end, (char *) *start);
}

static char *next_token(char **str)
{
    char *p;
    char *start;
    skip_spaces(str);
    if (**str == '\0') {
        return 0;
    }

    advance_token_pos((const char **) str, (const char **) &start, (const char **) &p);

    *p = '\0';
    return start;
}

static int read_digits(char **str, unsigned base, uint32_t *res)
{
    char *p = *str;
    char *start = p;
    uint32_t val = 0;

    while (1) {
        unsigned digit;
        if (*p >= '0' && *p < std::min(base, 10u) + '0')
            digit = *p - '0';
        else if (*p >= 'a' && *p < base - 10 + 'a')
            digit = *p - 'a' + 10;
        else if (*p >= 'A' && *p < base - 10 + 'A')
            digit = *p - 'A' + 10;
        else
            break;
        val = val * base + digit;
        ++p;
    }

    *res = val;
    *str = p;
    return p != start;
}

static int mystrtou32_modulo(char **p, unsigned base, uint32_t *res)
{
    char *start = *p;
    int sign = 1;

    skip_spaces(p);

    if (**p == '+')
        ++*p;
    else if (**p == '-')
        sign = -1, ++*p;

    if (base == 16 && !strncasecmp(*p, "0x", 2)) *p += 2;

    if (read_digits(p, base, res)) {
        *res *= sign;
        return 1;
    } else {
        *p = start;
        return 0;
    }
}

static inline uint32_t ass_bswap32(uint32_t x)
{
#ifdef _MSC_VER
    return x;
#else
    return (x & 0x000000FF) << 24 | (x & 0x0000FF00) << 8 | (x & 0x00FF0000) >> 8 | (x & 0xFF000000) >> 24;
#endif
}

static uint32_t parse_color_header(const char *str)
{
    uint32_t color = 0;
    unsigned base;

    if (!strncasecmp(str, "&h", 2) || !strncasecmp(str, "0x", 2)) {
        str += 2;
        base = 16;
    } else
        base = 10;

    mystrtou32_modulo((char**)&str, base, &color);
    return ass_bswap32(color);
}

uint32_t AssUtils::parseColorHeader(const char* str)
{
    return parse_color_header(str);
}

#define NEXT(str,token) \
    token = next_token(&str); \
    if (!token) break;


#define ALIAS(alias,name) \
        if (strcasecmp(tname, #alias) == 0) {tname = #name;}

/* One section started with PARSE_START and PARSE_END parses a single token
 * (contained in the variable named token) for the header indicated by the
 * variable tname. It does so by chaining a number of else-if statements, each
 * of which checks if the tname variable indicates that this header should be
 * parsed. The first parameter of the macro gives the name of the header.
 *
 * The string that is passed is in str. str is advanced to the next token if
 * a header could be parsed. The parsed results are stored in the variable
 * target, which has the type AssStyle or AssDialogue.
 */
#define PARSE_START if (0) {
#define PARSE_END   }

#define ANYVAL(name,func) \
    } else if (strcasecmp(tname, #name) == 0) { \
        target.name = func(token);

#define STRVAL(name) \
    } else if (strcasecmp(tname, #name) == 0) { \
        target.name = token; \

#define STARREDSTRVAL(name) \
    } else if (strcasecmp(tname, #name) == 0) { \
        while (*token == '*') ++token; \
        target.name = token; \

#define COLORVAL(name) ANYVAL(name,parse_color_header)
#define INTVAL(name) ANYVAL(name,atoi)
#define FPVAL(name) ANYVAL(name,atof)

static int numpad2align(int val)
{
    if (val < -INT_MAX)
        // Pick an alignment somewhat arbitrarily. VSFilter handles
        // INT32_MIN as a mix of 1, 2 and 3, so prefer one of those values.
        val = 2;
    else if (val < 0)
        val = -val;

    int res = ((val - 1) % 3) + 1;// horizontal alignment
    if (val <= 3)
        res |= VALIGN_SUB;
    else if (val <= 6)
        res |= VALIGN_CENTER;
    else
        res |= VALIGN_TOP;
    return res;
}

int AssUtils::Numpad2Align(int val)
{
    return numpad2align(val);
}

static int process_style(AssHeader &header, ParserState &state, char *str)
{

    char *token;
    char *tname;
    char *p = str;
    char *format;
    char *q;// format scanning pointer
    int sid;
    AssStyle style;
    AssStyle &target = style;

    q = format = strdup(header.styleFormat.c_str());
    if (!q) return -1;

    AF_LOGI("ASS Style: %s", str);

    // fill style with some default values
    style.ScaleX = 100.;
    style.ScaleY = 100.;

    while (1) {
        NEXT(q, tname);
        NEXT(p, token);

        PARSE_START
        STARREDSTRVAL(Name)
        STRVAL(FontName)
        COLORVAL(PrimaryColour)
        COLORVAL(SecondaryColour)
        COLORVAL(OutlineColour)// TertiaryColor
        COLORVAL(BackColour)
        // SSA uses BackColour for both outline and shadow
        // this will destroy SSA's TertiaryColour, but i'm not going to use it anyway
        if (header.Type == SubtitleTypeSsa) {style.OutlineColour = style.BackColour;}
        FPVAL(FontSize)
        INTVAL(Bold)
        INTVAL(Italic)
        INTVAL(Underline)
        INTVAL(StrikeOut)
        FPVAL(Spacing)
        FPVAL(Angle)
        INTVAL(BorderStyle)
        INTVAL(Alignment)
        if (header.Type == SubtitleTypeAss) {
            // style.Alignment = numpad2align(style.Alignment);
        }
        // VSFilter compatibility
        else if (style.Alignment == 8)
            style.Alignment = 3;
        else if (style.Alignment == 4)
            style.Alignment = 11;
        INTVAL(MarginL)
        INTVAL(MarginR)
        INTVAL(MarginV)
        INTVAL(Encoding)
        FPVAL(ScaleX)
        FPVAL(ScaleY)
        FPVAL(Outline)
        FPVAL(Shadow)
        PARSE_END
    }
    free(format);
    style.ScaleX = std::max(style.ScaleX, 0.) / 100.;
    style.ScaleY = std::max(style.ScaleY, 0.) / 100.;
    style.Spacing = std::max(style.Spacing, 0.);
    style.Outline = std::max(style.Outline, 0.);
    style.Shadow = std::max(style.Shadow, 0.);
    style.Bold = !!style.Bold;
    style.Italic = !!style.Italic;
    style.Underline = !!style.Underline;
    style.StrikeOut = !!style.StrikeOut;
    if (style.Name.empty()) {
        style.Name = "Default";
    }
    if (style.FontName.empty()) {
        style.FontName = "Arial";
    }
    header.styles[style.Name] = style;
    return 0;
}

static int process_styles_line(AssHeader &header, ParserState &state, char *str)
{
    int ret = 0;
    if (!strncmp(str, "Format:", 7)) {
        char *p = str + 7;
        skip_spaces(&p);
        header.styleFormat = p;
        if (header.styleFormat.empty()) {
            return -1;
        }
        AF_LOGI("Ass Style format: %s", header.styleFormat.c_str());
    } else if (!strncmp(str, "Style:", 6)) {
        char *p = str + 6;
        skip_spaces(&p);
        ret = process_style(header, state, p);
    }
    return ret;
}

static int process_events_line(AssHeader &header, ParserState &state, char *str)
{
    if (!strncmp(str, "Format:", 7)) {
        char *p = str + 7;
        skip_spaces(&p);
        header.eventFormat = p;
        AF_LOGI("ASS Event format: %s", header.eventFormat.c_str());
    }
    return 0;
}

static int process_line(AssHeader& header, ParserState& state, char *str)
{
    skip_spaces(&str);
    if (!strncasecmp(str, "[Script Info]", 13)) {
        state = PST_INFO;
    } else if (!strncasecmp(str, "[V4 Styles]", 11)) {
        state = PST_STYLES;
        header.Type = SubtitleTypeSsa;
    } else if (!strncasecmp(str, "[V4+ Styles]", 12)) {
        state = PST_STYLES;
        header.Type = SubtitleTypeAss;
    } else if (!strncasecmp(str, "[Events]", 8)) {
        state = PST_EVENTS;
    } else if (!strncasecmp(str, "[Fonts]", 7)) {
        state = PST_FONTS;
    } else {
        switch (state) {
            case PST_INFO:
                process_info_line(header, state, str);
                break;
            case PST_STYLES:
                process_styles_line(header, state, str);
                break;
            case PST_EVENTS:
                process_events_line(header, state, str);
                break;
            default:
                break;
        }
    }
    return 0;
}

static int process_event_tail(const AssHeader &header, AssDialogue& event, char *str, int n_ignored)
{
    char *token;
    char *tname;
    char *p = str;
    int i;
    AssDialogue &target = event;

    char *format = strdup(header.eventFormat.c_str());
    if (!format) {return -1;}
    char *q = format;// format scanning pointer

    for (i = 0; i < n_ignored; ++i) {
        NEXT(q, tname);
    }

    token = next_token(&p);
    while (1) {
        NEXT(q, tname);
        if (strcasecmp(tname, "Text") == 0) {
            event.Text = p;
            if (!event.Text.empty() && event.Text[event.Text.length() - 1] == '\r') {
                event.Text.erase(event.Text.length() - 1);
            }
            free(format);
            return !event.Text.empty() ? 0 : -1;// "Text" is always the last
        } else if (strcasecmp(tname, "Start") == 0 || strcasecmp(tname, "End") == 0) {
            continue;
        }
        NEXT(p, token);

        PARSE_START
        INTVAL(Layer)
        STRVAL(Style)
        STRVAL(Name)
        STRVAL(Effect)
        INTVAL(MarginL)
        INTVAL(MarginR)
        INTVAL(MarginV)
        PARSE_END
    }
    free(format);
    return 1;
}

AssHeader AssUtils::parseAssHeader(const std::string &header)
{
    AssHeader ret;
    std::string data(header);
    if (data.empty()) {
        return ret;
    }

    char *p = &data.at(0);
    ParserState state = PST_UNKNOWN;
    while (1) {
        char *q;
        while (1) {
            if ((*p == '\r') || (*p == '\n'))
                ++p;
            else if (p[0] == '\xef' && p[1] == '\xbb' && p[2] == '\xbf')
                p += 3;// U+FFFE (BOM)
            else
                break;
        }
        for (q = p; ((*q != '\0') && (*q != '\r') && (*q != '\n')); ++q) {
        };
        if (q == p) break;
        if (*q != '\0') *(q++) = '\0';
        process_line(ret, state, p);
        if (*q == '\0') break;
        p = q;
    }
    return ret;
}

AssDialogue AssUtils::parseAssDialogue(const AssHeader &header, const std::string &data)
{
    AssDialogue ret;
    std::string text(data);
    process_event_tail(header, ret, &text.at(0), 0);
    return ret;
}