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
int AppleCATextLayerRender::show(int64_t index,const string &data)
{
    AssDialogue ret = AssUtils::parseAssDialogue(mHeader, data);

    [(__bridge id) renderHandle showDialogue:ret atIndex:index];
    return 0;
}

int AppleCATextLayerRender::hide(int64_t index,const string &data)
{
    AssDialogue ret = AssUtils::parseAssDialogue(mHeader, data);
    [(__bridge id) renderHandle hideDialogue:ret atIndex:index];
    return 0;
}
void AppleCATextLayerRender::setView(void *view)
{
    [(__bridge id) renderHandle setup:(__bridge CALayer *) view];
}

@implementation DialogueObj

@end

@implementation AppleCATextLayerRenderImpl

- (void)showDialogue:(Cicada::AssDialogue)ret atIndex:(NSInteger)index
{
    dispatch_async(dispatch_get_main_queue(), ^{
        CATextLayer *textLayer = nil;
        NSString *layerKey = [NSString stringWithFormat:@"%i", ret.Layer];
        DialogueObj* obj = [self findIdleDialogueObj];
      if (obj) {
          textLayer = obj.layer;
      } else {
          obj = [DialogueObj new];
          [_dialogueArr addObject:obj];
          textLayer = [CATextLayer layer];
          obj.layer = textLayer;
            //                  textLayer.frame = player.playerView.bounds;
#if TARGET_OS_IPHONE
          textLayer.contentsScale = [UIScreen mainScreen].scale;
#endif
          
          textLayer.wrapped = YES;
          [self.mLayer addSublayer:textLayer];
      }

      
        textLayer.hidden = NO;
        obj.index = index;
        obj.dialogue = ret;

      [self buildAssStyle:textLayer withAssDialogue:ret];
    });
}

- (void)hideDialogue:(Cicada::AssDialogue)ret atIndex:(NSInteger)index
{
    DialogueObj* obj = [self findDialogueObjByIdx:index];
    if (!obj.layer.hidden) {
        dispatch_async(dispatch_get_main_queue(), ^{
            obj.layer.hidden = YES;
        });
    }
}

-(DialogueObj*)findDialogueObjByIdx:(NSInteger)index{
    for (DialogueObj *item in _dialogueArr) {
        if (item.index == index) {
            return item;
        }
    }
    return nil;
}

