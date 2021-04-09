//
// Created by yuyuan on 2021/03/16.
//

#include "DashUrl.h"
#include "demuxer/play_list/Helper.h"
#include "demuxer/play_list/Representation.h"
#include <cstring>
#include <sstream>

using namespace Cicada;
using namespace Cicada::Dash;

DashUrl::DashUrl()
{}

DashUrl::DashUrl(const Component &comp)
{
    prepend(comp);
}

DashUrl::DashUrl(const std::string &str)
{
    prepend(Component(str));
}

bool DashUrl::hasScheme() const
{
    if (components.empty()) {
        return false;
    }

    return components[0].b_scheme;
}

bool DashUrl::empty() const
{
    return components.empty();
}

DashUrl &DashUrl::prepend(const Component &comp)
{
    components.insert(components.begin(), comp);
    return *this;
}

DashUrl &DashUrl::append(const Component &comp)
{
    if (!components.empty() && !components.back().b_dir) {
        components.pop_back();
    }
    components.push_back(comp);
    return *this;
}

DashUrl &DashUrl::prepend(const DashUrl &url)
{
    components.insert(components.begin(), url.components.begin(), url.components.end());
    return *this;
}

DashUrl &DashUrl::append(const DashUrl &url)
{
    if (!components.empty() && url.components.front().b_absolute) {
        if (components.front().b_scheme) {
            while (components.size() > 1) {
                components.pop_back();
            }
            std::string scheme(components.front().component);
            std::size_t schemepos = scheme.find_first_of("://");
            if (schemepos != std::string::npos) {
                std::size_t pathpos = scheme.find_first_of('/', schemepos + 3);
                if (pathpos != std::string::npos) {
                    components.front().component = scheme.substr(0, pathpos);
                }
                /* otherwise should be domain only */
            }
        }
    }

    if (!components.empty() && !components.back().b_dir) {
        components.pop_back();
    }
    components.insert(components.end(), url.components.begin(), url.components.end());
    return *this;
}

std::string DashUrl::toString() const
{
    return toString(0, nullptr);
}

std::string DashUrl::toString(size_t index, const Representation *rep) const
{
    std::string ret;
    std::vector<Component>::const_iterator it;

    for (it = components.begin(); it != components.end(); ++it) {
        std::string part;
        const Component *comp = &(*it);
        if (rep) {
            part = rep->contextualize(index, comp->component, comp->templ);
        } else {
            part = comp->component;
        }

        if (ret.empty()) {
            ret = part;
        } else {
            std::string str_fixup = UriFixup(part.c_str());
            std::string str_resolved = Helper::combinePaths(ret, str_fixup.empty() ? part : str_fixup);
            ret = str_resolved;
        }
    }

    return ret;
}

static bool isurihex(int c)
{ /* Same as isxdigit() but does not depend on locale and unsignedness */
    return ((unsigned char) (c - '0') < 10) || ((unsigned char) (c - 'A') < 6) || ((unsigned char) (c - 'a') < 6);
}

static bool isurialnum(int c)
{
    return ((unsigned char) (c - 'a') < 26) || ((unsigned char) (c - 'A') < 26) || ((unsigned char) (c - '0') < 10);
}

static bool isurisafe(int c)
{
    /* These are the _unreserved_ URI characters (RFC3986 ��2.3) */
    return isurialnum(c) || (strchr("-._~", c) != NULL);
}

static bool isurisubdelim(int c)
{
    return strchr("!$&'()*+,;=", c) != NULL;
}

static void UriPutc(std::stringstream &sstream, char c, const char *extras)
{
    if (isurisafe(c) || isurisubdelim(c) || (strchr(extras, c) != NULL)) {
        sstream << c;
    } else {
        sstream.fill('0');
        sstream.flags(std::ios::right | std::ios::hex | std::ios::uppercase);
        sstream.width(2);
        sstream << (unsigned short) c;
    }
}

std::string DashUrl::UriFixup(const char *str) const
{
    if (str == nullptr) {
        return "";
    }

    /* If percent sign is consistently followed by two hexadecimal digits,
     * then URL encoding must be assumed.
     * Otherwise, the percent sign itself must be URL-encoded.
     */
    bool encode_percent = false;

    for (const char *p = str; *p != '\0'; p++) {
        if (p[0] == '%' && !(isurihex(p[1]) && isurihex(p[2]))) {
            encode_percent = true;
            break;
        }
    }

    std::stringstream sstream;

    /* Handle URI scheme */
    const char *p = str;
    bool absolute = false;
    bool encode_brackets = true;

    while (isurialnum(*p) || memchr("+-.", *p, 3) != NULL) {
        sstream << *p;
        p++;
    }

    if (p > str && *p == ':') { /* There is an URI scheme, assume an absolute URI. */
        sstream << *p;
        p++;
        absolute = true;
        encode_brackets = false;
    }

    /* Handle URI authority */
    if ((absolute || p == str) && strncmp(p, "//", 2) == 0) {
        sstream << *p << *(p + 1);
        p += 2;
        encode_brackets = true;

        while (memchr("/?#", *p, 4) == NULL) {
            UriPutc(sstream, *(p++), &"%:[]@"[encode_percent]);
        }
    }

    /* Handle URI path and what follows */
    const char *extras = encode_brackets ? "%/?#@" : "%:/?#[]@";

    while (*p != '\0') {
        UriPutc(sstream, *(p++), extras + encode_percent);
    }

    return sstream.str();
}

DashUrl::Component::Component(const std::string &str, const SegmentTemplate *templ_)
    : component(str), templ(templ_), b_scheme(false), b_dir(false), b_absolute(false)
{
    if (!component.empty()) {
        b_dir = (component[component.length() - 1] == '/');
        b_scheme = (component.find_first_of("://") == (component.find_first_of('/') - 1));
        b_absolute = (component[0] == '/');
    }
}
