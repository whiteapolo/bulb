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

static const int MIN_BRIGHTNESS_PER = 2;
static const int MAX_BRIGHTNESS_PER = 100;

typedef struct {
	string maxBrightnessPath;
	string brightnessPath;
	int brightness;
	int maxBrightness;
} Device;

void die(const char *label, ERROR e)
{
	printError(label, e);
	exit(EXIT_FAILURE);
}

string getBrightnessPath(const char *deviceName)
{
	return newStr(
			"/sys/class/backlight/%s/brightness", 
			deviceName
	);
}

string getMaxBrightnessPath(const char *deviceName)
{
	return newStr(
			"/sys/class/backlight/%s/max_brightness", 
			deviceName
	);
}

void initDevice(Device *device, const char *deviceName)
{
	device->maxBrightnessPath = getMaxBrightnessPath(deviceName);
	device->brightnessPath = getBrightnessPath(deviceName);

	ERROR e;

	if ((e = scanfFileByName(device->maxBrightnessPath.data, "%d", 
					&device->maxBrightness)) != OK)
	{
		die("initDevice", e);
	}

	if ((e = scanfFileByName(device->brightnessPath.data, "%d", 
					&device->brightness)) != OK)
	{
		die("initDevice", e);
	}
}

void printDevice(const char *dirName)
{
	printf("%s\n", dirName);
}

void printDevices()
{
	forEveryRegFileInDir("/sys/class/backlight/", printDevice);
}

void printUsage()
{
	printf("Usage: <deviceName>\n");
}

void writeDeviceChanges(Device device)
{
	ERROR e = writeFileByName(device.brightnessPath.data, 
			                  "%d", 
							  device.brightness);

	if (e != OK)
		die("writeDeviceChanges", e);
}

int brightnessToPer(Device device, int b)
{
	return ((float)b / device.maxBrightness) * 100.0f;
}

int perToBrightness(Device device, int b)
{
	return (b * device.maxBrightness) / 100.0f;
}

int getCurrentPer(Device device)
{
	return brightnessToPer(device, device.brightness);
}

void setBrightness(Device device, int n)
{
	int newPer = n;

	if (newPer > MAX_BRIGHTNESS_PER)
		newPer = MAX_BRIGHTNESS_PER;

	if (newPer < MIN_BRIGHTNESS_PER)
		newPer = MIN_BRIGHTNESS_PER;

	device.brightness = perToBrightness(device, newPer);
	writeDeviceChanges(device);
}

void increaseBrightness(Device device, int n)
{
	int newPer = getCurrentPer(device) + n;
	setBrightness(device, newPer);
}

void decreaseBrightness(Device device, int n)
{
	int newPer = getCurrentPer(device) - n;
	setBrightness(device, newPer);
}

void brightnessAction(Device device, 
		              strView action, 
					  strView parm)
{
	if (!strIsNumeric(parm))
		die("param should be numeric", UNKNOWN);

	int num = strToNumeric(parm);

	if (strIsEqualC(action, "set"))
		setBrightness(device, num);
	else if (strIsEqualC(action, "up"))
		increaseBrightness(device, num);
	else if (strIsEqualC(action, "down"))
		decreaseBrightness(device, num);
	else
		printUsage();
}

void printBrightness(Device device)
{
	printf("%d\n", brightnessToPer(device, device.brightness));
}

int main(int argc, char **argv)
{
	if (argc == 1) {
		printDevices();
		return EXIT_SUCCESS;
	}

	Device device;
	initDevice(&device, argv[1]);

	if (argc == 2)
		printBrightness(device);
	else if (argc == 4)
		brightnessAction(device, 
				         newStrView(argv[2]), 
						 newStrView(argv[3]));		
	else
		printUsage();

	return EXIT_SUCCESS;
}
