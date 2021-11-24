from pkg_resources import parse_version
import kaitaistruct
from kaitaistruct import KaitaiStruct, KaitaiStream, BytesIO


if parse_version(kaitaistruct.__version__) < parse_version('0.9'):
    raise Exception("Incompatible Kaitai Struct Python API: 0.9 or later is required, but you have %s" % (kaitaistruct.__version__))

class Protocol(KaitaiStruct):
    def __init__(self, _io, _parent=None, _root=None):
        self._io = _io
        self._parent = _parent
        self._root = _root if _root else self
        self._read()

    def _read(self):
        self.packet = []
        i = 0
        while not self._io.is_eof():
            self.packet.append(Protocol.Packet(self._io, self, self._root))
            i += 1


    class DataValue(KaitaiStruct):
        def __init__(self, _io, _parent=None, _root=None):
            self._io = _io
            self._parent = _parent
            self._root = _root if _root else self
            self._read()

        def _read(self):
            self.id = self._io.read_u4le()
            self.type = self._io.read_u1()
            _on = self.type
            if _on == 0:
                self.data = self._io.read_u1()
            elif _on == 4:
                self.data = self._io.read_u2le()
            elif _on == 6:
                self.data = self._io.read_s4le()
            elif _on == 7:
                self.data = self._io.read_u4le()
            elif _on == 1:
                self.data = self._io.read_s1()
            elif _on == 3:
                self.data = self._io.read_s1()
            elif _on == 5:
                self.data = self._io.read_s2le()
            elif _on == 8:
                self.data = self._io.read_f4le()
            elif _on == 2:
                self.data = self._io.read_bits_int_be(1) != 0
            else:
                self.data = self._io.read_bytes(4)


    class ValueMeta(KaitaiStruct):
        def __init__(self, _io, _parent=None, _root=None):
            self._io = _io
            self._parent = _parent
            self._root = _root if _root else self
            self._read()

        def _read(self):
            self.type = self._io.read_u1()
            self.mods_allowed = self._io.read_u1()
            self.name = Protocol.PrefixStr(self._io, self, self._root)
            self.desc = Protocol.PrefixStr(self._io, self, self._root)


    class DataModFrame(KaitaiStruct):
        def __init__(self, _io, _parent=None, _root=None):
            self._io = _io
            self._parent = _parent
            self._root = _root if _root else self
            self._read()

        def _read(self):
            self.frame_id = self._io.read_u4le()
            self.mod = Protocol.ModValue(self._io, self, self._root)


    class DataDescFrame(KaitaiStruct):
        def __init__(self, _io, _parent=None, _root=None):
            self._io = _io
            self._parent = _parent
            self._root = _root if _root else self
            self._read()

        def _read(self):
            self.id = self._io.read_u4le()
            self.meta = Protocol.ValueMeta(self._io, self, self._root)


    class ModValue(KaitaiStruct):
        def __init__(self, _io, _parent=None, _root=None):
            self._io = _io
            self._parent = _parent
            self._root = _root if _root else self
            self._read()

        def _read(self):
            self.type = self._io.read_u1()
            self.time = self._io.read_f4le()
            self.value = Protocol.DataValue(self._io, self, self._root)


    class PrefixStr(KaitaiStruct):
        def __init__(self, _io, _parent=None, _root=None):
            self._io = _io
            self._parent = _parent
            self._root = _root if _root else self
            self._read()

        def _read(self):
            self.len = self._io.read_u1()
            self.str = (self._io.read_bytes(self.len)).decode(u"ascii")


    class Packet(KaitaiStruct):
        def __init__(self, _io, _parent=None, _root=None):
            self._io = _io
            self._parent = _parent
            self._root = _root if _root else self
            self._read()

        def _read(self):
            self.marker = self._io.read_bytes(1)
            if not self.marker == b"\x7C":
                raise kaitaistruct.ValidationNotEqualError(b"\x7C", self.marker, self._io, u"/types/packet/seq/0")
            self.type = self._io.read_u1()
            self.len = self._io.read_u1()
            _on = self.type
            if _on == 50:
                self._raw_payload = self._io.read_bytes(self.len)
                _io__raw_payload = KaitaiStream(BytesIO(self._raw_payload))
                self.payload = Protocol.DataDescFrame(_io__raw_payload, self, self._root)
            elif _on == 100:
                self._raw_payload = self._io.read_bytes(self.len)
                _io__raw_payload = KaitaiStream(BytesIO(self._raw_payload))
                self.payload = Protocol.DataModFrame(_io__raw_payload, self, self._root)
            else:
                self.payload = self._io.read_bytes(self.len)
            self.crc = self._io.read_u1()