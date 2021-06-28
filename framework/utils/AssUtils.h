//
// Created by yuyuan on 2021/6/25.
//

#include "CicadaType.h"
#include <map>
#include <string>

#ifndef SAASMEDIA_ASSUTILS_H
#define SAASMEDIA_ASSUTILS_H

namespace Cicada {

#define VALIGN_SUB 0
#define VALIGN_CENTER 8
#define VALIGN_TOP 4
#define HALIGN_LEFT 1
#define HALIGN_CENTER 2
#define HALIGN_RIGHT 3

    typedef struct AssStyle_ {
        std::string Name;
        std::string FontName;
        double FontSize = 0;
        uint32_t PrimaryColour = 0;
        uint32_t SecondaryColour = 0;
        uint32_t OutlineColour = 0;
        uint32_t BackColour = 0;
        int Bold = 0;
        int Italic = 0;
        int Underline = 0;
        int StrikeOut = 0;
        double ScaleX = 100;
        double ScaleY = 100;
        double Spacing = 0;
        double Angle = 0;
        int BorderStyle = 0;
        double Outline = 0;
        double Shadow = 0;
        int Alignment = 0;
        int MarginL = 0;
        int MarginR = 0;
        int MarginV = 0;
        int Encoding = 1;
    } AssStyle;

    enum SubtitleType { SubtitleTypeUnknown, SubtitleTypeAss, SubtitleTypeSsa };

    typedef struct AssHeader_ {
        SubtitleType Type = SubtitleTypeUnknown;
        int PlayResX = 0;
        int PlayResY = 0;
        double Timer = 0;
        int WrapStyle = 0;
        int ScaledBorderAndShadow = 0;
        std::map<std::string, AssStyle> styles;
        std::string styleFormat;
        std::string eventFormat;
    } AssHeader;

    typedef struct AssDialogue_ {
        int Layer;
        std::string Style;
        std::string Name;
        int MarginL;
        int MarginR;
        int MarginV;
        std::string Effect;
        std::string Text;
    } AssDialogue;

    class CICADA_CPLUS_EXTERN AssUtils {
    public:
        static AssHeader parseAssHeader(const std::string &header);
        static AssDialogue parseAssDialogue(const AssHeader &header, const std::string &data);
    };
}// namespace Cicada

#endif//SAASMEDIA_ASSUTILS_H
