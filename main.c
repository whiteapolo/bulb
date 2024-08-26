#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#define ERROR_IMPL
#define STRING_IMPL
#define PATH_IMPL
#include "string.h"
#include "path.h"
#include "error.h"

static const int MIN_BRIGHTNESS_PERCENTAGE = 2;
static const int MAX_BRIGHTNESS_PERCENTAGE = 100;

typedef struct {
	string maxBrightnessPath;
	string brightnessPath;
	int brightness;
	int maxBrightness;
} Device;

enum DEVICE_ACTION {
    SET,
    UP,
    DONW,
    PRINT,
};

typedef struct {
    const char *deviceName;
    enum DEVICE_ACTION action;
    int amount;
} Argument;

const char *getUsage()
{
    return "Usage: <deviceName> <set|up|down> <amount>\n";
}

void pdie(const char *label, ERROR e)
{
	printError(label, e);
	exit(EXIT_FAILURE);
}

void die(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    exit(EXIT_FAILURE);
}

void dieUsage()
{
    die(getUsage());
}

Device newDevice(const char *deviceName)
{
    Device device;

    device.maxBrightnessPath = newStr("/sys/class/backlight/%s/max_brightness", deviceName);
	device.brightnessPath = newStr("/sys/class/backlight/%s/brightness", deviceName);

	ERROR e;
    e = scanFile(device.maxBrightnessPath.data, "%d", &device.maxBrightness);
	if (e != OK)
		pdie("initDevice", e);

    e = scanFile(device.brightnessPath.data, "%d", &device.brightness);
	if (e != OK)
		pdie("initDevice", e);

    return device;
}

void printDevice(const char *fileName)
{
	printf("%s\n", fileName);
}

void printDevices()
{
	dirTraverseVisibleFiles("/sys/class/backlight/", printDevice);
}

void writeDeviceChanges(Device device)
{
	ERROR e = echoToFile(
            device.brightnessPath.data, 
			"%d", 
			device.brightness
    );

	if (e != OK)
		pdie("writeDeviceChanges", e);
}

int brightnessToPercentage(Device device, int b)
{
	return ((float)b / device.maxBrightness) * 100.0f;
}

int percentageToBrightness(Device device, int b)
{
	return (b * device.maxBrightness) / 100.0f;
}

int getCurrentPercentage(Device device)
{
	return brightnessToPercentage(device, device.brightness);
}

void setBrightness(Device device, int n)
{
	int newPercentage = n;

	if (newPercentage > MAX_BRIGHTNESS_PERCENTAGE)
		newPercentage = MAX_BRIGHTNESS_PERCENTAGE;

	if (newPercentage < MIN_BRIGHTNESS_PERCENTAGE)
		newPercentage = MIN_BRIGHTNESS_PERCENTAGE;

	device.brightness = percentageToBrightness(device, newPercentage);
	writeDeviceChanges(device);
}

void adjustBrightness(Device device, int n)
{
	int newPercentage = getCurrentPercentage(device) + n;
	setBrightness(device, newPercentage);
}

enum DEVICE_ACTION getAction(const char *s)
{
	if (!strcmp(s, "set"))  return SET;
	if (!strcmp(s, "up"))   return UP;
	if (!strcmp(s, "down")) return DONW;
    die("Unkown Action: '%s'\n%s", s, getUsage());
    return 0;
}

const char *findDefaultDeviceName()
{
    if (isdir("/sys/class/backlight/intel_backlight"))
        return "intel_backlight";
    if (isdir("/sys/class/backlight/acpi_video0"))
        return "acpi_video0";
    die("Could not find suitable devices"); 
    return NULL;
}

Argument parseArg(int argc, char **argv)
{
    Argument arg;
    arg.deviceName = findDefaultDeviceName();

    if (argc == 1) {
        arg.action = PRINT;
        return arg;
    }

    if (argc != 3) {
        dieUsage();
    }

    arg.action = getAction(argv[1]);
    strView tmp = newStrView(argv[2]);
    if (!strIsNumeric(tmp))
        dieUsage();
    arg.amount = strToNumeric(tmp);

    return arg;
}

void printBrightness(Device device)
{
	printf("%d\n", brightnessToPercentage(device, device.brightness));
}

void excuteAction(Device device, enum DEVICE_ACTION action, int parm)
{
    switch (action) {
        case SET:
            setBrightness(device, parm);
            break;
        case UP:
            adjustBrightness(device, parm);
            break;
        case DONW:
            adjustBrightness(device, -parm);
            break;
        case PRINT:
            printBrightness(device);
            break;
        default:
            die("Unkown action");
    }
}

int main(int argc, char **argv)
{
    Argument arg = parseArg(argc, argv);
	Device device = newDevice(arg.deviceName);
    excuteAction(device, arg.action, arg.amount);
	return EXIT_SUCCESS;
}
