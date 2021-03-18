import 'package:flutter_driver/driver_extension.dart';
import 'ui/t_player.dart' as player;

void main() {
  // This line enables the extension.
  enableFlutterDriverExtension();

  // Call the `main()` function of the app, or call `runApp` with
  // any widget you are interested in testing.
  player.main();
}