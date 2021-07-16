//
// Created by pingkai on 2021/7/15.
//

#include "AppleCATextLayerRender.h"
using namespace Cicada;
using namespace std;
AppleCATextLayerRender::AppleCATextLayerRender()
{
    layerDic = @{}.mutableCopy;
}
AppleCATextLayerRender::~AppleCATextLayerRender()
{}

NSArray *AppleCATextLayerRender::matchStringWithRegx(NSString *string, NSString *regexStr)
{

    NSRegularExpression *regex = [NSRegularExpression regularExpressionWithPattern:regexStr
                                                                           options:NSRegularExpressionCaseInsensitive
                                                                             error:nil];
    NSArray *matches = [regex matchesInString:string options:0 range:NSMakeRange(0, [string length])];
    NSMutableArray *array = [NSMutableArray array];
    for (NSTextCheckingResult *match in matches) {
        for (int i = 0; i < [match numberOfRanges]; i++) {
            NSString *component = [string substringWithRange:[match rangeAtIndex:i]];
            [array addObject:component];
        }
    }
    return array;
}

NSObject *getSubTitleColor(bool isBGR, NSInteger value)
{
    if (isBGR) {
        return [CicadaColor colorWithRed:((float) (value & 0xFF)) / 255.0
                                   green:((float) ((value & 0xFF00) >> 8)) / 255.0
                                    blue:((float) ((value & 0xFF0000) >> 16)) / 255.0
                                   alpha:1.0];
    } else {
        return [CicadaColor colorWithRed:((float) ((value & 0xFF000000) >> 24)) / 255.0
                                   green:((float) ((value & 0xFF0000) >> 16)) / 255.0
                                    blue:((float) ((value & 0xFF00) >> 8)) / 255.0
                                   alpha:(1.0 - ((float) (value & 0xFF)) / 255.0)];
    }
}

NSAttributedString *AppleCATextLayerRender::buildAssStyleStr(NSString *style, NSString *text, AssStyle defaultstyle)
{
    NSMutableDictionary<NSAttributedStringKey, id> *attrs = @{}.mutableCopy;

    NSString *fontName = [NSString stringWithCString:defaultstyle.FontName.c_str() encoding:NSUTF8StringEncoding];
    int fontSize = defaultstyle.FontSize;

    NSObject *color = getSubTitleColor(false, defaultstyle.PrimaryColour);
    [attrs setValue:color forKey:NSForegroundColorAttributeName];

    if (style) {
        NSArray *styleArr = [style componentsSeparatedByCharactersInSet:[NSCharacterSet characterSetWithCharactersInString:@"{}"]];
        for (NSString *subStr in styleArr) {
            NSArray *subStyleArr = [subStr componentsSeparatedByString:@"\\"];
            for (NSString *item in subStyleArr) {
                NSString *itemStr = [item stringByReplacingOccurrencesOfString:@" " withString:@""];
                if ([itemStr hasPrefix:@"fn"]) {
                    fontName = [itemStr substringFromIndex:@"fn".length];
                } else if ([itemStr hasPrefix:@"fs"]) {
                    fontSize = [itemStr substringFromIndex:@"fs".length].intValue;
                } else if (matchStringWithRegx(itemStr, @"^b[0-9]+$").count > 0) {
                    itemStr = [itemStr stringByReplacingOccurrencesOfString:@"b" withString:@""];
                    [attrs setValue:[NSNumber numberWithInt:itemStr.intValue] forKey:NSExpansionAttributeName];
                } else if (matchStringWithRegx(itemStr, @"^i[0-9]+$").count > 0) {
                    itemStr = [itemStr stringByReplacingOccurrencesOfString:@"i" withString:@""];
                    [attrs setValue:[NSNumber numberWithInt:itemStr.intValue] forKey:NSObliquenessAttributeName];
                } else if (matchStringWithRegx(itemStr, @"^u[0-9]+$").count > 0) {
                    itemStr = [itemStr stringByReplacingOccurrencesOfString:@"u" withString:@""];
                    [attrs setValue:[NSNumber numberWithInt:itemStr.intValue] forKey:NSUnderlineStyleAttributeName];
                } else if (matchStringWithRegx(itemStr, @"^s[0-9]+$").count > 0) {
                    itemStr = [itemStr stringByReplacingOccurrencesOfString:@"s" withString:@""];
                    [attrs setValue:[NSNumber numberWithInt:itemStr.intValue] forKey:NSStrikethroughStyleAttributeName];
                } else if ([itemStr hasPrefix:@"c&H"] || [itemStr hasPrefix:@"1c&H"]) {
                    NSRange range = [itemStr rangeOfString:@"c&H"];
                    itemStr = [itemStr substringFromIndex:range.location + range.length];
                    itemStr = [itemStr stringByReplacingOccurrencesOfString:@"&" withString:@""];
                    unsigned colorInt = 0;
                    [[NSScanner scannerWithString:itemStr] scanHexInt:&colorInt];
                    NSObject *color = getSubTitleColor(true, colorInt);
                    [attrs setValue:color forKey:NSForegroundColorAttributeName];
                }
            }
        }
    }

    CicadaFont *font = [CicadaFont fontWithName:fontName size:fontSize];
    [attrs setValue:font ?: [CicadaFont systemFontOfSize:fontSize] forKey:NSFontAttributeName];

    return [[NSAttributedString alloc] initWithString:text attributes:attrs];
}

