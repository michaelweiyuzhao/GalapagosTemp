
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
    hls::stream <app_axis> & to_app,
    hls::stream <meta> & meta_from_udp,
    hls::stream <meta> & meta_to_app
) {

#pragma HLS PIPELINE II=1

#define INIT_UDP_TO_APP 0
#define STREAM_UDP_TO_APP 1
#define END_UDP_TO_APP 2

    static udp_axis buf[2];
    static ap_uint <1> bufen[2];
    static ap_uint <1> last = 1;
    static app_axis app_axis_packet;

    // buf[1] logic
    if(!bufen[1]){ // if buf1 is empty shift in buf0
        buf[1] = _buf[0];
        bufen[1] = bufen[0];
    }

    // buf[0] logic
    if(!from_udp.empty()){ // if packet is valid read packet into buf0
        buf[0] = from_udp.read();
        bufen[0] = 1;
    }

    // if bufen[1] is 0, do nothing
    // if bufen[1] is 1, check last
    // if last, buf[1] has tdest
    // if not last, buf[1] does not have tdest
    // if buf[1] is last, send buf[1], bufen[1] is 0
    // if buf[1] is not last, check buf[0]
    // if bufen[0] is 0, do nothing
    // if bufen[0] is 1, check buf[0] last
    // if buf[0] is last, send buf[1] + buf[0], bufen[1] is 0, check buf[0] tkeep
    // if buf[0] does not have data left, bufen[0] is 0
    // if buf[0] has data left, bufen[0] is 1
    // if buf[0] is not last, send buf[1] + buf[0]

    if(bufen[1]){
        app_axis_packet.tdest = last ? buf[1].tdata.range(511,496) : app_axis_packet.tdest;
        if(buf[1].tlast){
            app_axis_packet.tdata.range(511,16) = buf[1].tdata.range(495,0);
            app_axis_packet.tdata.range(15,0) = ap_uint <16> (0);
            app_axis_packet.tkeep.range(63,2) = buf[1].tkeep.range(61,0);
            app_axis_packet.tkeep.range(1,0) = ap_uint <2> (0);
            app_axis_packet.tlast = buf[1].tlast; //1
            bufen[1] = 0;
            to_app.write(app_axis_packet);
        }
        else{
            if(bufen[0]){
                app_axis_packet.tdata.range(511,16) = buf[1].tdata.range(495,0);
                app_axis_packet.tdata.range(15,0) = buf[0].tdata.range(511,496);
                app_axis_packet.tkeep.range(63,2) = buf[1].tkeep.range(61,0);
                app_axis_packet.tkeep.range(1,0) = buf[0].tkeep.range(63,62);
                if(buf[0].tlast){
                    bufen[0] = buf[0].tkeep[61]; // if more data exists, bufen0 = 1 else bufen0 = 0
                    app_axis_packet.tlast = !buf[0].tkeep[61]; // if more data exists, tlast = 0 else tlast = 1
                }
                else{
                    app_axis_packet.tlast = buf[0].tlast; //0
                }
                bufen[1] = 0;
                to_app.write(app_axis_packet);
            }
        }
        last = app_axis_packet.tlast;
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
    hls::stream <udp_axis> & from_udp,
    hls::stream <app_axis> & to_app,
    hls::stream <app_axis> & from_app,
    hls::stream <udp_axis> & to_udp
    // ,

    // hls::stream <meta> & meta_from_udp
    // hls::stream <meta> & meta_to_app,
    // hls::stream <meta> & meta_from_app,
    // hls::stream <meta> & meta_to_udp,
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