-(DialogueObj*)findIdleDialogueObj{
    for (DialogueObj *item in _dialogueArr) {
        if (item.layer.hidden) {
            return item;
        }
    }
    return nil;
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

CicadaColor *getSubTitleColor(bool isBGR, NSInteger value)
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

- (NSAttributedString *)buildAssStyleStr:(NSString *)style text:(NSString *)text defaultstyle:(Cicada::AssStyle)defaultstyle factor:(float)factor showOutline:(BOOL)showOutline
{
    NSMutableDictionary<NSAttributedStringKey, id> *attrs = @{}.mutableCopy;

    NSString *fontName = [NSString stringWithCString:defaultstyle.FontName.c_str() encoding:NSUTF8StringEncoding];
    int fontSize = defaultstyle.FontSize;

    NSObject *color = getSubTitleColor(false, defaultstyle.PrimaryColour);
    [attrs setValue:color forKey:NSForegroundColorAttributeName];
    if (showOutline && defaultstyle.BorderStyle==1) {
        if (defaultstyle.Outline>0) {
            NSObject *outlineColour = getSubTitleColor(true, defaultstyle.OutlineColour);
            [attrs setValue:outlineColour forKey:NSStrokeColorAttributeName];
            [attrs setValue:@(defaultstyle.Outline) forKey:NSStrokeWidthAttributeName];
        }
        if (defaultstyle.Shadow>0) {
            NSShadow *shadow = [[NSShadow alloc]init];
            shadow.shadowOffset = CGSizeMake(defaultstyle.Shadow, defaultstyle.Shadow);
            shadow.shadowColor = getSubTitleColor(true, defaultstyle.BackColour);
            shadow.shadowBlurRadius = defaultstyle.Shadow;
            [attrs setValue:shadow forKey:NSShadowAttributeName];
        }
    }
    
    if (defaultstyle.Bold==1) {
        [attrs setValue:@(1) forKey:NSExpansionAttributeName];
    }
    
    if (defaultstyle.Italic==1) {
        [attrs setValue:@(1) forKey:NSObliquenessAttributeName];
    }
    
    if (defaultstyle.Underline==1) {
//        [attrs setValue:color forKey:NSUnderlineColorAttributeName];
        [attrs setValue:@(1) forKey:NSUnderlineStyleAttributeName];
    }
    
    if (defaultstyle.StrikeOut==1) {
        [attrs setValue:@(0) forKey:NSBaselineOffsetAttributeName];
        [attrs setValue:@(NSUnderlineStyleSingle) forKey:NSStrikethroughStyleAttributeName];
        
//        [attrs addAttributes:@{NSStrikethroughStyleAttributeName:@(NSUnderlineStyleSingle),NSBaselineOffsetAttributeName:@(0)} range:NSMakeRange(0, attrs.length)];
    }
    
    
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
    
    fontSize *= factor;
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
    //need font only
    NSMutableDictionary *destDic = @{}.mutableCopy;
    for (NSString *key in dic.allKeys) {
        if ([key isEqualToString:NSParagraphStyleAttributeName]
            ||[key isEqualToString:NSFontAttributeName]) {
            [destDic setValue:dic[key] forKey:key];
        }
    }
 
    textSize = [attrStr.string boundingRectWithSize:CGSizeMake(viewWidth, MAXFLOAT)
                                            options:NSStringDrawingUsesLineFragmentOrigin | NSStringDrawingUsesFontLeading
                                         attributes:destDic
                                            context:nil].size;
    
    CGSize textSize2 = [attrStr.string boundingRectWithSize:CGSizeMake(MAXFLOAT,textSize.height)
                                            options:NSStringDrawingUsesLineFragmentOrigin | NSStringDrawingUsesFontLeading
                                         attributes:destDic
                                            context:nil].size;
    return CGSizeMake(textSize2.width, textSize.height);
}

-(NSArray*) buildAttributedStrBySubtitle:(NSString*)subtitle defaultstyle:(Cicada::AssStyle)assStyle factor:(float)factorY showOutline:(BOOL)showOutline{
    NSArray *lineCodes = [self matchString:subtitle withRegx:@"\\{[^\\{]+\\}"];

    NSMutableAttributedString *attributedStr = [[NSMutableAttributedString alloc] init];
    NSMutableString *allLineCodeStr = @"".mutableCopy;
    if (lineCodes.count > 0) {
        //TODO not for `p0` for the time being
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
                    [attributedStr appendAttributedString:[self buildAssStyleStr:nil text:text defaultstyle:assStyle factor:factorY showOutline:showOutline]];
                }
                NSUInteger begin = range.location + range.length;
                NSString *text = [subtitle substringWithRange:NSMakeRange(begin, end - begin)];
                if (text.length > 0) {
                    if (preStyle.length > 0) {
                        code = [preStyle stringByAppendingString:code];
                        preStyle = @"";
                    }
                    [attributedStr appendAttributedString:[self buildAssStyleStr:code text:text defaultstyle:assStyle factor:factorY showOutline:showOutline]];
                } else {
                    preStyle = [preStyle stringByAppendingString:code];
                }

                if (subtitle.length > end) {
                    subtitle = [subtitle substringFromIndex:end];
                }
            }
        }
    } else {
        [attributedStr appendAttributedString:[self buildAssStyleStr:nil text:subtitle defaultstyle:assStyle factor:factorY showOutline:showOutline]];
    }
    return @[attributedStr,allLineCodeStr];
}

