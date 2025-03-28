use std::fs;
use std::path::Path;
use std::error::Error;
use std::io;

const MIN_BRIGHTNESS_PECENTAGE: f32 = 0.1;
const MAX_BRIGHTNESS_PECENTAGE: f32 = 100.0;

fn read_int(path: &str) -> Result<u32, Box<dyn Error>>{
    Ok(fs::read_to_string(path)?.trim().parse::<u32>()?)
}

fn get_percentage(value: f32, max: f32) -> f32 {
    (value * 100.0) / max
}

fn get_value_from_percentage(percentage: f32, max: f32) -> f32 {
    (percentage * max) / 100.0
}

fn get_default_interface_name() -> Result<String, String> {
    let interfaces = ["intel_backlight", "acpi_video0"];

    for interface in interfaces {
        let path = format!("/sys/class/backlight/{interface}");
        if Path::new(&path).exists() {
            return Ok(interface.to_string())
        }
    }

    Err("couldn't find suitable interface".into())
}

struct Device {
    brightness_path: String,
    max_brightness_path: String,
    brightness: u32,
    max_brightness: u32,
}

impl Device {
    fn new(interface_name: &str) -> Result<Self, Box<dyn Error>> {

        let brightness_path = format!("/sys/class/backlight/{interface_name}/brightness");
        let max_brightness_path = format!("/sys/class/backlight/{interface_name}/max_brightness");

        let err_msg = &format!("Couldn't initialize interface: '{}'", interface_name);

        let brightness = read_int(&brightness_path)
            .expect(&err_msg);
        let max_brightness = read_int(&max_brightness_path)
            .expect(&err_msg);

        Ok(Device {
            brightness_path,
            max_brightness_path,
            brightness,
            max_brightness
        })
    }

    fn get_brightness(&self) -> f32 {
        get_percentage(self.brightness as f32, self.max_brightness as f32)
    }

    fn set_brightness(&mut self, percentage: f32) {
        let new_percentage = percentage.clamp(MIN_BRIGHTNESS_PECENTAGE, MAX_BRIGHTNESS_PECENTAGE);

        self.brightness = get_value_from_percentage(new_percentage, self.max_brightness as f32) as u32;
    }

    fn adjust_brightness(&mut self, delta: f32) {
        self.set_brightness(self.get_brightness() + delta);
    }

    fn save(&self) -> Result<(), io::Error> {
        fs::write(&self.brightness_path, self.brightness.to_string())?;
        Ok(())
    }
}

pub fn adjust_brightness(delta: f32) -> Result<(), Box<dyn Error>> {
    let interface_name = get_default_interface_name().unwrap();

    let mut device = Device::new(&interface_name)?;

    device.adjust_brightness(delta);
    device.save()?;

    Ok(())
}

pub fn set_brightness(percentage: f32) -> Result<(), Box<dyn Error>> {
    let interface_name = get_default_interface_name().unwrap();

    let mut device = Device::new(&interface_name)?;

    device.set_brightness(percentage);
    device.save()?;

    Ok(())
}

pub fn get_brightness() -> Result<f32, Box<dyn Error>> {

    let interface_name = get_default_interface_name().unwrap();

    let device = Device::new(&interface_name)?;

    Ok(device.get_brightness())
}
