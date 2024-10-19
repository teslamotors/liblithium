// Part of liblithium, under the Apache License v2.0.
// SPDX-License-Identifier: Apache-2.0

use std::{
    env,
    fs::read,
    io::{stdin, Read},
};

use liblithium::gimli_hash::{hash_hex_string, DEFAULT_LEN};

fn main() -> std::io::Result<()> {
    let mut args = env::args();
    let _prog = args.next();
    if args.len() < 1 {
        let mut bytes = Vec::new();
        stdin().read_to_end(&mut bytes)?;
        println!("{}  -", hash_hex_string::<DEFAULT_LEN>(bytes.as_slice()));
    } else {
        for path in args {
            let bytes = read(&path)?;
            println!(
                "{}  {}",
                hash_hex_string::<DEFAULT_LEN>(bytes.as_slice()),
                path
            );
        }
    }
    Ok(())
}
