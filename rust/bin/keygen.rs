// Part of liblithium, under the Apache License v2.0.
// SPDX-License-Identifier: Apache-2.0

use std::{
    env,
    fs::File,
    io::{Result, Write},
    process::exit,
};

use liblithium::sign::keygen;

fn main() -> Result<()> {
    let mut args = env::args();
    let prog = args.next().unwrap();
    let sk_path = args.next().unwrap_or_else(|| {
        eprintln!("usage: {} <key-filename>", prog);
        exit(1)
    });
    let (pk, sk) = keygen();
    File::create(&sk_path)?.write_all(&sk)?;
    File::create(sk_path + ".pub")?.write_all(&pk)
}
