#ifndef _BLOCK_SYNC_H_
#define _BLOCK_SYNC_H_

#include <ap_int.h>

void block_sync(
    ap_uint<2> sync_header_in,
    ap_uint<1> test_sh_in,
    ap_uint<1> & block_lock_out,
    ap_uint<1> & slip_out
);

#endif//_BLOCK_SYNC_H_