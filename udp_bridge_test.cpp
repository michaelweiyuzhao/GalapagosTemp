#include <iostream>
#include <fstream>
#include <string>
#include <random>
#include <cstdlib>
#include <queue>

#include "ap_int.h"
#include "hls_stream.h"
#include "ap_utils.h"

#include "udp_bridge.h"

// #define VERBOSE





int hard_coded_loopback(
    int iter,

    // ap_uint <16> & src_port,
    // ap_uint <32> & src_ip_addr,
    // ap_uint <48> & src_mac_addr,

    // ap_uint <16> & dest_port,
    // ap_uint <32> & dest_ip_addr,

    // ap_uint <32> & netmask,
    // ap_uint <32> & gateway,
    
    udp_axis udp_axis_packet,
    app_axis app_axis_packet,
    udp_axis udp_axis_packetg
){

    static hls::stream <app_axis> to_app;
    static hls::stream <app_axis> from_app;
    static hls::stream <udp_axis> to_udp;
    static hls::stream <udp_axis> from_udp;

    // ap_uint <16> src_port_int;
    // ap_uint <32> src_ip_addr_int;
    // ap_uint <48> src_mac_addr_int;

    // ap_uint <16> dest_port_int;
    // ap_uint <32> dest_ip_addr_int;

    // ap_uint <32> netmask_int;
    // ap_uint <32> gateway_int;

    ap_uint <512> tdata_int;
    ap_uint <64> tkeep_int;
    ap_uint <16> tdest_int;
    ap_uint <1> tlast_int;

    udp_axis _udp_axis_packet;
    app_axis _app_axis_packet;

    int empty;

    // int src_port_good;
    // int src_ip_addr_good;
    // int src_mac_addr_good;
    // int dest_port_good;
    // int dest_ip_addr_good;
    // int netmask_good;
    // int gateway_good;

    int tdata_good;
    int tkeep_good;
    int tdest_good;
    int tlast_good;

    int good;

    int retval = 0;

    std::cout << "Starting loopback iteration " << iter << std::endl;
    std::cout << std::endl;
    
    std::cout << "Sending UDP packet to APP" << std::endl;
    std::cout << std::endl;
    
    from_udp.write(udp_axis_packet);

    udp_bridge(
        to_app,
        from_app,
        to_udp,
        from_udp

        // subject to change
        // ,
        // src_port_int,
        // src_ip_addr_int,
        // src_mac_addr_int,

        // dest_port_int,
        // dest_ip_addr_int,

        // netmask_int,
        // gateway_int
    );

    if(!to_app.empty()){
        empty = 0;
        _app_axis_packet = to_app.read();
    }
    else{
        empty = 1;
        std::cout << "APP read empty" << std::endl;
        std::cout << std::endl;
        _app_axis_packet.tdata = 0;
        _app_axis_packet.tkeep = 0;
        _app_axis_packet.tdest = 0;
        _app_axis_packet.tlast = 0;
    }

//     src_port_good = src_port_int == src_port;
//     src_ip_addr_good = src_ip_addr_int == src_ip_addr;
//     src_mac_addr_good = src_mac_addr_int == src_mac_addr;
//     dest_port_good = dest_port_int == dest_port;
//     netmask_good = netmask_int == netmask;
//     gateway_good = gateway_int == gateway;

// #ifdef VERBOSE
//     std::cout << std::dec;
//     std::cout << "src_port_good " << src_port_good << " " << src_port_int << " " << src_port << std::endl;
//     std::cout << std::hex;
//     std::cout << "src_ip_addr_good " << src_ip_addr_good << " " << src_ip_addr_int << " " << src_ip_addr << std::endl;
//     std::cout << "src_mac_addr_good " << src_mac_addr_good << " " << src_mac_addr_int << " " << src_mac_addr << std::endl;
//     std::cout << std::dec;
//     std::cout << "dest_port_good " << dest_port_good << " " << dest_port_int << " " << dest_port << std::endl;
//     std::cout << std::hex;
//     std::cout << "netmask_good " << netmask_good << " " << netmask_int << " " << netmask << std::endl;
//     std::cout << "gateway_good " << gateway_good << " " << gateway_int << " " << gateway << std::endl;
//     std::cout << std::dec;
// #endif

//     good = src_port_good && src_ip_addr_good && src_mac_addr_good &&
//     	   dest_port_good && netmask_good && gateway_good;

//     std::cout << std::endl;
//     std::cout << "UDP to APP meta data result: " << good << std::endl;
//     std::cout << std::endl;

//     retval = retval || (!good);

    tdata_good = _app_axis_packet.tdata == app_axis_packet.tdata;
    tkeep_good = _app_axis_packet.tkeep == app_axis_packet.tkeep;
    tdest_good = _app_axis_packet.tdest == app_axis_packet.tdest;
    tlast_good = _app_axis_packet.tlast == app_axis_packet.tlast;

#ifdef VERBOSE
    std::cout << std::hex;
    std::cout << "tdata_good " << tdata_good << " " << _app_axis_packet.tdata << " " << app_axis_packet.tdata << std::endl;
    std::cout << "tkeep_good " << tkeep_good << " " << _app_axis_packet.tkeep << " " << app_axis_packet.tkeep << std::endl;
    std::cout << "tdest_good " << tdest_good << " " << _app_axis_packet.tdest << " " << app_axis_packet.tdest << std::endl;
    std::cout << "tlast_good " << tlast_good << " " << _app_axis_packet.tlast << " " << app_axis_packet.tlast << std::endl;
    std::cout << std::dec;
    std::cout << std::endl;
#endif

    good = tdata_good && tkeep_good && tdest_good && tlast_good;

    std::cout << "UDP to APP axis result: " << good << std::endl;
    std::cout << std::endl;

    retval = retval || (!good);

    if(!empty){
        std::cout << "Sending APP packet to UDP" << std::endl;
        std::cout << std::endl;
        from_app.write(_app_axis_packet);
    }

    udp_bridge(
        to_app,
        from_app,
        to_udp,
        from_udp

        // subject to change
        // ,
        // src_port_int,
        // src_ip_addr_int,
        // src_mac_addr_int,

        // dest_port_int,
        // dest_ip_addr_int,

        // netmask_int,
        // gateway_int
    );

    if(!to_udp.empty()){
        empty = 0;
        _udp_axis_packet = to_udp.read();
    }
    else{
        empty = 1;
        std::cout << "UDP read empty" << std::endl;
        std::cout << std::endl;
        _udp_axis_packet.tdata = 0;
        _udp_axis_packet.tkeep = 0;
        _udp_axis_packet.tlast = 0;
    }

//     src_port_good = src_port_int == src_port;
//     src_ip_addr_good = src_ip_addr_int == src_ip_addr;
//     src_mac_addr_good = src_mac_addr_int == src_mac_addr;
//     dest_port_good = dest_port_int == dest_port;
//     if(!empty){
//         dest_ip_addr_good = dest_ip_addr_int == dest_ip_addr;
//     }
//     else{
//         dest_ip_addr_good = 1;
//     }
//     netmask_good = netmask_int == netmask;
//     gateway_good = gateway_int == gateway;

// #ifdef VERBOSE
//     std::cout << std::dec;
//     std::cout << "src_port_good " << src_port_good << " " << src_port_int << " " << src_port << std::endl;
//     std::cout << std::hex;
//     std::cout << "src_ip_addr_good " << src_ip_addr_good << " " << src_ip_addr_int << " " << src_ip_addr << std::endl;
//     std::cout << "src_mac_addr_good " << src_mac_addr_good << " " << src_mac_addr_int << " " << src_mac_addr << std::endl;
//     std::cout << std::dec;
//     std::cout << "dest_port_good " << dest_port_good << " " << dest_port_int << " " << dest_port << std::endl;
//     std::cout << std::hex;
//     std::cout << "dest_ip_addr_good " << dest_ip_addr_good << " " << dest_ip_addr_int << " " << dest_ip_addr << std::endl;
//     std::cout << "netmask_good " << netmask_good << " " << netmask_int << " " << netmask << std::endl;
//     std::cout << "gateway_good " << gateway_good << " " << gateway_int << " " << gateway << std::endl;
//     std::cout << std::dec;
// #endif

//     good = src_port_good && src_ip_addr_good && src_mac_addr_good &&
//     	   dest_port_good && dest_ip_addr_good && netmask_good && gateway_good;

//     std::cout << std::endl;
//     std::cout << "APP to UDP meta data result: " << good << std::endl;
//     std::cout << std::endl;

//     retval = retval || (!good);

    tdata_good = _udp_axis_packet.tdata == udp_axis_packetg.tdata;
    tkeep_good = _udp_axis_packet.tkeep == udp_axis_packetg.tkeep;
    tlast_good = _udp_axis_packet.tlast == udp_axis_packetg.tlast;

#ifdef VERBOSE
    std::cout << std::hex;
    std::cout << "tdata_good " << tdata_good << " " << _udp_axis_packet.tdata << " " << udp_axis_packetg.tdata << std::endl;
    std::cout << "tkeep_good " << tkeep_good << " " << _udp_axis_packet.tkeep << " " << udp_axis_packetg.tkeep << std::endl;
    std::cout << "tlast_good " << tlast_good << " " << _udp_axis_packet.tlast << " " << udp_axis_packetg.tlast << std::endl;
    std::cout << std::dec;
    std::cout << std::endl;
#endif

    good = tdata_good && tkeep_good && tdest_good && tlast_good;

    std::cout << "APP to UDP axis result: " << good << std::endl;
    std::cout << std::endl;

    retval = retval || (!good);

    std::cout << "Finished loopback iteration " << iter << std::endl;
    std::cout << std::endl;

    return retval;
}





