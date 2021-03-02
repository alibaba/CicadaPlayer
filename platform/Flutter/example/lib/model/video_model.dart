class VideoListModel {
  int total;
  List<VideoModel> videoList;

  VideoListModel({this.total, this.videoList});

  VideoListModel.fromJson(Map<String, dynamic> json) {
    total = json['total'];
    if (json['videoList'] != null) {
      videoList = new List<VideoModel>();
      json['videoList'].forEach((v) {
        videoList.add(new VideoModel.fromJson(v));
      });
    }
  }

  Map<String, dynamic> toJson() {
    final Map<String, dynamic> data = new Map<String, dynamic>();
    data['total'] = this.total;
    if (this.videoList != null) {
      data['videoList'] = this.videoList.map((v) => v.toJson()).toList();
    }
    return data;
  }
}

class VideoModel {
  String videoId;
  String uuid;
  String title;
  String tags;
  String status;
  int size;
  double duration;
  String description;
  String createTime;
  String modifyTime;
  String modificationTime;
  String creationTime;
  String coverUrl;
  int cateId;
  String cateName;
  String storageLocation;
  String fileUrl;

  VideoModel(
      {this.videoId,
      this.uuid,
      this.title,
      this.tags,
      this.status,
      this.size,
      this.duration,
      this.description,
      this.createTime,
      this.modifyTime,
      this.modificationTime,
      this.creationTime,
      this.coverUrl,
      this.cateId,
      this.cateName,
      this.storageLocation,
      this.fileUrl});

  VideoModel.fromJson(Map<String, dynamic> json) {
    videoId = json['videoId'];
    uuid = json['uuid'];
    title = json['title'];
    tags = json['tags'];
    status = json['status'];
    size = json['size'];
    duration = json['duration'];
    description = json['description'];
    createTime = json['createTime'];
    modifyTime = json['modifyTime'];
    modificationTime = json['modificationTime'];
    creationTime = json['creationTime'];
    coverUrl = json['coverUrl'];
    cateId = json['cateId'];
    cateName = json['cateName'];
    storageLocation = json['storageLocation'];
    fileUrl = json['fileUrl'];
  }

  Map<String, dynamic> toJson() {
    final Map<String, dynamic> data = new Map<String, dynamic>();
    data['videoId'] = this.videoId;
    data['uuid'] = this.uuid;
    data['title'] = this.title;
    data['tags'] = this.tags;
    data['status'] = this.status;
    data['size'] = this.size;
    data['duration'] = this.duration;
    data['description'] = this.description;
    data['createTime'] = this.createTime;
    data['modifyTime'] = this.modifyTime;
    data['modificationTime'] = this.modificationTime;
    data['creationTime'] = this.creationTime;
    data['coverUrl'] = this.coverUrl;
    data['cateId'] = this.cateId;
    data['cateName'] = this.cateName;
    data['storageLocation'] = this.storageLocation;
    data['fileUrl'] = this.fileUrl;
    return data;
  }
}
