from lithium.gimli_hash import gimli_hash


def test_gimli_hash():
    assert (
        gimli_hash(bytes()).hex()
        == "27ae20e95fbc2bf01e972b0015eea431c20fc8818f25bc6dbe66232230db352f"
    )
