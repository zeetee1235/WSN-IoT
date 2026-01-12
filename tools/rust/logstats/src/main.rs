use clap::Parser;
use regex::Regex;
use std::fs::read_to_string;

#[derive(Parser)]
struct Args {
    logfile: String,
}

fn main() {
    let args = Args::parse();
    let content = read_to_string(&args.logfile).expect("read logfile");
    let tx_re = Regex::new(r"TX.*node=(\d+).*seq=(\d+).*ts=([\d\.]+)").unwrap();
    let rx_re = Regex::new(r"RX.*node=(\d+).*seq=(\d+).*ts=([\d\.]+)").unwrap();

    let mut tx_count = 0u32;
    let mut rx_count = 0u32;

    for line in content.lines() {
        if tx_re.is_match(line) {
            tx_count += 1;
        }
        if rx_re.is_match(line) {
            rx_count += 1;
        }
    }

    println!("tx_count={}", tx_count);
    println!("rx_count={}", rx_count);
}
