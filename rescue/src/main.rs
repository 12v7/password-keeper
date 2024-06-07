// If the physical device is lost, the app offers a backup method for obtaining the password.

use std::io::{stdin, stdout, Read, Write};
use sha256::digest;

const PIN_SIZE: usize = 5;

const ALLOWED_SCAN_CODES: [char; 54] = [  // for standard qwerty keyboard layout
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    'a', 'b', 'c', 'd', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'r', 's', 't', 'u', 'v', 'x', 'y',
    'A', 'B', 'C', 'D', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'R', 'S', 'T', 'U', 'V', 'X', 'Y',
];

//const ALLOWED_SCAN_CODES: [char; 54] = [ // for us-dvorak keyboard layout
//    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'x', 'j', 'e', 'u', 'i', 'd', 'c', 'h',
//    't', 'n', 'm', 'b', 'r', 'l', 'p', 'o', 'y', 'g', 'k', 'q', 'f', 'A', 'X', 'J', 'E', 'U', 'I',
//    'D', 'C', 'H', 'T', 'N', 'M', 'B', 'R', 'L', 'P', 'O', 'Y', 'G', 'K', 'Q', 'F',
//];

fn read_user_input(prompt: &str) -> String {
    let mut s = String::new();
    stdout().write_all(prompt.as_bytes()).unwrap();
    let _ = stdout().flush();
    stdin()
        .read_line(&mut s)
        .expect("Did not enter a correct string");
    if let Some('\n') = s.chars().next_back() {
        s.pop();
    }
    if let Some('\r') = s.chars().next_back() {
        s.pop();
    }
    s
}

fn main() {
    let salt_hex = read_user_input("Enter Salt: ");
    let salt = hex::decode(salt_hex).expect("Salt should contains hex digits");
    if salt.len() != 8 {
        panic!("Salt should contains 16 hex digits");
    }

    let passphrase = read_user_input("Enter Pass Phrase: ");
    let mut buf = hex::decode(digest(passphrase)).unwrap();

    for _ in 0..1000 {
        for j in 0..salt.len() {
            buf[j] = buf[j] ^ salt[j];
        }
        buf = hex::decode(digest(buf)).unwrap();
    }

    print!("Password: ");
    for i in 0..30 {
        print!("{}", ALLOWED_SCAN_CODES[buf[i] as usize % ALLOWED_SCAN_CODES.len()]);
    }
    print!("\n");

    let pin = buf[31];
    print!("Pin: ");
    for i in 0..PIN_SIZE {
        print!(
            "{}",
            if (pin & (1 << (PIN_SIZE - 1 - i))) == 0 {
                "."
            } else {
                "-"
            }
        );
    }
    print!("\n");

    let mut arr1: [u8; 1] = [0];
    stdin().read_exact(&mut arr1).ok();
}