- (void)buildAssStyle:(CATextLayer *)textLayer withAssDialogue:(const AssDialogue)ret
{
    CGFloat x = 0;
    CGFloat y = 0;
    CGFloat w = CGRectGetWidth(self.mLayer.frame);
    CGFloat h = CGRectGetHeight(self.mLayer.frame);

    float factorX = 1.0;
    float factorY = 1.0;
    if (self.mHeader.PlayResX > 0) {
        factorX = w * 1.0f / self.mHeader.PlayResX;
    }
    if (self.mHeader.PlayResY > 0) {
        factorY = h * 1.0f / self.mHeader.PlayResY;
    }

    //TODO detect whether it is the default style
    std::map<std::string, AssStyle> styles = self.mHeader.styles;
    auto iter = styles.begin();
    AssStyle assStyle = iter->second;

    NSString *fontName = [NSString stringWithCString:assStyle.FontName.c_str() encoding:NSUTF8StringEncoding];
    NSString *subtitle = [NSString stringWithCString:ret.Text.c_str() encoding:NSUTF8StringEncoding];
    subtitle = [subtitle stringByReplacingOccurrencesOfString:@"\\N" withString:@"\n"];

//            NSLog(@"====%@",subtitle);
    NSArray *arr = [self buildAttributedStrBySubtitle:subtitle defaultstyle:assStyle factor:factorY showOutline:YES];
    if (arr.count!=2) {
        return;
    }
    NSMutableAttributedString *attributedStr = arr[0];
    NSMutableString *allLineCodeStr = arr[1];
    
    CGSize textSize = CGSizeZero;
    if (attributedStr.length > 0) {
        textLayer.string = attributedStr;
        textSize = getSubTitleHeight(attributedStr, w);
    }
    
    int alignment = assStyle.Alignment;
    int marginL = assStyle.MarginL*factorX;
    int marginR = assStyle.MarginR*factorX;
    int marginV = assStyle.MarginV*factorY;
    
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
                if (anArr.count<=0) {
                    alignment = 7;
                }
                marginV = ((NSString*)posArr[2]).intValue*factorY - textSize.height/2;
                
                if (alignment%3==0) {
                    marginR = ((NSString*)posArr[1]).intValue*factorX - textSize.width/2;
                }else{
                    marginL = ((NSString*)posArr[1]).intValue*factorX - textSize.width/2;
                }
            }
        }
    }
    switch (alignment % 3) {
        //align right
        case 0:
            x = w - textSize.width;
            x -= marginR;
            textLayer.alignmentMode = kCAAlignmentRight;
            break;
        //align left
        case 1:
            x = 0;
            x += marginL;
            textLayer.alignmentMode = kCAAlignmentLeft;
            break;
            //Center
        case 2:
            x = (w - textSize.width) / 2;
            textLayer.alignmentMode = kCAAlignmentCenter;
            break;

        default:
            break;
    }
    switch (alignment / 4) {
            //top
        case 2:
#if TARGET_OS_IPHONE
            y = 0;
            y += marginV;
#else
            y = h - textSize.height;
            y -= marginV;
#endif
            break;
            //Center
        case 1:
            y = (h - textSize.height) / 2;
            break;
        //bottom
        case 0:
#if TARGET_OS_IPHONE
            y = h - textSize.height;
            y -= marginV;
#else
            y = 0;
            y += marginV;
#endif
            break;
        default:
            break;
    }
    
    for (CALayer *subLayer in textLayer.sublayers) {
        [subLayer removeFromSuperlayer];
    }

    [CATransaction begin];
    [CATransaction setDisableActions:YES];
    textLayer.frame = CGRectMake(x, y, textSize.width, textSize.height);
    [CATransaction commit];
    
    NSRange range = NSMakeRange(0, attributedStr.length);
    if (range.length > 0) {
        NSDictionary *dic = [attributedStr attributesAtIndex:0 effectiveRange:&range];
        if ([dic objectForKey:NSStrokeWidthAttributeName]) {
            CATextLayer *strokeLayer = [[CATextLayer alloc] init];
            strokeLayer.frame = textLayer.bounds;

            NSArray *arr = [self buildAttributedStrBySubtitle:subtitle defaultstyle:assStyle factor:factorY showOutline:NO];

            NSMutableAttributedString *frontAttributedStr = arr.firstObject;
            strokeLayer.string = frontAttributedStr;
            strokeLayer.alignmentMode = textLayer.alignmentMode;
            [textLayer addSublayer:strokeLayer];
        }
    }
//#if TARGET_OS_IPHONE
//    textLayer.backgroundColor = [UIColor redColor].CGColor;
//#elif TARGET_OS_OSX
    //    textLayer.backgroundColor = [NSColor red_dialogueArrlor;
//#endif
}

- (void)setup:(CALayer *)view
{
    _mLayer = view;
    _dialogueArr = @[].mutableCopy;
    dispatch_async(dispatch_get_main_queue(), ^{
      [_mLayer addObserver:self forKeyPath:@"bounds" options:NSKeyValueObservingOptionNew context:nil];
    });
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary<NSString *, id> *)change context:(void *)context
{
    if ([keyPath isEqualToString:@"bounds"]) {
#if TARGET_OS_IPHONE
        CGRect bounds = [change[NSKeyValueChangeNewKey] CGRectValue];
#elif TARGET_OS_OSX
        NSRect bounds = [change[@"new"] rectValue];
#endif
        for (DialogueObj *item in _dialogueArr) {
            if (!item.layer.hidden) {
                [self buildAssStyle:item.layer withAssDialogue:item.dialogue];
            }
        }
    }
}

- (void)dealloc
{
    if (strcmp(dispatch_queue_get_label(DISPATCH_CURRENT_QUEUE_LABEL), dispatch_queue_get_label(dispatch_get_main_queue())) == 0) {
        if (self.mLayer) {
            [_mLayer removeObserver:self forKeyPath:@"bounds" context:nil];
        }
    } else {
        // FIXME: use async
        dispatch_sync(dispatch_get_main_queue(), ^{
          if (_mLayer) {
              [_mLayer removeObserver:self forKeyPath:@"bounds" context:nil];
          }
        });
    }
}

@end
