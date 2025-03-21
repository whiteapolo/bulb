#![allow(dead_code)]
use std::io;
use std::fs;
use std::path::Path;
use std::error::Error;
use std::env;
use std::process::exit;

const MIN_BRIGHTNESS_PECENTAGE: f32 = 0.1;
const MAX_BRIGHTNESS_PECENTAGE: f32 = 100.0;

enum Action {
    Set(f32),
    Up(f32),
    Down(f32),
    Print(),
}

fn print_usage() {
    println!("Usage: <command> <amount>");
    println!("Commands:");
    println!("\tset <amount>");
    println!("\tup <amount>");
    println!("\tdown <amount>");
}

fn die_usage() {
    print_usage();
    exit(1);
}

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
        if Path::new(&format!("/sys/class/backlight/{interface}")).exists() {
            return Ok(interface.to_string())
        }
    }

    Err("couldn't find suitable interface".to_string())
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

        let brightness = read_int(&brightness_path)?;
        let max_brightness = read_int(&max_brightness_path)?;

        Ok(Device {
            brightness_path,
            max_brightness_path,
            brightness,
            max_brightness
        })
    }

    fn get_brightness_percentage(&self) -> f32 {
        get_percentage(self.brightness as f32, self.max_brightness as f32)
    }

    fn set_brightness(&mut self, percentage: f32) {
        let new_percentage = percentage.clamp(MIN_BRIGHTNESS_PECENTAGE, MAX_BRIGHTNESS_PECENTAGE);

        self.brightness = get_value_from_percentage(new_percentage, self.max_brightness as f32) as u32;
    }

    fn adjust_brightness(&mut self, delta: f32) {
        self.set_brightness(self.get_brightness_percentage() + delta);
    }

    fn save(&self) -> Result<(), io::Error> {
        fs::write(&self.brightness_path, self.brightness.to_string())?;
        Ok(())
    }
}

fn parse_args(args: &Vec<String>) -> Result<Action, Box<dyn Error>> {

    if args.len() == 1 {
        return Ok(Action::Print());
    }

    if args.len() != 3 {
        return Err("Invalid number of arguments".into());
    }

    let action = args[1].as_str();
    let amount = args[2].parse::<f32>()?;

    match action {
        "set" => Ok(Action::Set(amount)),
        "up" => Ok(Action::Up(amount)),
        "down" => Ok(Action::Down(amount)),
        _ => Err(format!("Unkown action: '{}'", action).into()),
    }
}

fn execute_action(device: &mut Device, action: Action) {
    match action {
        Action::Set(amount) => device.set_brightness(amount),
        Action::Up(amount) => device.adjust_brightness(amount),
        Action::Down(amount) => device.adjust_brightness(-amount),
        Action::Print() => println!("{}%", device.get_brightness_percentage()),
    }
}

fn main() -> Result<(), Box<dyn Error>> {

    let interface_name = get_default_interface_name().unwrap();

    let mut device = Device::new(&interface_name)
        .expect(&format!("Couldn't initialize interface: '{}'", interface_name));

    let action = parse_args(&env::args().collect());

    if let Err(_) = action {
        die_usage();
    }

    execute_action(&mut device, action?);
    device.save()?;

    Ok(())
}
