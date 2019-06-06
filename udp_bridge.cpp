
#include <iostream>

#include "ap_int.h"
#include "hls_stream.h"
#include "ap_utils.h"

#include "udp_bridge.h"

ap_uint <512> reverse_endian_512_tdata(ap_uint <512> X) {
   return 0;
}

ap_uint <64> reverse_endian_512_tkeep(ap_uint <64> X) {
   return 0;
}

void udp_to_app(
    hls::stream <udp_axis> & from_udp,
    hls::stream <app_axis> & to_app
) {

#pragma HLS PIPELINE II=1

#define INIT_UDP_TO_APP 0
#define STREAM_UDP_TO_APP 1
#define END_UDP_TO_APP 2

    static udp_axis udp_axis_packet;
    static app_axis app_axis_packet;
    static ap_uint <496> tdata_buf; //512 - 16 = 496, tdest takes 16 bits
    static ap_uint <62> tkeep_buf; //64 - 2 = 62, tdest takes 2 bytes
    static int state = INIT_UDP_TO_APP;

    switch(state){
        case INIT_UDP_TO_APP:{
            if(!from_udp.empty()){
                udp_axis_packet = from_udp.read();
                app_axis_packet.tdest = udp_axis_packet.tdata.range(511,496);
                if(udp_axis_packet.tlast){ // all data is present, send
                    app_axis_packet.tdata.range(511,16) = udp_axis_packet.tdata.range(495,0);
                    app_axis_packet.tdata.range(15,0) = ap_uint <16> (0);
                    app_axis_packet.tkeep.range(63,2) = udp_axis_packet.tkeep.range(61,0);
                    app_axis_packet.tkeep.range(1,0) = ap_uint <2> (0);
                    app_axis_packet.tlast = udp_axis_packet.tlast; //1
                    to_app.write(app_axis_packet);
                    state = INIT_UDP_TO_APP;
                    break;
                }
                else{ // buffer and wait for bottom 2 bytes in next flit
                    tdata_buf = udp_axis_packet.tdata.range(495,0);
                    tkeep_buf = udp_axis_packet.tkeep.range(61,0);
                    state = STREAM_UDP_TO_APP;
                    break;
                }
            }
            state = INIT_UDP_TO_APP;
            break;
        }
        case STREAM_UDP_TO_APP:{
            if(!from_udp.empty()){
                udp_axis_packet = from_udp.read();
                app_axis_packet.tdata.range(511,16) = tdata_buf;
                app_axis_packet.tdata.range(15,0) = udp_axis_packet.tdata.range(511,496);
				app_axis_packet.tkeep.range(63,2) = tkeep_buf;
				app_axis_packet.tkeep.range(1,0) = udp_axis_packet.tkeep.range(63,62);
                tdata_buf = udp_axis_packet.tdata.range(495,0);
                tkeep_buf = udp_axis_packet.tkeep.range(61,0);
                if(udp_axis_packet.tlast){
                    if(udp_axis_packet.tkeep[61]){ //extra data exists that doesn't fit
                        app_axis_packet.tlast = 0;
                        state = END_UDP_TO_APP;
                    }
                    else{ // all data fits in last packet
                        app_axis_packet.tlast = udp_axis_packet.tlast; //1
                        state = INIT_UDP_TO_APP;
                    }
                }
                else{
                    app_axis_packet.tlast = udp_axis_packet.tlast; //0
                    state = STREAM_UDP_TO_APP;
                }
                to_app.write(app_axis_packet);
                break;
            }
            state = STREAM_UDP_TO_APP;
            break;
        }
        case END_UDP_TO_APP:{
            app_axis_packet.tdata.range(511,16) = tdata_buf;
            app_axis_packet.tdata.range(15,0) = ap_uint <16> (0);
            app_axis_packet.tkeep.range(63,2) = tkeep_buf;
            app_axis_packet.tkeep.range(1,0) = ap_uint <2> (0);
            app_axis_packet.tlast = udp_axis_packet.tlast; //1
            to_app.write(app_axis_packet);
            state = INIT_UDP_TO_APP;
            break;
        }
    }
}

