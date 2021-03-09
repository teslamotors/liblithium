from lithium._lithium import ffi, lib


class GimliHash:
    """
    An object for incrementally hashing a message.
    """

    def __init__(self):
        self._hash_state = ffi.new("gimli_hash_state *")
        lib.gimli_hash_init(self._hash_state)

    def update(self, data):
        """
        Feed new data to be hashed.
        """
        buf = ffi.from_buffer(bytes(data))
        lib.gimli_hash_update(self._hash_state, buf, len(buf))

    def final(self, n=lib.GIMLI_HASH_DEFAULT_LEN):
        """
        Emit an n-byte hash of the data given to update.
        """
        buf = ffi.new("unsigned char[%s]" % n)
        lib.gimli_hash_final(self._hash_state, buf, n)
        return bytes(buf)


def gimli_hash(data, n=lib.GIMLI_HASH_DEFAULT_LEN):
    """
    Compute an n-byte GimliHash of data in one shot.
    """
    g = GimliHash()
    g.update(data)
    return g.final(n)
