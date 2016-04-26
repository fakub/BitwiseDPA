#include <stdexcept>

#include "types.hpp"


// hex conversion tools ========================================================

byte hex2val(const byte c) {
	if (c >= '0' && c <= '9') return c - '0';
	if (c >= 'a' && c <= 'f') return c - 'a' + 10;
	if (c >= 'A' && c <= 'F') return c - 'A' + 10;
	throw runtime_error("Invalid hex code.");
}

void hex2nbytes(const char * hex, byte * bytes, const byte n = 16) {
	for (byte i=0; i<n; i++)
		bytes[i] = 16 * hex2val(hex[2*i]) + hex2val(hex[2*i+1]);
}

void byte2hex(const byte a, byte * b) {
	snprintf((char *) b, 2+1, "%02x", a);
}

void nbytes2hex(const byte * bytes, byte * hex, const byte n = 16) {
	for (byte i=0; i<n; i++)
		byte2hex(bytes[i], hex + 2*i);
}
