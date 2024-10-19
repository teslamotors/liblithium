// Part of liblithium, under the Apache License v2.0.
// SPDX-License-Identifier: Apache-2.0

use std::{env, path::PathBuf};

static SOURCES: &[&str] = &[
    "fe.c",
    "gimli.c",
    "gimli_aead.c",
    "gimli_common.c",
    "gimli_hash.c",
    "memzero.c",
    "random.c",
    "sign.c",
    "x25519.c",
];

fn main() {
    let root_dir = PathBuf::from(env::var("CARGO_MANIFEST_DIR").unwrap());

    let src_dir = root_dir.join("src");
    let include_dir = root_dir.join("include");

    let mut cc = cc::Build::new();
    cc.files(SOURCES.iter().map(|s| src_dir.join(s)));
    cc.include(&include_dir);
    cc.flag("-ansi");
    cc.compile("liblithium");

    let wrapper = "rust/wrapper.h";

    println!("cargo:rerun-if-changed={}", wrapper);
    for d in [&src_dir, &include_dir] {
        println!("cargo:rerun-if-changed={}", d.display());
    }

    let out_dir = PathBuf::from(env::var("OUT_DIR").unwrap());

    bindgen::builder()
        .header(wrapper)
        .clang_arg(format!("-I{}", include_dir.display()))
        .disable_nested_struct_naming()
        .use_core()
        .default_enum_style(bindgen::EnumVariation::NewType {
            is_bitfield: false,
            is_global: false,
        })
        .parse_callbacks(Box::new(bindgen::CargoCallbacks::new()))
        .generate()
        .expect("Failed to generate bindings.")
        .write_to_file(out_dir.join("bindings.rs"))
        .expect("Failed to write bindings.");
}