void app_to_udp(
    hls::stream <app_axis> & from_app,
    hls::stream <udp_axis> & to_udp
) {

#pragma HLS PIPELINE II=1

#define INIT_APP_TO_UDP 0
#define STREAM_APP_TO_UDP 1
#define END_APP_TO_UDP 2

    static app_axis app_axis_packet;
    static udp_axis udp_axis_packet;
    static ap_uint <16> tdata_buf;
    static ap_uint <2> tkeep_buf;
    static int state = INIT_APP_TO_UDP;

    switch(state){
        case INIT_APP_TO_UDP:{
            if(!from_app.empty()){
                app_axis_packet = from_app.read();
				udp_axis_packet.tdata.range(511,496) = app_axis_packet.tdest;
                udp_axis_packet.tdata.range(495,0) = app_axis_packet.tdata.range(511,16);
				udp_axis_packet.tkeep.range(63,62) = ap_uint <2> (0b11);
                udp_axis_packet.tkeep.range(61,0) = app_axis_packet.tkeep.range(63,2);
                tdata_buf = app_axis_packet.tdata.range(15,0);
                tkeep_buf = app_axis_packet.tkeep.range(1,0);
                if(app_axis_packet.tlast){
                    if(app_axis_packet.tkeep[1]){
                        udp_axis_packet.tlast = 0;
                        state = END_APP_TO_UDP;
                    }
                    else{
                        udp_axis_packet.tlast = app_axis_packet.tlast; //1
                        state = INIT_APP_TO_UDP;
                    }
                }
                else{
                    udp_axis_packet.tlast = app_axis_packet.tlast; //0
                    state = STREAM_APP_TO_UDP;
                }
                to_udp.write(udp_axis_packet);
                break;
            }
            state = INIT_UDP_TO_APP;
            break;
        }
        case STREAM_APP_TO_UDP:{
            if(!from_app.empty()){
                app_axis_packet = from_app.read();
                udp_axis_packet.tdata.range(511,496) = tdata_buf;
				udp_axis_packet.tdata.range(495,0) = app_axis_packet.tdata.range(511,16);
				udp_axis_packet.tkeep.range(63,62) = tkeep_buf;
				udp_axis_packet.tkeep.range(61,0) = app_axis_packet.tkeep.range(63,2);
                tdata_buf = app_axis_packet.tdata.range(15,0);
                tkeep_buf = app_axis_packet.tkeep.range(1,0);
                if(app_axis_packet.tlast){
                    if(app_axis_packet.tkeep[1]){
                        udp_axis_packet.tlast = 0;
                        state = END_APP_TO_UDP;
                    }
                    else{
                        udp_axis_packet.tlast = app_axis_packet.tlast; //1
                        state = INIT_APP_TO_UDP;
                    }
                }
                else{
                    udp_axis_packet.tlast = app_axis_packet.tlast; //0
                    state = STREAM_APP_TO_UDP;
                }
                to_udp.write(udp_axis_packet);
                break;
            }
            state = STREAM_UDP_TO_APP;
            break;
        }
        case END_APP_TO_UDP:{
            udp_axis_packet.tdata.range(511,496) = tdata_buf;
            udp_axis_packet.tdata.range(495,0) = ap_uint <496> (0);
            udp_axis_packet.tkeep.range(63,62) = tkeep_buf;
            udp_axis_packet.tkeep.range(61,0) = ap_uint <62> (0);
            udp_axis_packet.tlast = app_axis_packet.tlast; //1
            to_udp.write(udp_axis_packet);
            state = INIT_APP_TO_UDP;
            break;
        }
    }
}

void udp_bridge (
    hls::stream <app_axis> & to_app,
    hls::stream <app_axis> & from_app,
    hls::stream <udp_axis> & to_udp,
    hls::stream <udp_axis> & from_udp
    // ,

    // hls::stream <meta> & meta_in,
    // hls::stream <meta> & meta_out
) {

#pragma HLS DATAFLOW

#pragma HLS resource core=AXI4Stream variable=to_app
#pragma HLS resource core=AXI4Stream variable=from_app
#pragma HLS resource core=AXI4Stream variable=to_udp
#pragma HLS resource core=AXI4Stream variable=from_udp

#pragma HLS DATA_PACK variable=to_app
#pragma HLS DATA_PACK variable=from_app
#pragma HLS DATA_PACK variable=to_udp
#pragma HLS DATA_PACK variable=from_udp

// #pragma HLS INTERFACE ap_none port=meta_in
// #pragma HLS INTERFACE ap_none port=meta_out

#pragma HLS INTERFACE ap_ctrl_none port=return

    udp_to_app(from_udp, to_app);
    app_to_udp(from_app, to_udp);
}
