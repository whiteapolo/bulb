import std.file;
import std.stdio;
import std.conv;
import std.string;
import std.algorithm;
import std.exception;

const float MIN_BRIGHTNESS = 0.001;
const float MAX_BRIGHTNESS = 1.0;

const string[] POSSIBLE_DEVICES = [
  "/sys/class/backlight/intel_backlight",
  "/sys/class/backlight/acpi_video0"
];

string detectDevicePath() {

  foreach (devicePath; POSSIBLE_DEVICES) {
    if (devicePath.exists) {
      return devicePath;
    }
  }

  throw new Exception("device name not found.");
}

string getDevicePath() {
  static string devicePath = null;
  return devicePath ? devicePath : (devicePath = detectDevicePath());
}

float getBrightness() {
  int maxBrightness = to!int(readText(getDevicePath() ~ "/max_brightness").strip());
  int brightness = to!int(readText(getDevicePath() ~ "/brightness").strip());

  return cast(float)brightness / maxBrightness;
}

void setBrightness(float value) {
  int maxBrightness = to!int(readText(getDevicePath() ~ "/max_brightness").strip());
  int newBrightness = cast(int)(maxBrightness * clamp(value, MIN_BRIGHTNESS, MAX_BRIGHTNESS));

  std.file.write(getDevicePath() ~ "/brightness", to!string(newBrightness));
}

int main(string[] args) {

  if (args.length == 1) {
    writeln(getBrightness());
    return 0;
  }

  enforce(args.length == 3, "Usage: [up|down|set] <percent>");

  auto operator = args[1];
  auto value = args[2];

  enforce(value.isNumeric(), "Argument must be a number");

  final switch (args[1]) {
    case "up":
      setBrightness(getBrightness() + (to!float(args[2]) / 100.0));
      break;
    case "down":
      setBrightness(getBrightness() - (to!float(args[2]) / 100.0));
      break;
    case "set":
      setBrightness(to!float(args[2]) / 100.0);
      break;
  }

  return 0;
}
