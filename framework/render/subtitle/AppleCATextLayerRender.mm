//
// Created by pingkai on 2021/7/15.
//

#include "AppleCATextLayerRender.h"
using namespace Cicada;
using namespace std;
AppleCATextLayerRender::AppleCATextLayerRender()
{
}
AppleCATextLayerRender::~AppleCATextLayerRender()
{
    if (renderHandle) {
        CFRelease(renderHandle);
    }
}

int AppleCATextLayerRender::intHeader(const char *header)
{
    mHeader = AssUtils::parseAssHeader(header);
    if (mHeader.Type != SubtitleTypeUnknown) {
        AppleCATextLayerRenderImpl *object = [[AppleCATextLayerRenderImpl alloc] init];
        object.mHeader = mHeader;
        renderHandle = (__bridge_retained void *) object;
        return 0;
    }
    return -EINVAL;
}
int AppleCATextLayerRender::show(const string &data)
{
    AssDialogue ret = AssUtils::parseAssDialogue(mHeader, data);

    [(__bridge id) renderHandle showDialogue:ret];
    return 0;
}

int AppleCATextLayerRender::hide(const string &data)
{
    AssDialogue ret = AssUtils::parseAssDialogue(mHeader, data);
    [(__bridge id) renderHandle hideDialogue:ret];
    return 0;
}
#if TARGET_OS_OSX
void AppleCATextLayerRender::setView(NSView *view)
#elif TARGET_OS_IPHONE
void AppleCATextLayerRender::setView(UIView *view)
#endif
{
    [(__bridge id) renderHandle setup:view];
}

@implementation DialogueObj

@end

@implementation AppleCATextLayerRenderImpl

- (void)showDialogue:(Cicada::AssDialogue)ret
{
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
          [self.mView.layer insertSublayer:textLayer atIndex:ret.Layer + 1];

          [layerDic setValue:textLayer forKey:layerKey];
      }

      textLayer.hidden = NO;
      DialogueObj *obj = [DialogueObj new];
      obj.dialogue = ret;
      [dialogueDic setObject:obj forKey:layerKey];
      [self buildAssStyle:textLayer withAssDialogue:ret];
    });
}

- (void)hideDialogue:(Cicada::AssDialogue)ret
{
    NSString *layerKey = [NSString stringWithFormat:@"%i", ret.Layer];
    CALayer *assLab = [layerDic objectForKey:layerKey];
    [dialogueDic removeObjectForKey:layerKey];
    if (assLab) {
        dispatch_async(dispatch_get_main_queue(), ^{
          assLab.hidden = YES;
        });
    }
}

- (NSArray *)matchString:(NSString *)string withRegx:(NSString *)regexStr
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

