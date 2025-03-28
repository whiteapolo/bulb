#![allow(dead_code)]
mod brightness;
use brightness::*;

use std::error::Error;
use std::env;
use std::process::exit;

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

fn execute_action(action: Action) -> Result<(), Box<dyn Error>> {
    match action {
        Action::Set(amount) => set_brightness(amount),
        Action::Up(amount) => adjust_brightness(amount),
        Action::Down(amount) => adjust_brightness(-amount),
        Action::Print() => Ok(println!("{}%", get_brightness()? as i32)),
    }
}

fn main() -> Result<(), Box<dyn Error>> {

    let action = parse_args(&env::args().collect());

    if let Err(_) = action {
        die_usage();
    }

    execute_action(action?).expect("Couldn't excute action");

    Ok(())
}
