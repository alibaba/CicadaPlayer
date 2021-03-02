class FormatterUtils {
  static String getFileSizeDescription(int size) {
    StringBuffer bytes = new StringBuffer();
    if (size >= 1024 * 1024 * 1024) {
      double i = (size / (1024.00 * 1024.00 * 1024.00));
      bytes..write(i.toStringAsFixed(2))..write("G");
    } else if (size >= 1024 * 1024) {
      double i = (size / (1024.00 * 1024.00));
      bytes..write(i.toStringAsFixed(2))..write("M");
    } else if (size >= 1024) {
      double i = (size / (1024.00));
      bytes..write(i.toStringAsFixed(2))..write("K");
    } else if (size < 1024) {
      if (size <= 0) {
        bytes.write("0B");
      } else {
        bytes..write(size)..write("B");
      }
    }
    return bytes.toString();
  }

  ///格式化毫秒数为 xx:xx:xx这样的时间格式。
  static String getTimeformatByMs(int ms) {
    if (ms == null) {
      return "00:00:00";
    }
    int seconds = (ms / 1000).round();
    int finalSec = seconds % 60;
    int finalMin = (seconds / 60 % 60).floor();
    int finalHour = (seconds / 3600).round();

    StringBuffer msBuilder = new StringBuffer("");
    if (finalHour > 9) {
      msBuilder..write(finalHour)..write(":");
    } else if (finalHour > 0) {
      msBuilder..write("0")..write(finalHour)..write(":");
    } else {
      msBuilder..write("00")..write(":");
    }

    if (finalMin > 9) {
      msBuilder..write(finalMin)..write(":");
    } else if (finalMin > 0) {
      msBuilder..write("0")..write(finalMin)..write(":");
    } else {
      msBuilder..write("00")..write(":");
    }

    if (finalSec > 9) {
      msBuilder..write(finalSec);
    } else if (finalSec > 0) {
      msBuilder..write("0")..write(finalSec);
    } else {
      msBuilder.write("00");
    }

    return msBuilder.toString();
  }
}
