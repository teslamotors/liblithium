#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include <lithium/sign.h>

// Minimal assertion helper
static int expect(bool cond, const char* msg) {
  if (!cond) {
    fprintf(stderr, "FAIL: %s\n", msg);
    return 1;
  }
  return 0;
}

int main(void) {
  int fails = 0;

  // Setup test buffers with defined sizes from sign.h
  uint8_t msg[32];
  memset(msg, 0xA5, sizeof(msg));
  
  uint8_t sig[LITH_SIGN_LEN];  // 64 bytes
  memset(sig, 0x5A, sizeof(sig));
  
  uint8_t pub[LITH_SIGN_PUBLIC_KEY_LEN];  // 32 bytes
  memset(pub, 0x11, sizeof(pub));

  lith_sign_state state;

  // Test 1: Null state pointer should return false
  bool rc = lith_sign_final_verify(NULL, sig, pub);
  fails += expect(rc == false, "null state pointer should return false");

  // Test 2: Null signature pointer should return false
  lith_sign_init(&state);
  lith_sign_update(&state, msg, sizeof(msg));
  rc = lith_sign_final_verify(&state, NULL, pub);
  fails += expect(rc == false, "null signature pointer should return false");

  // Test 3: Null pubkey pointer should return false
  lith_sign_init(&state);
  lith_sign_update(&state, msg, sizeof(msg));
  rc = lith_sign_final_verify(&state, sig, NULL);
  fails += expect(rc == false, "null pubkey pointer should return false");

  // Test 4: Corrupted signature should return false
  lith_sign_init(&state);
  lith_sign_update(&state, msg, sizeof(msg));
  sig[0] ^= 0xFF;  // Corrupt first byte
  rc = lith_sign_final_verify(&state, sig, pub);
  fails += expect(rc == false, "corrupted signature should return false");

  // Test 5: One-shot verify with invalid signature
  rc = lith_sign_verify(sig, msg, sizeof(msg), pub);
  fails += expect(rc == false, "one-shot verify with bad sig should return false");

  if (fails) {
    fprintf(stderr, "Edge-case test suite: %d failures\n", fails);
    return 1;
  }

  printf("Edge-case tests: PASS\n");
  return 0;
}