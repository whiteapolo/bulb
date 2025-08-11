import std.file;
import std.stdio;
import std.conv;
import std.string;
import std.algorithm;
import std.exception;
import std.getopt;

const float MIN_BRIGHTNESS = 0.1;
const float MAX_BRIGHTNESS = 100.0;

const string[] POSSIBLE_DEVICES = [
    "/sys/class/backlight/intel_backlight", "/sys/class/backlight/acpi_video0"
];

string detectDevicePath()
{
    foreach (devicePath; POSSIBLE_DEVICES)
    {
        if (devicePath.exists)
        {
            return devicePath;
        }
    }

    throw new Exception("device name not found.");
}

string getDevicePath()
{
    static string devicePath = null;
    return devicePath ? devicePath : (devicePath = detectDevicePath());
}

float getBrightness()
{
    int maxBrightness = to!int(readText(getDevicePath() ~ "/max_brightness").strip());
    int brightness = to!int(readText(getDevicePath() ~ "/brightness").strip());

    return (cast(float) brightness / maxBrightness) * 100;
}

void setBrightness(float value)
{
    int maxBrightness = to!int(readText(getDevicePath() ~ "/max_brightness").strip());
    int newBrightness = cast(int)(maxBrightness * clamp(value, MIN_BRIGHTNESS,
            MAX_BRIGHTNESS) / 100.0);

    std.file.write(getDevicePath() ~ "/brightness", to!string(newBrightness));
}

void argumentHandler(string option, string value)
{
    final switch (option)
    {
    case "up":
        setBrightness(getBrightness() + (to!float(value)));
        break;
    case "down":
        setBrightness(getBrightness() - (to!float(value)));
        break;
    case "set":
        setBrightness(to!float(value));
        break;
    }
}

int main(string[] args)
{
    if (args.length == 1)
    {
        writeln(getBrightness(), "%");
        return 0;
    }

    try
    {
        auto opt = getopt(args, "set", &argumentHandler, "up",
                &argumentHandler, "down", &argumentHandler);

        if (opt.helpWanted)
        {
            defaultGetoptPrinter("set screen backlight.", opt.options);
        }
    }
    catch (Exception e)
    {
        writeln(e.msg);
    }

    return 0;
}