int random_loopback(
    int iter,

    udp_axis udp_axis_packet_in,
    udp_axis & udp_axis_packet_out
) {
    
    static hls::stream <app_axis> to_app;
    static hls::stream <app_axis> from_app;
    static hls::stream <udp_axis> to_udp;
    static hls::stream <udp_axis> from_udp;

    app_axis app_axis_packet;

    int empty;

    std::cout << "Starting loopback iteration " << iter << std::endl;
    std::cout << std::endl;
    
    std::cout << "Sending UDP packet to APP" << std::endl;
    std::cout << std::endl;

    from_udp.write(udp_axis_packet_in);

    udp_bridge(
        to_app,
        from_app,
        to_udp,
        from_udp
    );

    if(!to_app.empty()){
        empty = 0;
        app_axis_packet = to_app.read();
    }
    else{
        empty = 1;
        std::cout << "APP read empty" << std::endl;
        std::cout << std::endl;
    }

    if(!empty){
        std::cout << std::hex;
        std::cout << app_axis_packet.tdata << std::endl;
        std::cout << app_axis_packet.tkeep << std::endl;
        std::cout << app_axis_packet.tdest << std::endl;        
        std::cout << app_axis_packet.tlast << std::endl;
        std::cout << std::dec;
        std::cout << std::endl;

        std::cout << "Sending APP packet to UDP" << std::endl;
        std::cout << std::endl;
        from_app.write(app_axis_packet);
    }

    udp_bridge(
        to_app,
        from_app,
        to_udp,
        from_udp
    );

    if(!to_udp.empty()){
        empty = 0;
        udp_axis_packet_out = to_udp.read();
    }
    else{
        empty = 1;
        std::cout << "UDP read empty" << std::endl;
        std::cout << std::endl;
        udp_axis_packet_out.tdata = 0;
        udp_axis_packet_out.tkeep = 0;
        udp_axis_packet_out.tlast = 0;
    }

    std::cout << "Finished loopback iteration " << iter << std::endl;
    std::cout << std::endl;
}





