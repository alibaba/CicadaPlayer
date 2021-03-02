import 'package:sqflite/sqflite.dart';

class DBUtils {
  static final DBUtils _instance = DBUtils._privateConstructor();

  ///数据库名字
  static const String DB_NAME = "AliyunPlayerDownload.db";

  ///表名
  static const String TABLE_NAME = "player_download_info";

  ///主键
  static const String ID = "id";

  ///视频 id
  static const String VID = "mVideoId";

  ///清晰度
  static const String DENFITION = "mVodDefinition";

  ///trackId
  static const String TRACKINDEX = "mIndex";

  ///标题
  static const String TITLE = "mTitle";

  ///封面
  static const String COVER = "mCoverUrl";

  ///大小
  static const String SIZE = "mVodFileSize";

  ///本地路径
  static const String PATH = "mSavePath";

  ///格式
  static const String FORMAT = "mVodFormat";

  ///下载状态
  static const String DOWNLOADSTATEE = "mDownloadState";

  ///下载信息
  static const String DOWNLOADMSG = "mDownloadMsg";

  ///下载方式
  static const String DOWNLOADMODETYPE = "mDownloadModeType";

  ///下载进度
  static const String PROGRESS = "progress";

  ///建表语句
  static const String CREATE_TABLE_SQL =
      'CREATE TABLE if not exists $TABLE_NAME ( $ID integer primary key autoincrement, $VID text, $DENFITION text, $TRACKINDEX integer, $TITLE text, $COVER text, $SIZE text, $PATH text, $FORMAT text, $PROGRESS integer, $DOWNLOADSTATEE integer, $DOWNLOADMSG text, $DOWNLOADMODETYPE integer)';

  static var _dbPath;
  static Database _dataBase;

  DBUtils._privateConstructor();

  static DBUtils get instance {
    return _instance;
  }

  static Future<void> openDB() async {
    var databasePath = await getDatabasesPath();
    _dbPath = databasePath + "/" + DB_NAME;
    print("cicadaPlayer : dbPath = $_dbPath");

    _dataBase = await openDatabase(_dbPath, version: 1,
        onCreate: (Database db, int version) async {
      await db.execute(CREATE_TABLE_SQL);
    });
  }

  void deleteDB() async {
    await deleteDatabase(_dbPath);
  }

  void closeDB() async {
    await _dataBase.close();
  }

  Future<void> insert(Map<String, dynamic> values) async {
    await _dataBase.insert(TABLE_NAME, values);
  }

  Future<void> update(Map<String, dynamic> value) async {
    String where = "$VID = ? and $TRACKINDEX = ?";
    var whereArgs = [value["mVideoId"], value["mIndex"]];
    await _dataBase.update(TABLE_NAME, value,
        where: where, whereArgs: whereArgs);
  }

  Future<void> delete(Map<String, dynamic> value) async {
    String where = "$VID = ? and $TRACKINDEX = ?";
    var whereArgs = [value["mVideoId"], value["mIndex"]];
    await _dataBase.delete(TABLE_NAME, where: where, whereArgs: whereArgs);
  }

  Future<List<Map<String, dynamic>>> selectAll() async {
    return await _dataBase.query(TABLE_NAME);
  }
}
