#pragma once

// hex conversion tools
byte hex2val(const byte c);
void hex2nbytes(const char * hex, byte * bytes, const byte n = 16);
void byte2hex(const byte a, byte * b);
void nbytes2hex(const byte * bytes, byte * hex, const byte n = 16);
