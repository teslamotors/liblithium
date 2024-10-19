// Part of liblithium, under the Apache License v2.0.
// SPDX-License-Identifier: Apache-2.0

use std::{
    env,
    fs::{read, File},
    io::Write,
    process::exit,
};

use liblithium::sign::create;

fn main() -> std::io::Result<()> {
    let mut args = env::args();
    let prog = args.next().unwrap();
    if args.len() != 3 {
        eprintln!(
            "usage: {} <secret-key-file> <message-file> <signature-file>",
            prog
        );
        exit(1);
    }
    let sk_path = args.next().unwrap();
    let msg_path = args.next().unwrap();
    let sig_path = args.next().unwrap();

    let sk: [u8; liblithium::sign::SECRET_KEY_LEN] = read(sk_path)?
        .as_slice()
        .try_into()
        .expect("incorrect secret key length");
    let msg = read(msg_path)?;

    let sig = create(&msg, &sk);
    File::create(sig_path)?.write_all(&sig)
}
