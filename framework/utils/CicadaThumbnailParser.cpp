
#include <cstdlib>
#include <cinttypes>
#include <algorithm>
#include "CicadaThumbnailParser.h"
#include "af_string.h"

const unsigned int HOUR_IN_SECOND = 3600;
const unsigned int MINUTE_IN_SECOND = 60;
const unsigned int SECOND_IN_MICROSECOND = 1000;

CicadaThumbnailParser::CicadaThumbnailParser(void)
{
}

CicadaThumbnailParser::~CicadaThumbnailParser(void)
{
}

list<ThumbnailInfo> CicadaThumbnailParser::setInputData(const std::string &input)
{
    list<ThumbnailInfo> infos;

    if (input.empty()) {
        return infos;
    }

    std::string line;
    std::size_t startPos = 0;
    if (!getLine(input, startPos, line)) {
        return infos;
    }

    if (std::string::npos == line.find("WEBVTT")) {
        return infos;
    }

    ThumbnailInfo info;
    while (getLine(input, startPos, line)) {
        if (!getTime(line, info.timestamp, info.duration)) {
            if (getText(line, info)) {
                infos.push_back(info);
            }
        }
    }

    return infos;
}

bool CicadaThumbnailParser::getLine(const std::string &input, std::size_t &startPos, std::string &outLine)
{
    if (startPos >= input.length()) {
        return false;
    }
    
    std::size_t pos = input.find('\n', startPos);
    if (pos == std::string::npos) {
        return false;
    }

    outLine = input.substr(startPos, pos - startPos);
    AfString::trimString(outLine);
    startPos = pos + 1;
    return true;
}

bool CicadaThumbnailParser::getText(const std::string &line, ThumbnailInfo &info)
{
    if (line.empty()) {
        return false;
    }

    std::size_t pos = line.find("#");
    if (pos == 0) {
        return false;
    }

    //TODO move this, not standard syntax
    pos = line.find("#xywh=");
    if (pos == std::string::npos) {
        info.URI = line;
        return true;
    }

    info.URI = line.substr(0, pos);

    pos = line.find('=', pos);
    string rect = line.substr(pos+1);
    AfString::trimString(rect);
    std::replace(rect.begin(), rect.end(), ',', ' ');
    std::istringstream strm(rect);

    strm >> info.posX >> info.posY >> info.width >> info.height;

    return true;
}

bool CicadaThumbnailParser::getTime(const std::string &line, int64_t &startTime, unsigned int &duration)
{
    std::size_t pos = line.find("-->");

    if (pos == std::string::npos) {
        return false;
    }

    std::string startStr = line.substr(0, pos);
    std::string endStr = line.substr(pos + 3);

    int64_t hour = 0, min = 0, sec = 0, ms = 0;
    covertTime(startStr, hour, min, sec, ms);
    startTime = (hour * HOUR_IN_SECOND + min * MINUTE_IN_SECOND + sec) * SECOND_IN_MICROSECOND + ms;

    covertTime(endStr, hour, min, sec, ms);
    int64_t endTime = (hour * HOUR_IN_SECOND + min * MINUTE_IN_SECOND + sec) * SECOND_IN_MICROSECOND + ms;
    duration = static_cast<int>(endTime - startTime);
    return true;
}

bool CicadaThumbnailParser::covertTime(std::string &time, int64_t &hour, int64_t &min, int64_t &sec, int64_t &ms)
{
    hour = min = sec = ms = 0;

    AfString::trimString(time);
    std::replace(time.begin(), time.end(), ':', ' ');
    std::replace(time.begin(), time.end(), '.', ' ');
    std::istringstream strm(time);
    if (time.length() > 9) {
        strm >> hour;
    }
    strm >> min >> sec >> ms;

    return true;
}
