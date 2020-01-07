package com.cicada.player.demo.bean;

import java.io.Serializable;
import java.util.List;

/**
 * 播放数据对象
 */
public class PlayerMediaInfo implements Serializable{
        //通用
        private String name;
        private List<TypeInfo> samples;

        public static class TypeInfo implements Serializable{
            //通用
            private String name;
            private String type;
            //URI
            private String url;
            //字幕
            private SubtitleInfo subtitle;

            public static class SubtitleInfo implements Serializable{
                private String cn;
                private String en;

                public String getCn() {
                    return cn;
                }

                public void setCn(String cn) {
                    this.cn = cn;
                }

                public String getEn() {
                    return en;
                }

                public void setEn(String en) {
                    this.en = en;
                }
            }

            private boolean isChecked;

            public String getName() {
                return name;
            }

            public void setName(String name) {
                this.name = name;
            }

            public String getUrl() {
                return url;
            }

            public void setUrl(String uri) {
                this.url = uri;
            }

            public String getType() {
                return type;
            }

            public void setType(String type) {
                this.type = type;
            }

            public boolean isChecked() {
                return isChecked;
            }

            public void setChecked(boolean checked) {
                isChecked = checked;
            }

            public SubtitleInfo getSubtitle() {
                return subtitle;
            }

            public void setSubtitle(SubtitleInfo subtitle) {
                this.subtitle = subtitle;
            }
        }

        public String getName() {
            return name;
        }

        public void setName(String name) {
            this.name = name;
        }

        public List<TypeInfo> getSamples() {
            return samples;
        }

        public void setSamples(List<TypeInfo> samples) {
            this.samples = samples;
        }
}