static CGSize getSubTitleHeight(NSMutableAttributedString *attrStr, CGFloat viewWidth)
{
    CGSize textSize = CGSizeZero;
    if (attrStr.length == 0) {
        return textSize;
    }

    //    CGSize size  = [attrStr boundingRectWithSize:CGSizeMake(viewWidth, MAXFLOAT) options: NSStringDrawingUsesLineFragmentOrigin | NSStringDrawingUsesFontLeading  context:nil].size;
    NSMutableParagraphStyle *paragraphStyle = [NSMutableParagraphStyle new];
    //    [paragraphStyle setLineSpacing:lineSpace];
    paragraphStyle.lineBreakMode = NSLineBreakByCharWrapping;
    NSRange range = NSMakeRange(0, attrStr.length);
    [attrStr addAttribute:NSParagraphStyleAttributeName value:paragraphStyle range:range];

    NSDictionary *dic = [attrStr attributesAtIndex:0 effectiveRange:&range];
    textSize = [attrStr.string boundingRectWithSize:CGSizeMake(viewWidth, MAXFLOAT)
                                            options:NSStringDrawingUsesLineFragmentOrigin | NSStringDrawingUsesFontLeading
                                         attributes:dic
                                            context:nil]
                       .size;
    return CGSizeMake(viewWidth, textSize.height);
}

