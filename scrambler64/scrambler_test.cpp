#include <ap_int.h>
#include <iostream>
#include <cstdlib>
#include "scrambler.h"

using namespace std;

int main(){
    ap_uint<64> data = 0x1e00000000000000;
    // ap_uint<64> data = 0xE100CAFEA5421642;
    ap_uint<64> data_scrambled;
    ap_uint<64> data_unscrambled;

    for(int i=0; i<10; i++){
        scrambler(data, data_scrambled, data_scrambled, data_unscrambled);
        cout << hex << data << " " << data_scrambled << " " << data_unscrambled << "\n";
    }
    if(data_unscrambled != data){
        return 1;
    }

    // send random data
    srand(2);

    int returnval = 0;
    for(int i=0; i<1000; i++){
        ap_uint<64> data_in = data;
        data.range(32,0) = rand();
        data.range(63,32) = rand();
        scrambler(data, data_scrambled, data_scrambled, data_unscrambled);
        cout << hex << data << " " << data_scrambled << " " << data_unscrambled << "\n";
        if(data_unscrambled != data_in){
            returnval = 1;
        }
    }
    return returnval;
}
