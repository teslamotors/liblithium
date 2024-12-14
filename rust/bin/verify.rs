// Part of liblithium, under the Apache License v2.0.
// SPDX-License-Identifier: Apache-2.0

use std::{env, fs::read, process::exit};

use liblithium::sign::verify;

fn main() -> std::io::Result<()> {
    let mut args = env::args();
    let prog = args.next().unwrap();
    if args.len() != 3 {
        eprintln!(
            "usage: {} <public-key-file> <message-file> <signature-file>",
            prog
        );
        exit(1);
    }
    let pk_path = args.next().unwrap();
    let msg_path = args.next().unwrap();
    let sig_path = args.next().unwrap();

    let pk: [u8; liblithium::sign::PUBLIC_KEY_LEN] = read(pk_path)?
        .as_slice()
        .try_into()
        .expect("incorrect public key length");
    let sig: [u8; liblithium::sign::SIGN_LEN] = read(sig_path)?
        .as_slice()
        .try_into()
        .expect("incorrect signature length");
    let msg = read(msg_path)?;

    if !verify(msg.as_slice(), &sig, &pk) {
        eprintln!("could not verify signature");
        exit(1);
    }
    Ok(())
}
