package com.cicada.player.utils.ass;

import android.content.Context;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Typeface;
import android.text.SpannableStringBuilder;
import android.text.Spanned;
import android.text.TextUtils;
import android.text.style.AbsoluteSizeSpan;
import android.text.style.ForegroundColorSpan;
import android.text.style.StrikethroughSpan;
import android.text.style.StyleSpan;
import android.text.style.TypefaceSpan;
import android.text.style.UnderlineSpan;
import android.view.Gravity;
import android.widget.RelativeLayout;
import android.widget.TextView;

import java.util.HashMap;
import java.util.LinkedList;
import java.util.Map;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class AssResolver {

    private static final String TAG = AssResolver.class.getSimpleName();

    public static final String TEXT_PATTERN = "\\{[^\\{]+\\}";
    public static final Pattern pattern = Pattern.compile(TEXT_PATTERN);

    private Map<String, Typeface> mFontTypeface = new HashMap<>();
    private AssHeader mAssHeader;
    private TextViewPool mTextViewPool;
    private Context mContext;

    public AssResolver(Context context) {
        mContext = context;
        mTextViewPool = new TextViewPool(context);
    }

    public void destroy() {
    }

    public void setFontTypeMap(Map<String, Typeface> fontTypeface) {
        this.mFontTypeface = fontTypeface;
    }

    public void setAssHeaders(String header) {
        mAssHeader = AssUtils.parseAssHeader(header);
    }

    public void dismiss(AssTextView remove) {
        if (mTextViewPool != null) {
            mTextViewPool.recycle(remove);
        }

    }

    private int videoDisplayWidth = -1;
    private int videoDisplayHeight = -1;

    public void setVideoDisplaySize(int width, int height) {
        videoDisplayWidth = width;
        videoDisplayHeight = height;
    }


    private class ContentAttribute {
        public String text;
        public String overrideStyle;
        public String fontName;
        public double fontSize;
        public int mPrimaryColour;
        public int mSecondaryColour;
        public boolean mBold;
        public boolean mItalic;
        public boolean mUnderline;
        public boolean mStrikeOut;
        public int mOutlineColour;
        public double mOutlineWidth;
        public int mBorderStyle;
        public int mBackColour;//used for outline or shadow
        public double mShadow;

        @Override
        public String toString() {
            return "ContentAttribute{" +
                    "text='" + text + '\'' +
                    ", overrideStyle='" + overrideStyle + '\'' +
                    ", fontName='" + fontName + '\'' +
                    ", fontSize=" + fontSize +
                    ", mPrimaryColour=" + mPrimaryColour +
                    ", mSecondaryColour=" + mSecondaryColour +
                    ", mBold=" + mBold +
                    ", mItalic=" + mItalic +
                    ", mUnderline=" + mUnderline +
                    ", mStrikeOut=" + mStrikeOut +
                    ", mOutlineColour=" + mOutlineColour +
                    ", mOutlineWidth=" + mOutlineWidth +
                    ", mBorderStyle=" + mBorderStyle +
                    ", mBackColour=" + mBackColour +
                    ", mShadow=" + mShadow +
                    '}';
        }
    }

    private class LocationAttribute {
        public int posX;
        public int posY;
        public int mAlignment;
        public int marginL;
        public int marginR;
        public int marginV;

        public double mScaleX;
        public double mScaleY;
        public double mAngle;
    }

    private class OverrideStyle {
        public String fontName;
        public double fontSize;
        public int mPrimaryColour;
        public boolean mBold;
        public boolean mItalic;
        public boolean mUnderline;
        public boolean mStrikeOut;

        public int posX;
        public int posY;
    }


    public AssTextView setAssDialog(String content) {

        AssTextView assTextView = mTextViewPool.obtain();
        assTextView.setContent(content);
        AssDialogue assDialogue = AssUtils.parseAssDialogue(mAssHeader, content);
        AssStyle assStyle = mAssHeader.mStyles.get(assDialogue.mStyle.replace("*", ""));

        if (assDialogue.mText.contains("{\\p0}")) {
            //TODO DrawingCommands not support
            return assTextView;
        }

        //1.split content by {}...
        LinkedList<ContentAttribute> contentAttributeLinkedList = splitContent(assDialogue);
        //2.fill LocationAttribute and ContentAttribute
        LocationAttribute locationAttribute = getLocationAttribute(assDialogue, assStyle);
        fillContentAttribute(assStyle, contentAttributeLinkedList, locationAttribute);
        //3.generate SpannableString.
        SpannableStringBuilder displayStr = getFinalStr(contentAttributeLinkedList);
        //4.fill view Location info.
        RelativeLayout.LayoutParams params = getLayoutParams(locationAttribute);
        assTextView.setText(displayStr);
        assTextView.setScaleX((float) locationAttribute.mScaleX);
        assTextView.setScaleY((float) locationAttribute.mScaleY);
        assTextView.setRotation((float) locationAttribute.mAngle);
        float measuredWidth = getFinalStrWidth(contentAttributeLinkedList);
        params.width = (int) measuredWidth;
        params.height = RelativeLayout.LayoutParams.WRAP_CONTENT;
        assTextView.setLayoutParams(params);
        assTextView.setGravity(Gravity.CENTER);
        return assTextView;
    }

    private RelativeLayout.LayoutParams getLayoutParams(LocationAttribute locationAttribute) {
        RelativeLayout.LayoutParams layoutParams = new RelativeLayout.LayoutParams(RelativeLayout.LayoutParams.WRAP_CONTENT, RelativeLayout.LayoutParams.WRAP_CONTENT);

        if (locationAttribute.posX > 0 || locationAttribute.posY > 0) {
            layoutParams.leftMargin = (int) scaleXSize(locationAttribute.posX);
            layoutParams.topMargin = (int) scaleYSize(locationAttribute.posY);
        } else {
            int mAlignment = locationAttribute.mAlignment;
            switch (mAlignment) {
                case 1:
                    layoutParams.addRule(RelativeLayout.ALIGN_PARENT_BOTTOM);
                    layoutParams.addRule(RelativeLayout.ALIGN_PARENT_LEFT);
                    break;
                case 2:
                    layoutParams.addRule(RelativeLayout.ALIGN_PARENT_BOTTOM);
                    layoutParams.addRule(RelativeLayout.CENTER_HORIZONTAL);
                    break;
                case 3:
                    layoutParams.addRule(RelativeLayout.ALIGN_PARENT_BOTTOM);
                    layoutParams.addRule(RelativeLayout.ALIGN_PARENT_RIGHT);
                    break;
                case 4:
                    layoutParams.addRule(RelativeLayout.CENTER_VERTICAL);
                    layoutParams.addRule(RelativeLayout.ALIGN_PARENT_LEFT);
                    break;
                case 5:
                    layoutParams.addRule(RelativeLayout.CENTER_IN_PARENT);
                    break;
                case 6:
                    layoutParams.addRule(RelativeLayout.CENTER_VERTICAL);
                    layoutParams.addRule(RelativeLayout.ALIGN_PARENT_RIGHT);
                    break;
                case 7:
                    layoutParams.addRule(RelativeLayout.ALIGN_PARENT_TOP);
                    layoutParams.addRule(RelativeLayout.ALIGN_PARENT_LEFT);
                    break;
                case 8:
                    layoutParams.addRule(RelativeLayout.ALIGN_PARENT_TOP);
                    layoutParams.addRule(RelativeLayout.CENTER_HORIZONTAL);
                    break;
                case 9:
                    layoutParams.addRule(RelativeLayout.ALIGN_PARENT_TOP);
                    layoutParams.addRule(RelativeLayout.ALIGN_PARENT_RIGHT);
                    break;
                default:
                    layoutParams.addRule(RelativeLayout.ALIGN_PARENT_BOTTOM);
                    layoutParams.addRule(RelativeLayout.CENTER_HORIZONTAL);
                    break;
            }
            layoutParams.leftMargin = locationAttribute.marginL;
            layoutParams.rightMargin = locationAttribute.marginR;
            layoutParams.topMargin = locationAttribute.marginV;
        }


        return layoutParams;
    }

    private float getFinalStrWidth(LinkedList<ContentAttribute> contentAttributeLinkedList) {
        float newlineLen = 0;
        float maxLineLen = 0;
        TextView measureTextView = new TextView(mContext);
        for (ContentAttribute contentAttribute : contentAttributeLinkedList) {
            measureTextView.setTextSize((float) contentAttribute.fontSize);
            Paint paint = measureTextView.getPaint();
            float tmplen = paint.measureText(contentAttribute.text);
            newlineLen += tmplen;
            if (contentAttribute.text.endsWith("\n")) {
                if (maxLineLen < newlineLen) {
                    maxLineLen = newlineLen;
                    newlineLen = 0;
                }
            }
        }
        if (maxLineLen < newlineLen) {
            maxLineLen = newlineLen;
        }

        return maxLineLen;
    }

    private SpannableStringBuilder getFinalStr(LinkedList<ContentAttribute> contentAttributeLinkedList) {
        SpannableStringBuilder spanBuilder = new SpannableStringBuilder();
        int start = 0;
        int end = 0;
        for (ContentAttribute contentAttribute : contentAttributeLinkedList) {
            end += contentAttribute.text.length();
            spanBuilder.append(contentAttribute.text);

            if (contentAttribute.mOutlineWidth > 0) {
                BorderedSpan.BorderStyle borderStyle = new BorderedSpan.BorderStyle();
                borderStyle.fontName = contentAttribute.fontName;
                borderStyle.fontSize = contentAttribute.fontSize;
                borderStyle.mPrimaryColour = contentAttribute.mPrimaryColour;
                borderStyle.mSecondaryColour = contentAttribute.mSecondaryColour;
                borderStyle.mBold = contentAttribute.mBold;
                borderStyle.mItalic = contentAttribute.mItalic;
                borderStyle.mUnderline = contentAttribute.mUnderline;
                borderStyle.mStrikeOut = contentAttribute.mStrikeOut;
                borderStyle.mOutlineColour = contentAttribute.mOutlineColour;
                borderStyle.mOutlineWidth = contentAttribute.mOutlineWidth;
                if (contentAttribute.mBorderStyle == 1) {
                    borderStyle.mShadowWidth = contentAttribute.mShadow;
                    borderStyle.mShadowColor = contentAttribute.mBackColour;
                }
                spanBuilder.setSpan(new BorderedSpan(borderStyle), start, end, Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
            }

            spanBuilder.setSpan(new TypefaceSpan(contentAttribute.fontName), start, end, Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
            spanBuilder.setSpan(new AbsoluteSizeSpan((int) contentAttribute.fontSize), start, end, Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
            spanBuilder.setSpan(new ForegroundColorSpan(contentAttribute.mPrimaryColour), start, end, Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);

            if (contentAttribute.mBold && contentAttribute.mItalic) {
                spanBuilder.setSpan(new StyleSpan(Typeface.BOLD_ITALIC), start, end, Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
            } else if (contentAttribute.mBold) {
                spanBuilder.setSpan(new StyleSpan(Typeface.BOLD), start, end, Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
            } else if (contentAttribute.mItalic) {
                spanBuilder.setSpan(new StyleSpan(Typeface.ITALIC), start, end, Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
            }

            if (contentAttribute.mUnderline) {
                spanBuilder.setSpan(new UnderlineSpan(), start, end, Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
            }

            if (contentAttribute.mStrikeOut) {
                spanBuilder.setSpan(new StrikethroughSpan(), start, end, Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
            }


            start += contentAttribute.text.length();
        }
        return spanBuilder;
    }

    private void fillContentAttribute(AssStyle assStyle, LinkedList<ContentAttribute> contentAttributeLinkedList, LocationAttribute locationAttribute) {
        int size = contentAttributeLinkedList.size();
        for (int i = 0; i < size; i++) {
            ContentAttribute contentAttribute = contentAttributeLinkedList.get(i);
            String text = contentAttribute.text;
            // only need replace \N
            contentAttribute.text = text.replace("\\N", "\n").replace("\\n", "\n");
            contentAttribute.fontName = assStyle.mFontName;
            contentAttribute.fontSize = scaleYSize(assStyle.mFontSize);
            contentAttribute.mBold = (assStyle.mBold == -1);
            contentAttribute.mItalic = (assStyle.mItalic == -1);
            contentAttribute.mStrikeOut = (assStyle.mStrikeOut == -1);
            contentAttribute.mUnderline = (assStyle.mUnderline == -1);
            contentAttribute.mBorderStyle = assStyle.mBorderStyle;
            contentAttribute.mBackColour = assStyle.mBackColour;
            contentAttribute.mOutlineColour = rgbaToArgb(assStyle.mOutlineColour);
            contentAttribute.mOutlineWidth = scaleYSize(assStyle.mOutline);
            contentAttribute.mPrimaryColour = rgbaToArgb(assStyle.mPrimaryColour);
            contentAttribute.mSecondaryColour = assStyle.mSecondaryColour;
            contentAttribute.mShadow = scaleYSize(assStyle.mShadow);

            if (!TextUtils.isEmpty(contentAttribute.overrideStyle)) {

                Map<String, Object> overrideStyle = parseOverrideStyle(contentAttribute.overrideStyle);
                if (overrideStyle != null) {
                    if (overrideStyle.containsKey("primaryColour")) {
                        contentAttribute.mPrimaryColour = (int) overrideStyle.get("primaryColour");
                    }
                    if (overrideStyle.containsKey("strikeOut")) {
                        contentAttribute.mStrikeOut = (boolean) overrideStyle.get("strikeOut");
                    }
                    if (overrideStyle.containsKey("underline")) {
                        contentAttribute.mUnderline = (boolean) overrideStyle.get("underline");
                    }
                    if (overrideStyle.containsKey("italic")) {
                        contentAttribute.mItalic = (boolean) overrideStyle.get("italic");
                    }
                    if (overrideStyle.containsKey("bold")) {
                        contentAttribute.mBold = (boolean) overrideStyle.get("bold");
                    }
                    if (overrideStyle.containsKey("fontSize")) {
                        contentAttribute.fontSize = scaleYSize((double) overrideStyle.get("fontSize"));
                    }
                    if (overrideStyle.containsKey("fontName")) {
                        contentAttribute.fontName = (String) overrideStyle.get("fontName");
                    }
                    if (overrideStyle.containsKey("posX")) {
                        locationAttribute.posX = (int) overrideStyle.get("posX");
                    }
                    if (overrideStyle.containsKey("posY")) {
                        locationAttribute.posY = (int) overrideStyle.get("posY");
                    }
                }
            } else {
            }
        }
    }

    private LocationAttribute getLocationAttribute(AssDialogue assDialogue, AssStyle assStyle) {
        LocationAttribute locationAttribute = new LocationAttribute();
        locationAttribute.mAlignment = assStyle.mAlignment;
        locationAttribute.marginL = assStyle.mMarginL;
        locationAttribute.marginR = assStyle.mMarginR;
        locationAttribute.marginV = assStyle.mMarginV;

        if (assDialogue.mMarginL != 0) {
            locationAttribute.marginL = assDialogue.mMarginL;
        }
        if (assDialogue.mMarginR != 0) {
            locationAttribute.marginR = assDialogue.mMarginR;
        }
        if (assDialogue.mMarginV != 0) {
            locationAttribute.marginV = assDialogue.mMarginV;
        }

        locationAttribute.mAngle = assStyle.mAngle;
        locationAttribute.mScaleX = assStyle.mScaleX;
        locationAttribute.mScaleY = assStyle.mScaleY;

        return locationAttribute;
    }

    private LinkedList<ContentAttribute> splitContent(AssDialogue assDialogue) {
        LinkedList<ContentAttribute> contentAttributeLinkedList = new LinkedList<>();
        Matcher findMatch = pattern.matcher(assDialogue.mText);
        if (findMatch.find()) {
            String[] splitContent = assDialogue.mText.split(TEXT_PATTERN, -1);
            for (int i = 0; i < splitContent.length; i++) {
                ContentAttribute contentAttribute = null;
                if (!TextUtils.isEmpty(splitContent[i])) {
                    contentAttribute = new ContentAttribute();
                    contentAttribute.text = splitContent[i];
                }

                if (i != 0) {
                    String styleStr = findMatch.group();
                    if (contentAttribute != null) {
                        contentAttribute.overrideStyle = styleStr;
                    }
                }

                if (contentAttribute != null) {
                    contentAttributeLinkedList.add(contentAttribute);
                }
            }

        } else {
            ContentAttribute contentAttribute = new ContentAttribute();
            contentAttribute.text = assDialogue.mText;
            contentAttributeLinkedList.add(contentAttribute);
        }
        return contentAttributeLinkedList;
    }

    private Map<String, Object> parseOverrideStyle(String styleStr) {
        int start = styleStr.indexOf("{");
        int end = styleStr.lastIndexOf("}");
        //substring() delete "{" and "}"，replaceAll() change "\" to "$"
        styleStr = styleStr.substring(start + 1, end).replaceAll("\\\\", "\\$");

        if (styleStr.contains("$")) {
            Map<String, Object> overrideStyle = new HashMap<>();
            String[] styles = styleStr.split("\\$");
            for (String style : styles) {
                if (style.startsWith("fn")) {
                    overrideStyle.put("fontName", style.substring("fn".length()).trim());
                } else if (style.startsWith("fs")) {
                    String size = style.substring("fs".length()).trim();
                    overrideStyle.put("fontSize", Double.valueOf(size));
                } else if (style.startsWith("b")) {
                    overrideStyle.put("bold", style.startsWith("b1"));
                } else if (style.startsWith("i")) {
                    overrideStyle.put("italic", style.startsWith("i1"));
                } else if (style.startsWith("u")) {
                    overrideStyle.put("underline", style.startsWith("u1"));
                } else if (style.startsWith("s")) {
                    overrideStyle.put("strikeOut", style.startsWith("s1"));
                } else if (style.startsWith("c&H") || style.startsWith("1c&H")) {
                    int endIndex = style.lastIndexOf("&");
                    style = style.substring(0, endIndex).trim();
                    String color = "#";
                    if (style.startsWith("c&H")) {
                        color += convertRgbColor(style.substring("c&H".length()).trim());
                    } else {
                        color += convertRgbColor(style.substring("1c&H".length()).trim());
                    }
                    overrideStyle.put("primaryColour", Color.parseColor(color));

                } else if (style.startsWith("pos")) {
                    //position
                    String[] posSplit = style.substring("pos(".length(), style.length() - 1).split(",");
                    overrideStyle.put("posX", Integer.valueOf(posSplit[0]));
                    overrideStyle.put("posY", Integer.valueOf(posSplit[1]));
                }
            }
            return overrideStyle;
        }
        return null;
    }

    private int rgbaToArgb(int color) {
        //vb alpha convertTo Android alpha
        int alpha = (0xff - (color & 0xff)) << 24;
        String argbStr = String.format("#%04x", (alpha | (color >>> 8)));
        return Color.parseColor(argbStr);
    }

    private String convertRgbColor(String abgrColorString) {
        if (abgrColorString.length() == 8) {
            return abgrColorString.substring(6, 8) + abgrColorString.substring(4, 6) + abgrColorString.substring(2, 4);
        }
        return abgrColorString.substring(4, 6) + abgrColorString.substring(2, 4) + abgrColorString.substring(0, 2);
    }

    private double scaleYSize(double size) {
        if (videoDisplayHeight > 0 && mAssHeader.mPlayResY > 0) {
            return size * videoDisplayHeight / mAssHeader.mPlayResY;
        } else {
            return size;
        }
    }

    private double scaleXSize(double size) {
        if (videoDisplayWidth > 0 && mAssHeader.mPlayResX > 0) {
            return size * videoDisplayWidth / mAssHeader.mPlayResX;
        } else {
            return size;
        }
    }
}
