# HLS support
- Supported format
  - Container: ts, mp4, aac
  - Codec: AVC, HEVC, AAC, ac3(iOS)
  - Subtitle: webvtt
  - Data source: http, https, local file
  - Type: live, vod
  - Encryption: aes-128, **sample-aes**, private extend
  - Play list type: media and master playlist

- Seamless switch supported
  - Codec: the sample codec in each media play list
  - Aligned: by **sequence No.** for now
  
- Not fully supported tags
  - [EXT-X-DISCONTINUITY](https://tools.ietf.org/html/rfc8216#page-14)

        Supported features:

            1. timestamp sequence change
            2. encoding parameters change
            3. encoding sequence change

        Not supported features

            1. file format change
            2. number, type, and identifiers of tracks change


- Not be Supported yet tags
  - [EXT-X-PROGRAM-DATE-TIME](https://tools.ietf.org/html/rfc8216#page-18)
  - [EXT-X-DATERANGE](https://tools.ietf.org/html/rfc8216#page-18)
  - [EXT-X-DISCONTINUITY-SEQUENCE](https://tools.ietf.org/html/rfc8216#page-23)
  - [EXT-X-I-FRAMES-ONLY](https://tools.ietf.org/html/rfc8216#page-24)
  - [EXT-X-I-FRAME-STREAM-INF](https://tools.ietf.org/html/rfc8216#page-33)
  - [EXT-X-SESSION-DATA](https://tools.ietf.org/html/rfc8216#page-34)
  - [EXT-X-SESSION-KEY](https://tools.ietf.org/html/rfc8216#page-35)
  - ...

- Plan
    - Fully support EXT-X-DISCONTINUITY
    - DRM support


  
  
