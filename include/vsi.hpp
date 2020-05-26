// This header file implement some utility function to encode/decode Variable Size Integer

#include <iostream>
#include <cstdint>

// Utility function that return the required number of bytes to encode a value as ULEB128
unsigned nbytesULEB128(uint64_t val) {
	unsigned size = 0;
	do {
		val >>= 7;
		size += sizeof(int8_t);
	} while (val);
	return size;
}

// Encode a uint64 as a ULEB128 value into a buffer
// Return the size of the encoded value
inline unsigned encodeULEB128(uint64_t value, uint8_t* p, unsigned padto = 0) {
	uint8_t* orig_p = p;
	unsigned count = 0;
	do {
		uint8_t byte = value & 0x7f;
		value >>= 7;
		count++;
		if (value != 0 || count < padto)
			byte |= 0x80; // Mark this byte to show that more bytes will follow.
		*p++ = byte;
	} while (value != 0);

	// Pad with 0x80 and emit a null byte at the end.
	if (count < padto) {
		for (; count < padto - 1; ++count)
			*p++ = '\x80';
		*p++ = '\x00';
	}

	return (unsigned)(p - orig_p);
}

// Decode a ULEB128 value into a uint64
inline uint64_t decodeULEB128(const uint8_t* p) {
	uint64_t value = 0;
	unsigned shift = 0;
	do {
		uint64_t slice = *p & 0x7f;
		if (shift >= 64 || slice << shift >> shift != slice) {
			std::cerr << "ULEB128 encoded value is too big for uint64" << std::endl;
			return 0;
		}
		value += uint64_t(*p & 0x7f) << shift;
		shift += 7;
	} while (*p++ >= 128);
	
    return value;
}