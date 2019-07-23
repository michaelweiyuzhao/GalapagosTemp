#include <iostream>
#include <cstdlib>
#include <ap_int.h>
#include "block_sync.h"

using namespace std;
int main(){
    ap_uint<66> test_data = "0x10000000000000000";
    ap_uint<2>  sync_header;
    ap_uint<1>  test_sh;
    ap_uint<1>  block_lock;
    ap_uint<1>  slip;

    test_sh = 1;
    block_lock = 0;
    slip = 0;

    int i = 0;
    while(block_lock == 0){
        sync_header = test_data;

        cout << dec << "iteration " << i++ << endl;
        cout << hex;
        cout << "test_data " << test_data << endl;
        cout << "sync_header " << sync_header << endl;
        cout << "test_sh " << test_sh << endl;
        cout << "block_lock " << block_lock << endl;
        cout << "slip " << slip << endl;

        block_sync(
            sync_header,
            test_sh,
            block_lock,
            slip
        );

        if(slip){
            test_data = test_data >> 2;
            if(test_data == 0){
            	return 1;
            }
        }
    }
}
