///对应阿里云 SDK 下载的 MediaInfo class
class DownloadModel {
  String videoId;
  String title;
  String coverUrl;
  List<TrackInfoModel> trackInfos;

  DownloadModel({this.videoId, this.title, this.coverUrl, this.trackInfos});

  DownloadModel.fromJson(Map<String, dynamic> jsonMap) {
    videoId = jsonMap['mVideoId'];
    title = jsonMap['mTitle'];
    coverUrl = jsonMap['mCoverUrl'];
    if (jsonMap['mTrackInfos'] != null) {
      trackInfos = List();
      jsonMap['mTrackInfos'].forEach((v) {
        trackInfos.add(TrackInfoModel.fromJson(v));
      });
    }
  }

  Map<String, dynamic> toJson() {
    final Map<String, dynamic> data = Map<String, dynamic>();
    data['mVideoId'] = this.videoId;
    data['mTitle'] = this.title;
    data['mCoverUrl'] = this.coverUrl;
    if (this.trackInfos.isNotEmpty) {
      data['mTrackInfos'] = this.trackInfos.map((e) => e.toJson()).toList();
    }
  }
}

class TrackInfoModel {
  int index;
  int vodFileSize;
  String vodFormat;
  String vodDefinition;

  TrackInfoModel(
      {this.index, this.vodFileSize, this.vodFormat, this.vodDefinition});

  TrackInfoModel.fromJson(Map<String, dynamic> jsonMap) {
    index = jsonMap['index'];
    vodFileSize = jsonMap['vodFileSize'];
    vodFormat = jsonMap['vodFormat'];
    vodDefinition = jsonMap['vodDefinition'];
  }

  Map<String, dynamic> toJson() {
    final Map<String, dynamic> data = Map<String, dynamic>();
    data['index'] = this.index;
    data['vodFileSize'] = this.vodFileSize;
    data['vodFormat'] = this.vodFormat;
    data['vodDefinition'] = this.vodDefinition;
    return data;
  }
}