void AppleCATextLayerRender::buildAssStyle(CATextLayer *textLayer, const AssDialogue &ret)
{
    //TODO detect whether it is the default style
    std::map<std::string, AssStyle> styles = mHeader.styles;
    auto iter = styles.begin();
    AssStyle assStyle = iter->second;

    NSString *fontName = [NSString stringWithCString:assStyle.FontName.c_str() encoding:NSUTF8StringEncoding];
    NSString *subtitle = [NSString stringWithCString:ret.Text.c_str() encoding:NSUTF8StringEncoding];

    subtitle = [subtitle stringByReplacingOccurrencesOfString:@"\\N" withString:@"\n"];

    //    NSLog(@"====%@",subtitle);

    NSArray *lineCodes = matchStringWithRegx(subtitle, @"\\{[^\\{]+\\}");

    //TODO not for `p0` for the time being
    NSMutableAttributedString *attributedStr = [[NSMutableAttributedString alloc] init];
    if (lineCodes.count > 0) {
        if ([subtitle hasSuffix:@"p0}"]) {
            subtitle = @"";
        } else {
            NSString *preStyle = @"";
            for (int i = 0; i < lineCodes.count; i++) {
                NSString *code = [lineCodes objectAtIndex:i];
                NSRange range = [subtitle rangeOfString:code];
                NSUInteger end = 0;
                if (lineCodes.count > i + 1) {
                    NSString *nextCode = [lineCodes objectAtIndex:i + 1];
                    NSRange nextRange = [subtitle rangeOfString:nextCode];
                    end = nextRange.location;
                } else {
                    end = [subtitle length];
                }
                NSUInteger begin = range.location + range.length;
                NSString *text = [subtitle substringWithRange:NSMakeRange(begin, end - begin)];
                if (text.length > 0) {
                    if (preStyle.length > 0) {
                        code = [preStyle stringByAppendingString:code];
                        preStyle = @"";
                    }
                    [attributedStr appendAttributedString:buildAssStyleStr(code, text, assStyle)];
                } else {
                    preStyle = [preStyle stringByAppendingString:code];
                }

                if (subtitle.length > end) {
                    subtitle = [subtitle substringFromIndex:end];
                }
            }
        }
    } else {
        [attributedStr appendAttributedString:buildAssStyleStr(nil, subtitle, assStyle)];
    }

    CGFloat x = 0;
    CGFloat y = 0;
    CGFloat w = CGRectGetWidth(mView.frame);
    CGFloat h = CGRectGetHeight(mView.frame);

    CGSize textSize = CGSizeZero;
    if (attributedStr.length > 0) {
        textLayer.string = attributedStr;
        textSize = getSubTitleHeight(attributedStr, w);
    }

    switch (assStyle.Alignment % 4) {
        //align left
        case 1:
            x = 0;
            x += assStyle.MarginL;
            textLayer.alignmentMode = kCAAlignmentLeft;
            break;
            //Center
        case 2:
            //            x = (w - textSize.width) / 2;
            textLayer.alignmentMode = kCAAlignmentCenter;
            break;
            //align right
        case 3:
            x = w - textSize.width;
            x -= assStyle.MarginR;
            textLayer.alignmentMode = kCAAlignmentRight;
            break;

        default:
            break;
    }
    switch (assStyle.Alignment / 4) {
        //bottom
        case 0:
#if TARGET_OS_IPHONE
            y = h - textSize.height;
            y -= assStyle.MarginV;
#else
            y = 0;
            x += assStyle.MarginV;
#endif
            break;
            //top
        case 1:
#if TARGET_OS_IPHONE
            y = 0;
            x += assStyle.MarginV;
#else
            y = h - textSize.height;
            y -= assStyle.MarginV;
#endif
            break;
            //Center
        case 2:
            y = (h - textSize.height) / 2;
            break;

        default:
            break;
    }

    textLayer.frame = CGRectMake(x, y, textSize.width, textSize.height);
}
int AppleCATextLayerRender::intHeader(const char *header)
{
    mHeader = AssUtils::parseAssHeader(header);
    if (mHeader.Type != SubtitleTypeUnknown) {
        return 0;
    }
    return -EINVAL;
}
int AppleCATextLayerRender::show(const string &data)
{
    AssDialogue ret = AssUtils::parseAssDialogue(mHeader, data);

    dispatch_async(dispatch_get_main_queue(), ^{
      CATextLayer *textLayer = nil;
      NSString *layerKey = [NSString stringWithFormat:@"%i", ret.Layer];
      if ([layerDic objectForKey:layerKey]) {
          textLayer = [layerDic objectForKey:layerKey];
      } else {
          textLayer = [CATextLayer layer];
            //                  textLayer.frame = player.playerView.bounds;
#if TARGET_OS_IPHONE
          textLayer.contentsScale = [UIScreen mainScreen].scale;
#endif
          textLayer.wrapped = YES;
          [mView.layer insertSublayer:textLayer atIndex:ret.Layer + 1];

          [layerDic setValue:textLayer forKey:layerKey];
      }

      textLayer.hidden = NO;

      buildAssStyle(textLayer, ret);
    });

    return 0;
}

int AppleCATextLayerRender::hide(const string &data)
{
    AssDialogue ret = AssUtils::parseAssDialogue(mHeader, data);
    NSString *layerKey = [NSString stringWithFormat:@"%i", ret.Layer];
    CALayer *assLab = [layerDic objectForKey:layerKey];
    if (assLab) {
        dispatch_async(dispatch_get_main_queue(), ^{
          assLab.hidden = YES;
        });
    }
    return 0;
}
#if TARGET_OS_OSX
void AppleCATextLayerRender::setView(NSView *view)
#elif TARGET_OS_IPHONE
void AppleCATextLayerRender::setView(UIView *view)
#endif
{
    mView = view;
}
