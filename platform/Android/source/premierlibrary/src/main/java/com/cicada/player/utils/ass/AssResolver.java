package com.cicada.player.utils.ass;

import android.content.Context;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Typeface;
import android.text.Html;
import android.text.TextUtils;
import android.util.TypedValue;
import android.view.Gravity;
import android.view.ViewGroup;
import android.widget.RelativeLayout;
import android.widget.TextView;

import java.util.HashMap;
import java.util.Map;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class AssResolver {
    public static final String TEXT_PATTERN = "\\{[^\\{]+\\}";
    public static final Pattern pattern = Pattern.compile(TEXT_PATTERN);

    private Map<String, Typeface> mFontTypeface = new HashMap<>();
    private AssHeader mAssHeader;
    private TextViewPool mTextViewPool;

    public AssResolver(Context context) {
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
        if(mTextViewPool != null){
            mTextViewPool.recycle(remove);
        }

    }

    public AssTextView setAssDialog(String content) {
        AssDialogue assDialogue = AssUtils.parseAssDialogue(mAssHeader, content);

        String mText = assDialogue.mText;
        Matcher match = pattern.matcher(mText);
        AssTextView assTextView = mTextViewPool.obtain();
        initTextViewStyle(assTextView, mAssHeader, assDialogue);
        if(match.find()){
            String text = parseSubtitleText(assDialogue);
            assTextView.setText(Html.fromHtml(text));
        }else{
            assTextView.setText(mText);

        }
        return assTextView;
    }

    public void initTextViewStyle(AssTextView assTextView, AssHeader assHeader, AssDialogue assDialogue) {
        Map<String, AssStyle> mStyles = assHeader.mStyles;
        if (mStyles != null) {
            AssStyle assStyle = mStyles.get(assDialogue.mStyle.replace("*", ""));
            setStyle(assTextView, assStyle);
        }
    }

    //set TextView Style
    private void setStyle(AssTextView textView, AssStyle assStyle) {
        if (assStyle != null) {
            textView.setGravity(Gravity.CENTER);
            int style = Typeface.NORMAL;
            String mFontName = assStyle.mFontName;
            Typeface typeface = mFontTypeface.get(mFontName);
            if (typeface != null) {
                textView.setTypeface(typeface);
            }
            double mFontSize = assStyle.mFontSize;
            textView.setTextSize(TypedValue.COMPLEX_UNIT_SP, Math.round(mFontSize));
            int mPrimaryColour = rgbaToArgb(assStyle.mPrimaryColour);
            int mSecondaryColour = assStyle.mSecondaryColour;
            textView.setTextColor(mPrimaryColour);
            //-1 close，0 open
            int mBold = assStyle.mBold;
            if (mBold == -1) {
                style = Typeface.BOLD;
            }
            //-1 close，0 open
            int mItalic = assStyle.mItalic;
            if (mItalic == -1) {
                style = Typeface.ITALIC;
            }
            if (mBold == -1 && mItalic == -1) {
                style = Typeface.BOLD_ITALIC;
            }
            textView.setTypeface(null, style);
            //-1 close，0 open
            int mUnderline = assStyle.mUnderline;
            if (mUnderline == -1) {
                textView.setPaintFlags(textView.getPaintFlags() | Paint.STRIKE_THRU_TEXT_FLAG);
            }
            //-1 close，0 open
            int mStrikeOut = assStyle.mStrikeOut;
            if (mStrikeOut == -1) {
                textView.setPaintFlags(textView.getPaintFlags() | Paint.STRIKE_THRU_TEXT_FLAG);
            }

            int mOutlineColour = assStyle.mOutlineColour;
            double mOutline = assStyle.mOutline;

            int mBorderStyle = assStyle.mBorderStyle;
            int mBackColour = assStyle.mBackColour;
            double mShadow = assStyle.mShadow;
            if (mBorderStyle == 1) {
                textView.setShadowLayer(0, (float) mShadow, (float) mShadow, mBackColour);
            }

            double mScaleX = assStyle.mScaleX;
            textView.setScaleX((float) (mScaleX));
            double mScaleY = assStyle.mScaleY;
            textView.setScaleY((float) (mScaleY));
            double mAngle = assStyle.mAngle;
            textView.setRotation((float) mAngle);
            int mAlignment = assStyle.mAlignment;
            RelativeLayout.LayoutParams layoutParams = new RelativeLayout.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT);
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
            }
            layoutParams.leftMargin = assStyle.mMarginL;
            layoutParams.rightMargin = assStyle.mMarginR;
            layoutParams.topMargin = assStyle.mMarginV;
            int mEncoding = assStyle.mEncoding;
            textView.setLayoutParams(layoutParams);
        }
    }

    private int rgbaToArgb(int mPrimaryColour) {
        //vb alpha convertTo Android alpha
        int alpha = (0xff - (mPrimaryColour & 0xff)) << 24;
        String argbStr = String.format("#%04x", (alpha | (mPrimaryColour >>> 8)));
        return Color.parseColor(argbStr);
    }

    private String parseSubtitleText(AssDialogue assDialogue) {
        String result;
        Matcher compleMatcher = pattern.matcher(assDialogue.mText);
        //contains Style Cover Code
        if (compleMatcher.find()) {
            //DrawingCommands
            if(assDialogue.mText.contains("{\\p0}")){
                return "";
            }else{
                StringBuilder subtitleTextStringBuilder = new StringBuilder();
                String[] splitArrays = assDialogue.mText.split(TEXT_PATTERN, -1);
                int index = 0;
                Matcher findMatch = pattern.matcher(assDialogue.mText);
                while (findMatch.find()) {
                    String styleStr = findMatch.group();

                    //append Subtitle Text
                    if (index == 0 && !TextUtils.isEmpty(splitArrays[0])) {
                        subtitleTextStringBuilder.append(splitArrays[0]);
                    }

                    //parse style to HTML Text
                    if (index < splitArrays.length) {
                        subtitleTextStringBuilder.append(convertToHtmlText(styleStr, splitArrays[index + 1]));
                    }

                    index++;
                }

                if (index == 0 && splitArrays.length > 0 && !TextUtils.isEmpty(splitArrays[0])) {
                    subtitleTextStringBuilder.append(splitArrays[0]);
                }

                for (index++; index < splitArrays.length; index++) {
                    if (!TextUtils.isEmpty(splitArrays[index])) {
                        subtitleTextStringBuilder.append(splitArrays[index]);
                    }
                }
                result = subtitleTextStringBuilder.toString();
            }
        } else {
            result = assDialogue.mText;
        }
        return result.replaceAll(TEXT_PATTERN, "").replace("\\n", "<br />").replace("\\N", "<br />");
    }

    /**
     * Subtitle Style convertTo HTML Style Text
     */
    private String convertToHtmlText(String styleStr, String splitSubtitle) {
        StringBuilder result = new StringBuilder();
        int start = styleStr.indexOf("{");
        int end = styleStr.lastIndexOf("}");
        //substring() delete "{" and "}"，replaceAll() change "\" to "$"
        styleStr = styleStr.substring(start + 1, end).replaceAll("\\\\", "\\$");
        if (styleStr.contains("$")) {
            result.append("<font");
            String[] styles = styleStr.split("\\$");
            for (int i = 0; i < styles.length; i++) {
                String style = styles[i];
                if (style.startsWith("fn")) {
                    String face = style.substring("fn".length()).trim();
                    result.append(" face=\"").append(face).append("\"");

                } else if (style.startsWith("fs")) {
                    String size = style.substring("fs".length()).trim();
                    result.append(" size=\"").append(size).append("\"");

                } else if (style.startsWith("b1") || style.startsWith("i1") || style.startsWith("u1") || style.startsWith("s1")) {
                    if (style.startsWith("b1")) {
                        splitSubtitle = "<b>" + splitSubtitle + "</b>";
                    } else if (style.startsWith("i1")) {
                        splitSubtitle = "<i>" + splitSubtitle + "</i>";
                    } else if (style.startsWith("u1")) {
                        splitSubtitle = "<u>" + splitSubtitle + "</u>";
                    } else if (style.startsWith("s1")) {
                        splitSubtitle = "<s>" + splitSubtitle + "</s>";
                    }

                } else if (style.startsWith("c&H") || style.startsWith("1c&H")) {
                    int endIndex = style.lastIndexOf("&");
                    style = style.substring(0, endIndex).trim();
                    String color = "";
                    if (style.startsWith("c&H")) {
                        color = convertRgbColor(style.substring("c&H".length()).trim());
                    } else {
                        color = convertRgbColor(style.substring("1c&H".length()).trim());
                    }
                    result.append(" color=\"#").append(color).append("\"");

                }
            }
            result.append(">");
        }
        if (result.length() > 0) {
            result.append(splitSubtitle);
            result.append("</font>");
        } else {
            result.append(splitSubtitle);
        }
        return result.toString();
    }

    public String convertRgbColor(String abgrColorString) {
        if (abgrColorString.length() == 8) {
            return abgrColorString.substring(6, 8) + abgrColorString.substring(4, 6) + abgrColorString.substring(2, 4);
        }
        return abgrColorString.substring(4, 6) + abgrColorString.substring(2, 4) + abgrColorString.substring(0, 2);
    }
}