- (NSAttributedString *)buildAssStyleStr:(NSString *)style text:(NSString *)text defaultstyle:(Cicada::AssStyle)defaultstyle
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
                } else if ([self matchString:itemStr withRegx:@"^b[0-9]+$"].count > 0) {
                    itemStr = [itemStr stringByReplacingOccurrencesOfString:@"b" withString:@""];
                    [attrs setValue:[NSNumber numberWithInt:itemStr.intValue] forKey:NSExpansionAttributeName];
                } else if ([self matchString:itemStr withRegx:@"^i[0-9]+$"].count > 0) {
                    itemStr = [itemStr stringByReplacingOccurrencesOfString:@"i" withString:@""];
                    [attrs setValue:[NSNumber numberWithInt:itemStr.intValue] forKey:NSObliquenessAttributeName];
                } else if ([self matchString:itemStr withRegx:@"^u[0-9]+$"].count > 0) {
                    itemStr = [itemStr stringByReplacingOccurrencesOfString:@"u" withString:@""];
                    [attrs setValue:[NSNumber numberWithInt:itemStr.intValue] forKey:NSUnderlineStyleAttributeName];
                } else if ([self matchString:itemStr withRegx:@"^s[0-9]+$"].count > 0) {
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

- (void)buildAssStyle:(CATextLayer *)textLayer withAssDialogue:(const AssDialogue)ret
{
    //TODO detect whether it is the default style
    std::map<std::string, AssStyle> styles = self.mHeader.styles;
    auto iter = styles.begin();
    AssStyle assStyle = iter->second;

    NSString *fontName = [NSString stringWithCString:assStyle.FontName.c_str() encoding:NSUTF8StringEncoding];
    NSString *subtitle = [NSString stringWithCString:ret.Text.c_str() encoding:NSUTF8StringEncoding];
    subtitle = [subtitle stringByReplacingOccurrencesOfString:@"\\N" withString:@"\n"];

//            NSLog(@"====%@",subtitle);

    NSArray *lineCodes = [self matchString:subtitle withRegx:@"\\{[^\\{]+\\}"];

    //TODO not for `p0` for the time being
    NSMutableAttributedString *attributedStr = [[NSMutableAttributedString alloc] init];
    NSMutableString *allLineCodeStr = @"".mutableCopy;
    if (lineCodes.count > 0) {
        if ([subtitle hasSuffix:@"p0}"]) {
            subtitle = @"";
        } else {
            NSString *preStyle = @"";
            for (int i = 0; i < lineCodes.count; i++) {
                NSString *code = [lineCodes objectAtIndex:i];
                [allLineCodeStr appendString:code];
                NSRange range = [subtitle rangeOfString:code];
                NSUInteger end = 0;
                if (lineCodes.count > i + 1) {
                    NSString *nextCode = [lineCodes objectAtIndex:i + 1];
                    NSRange nextRange = [subtitle rangeOfString:nextCode];
                    end = nextRange.location;
                } else {
                    end = [subtitle length];
                }

                if (i == 0 && range.location > 0) {
                    NSString *text = [subtitle substringWithRange:NSMakeRange(0, range.location)];
                    [attributedStr appendAttributedString:[self buildAssStyleStr:nil text:text defaultstyle:assStyle]];
                }
                NSUInteger begin = range.location + range.length;
                NSString *text = [subtitle substringWithRange:NSMakeRange(begin, end - begin)];
                if (text.length > 0) {
                    if (preStyle.length > 0) {
                        code = [preStyle stringByAppendingString:code];
                        preStyle = @"";
                    }
                    [attributedStr appendAttributedString:[self buildAssStyleStr:code text:text defaultstyle:assStyle]];
                } else {
                    preStyle = [preStyle stringByAppendingString:code];
                }

                if (subtitle.length > end) {
                    subtitle = [subtitle substringFromIndex:end];
                }
            }
        }
    } else {
        [attributedStr appendAttributedString:[self buildAssStyleStr:nil text:subtitle defaultstyle:assStyle]];
    }

    CGFloat x = 0;
    CGFloat y = 0;
    CGFloat w = CGRectGetWidth(self.mView.frame);
    CGFloat h = CGRectGetHeight(self.mView.frame);

    CGSize textSize = CGSizeZero;
    if (attributedStr.length > 0) {
        textLayer.string = attributedStr;
        textSize = getSubTitleHeight(attributedStr, w);
    }
    
    int alignment = assStyle.Alignment;
    int marginL = assStyle.MarginL;
    int marginR = assStyle.MarginR;
    int marginV = assStyle.MarginV;
    
    if (allLineCodeStr.length>0) {
        NSArray *anArr = [self matchString:allLineCodeStr withRegx:@"an[0-9]+"];
        if (anArr.count>0) {
            NSString *anStr = anArr.firstObject;
            anStr = [anStr stringByReplacingOccurrencesOfString:@"an" withString:@""];
            alignment = anStr.intValue;
        }
        NSArray *posArr = [self matchString:allLineCodeStr withRegx:@"pos\\(-?\\d+,-?\\d+\\)"];
        if (posArr.count) {
            NSString *posStr = posArr.firstObject;
            NSArray *posArr = [self matchString:posStr withRegx:@"pos\\((-?\\d+),(-?\\d+)\\)"];
            if (posArr.count==3) {
                marginV = ((NSString*)posArr[2]).intValue;
            }
            if (alignment%4==3) {
                marginR = ((NSString*)posArr[1]).intValue;
            }else{
                marginL = ((NSString*)posArr[1]).intValue;
            }
        }
    }
    switch (alignment % 4) {
        //align left
        case 1:
            x = 0;
            x += marginL;
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
            x -= marginR;
            textLayer.alignmentMode = kCAAlignmentRight;
            break;

        default:
            break;
    }
    switch (alignment / 4) {
        //bottom
        case 0:
#if TARGET_OS_IPHONE
            y = h - textSize.height;
            y -= marginV;
#else
            y = 0;
            x += marginV;
#endif
            break;
            //top
        case 1:
#if TARGET_OS_IPHONE
            y = 0;
            x += marginV;
#else
            y = h - textSize.height;
            y -= marginV;
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

- (void)setup:(CicadaView *)view
{
    _mView = view;
    layerDic = @{}.mutableCopy;
    dialogueDic = @{}.mutableCopy;

    [_mView.layer addObserver:self forKeyPath:@"bounds" options:NSKeyValueObservingOptionNew context:nil];
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary<NSString *, id> *)change context:(void *)context
{
    if ([keyPath isEqualToString:@"bounds"]) {
#if TARGET_OS_IPHONE
        CGRect bounds = [change[NSKeyValueChangeNewKey] CGRectValue];
#elif TARGET_OS_OSX
        NSRect bounds = [change[@"new"] rectValue];
#endif
        for (NSString *key in layerDic.allKeys) {
            CATextLayer *layer = [layerDic objectForKey:key];
            if (!layer.hidden) {
                DialogueObj *obj = dialogueDic[key];
                [self buildAssStyle:layer withAssDialogue:obj.dialogue];
            }
        }
    }
}

- (void)dealloc
{
    if (strcmp(dispatch_queue_get_label(DISPATCH_CURRENT_QUEUE_LABEL), dispatch_queue_get_label(dispatch_get_main_queue())) == 0) {
        if (self.mView.layer) {
            [_mView.layer removeObserver:self forKeyPath:@"bounds" context:nil];
        }
    } else {
        // FIXME: use async
        dispatch_sync(dispatch_get_main_queue(), ^{
          if (_mView.layer) {
              [_mView.layer removeObserver:self forKeyPath:@"bounds" context:nil];
          }
        });
    }
}

@end
