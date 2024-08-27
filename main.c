#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#define MAP_IMPL
#include "map.h"

#define STRING_IMPL
#include "string.h"

#define ERROR_IMPL
#include "error.h"

#define PATH_IMPL
#include "path.h"

#define MAX_PATH_LEN 256
#define MIN_BRIGHTNESS_PERCENTAGE 0.1
#define MAX_BRIGHTNESS_PERCENTAGE 100.0

typedef struct {
	char maxBrightnessPath[MAX_PATH_LEN];
	char brightnessPath[MAX_PATH_LEN];
	int brightness;
	int maxBrightness;
} Device;

typedef struct {
    const char *deviceName;
    const char *action;
    float amount;
} Argument;

Device setBrightness(Device, float percentage);
Device increaseBrightness(Device, float percentage);
Device decreaseBrightness(Device, float percentage);
Device printBrightness(Device, float);

const char *getUsage()
{
    return "Usage: <set|up|down> <amount>";
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
    fprintf(stderr, "\n");
    exit(EXIT_FAILURE);
}

void dieUsage()
{
    die(getUsage());
}

float getPercentage(float value, float max)
{
    return (value * 100) / max;
}

float getValueFromPercentage(float percentage, float max)
{
    return (percentage * max) / 100;
}

int getCurrentPercentage(Device device)
{
    return getPercentage(device.brightness, device.maxBrightness);
}

float clampValue(float min, float value, float max)
{
    return value < min ? min : (value > max ? max : value);
}

Device newDevice(const char *deviceName)
{
	ERROR e;
    Device device;

    snprintf(device.brightnessPath, MAX_PATH_LEN, "/sys/class/backlight/%s/brightness", deviceName);
    snprintf(device.maxBrightnessPath, MAX_PATH_LEN, "/sys/class/backlight/%s/max_brightness", deviceName);

    e = scanFile(device.maxBrightnessPath, "%d", &device.maxBrightness);
	if (e != OK)
		pdie("initDevice", e);

    e = scanFile(device.brightnessPath, "%d", &device.brightness);
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
	ERROR e = echoToFile(device.brightnessPath, "%d", device.brightness);

	if (e != OK)
		pdie("writeDeviceChanges", e);
}

Device setBrightness(Device device, float percentage)
{
    float min = MIN_BRIGHTNESS_PERCENTAGE;
    float max = MAX_BRIGHTNESS_PERCENTAGE;
    device.brightness = getValueFromPercentage(clampValue(min, percentage, max), device.maxBrightness);
    return device;
}

Device increaseBrightness(Device device, float percentage)
{
	return setBrightness(device, getCurrentPercentage(device) + percentage);
}

Device decreaseBrightness(Device device, float percentage)
{
	return setBrightness(device, getCurrentPercentage(device) - percentage);
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
        arg.action = "print";
        return arg;
    }

    if (argc != 3) {
        dieUsage();
    }

    arg.action = argv[1];

    char *endPtr;
    arg.amount = strtof(argv[2], &endPtr);
    if (*endPtr)
        dieUsage();
    return arg;
}

Device printBrightness(Device device, float)
{
	printf("%d%%\n", (int)getPercentage(device.brightness, device.maxBrightness));
    return device;
}

map newActionMap()
{
    map m = newMap((cmpKeysType)strcmp);
    mapInsert(&m, "set", setBrightness);
    mapInsert(&m, "up", increaseBrightness);
    mapInsert(&m, "down", decreaseBrightness);
    mapInsert(&m, "print", printBrightness);
    return m;
}

Device executeAction(map actionMap, Device device, Argument arg)
{
    Device (*actionSelected)(Device, float percentage) = mapFind(actionMap, arg.action);

    if (actionSelected == NULL)
        die("Unkown action: '%s'", arg.action);

    return actionSelected(device, arg.amount);
}

int main(int argc, char **argv)
{
    map actionMap = newActionMap();
    Argument arg = parseArg(argc, argv);
    Device device = newDevice(arg.deviceName);

    device = executeAction(actionMap, device, arg);
    writeDeviceChanges(device);

    mapFree(actionMap, NULL, NULL);
	return EXIT_SUCCESS;
}
