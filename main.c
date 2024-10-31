#include <linux/limits.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>

#define STRING_IMPL
#include "mystb/string.h"

#define PATH_IMPL
#include "mystb/path.h"

const float MAX_BRIGHTNESS_PERCENTAGE = 100.0;
const float MIN_BRIGHTNESS_PERCENTAGE = 0.1;

typedef struct {
	string maxBrightnessPath;
	string brightnessPath;
	int brightness;
	int maxBrightness;
} Device;

typedef struct {
	const char *action;
	float amount;
} Arg;

const char *USAGE =
"Usage: <command> <amount>\n"
"Commands:\n"
"	set <amount>\n"
"	up <amount>\n"
"	down <amount>";

const Arg DEFAULT_ARGUMENT = {
	.action = "print",
	.amount = 0,
};

void die(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(EXIT_FAILURE);
}

float getPercentage(const float value, const float max)
{
	return (value * 100) / max;
}

float getValueFromPercentage(const float percentage, const float max)
{
	return (percentage * max) / 100;
}

int getCurrentPercentage(const Device *device)
{
	return getPercentage(device->brightness, device->maxBrightness);
}

float clampValue(float min, float value, float max)
{
	if (value < min)
		return min;
	if (value > max)
		return max;
	return value;
}

Device createDevice(const char *deviceName)
{
	Device device;
	device.brightnessPath = newStr("/sys/class/backlight/%s/brightness", deviceName);
	device.maxBrightnessPath = newStr("/sys/class/backlight/%s/max_brightness", deviceName);

	if (readFile(device.maxBrightnessPath.data, "%d", &device.maxBrightness) == Err)
		die("Error: Could not read brightness from %s", device.maxBrightnessPath.data);

	if (readFile(device.brightnessPath.data, "%d", &device.brightness) == Err)
		die("Error: Could not read brightness from %s", device.brightnessPath.data);

	return device;
}

void freeDevice(Device *device)
{
	strFree(&device->brightnessPath);
	strFree(&device->maxBrightnessPath);
}

void writeDeviceChanges(const Device *device)
{
	if (echoFileWrite(device->brightnessPath.data, "%d", device->brightness) == Err)
		die("failed to write file %s", device->brightnessPath);
}

void setBrightness(Device *device, const float percentage)
{
	const float min = MIN_BRIGHTNESS_PERCENTAGE;
	const float max = MAX_BRIGHTNESS_PERCENTAGE;
	const float newPercentage = clampValue(min, percentage, max);
	device->brightness = getValueFromPercentage(newPercentage, device->maxBrightness);
}

void adjustBrightness(Device *device, const float deltaPercentage)
{
	const int newPercentage = getCurrentPercentage(device) + deltaPercentage;
	setBrightness(device, newPercentage);
}

const char *findDefaultDeviceName()
{
	if (isDir("/sys/class/backlight/intel_backlight"))
		return "intel_backlight";
	if (isDir("/sys/class/backlight/acpi_video0"))
		return "acpi_video0";
	die("Could not find suitable devices");
	return NULL;
}

Arg parseArg(int argc, char **argv)
{
	Arg arg;
	if (argc == 1)
		arg = DEFAULT_ARGUMENT;
	else if (argc == 3 && sscanf(argv[2], "%f", &arg.amount) == 1)
		arg.action = argv[1];
	else
		die(USAGE);
	return arg;
}

void printBrightness(const Device *device)
{
	const int percentInt = getPercentage(device->brightness, device->maxBrightness);
	printf("%d%%\n", percentInt);
}

void executeAction(Device *device, const Arg *arg)
{
	if (strcmp(arg->action, "set") == 0)
		setBrightness(device, arg->amount);
	else if (strcmp(arg->action, "up") == 0)
		adjustBrightness(device, arg->amount);
	else if (strcmp(arg->action, "down") == 0)
		adjustBrightness(device, -arg->amount);
	else if (strcmp(arg->action, "print") == 0)
		printBrightness(device);
	else
		die("Unkown action: '%s'", arg->action);
}

int main(int argc, char **argv)
{
	const Arg arg = parseArg(argc, argv);
	Device device = createDevice(findDefaultDeviceName());
	executeAction(&device, &arg);
	writeDeviceChanges(&device);
	freeDevice(&device);
	return EXIT_SUCCESS;
}