int main(){

    // ap_uint <16> src_port = SRC_PORT;
    // ap_uint <32> src_ip_addr = SRC_IP_ADDR;
    // ap_uint <48> src_mac_addr = SRC_MAC_ADDR;

    // ap_uint <16> dest_port = DEST_PORT;
    // ap_uint <32> dest_ip_addr = 0;

    // ap_uint <32> netmask = NETMASK;
    // ap_uint <32> gateway = GATEWAY;

    ap_uint <512> tdata;
    ap_uint <64> tkeep;
    ap_uint <16> tdest;
    ap_uint <1> tlast;

    udp_axis udp_axis_packet;
    app_axis app_axis_packet;
    udp_axis udp_axis_packetg;

    int good;

    // std::cout << "*******************************" << std::endl;
    // std::cout << "* Starting Hard-Coded Tests" << std::endl;
    // std::cout << "*******************************" << std::endl;

    // int hard_coded_retval = 0;

    // udp_axis udp_packet_stream[32];
    // app_axis app_packet_golden[32];
    // udp_axis udp_packet_golden[32];

    // int udp_stream_size = 0;
    // int app_golden_size = 0;
    // int udp_golden_size = 0;

    // udp_packet_stream[0].tdata = "0x11111111111110000";
    // udp_packet_stream[0].tkeep = "0xFFFFFFFFFFFFFFFF";
    // udp_packet_stream[0].tlast = "0x1";
    
    // udp_packet_stream[1].tdata = "0x11111111111111111";
    // udp_packet_stream[1].tkeep = "0xFFFFFFFFFFFFFFFF";
    // udp_packet_stream[1].tlast = "0x0";

    // udp_packet_stream[2].tdata = "0x1111111111111FFFF";
    // udp_packet_stream[2].tkeep = "0x0000000000000003";
    // udp_packet_stream[2].tlast = "0x1";
    
    // udp_packet_stream[3].tdata = "0x11111111111110002";
    // udp_packet_stream[3].tkeep = "0xFFFFFFFFFFFFFFFF";
    // udp_packet_stream[3].tlast = "0x0";
    
    // udp_packet_stream[4].tdata = "0x11111111111111111";
    // udp_packet_stream[4].tkeep = "0xFFFFFFFFFFFFFFFF";
    // udp_packet_stream[4].tlast = "0x0";

    // udp_packet_stream[5].tdata = "0x11111111111111111";
    // udp_packet_stream[5].tkeep = "0xFFFFFFFFFFFFFFFF";
    // udp_packet_stream[5].tlast = "0x1";

    // udp_packet_stream[6].tdata = "0x11111111111110003";
    // udp_packet_stream[6].tkeep = "0xFFFFFFFFFFFFFFFF";
    // udp_packet_stream[6].tlast = "0x0";

    // udp_packet_stream[7].tdata = "0x11111111111111111";
    // udp_packet_stream[7].tkeep = "0xFFFFFFFFFFFFFFFF";
    // udp_packet_stream[7].tlast = "0x0";

    // udp_packet_stream[8].tdata = "0x0";
    // udp_packet_stream[8].tkeep = "0x0000000000000001";
    // udp_packet_stream[8].tlast = "0x1";

    // udp_packet_stream[9].tdata = "0x0";
    // udp_packet_stream[9].tkeep = "0x0";
    // udp_packet_stream[9].tlast = "0x0";

    // app_packet_golden[0].tdata = "0x1111111111111";
    // app_packet_golden[0].tkeep = "0x3FFFFFFFFFFFFFFF";
    // app_packet_golden[0].tdest = "0x0000";
    // app_packet_golden[0].tlast = "0x1";

    // app_packet_golden[1].tdata = "0x0";
    // app_packet_golden[1].tkeep = "0x0";
    // app_packet_golden[1].tdest = "0x0";
    // app_packet_golden[1].tlast = "0x0";

    // app_packet_golden[2].tdata = "0xFFFF0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001111111111111";
    // app_packet_golden[2].tkeep = "0xFFFFFFFFFFFFFFFF";
    // app_packet_golden[2].tdest = "0x1111";
    // app_packet_golden[2].tlast = "0x1";

    // app_packet_golden[3].tdata = "0x0";
    // app_packet_golden[3].tkeep = "0x0";
    // app_packet_golden[3].tdest = "0x0";
    // app_packet_golden[3].tlast = "0x0";

    // app_packet_golden[4].tdata = "0x11110000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001111111111111";
    // app_packet_golden[4].tkeep = "0xFFFFFFFFFFFFFFFF";
    // app_packet_golden[4].tdest = "0x0002";
    // app_packet_golden[4].tlast = "0x0";
    
    // app_packet_golden[5].tdata = "0x11110000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001111111111111";
    // app_packet_golden[5].tkeep = "0xFFFFFFFFFFFFFFFF";
    // app_packet_golden[5].tdest = "0x0002";
    // app_packet_golden[5].tlast = "0x0";

    // app_packet_golden[6].tdata = "0x1111111111111";
    // app_packet_golden[6].tkeep = "0x3FFFFFFFFFFFFFFF";
    // app_packet_golden[6].tdest = "0x0002";
    // app_packet_golden[6].tlast = "0x1";

    // app_packet_golden[7].tdata = "0x11110000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001111111111111";
    // app_packet_golden[7].tkeep = "0xFFFFFFFFFFFFFFFF";
    // app_packet_golden[7].tdest = "0x0003";
    // app_packet_golden[7].tlast = "0x0";

    // app_packet_golden[8].tdata = "0x1111111111111";
    // app_packet_golden[8].tkeep = "0x7FFFFFFFFFFFFFFF";
    // app_packet_golden[8].tdest = "0x0003";
    // app_packet_golden[8].tlast = "0x1";

    // app_packet_golden[9].tdata = "0x0";
    // app_packet_golden[9].tkeep = "0x0";
    // app_packet_golden[9].tdest = "0x0";
    // app_packet_golden[9].tlast = "0x0";

    // udp_packet_golden[0].tdata = "0x11111111111110000";
    // udp_packet_golden[0].tkeep = "0xFFFFFFFFFFFFFFFF";
    // udp_packet_golden[0].tlast = "0x1";

    // udp_packet_golden[1].tdata = "0x0";
    // udp_packet_golden[1].tkeep = "0x0";
    // udp_packet_golden[1].tlast = "0x0";    
    
    // udp_packet_golden[2].tdata = "0x11111111111111111";
    // udp_packet_golden[2].tkeep = "0xFFFFFFFFFFFFFFFF";
    // udp_packet_golden[2].tlast = "0x0";

    // udp_packet_golden[3].tdata = "0xFFFF";
    // udp_packet_golden[3].tkeep = "0x0000000000000003";
    // udp_packet_golden[3].tlast = "0x1";
    
    // udp_packet_golden[4].tdata = "0x11111111111110002";
    // udp_packet_golden[4].tkeep = "0xFFFFFFFFFFFFFFFF";
    // udp_packet_golden[4].tlast = "0x0";
    
    // udp_packet_golden[5].tdata = "0x11111111111111111";
    // udp_packet_golden[5].tkeep = "0xFFFFFFFFFFFFFFFF";
    // udp_packet_golden[5].tlast = "0x0";

    // udp_packet_golden[6].tdata = "0x11111111111111111";
    // udp_packet_golden[6].tkeep = "0xFFFFFFFFFFFFFFFF";
    // udp_packet_golden[6].tlast = "0x1";

    // udp_packet_golden[7].tdata = "0x11111111111110003";
    // udp_packet_golden[7].tkeep = "0xFFFFFFFFFFFFFFFF";
    // udp_packet_golden[7].tlast = "0x0";

    // udp_packet_golden[8].tdata = "0x11111111111111111";
    // udp_packet_golden[8].tkeep = "0xFFFFFFFFFFFFFFFF";
    // udp_packet_golden[8].tlast = "0x0";

    // udp_packet_golden[9].tdata = "0x0";
    // udp_packet_golden[9].tkeep = "0x0000000000000001";
    // udp_packet_golden[9].tlast = "0x1";
    
    // udp_stream_size = 9;
    // app_golden_size = 9;
    // udp_golden_size = 10;

    // for(int i=0; i<udp_stream_size; i++){
    //     udp_axis_packet = udp_packet_stream[i];
    //     app_axis_packet = app_packet_golden[i];
    //     udp_axis_packetg = udp_packet_golden[i];

    //     // if(app_axis_packet.tdata != 0x0 && app_axis_packet.tkeep != 0x0){
    //     //     dest_ip_addr = IP_TABLE[app_axis_packet.tdest];
    //     // }
        
    //     good = hard_coded_loopback(i,
    //         // src_port, src_ip_addr, src_mac_addr,
    //         // dest_port, dest_ip_addr,
    //         // netmask, gateway,
    //         udp_axis_packet, app_axis_packet, udp_axis_packetg);

    //     hard_coded_retval = hard_coded_retval || good;
    // }

    // if(hard_coded_retval){
    //     std::cout << "*******************************" << std::endl;
    //     std::cout << "* Hard-Coded Test Result Bad" << std::endl;
    //     std::cout << "*******************************" << std::endl;
    // }
    // else{
    //     std::cout << "*******************************" << std::endl;
    //     std::cout << "* Hard-Coded Test Result Good" << std::endl;
    //     std::cout << "*******************************" << std::endl;
    // }
    
    std::cout << "*******************************" << std::endl;
    std::cout << "* Starting Random Tests" << std::endl;
    std::cout << "*******************************" << std::endl;

    udp_axis udp_axis_packet_in;
    udp_axis udp_axis_packet_out;

    int tkeep_index;

    std::queue <udp_axis> udp_axis_buf;
    std::queue <int> tkeep_index_buf;
    
    // std::random_device rd;
    std::srand(2398472937);

    int first = 1;
    int test_runs = 20;

    int tdata_good;
    int tkeep_good;
    int tlast_good;
    int random_retval = 0;

    // udp_axis_packet_in.tdata = "0x24AE65CCCBC496C26C2F4B75625D082CA1281D5A28EA6DBCCF447F30B15D0FA30BF33867B2A6D511664393C6DBABD04B2F1B2F205867B0A4AF2A2C3164C64DD3";
    // udp_axis_packet_in.tkeep = "0x1";
    // udp_axis_packet_in.tlast = "0x1";

    // good = random_loopback(0,
    //     udp_axis_packet_in,
    //     udp_axis_packet_out
    // );

    // tlast_good = udp_axis_packet_out.tlast == udp_axis_packet_in.tlast;
    // tkeep_good = udp_axis_packet_out.tkeep == udp_axis_packet_in.tkeep;
    // tdata_good = udp_axis_packet_out.tdata(1*8+7,0) == udp_axis_packet_in.tdata(1*8+7,0);

    // std::cout << std::hex;
    // std::cout << "tdata_good " << tdata_good << " " << udp_axis_packet_out.tdata << " " << udp_axis_packet_in.tdata << std::endl;
    // std::cout << "tkeep_good " << tkeep_good << " " << udp_axis_packet_out.tkeep << " " << udp_axis_packet_in.tkeep << std::endl;
    // std::cout << "tlast_good " << tlast_good << " " << udp_axis_packet_out.tlast << " " << udp_axis_packet_in.tlast << std::endl;
    // std::cout << std::dec;
    // std::cout << std::endl;

    // good = tdata_good && tkeep_good && tlast_good;
    // random_retval = random_retval || (!good);

    // std::cout << "Random Test iteration " << " results " << good << std::endl;
    // std::cout << std::endl;

    for(int i=0; i<test_runs; i++){
        udp_axis_packet_in.tdata.range(31,0) = std::rand();
        udp_axis_packet_in.tdata.range(63,32) = std::rand();
        udp_axis_packet_in.tdata.range(95,64) = std::rand();
        udp_axis_packet_in.tdata.range(127,96) = std::rand();
        udp_axis_packet_in.tdata.range(159,128) = std::rand();
        udp_axis_packet_in.tdata.range(191,160) = std::rand();
        udp_axis_packet_in.tdata.range(223,192) = std::rand();
        udp_axis_packet_in.tdata.range(255,224) = std::rand();
        udp_axis_packet_in.tdata.range(287,256) = std::rand();
        udp_axis_packet_in.tdata.range(319,288) = std::rand();
        udp_axis_packet_in.tdata.range(351,320) = std::rand();
        udp_axis_packet_in.tdata.range(383,352) = std::rand();
        udp_axis_packet_in.tdata.range(415,384) = std::rand();
        udp_axis_packet_in.tdata.range(447,416) = std::rand();
        udp_axis_packet_in.tdata.range(479,448) = std::rand();
        udp_axis_packet_in.tdata.range(511,480) = std::rand();

        udp_axis_packet_in.tlast = std::rand() % 2;

        if(udp_axis_packet_in.tlast){
            udp_axis_packet_in.tkeep = 0x0;
            if(first){
                tkeep_index = (std::rand() % 62); // first packet 2 bytes minimum for tdest
            }
            else{
                tkeep_index = (std::rand() % 63); // every other packet 1 byte minimum
            }
            udp_axis_packet_in.tkeep.range(63, tkeep_index) = 0xFFFFFFFFFFFFFFFF;
        }
        else{
            tkeep_index = 63;
            udp_axis_packet_in.tkeep = 0xFFFFFFFFFFFFFFFF;
        }

        first = udp_axis_packet_in.tlast; // first is equal to last iteration's tlast

        std::cout << std::hex;
        std::cout << "udp_axis_packet_in.tdata " << udp_axis_packet_in.tdata << std::endl;
        std::cout << "udp_axis_packet_in.tkeep " << udp_axis_packet_in.tkeep << std::endl;
        std::cout << "udp_axis_packet_in.tlast " << udp_axis_packet_in.tlast << std::endl;
        std::cout << std::dec;
        std::cout << std::endl;

        udp_axis_buf.push(udp_axis_packet_in);
        tkeep_index_buf.push(tkeep_index);
        
        good = random_loopback(i,
            udp_axis_packet_in,
            udp_axis_packet_out
        );

        if(udp_axis_packet_out.tdata != 0 || udp_axis_packet_out.tkeep != 0 || udp_axis_packet_out.tlast != 0){
            udp_axis_packet_in = udp_axis_buf.front();
            udp_axis_buf.pop();
            tlast_good = udp_axis_packet_out.tlast == udp_axis_packet_in.tlast;
            tkeep_good = udp_axis_packet_out.tkeep == udp_axis_packet_in.tkeep;
            tkeep_index = tkeep_index_buf.front();
            tkeep_index_buf.pop();
            tdata_good = udp_axis_packet_out.tdata(tkeep_index*8+7,0) == udp_axis_packet_in.tdata(tkeep_index*8+7,0);

            std::cout << std::hex;
            std::cout << "tdata_good " << tdata_good << " " << udp_axis_packet_out.tdata << " " << udp_axis_packet_in.tdata << std::endl;
            std::cout << "tkeep_good " << tkeep_good << " " << udp_axis_packet_out.tkeep << " " << udp_axis_packet_in.tkeep << std::endl;
            std::cout << "tlast_good " << tlast_good << " " << udp_axis_packet_out.tlast << " " << udp_axis_packet_in.tlast << std::endl;
            std::cout << std::dec;
            std::cout << std::endl;

            good = tdata_good && tkeep_good && tlast_good;
            random_retval = random_retval || (!good);

            std::cout << "Random Test iteration " << i << " results " << good << std::endl;
            std::cout << std::endl;
        }
        else{
            std::cout << "No udp packet read for iteration " << i << std::endl;
        }
    }

    if(random_retval){
        std::cout << "*******************************" << std::endl;
        std::cout << "* Random Test Result Bad" << std::endl;
        std::cout << "*******************************" << std::endl;
    }
    else{
        std::cout << "*******************************" << std::endl;
        std::cout << "* Random Test Result Good" << std::endl;
        std::cout << "*******************************" << std::endl;
    }

    int retval = /*hard_coded_retval || */random_retval;
    return retval;
}
