// Part of liblithium, under the Apache License v2.0.
// SPDX-License-Identifier: Apache-2.0

use std::mem::MaybeUninit;

use crate::bindings::{
    gimli_hash_final, gimli_hash_init, gimli_hash_state, gimli_hash_update, GIMLI_HASH_DEFAULT_LEN,
};

/// An object for incrementally hashing a message.
#[repr(transparent)]
pub struct GimliHash {
    state: gimli_hash_state,
}

pub const DEFAULT_LEN: usize = GIMLI_HASH_DEFAULT_LEN as usize;

impl GimliHash {
    pub fn new() -> GimliHash {
        let mut h = MaybeUninit::<GimliHash>::uninit();
        unsafe {
            gimli_hash_init(&raw mut (*h.as_mut_ptr()).state);
            h.assume_init()
        }
    }

    pub fn update(&mut self, data: &[u8]) {
        unsafe { gimli_hash_update(&mut self.state, data.as_ptr(), data.len()) }
    }

    pub fn final_<const N: usize>(mut self) -> [u8; N] {
        let mut h = MaybeUninit::<[u8; N]>::uninit();
        unsafe {
            gimli_hash_final(&mut self.state, &raw mut (*h.as_mut_ptr())[0], N);
            h.assume_init()
        }
    }
}

impl Default for GimliHash {
    fn default() -> Self {
        Self::new()
    }
}

/// Compute an N-byte GimliHash of data in one shot.
pub fn gimli_hash<const N: usize>(data: &[u8]) -> [u8; N] {
    let mut g = GimliHash::new();
    g.update(data);
    g.final_()
}

#[doc(hidden)]
pub fn hash_hex_string<const N: usize>(data: &[u8]) -> String {
    use std::fmt::Write;

    let mut s = String::with_capacity(N * 2);
    let h = gimli_hash::<N>(data);
    for b in h {
        write!(s, "{:02x}", b).unwrap();
    }
    s
}

#[cfg(test)]
mod test {
    use crate::gimli_hash::{hash_hex_string, DEFAULT_LEN};

    #[test]
    fn test_gimli_hash() {
        assert_eq!(
            hash_hex_string::<DEFAULT_LEN>(&[]),
            "27ae20e95fbc2bf01e972b0015eea431c20fc8818f25bc6dbe66232230db352f"
        );
    }
}
