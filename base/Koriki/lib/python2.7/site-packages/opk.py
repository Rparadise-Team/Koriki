from sys import argv
from cStringIO import StringIO
from ctypes import POINTER, Structure, cdll, c_char_p, c_void_p, \
		c_int, c_uint, byref, string_at

def _checkOpen(result, func, arguments):
	if result:
		return result
	else:
		raise IOError("Failed to open OPK file: '%s'" % arguments[0])

def _checkRead(result, func, arguments):
	if result >= 0:
		return result
	else:
		raise SyntaxError("Error occured while reading meta-data")

def _checkExtract(result, func, arguments):
	if result == 0:
		return result
	else:
		raise IOError("Unable to extract file from OPK")

def _init():
	class _OPK(Structure):
		pass
	OpkPtr = POINTER(_OPK)

	lib = cdll.LoadLibrary('libopk.so.1')

	opk_open = lib.opk_open
	opk_open.restype = OpkPtr
	opk_open.archtypes = (c_char_p, )
	opk_open.errcheck = _checkOpen
	global _opk_open
	_opk_open = opk_open

	opk_close = lib.opk_close
	opk_close.restype = None
	opk_close.archtypes = (OpkPtr, )
	global _opk_close
	_opk_close = opk_close

	opk_open_metadata = lib.opk_open_metadata
	opk_open_metadata.restype = c_int
	opk_open_metadata.archtypes = (OpkPtr, c_char_p, )
	opk_open_metadata.errcheck = _checkRead
	global _opk_open_metadata
	_opk_open_metadata = opk_open_metadata

	opk_read_pair = lib.opk_read_pair
	opk_read_pair.restype = c_int
	opk_read_pair.archtypes = (OpkPtr, c_char_p, c_uint, c_char_p, c_uint)
	global _opk_read_pair
	_opk_read_pair = opk_read_pair

	opk_extract_file = lib.opk_extract_file
	opk_extract_file.restype = c_int
	opk_extract_file.archtypes = (OpkPtr, c_char_p, c_void_p, c_uint)
	opk_extract_file.errcheck = _checkExtract
	global _opk_extract_file
	_opk_extract_file = opk_extract_file

_init()

class OPK(object):

	def __init__(self, path):
		self._opk = _opk_open(path)
	
	def __del__(self):
		_opk_close(self._opk)
	
	def open_metadata(self):
		name = c_char_p()
		ret = _opk_open_metadata(self._opk, byref(name))
		if ret == 1:
			return name.value
	
	def read_pair(self):
		key = c_char_p()
		key_len = c_uint()
		val = c_char_p()
		val_len = c_uint()
		ret = _opk_read_pair(self._opk, \
				byref(key), byref(key_len), \
				byref(val), byref(val_len))
		if ret == 1:
			return (key.value[:key_len.value], val.value[:val_len.value])
		return (None, None)

	def extract_file(self, filename):
		ptr = c_char_p()
		size = c_uint()
		_opk_extract_file(self._opk, filename, byref(ptr), byref(size))
		output = StringIO()
		output.write(string_at(ptr, size.value))
		return output


def read_metadata(filename):
	opk = OPK(filename)
	opk_dict = {}

	while True:
		mdata = opk.open_metadata()
		if not mdata:
			break

		mdata_dict = {}
		while True:
			key, value = opk.read_pair()
			if not key:
				break
			mdata_dict[key] = value

		# We try to be future-proof here; libopk will maybe include support for
		# other sections than the regular [Desktop Entry], so for now we
		# register the meta-data in another dictionnary with only one key
		# named after the 'Desktop Entry' section.
		opk_dict[mdata] = {'Desktop Entry': mdata_dict }
	
	return opk_dict

def extract_file(opk_filename, filename):
	return OPK(opk_filename).extract_file(filename)


def main():
	if len(argv) < 2:
		print "Usage: opk.py app1.opk [app2.opk ...]"
		return

	for arg in argv[1:]:
		print read_metadata(arg)
		print ""

if __name__ == '__main__':
	main()
