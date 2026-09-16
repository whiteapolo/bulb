#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "zlib/include/z_path.h"
#include "zlib/include/z_file.h"
#include "zlib/include/z_error.h"
#include "zlib/include/z_heap.h"
#include "zlib/include/z_string.h"

#define MIN_BRIGHTNESS 0.1
#define MAX_BRIGHTNESS 100.0

#define ARRAY_LENGTH(arr) (sizeof(arr) / sizeof(arr[0]))

const char *POSSIBLE_DEVICES[] = {
    "/sys/class/backlight/intel_backlight",
    "/sys/class/backlight/acpi_video0"
};

const char *search_device()
{
    for (size_t i = 0; i < ARRAY_LENGTH(POSSIBLE_DEVICES); i++) {
        if (z_is_directory(POSSIBLE_DEVICES[i])) {
            return POSSIBLE_DEVICES[i];
        }
    }

    return NULL;
}

int get_max_brightness(const char *device)
{
    Z_Heap_Auto heap = {0};
    Z_String max_brightness_path = z_str_new(&heap, "%s/max_brightness", device);

    int max_brightness;
    z_file_scanf(max_brightness_path.ptr, "%d", &max_brightness);

    return max_brightness;
}

int get_brightness(const char *device)
{
    Z_Heap_Auto heap = {0};
    Z_String brightness_path = z_str_new(&heap, "%s/brightness", device);

    int brightness;
    z_file_scanf(brightness_path.ptr, "%d", &brightness);

    return brightness;
}

void set_brightness(const char *device, float new_brightness)
{
    Z_Heap_Auto heap = {0};
    Z_String brightness_path = z_str_new(&heap, "%s/brightness", device);

    z_file_write(brightness_path.ptr, "%d", new_brightness);
}

float brightness_to_percentage(int brightness, int max_brightness)
{
    return ((float)brightness) / ((float)max_brightness) * 100.0;
}

void print_current_brightness(const char *device)
{
    int brightness = get_brightness(device);
    int max_brightness = get_max_brightness(device);
    printf("%.2f%%\n", brightness_to_percentage(brightness, max_brightness));
}

void print_usage()
{
    printf("bulb [set|up|down] [0-100]\n");
}

int handle_action(int argc, char **argv)
{
    if (argc != 3) {
        print_usage();
        return 1;
    }

    const char *action = argv[1];
    const char *value = argv[2];

    return 0;
}

int main(int argc, char **argv)
{
    const char *device = search_device();
    z_enforce(device, "No device found :(\n");

    if (argc == 1) {
        print_current_brightness(device);
        return 0;
    }

    return handle_action(argc, argv);
}
