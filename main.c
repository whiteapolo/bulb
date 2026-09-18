#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "zlib/include/z_path.h"
#include "zlib/include/z_file.h"
#include "zlib/include/z_error.h"
#include "zlib/include/z_heap.h"
#include "zlib/include/z_string.h"
#include "zlib/include/z_clamp.h"

#define MIN_BRIGHTNESS 0.1f
#define MAX_BRIGHTNESS 100.0f

#define ARRAY_LENGTH(arr) (sizeof(arr) / sizeof(arr[0]))

typedef enum {
    SET,
    UP,
    DOWN,
} Bulb_Action;

typedef struct {
    Bulb_Action action;
    float value;
} Bulb_Input;

const char *POSSIBLE_DEVICES[] = {
    "/sys/class/backlight/intel_backlight",
    "/sys/class/backlight/acpi_video0"
};

bool str_to_float(const char *s, float *value)
{
    char *end_ptr;
    *value = strtof(s, &end_ptr);

    if (end_ptr == s + strlen(s)) {
        return true;
    }

    return false;
}

bool str_to_action(const char *s, Bulb_Action *action)
{
    if (strcmp(s, "set") == 0) {
        *action = SET;
    } else if (strcmp(s, "up") == 0) {
        *action = UP;
    } else if (strcmp(s, "down") == 0) {
        *action = DOWN;
    } else {
        return false;
    }

    return true;
}

const char *search_device()
{
    for (size_t i = 0; i < ARRAY_LENGTH(POSSIBLE_DEVICES); i++) {
        if (z_is_directory(POSSIBLE_DEVICES[i])) {
            return POSSIBLE_DEVICES[i];
        }
    }

    return NULL;
}

float brightness_to_percentage(int brightness, int max_brightness)
{
    return ((float)brightness) / ((float)max_brightness) * 100.0f;
}

int percentage_to_brightness(float percentage, int max_brightness)
{
    return (int)((percentage / 100) * max_brightness);
}

int get_max_brightness(const char *device)
{
    Z_Heap_Auto heap = {0};
    Z_String path = z_str_new(&heap, "%s/max_brightness", device);
    int max_brightness;

    if (!z_file_scanf(path.ptr, "%d", &max_brightness)) {
        z_perror_format("%s", path.ptr);
        exit(1);
    }

    return max_brightness;
}

int get_brightness(const char *device)
{
    Z_Heap_Auto heap = {0};
    Z_String path = z_str_new(&heap, "%s/brightness", device);
    int brightness;

    if (!z_file_scanf(path.ptr, "%d", &brightness)) {
        z_perror_format("%s", path.ptr);
        exit(1);
    }

    return brightness;
}

float get_brightness_percentage(const char *device)
{
    int brightness = get_brightness(device);
    int max_brightness = get_max_brightness(device);
    return brightness_to_percentage(brightness, max_brightness);
}

void set_brightness(const char *device, float brightness_percentage)
{
    Z_Heap_Auto heap = {0};
    Z_String brightness_path = z_str_new(&heap, "%s/brightness", device);

    int max_brightness = get_max_brightness(device);
    float clamped_percentage = Z_CLAMP(MIN_BRIGHTNESS, brightness_percentage, MAX_BRIGHTNESS);
    int new_brightness = percentage_to_brightness(clamped_percentage, max_brightness);

    if (!z_file_write(brightness_path.ptr, "%d", new_brightness)) {
        z_perror_format("%s", brightness_path.ptr);
        exit(1);
    }
}

void print_current_brightness(const char *device)
{
    printf("%.2f%%\n", get_brightness_percentage(device));
}

void print_usage()
{
    printf("bulb [set|up|down] [0-100]\n");
}

bool parse_input(int argc, char **argv, Bulb_Input *input)
{
    if (argc != 3) {
        return false;
    }

    const char *action = argv[1];
    const char *value = argv[2];

    if (!str_to_action(action, &input->action)) {
        return false;
    }

    if (!str_to_float(value, &input->value)) {
        return false;
    }

    return true;
}

void move_brightness(const char *device, float amount)
{
    float current = get_brightness_percentage(device);
    set_brightness(device, current + amount);
}

bool handle_action(int argc, char **argv, const char *device)
{
    Bulb_Input input = {0};

    if (!parse_input(argc, argv, &input)) {
        return false;
    }

    switch (input.action) {
        case SET:
            set_brightness(device, input.value);
            break;
        case UP:
            move_brightness(device, input.value);
            break;
        case DOWN:
            move_brightness(device, -input.value);
            break;
    }

    return true;
}

int main(int argc, char **argv)
{
    const char *device = search_device();
    z_enforce(device, "No device found :(\n");

    if (argc == 1) {
        print_current_brightness(device);
        return 0;
    }

    if(!handle_action(argc, argv, device)) {
        print_usage();
        return 1;
    }

    return 0;
}
