from lithium.sign import (
    Sign,
    keygen,
    create,
    verify,
    create_from_prehash,
    verify_prehash,
)


def test_sign():
    (pk, sk) = keygen()
    data = bytearray(b"foo")
    sig = create(data, sk)
    assert verify(data, sig, pk)
    data[0] = data[0] ^ 0xFF
    assert not verify(data, sig, pk)


def test_sign_prehash():
    (pk, sk) = keygen()
    data = bytearray(b"foo")
    s = Sign()
    s.update(data)
    prehash = bytearray(s.final_prehash())
    sig = create_from_prehash(prehash, sk)
    assert verify_prehash(sig, prehash, pk)
    prehash[0] = prehash[0] ^ 0xFF
    assert not verify_prehash(sig, prehash, pk)
