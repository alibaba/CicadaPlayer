//
// Created by yuyuan on 2021/03/16.
//

#include "UrlTemplate.h"
#include <cctype>
#include <sstream>

using namespace Cicada::Dash;

/* Checks if current char is a token end, including format string tokens */
static std::string::size_type TokenEnd(const std::string &str, std::string::size_type pos, bool formatstring, int &formatwidth)
{
    std::string::size_type remain = str.length() - pos;
    if (remain < 1) {
        return std::string::npos;
    } else if (str[pos] == '$') {
        formatwidth = -1;
        return pos;
    } else if (formatstring && str[pos] == '%' && remain >= 3) {
        /* parse %[n]d format string if any */
        std::string::size_type fmtend = str.find('$', pos + 1);
        if (fmtend != std::string::npos) {
            std::istringstream iss(str.substr(pos + 1, fmtend - pos));
            iss.imbue(std::locale("C"));
            formatwidth = 1;              /* %d -> default width = 1 */
            if (std::isdigit(iss.peek())) /* [n]d */
            {
                iss >> formatwidth;
            }
            if (iss.peek() != 'd') /* should be trailing d */
            {
                return std::string::npos;
            }
            /* return end of token position */
            return fmtend;
        }
    }
    return std::string::npos;
}

static int CompareToken(const std::string &str, std::string::size_type pos, const char *token, const size_t token_len, bool formatstring,
                        std::string::size_type &fulllength, int &width)
{
    if (pos + 1 + token_len >= str.length() || str.compare(pos + 1, token_len, token, token_len)) {
        return -1;
    }

    std::string::size_type end = TokenEnd(str, pos + 1 + token_len, formatstring, width);
    if (end == std::string::npos) {
        return -1;
    }

    fulllength = 1 + end - pos;
    return 0;
}

bool UrlTemplate::IsDASHToken(const std::string &str, std::string::size_type pos, UrlTemplate::Token &token)
{
    std::string::size_type remain = str.length() - pos;
    if (remain < 2 || str[pos] != '$') {
        return false;
    }
    if (str[pos + 1] == '$') { /* escaping */
        token.type = UrlTemplate::Token::TOKEN_ESCAPE;
        token.fulllength = 2;
        token.width = -1;
        return true;
    } else if (!CompareToken(str, pos, "RepresentationID", 16, false, token.fulllength, token.width)) {
        token.type = UrlTemplate::Token::TOKEN_REPRESENTATION;
        return true;
    } else if (!CompareToken(str, pos, "Time", 4, true, token.fulllength, token.width)) {
        token.type = UrlTemplate::Token::TOKEN_TIME;
        return true;
    } else if (!CompareToken(str, pos, "Number", 6, true, token.fulllength, token.width)) {
        token.type = UrlTemplate::Token::TOKEN_NUMBER;
        return true;
    } else if (!CompareToken(str, pos, "Bandwidth", 9, true, token.fulllength, token.width)) {
        token.type = UrlTemplate::Token::TOKEN_BANDWIDTH;
        return true;
    } else {
        return false;
    }
}

static std::string FormatValue(uint64_t value, int width)
{
    std::stringstream oss;
    oss.imbue(std::locale("C"));
    if (width > 0) {
        oss.width(width); /* set format string length */
        oss.fill('0');
    }
    oss << value;
    return oss.str();
}

std::string::size_type UrlTemplate::ReplaceDASHToken(std::string &str, std::string::size_type pos, const UrlTemplate::Token &token,
                                                     const UrlTemplate::TokenReplacement &repl)
{
    switch (token.type) {
        case UrlTemplate::Token::TOKEN_TIME:
        case UrlTemplate::Token::TOKEN_BANDWIDTH:
        case UrlTemplate::Token::TOKEN_NUMBER: {
            std::string newstr = FormatValue(repl.value, token.width);
            str.replace(pos, token.fulllength, newstr);
            return newstr.length();
        }
        case UrlTemplate::Token::TOKEN_REPRESENTATION: {
            str.replace(pos, token.fulllength, repl.str);
            return repl.str.length();
        }
        case UrlTemplate::Token::TOKEN_ESCAPE: {
            str.erase(pos, 1);
            return 1;
        }
        default: {
            return std::string::npos;
        }
    }
}
