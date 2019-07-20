#ifndef _SCRAMBLER_H_
#define _SCRAMBLER_H_

#include <ap_int.h>

#define TAP1    38
#define TAP2    57

void scrambler64(
    ap_uint<64> data_in,
    ap_uint<64>& data_out
);

void descrambler64(
    ap_uint<64> data_in,
    ap_uint<64>& data_out
);

void scrambler(
    ap_uint<64> scrambler_data_in,
    ap_uint<64>& scrambler_data_out,
    ap_uint<64> descrambler_data_in,
    ap_uint<64>& descrambler_data_out
);
#endif//_SCRAMBLER_H_
