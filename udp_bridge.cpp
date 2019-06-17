
#include <iostream>

#include "ap_int.h"
#include "hls_stream.h"
#include "ap_utils.h"

#include "udp_bridge.h"

// ap_uint <512> reverse_endian_512_tdata(ap_uint <512> X) {
//    return 0;
// }

// ap_uint <64> reverse_endian_512_tkeep(ap_uint <64> X) {
//    return 0;
// }

void udp_to_app(
    hls::stream <udp_axis> & axis_from_udp,
    hls::stream <app_axis> & axis_to_app,

    meta meta_from_udp,
    meta & meta_to_app
) {
// #pragma HLS INLINE
#pragma HLS PIPELINE II=1

#define INIT_UDP_TO_APP 0
#define STREAM_UDP_TO_APP 1
#define END_UDP_TO_APP 2

    static udp_axis axis_buf[2];
    static meta meta_buf[2];
    static ap_uint <1> bufen[2];
    static ap_uint <1> last = 1;
    static app_axis app_axis_packet;

    // buf[1] logic
    if(!bufen[1]){ // if buf[1] is empty shift in buf[0]
        axis_buf[1] = axis_buf[0]; // implemented as mux instead of enable
        meta_buf[1] = meta_buf[0];
        bufen[1] = bufen[0];
        bufen[0] = 0;
    }

    // buf[0] logic
    if(!axis_from_udp.empty()){ // if packet is valid read packet into buf[0]
        axis_buf[0] = axis_from_udp.read();
        meta_buf[0] = meta_from_udp;
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
        app_axis_packet.tdest = last ? axis_buf[1].tdata.range(511,496) : app_axis_packet.tdest;
        if(axis_buf[1].tlast){
            app_axis_packet.tdata.range(511,16) = axis_buf[1].tdata.range(495,0);
            app_axis_packet.tdata.range(15,0) = ap_uint <16> (0);
            app_axis_packet.tkeep.range(63,2) = axis_buf[1].tkeep.range(61,0);
            app_axis_packet.tkeep.range(1,0) = ap_uint <2> (0);
            app_axis_packet.tlast = axis_buf[1].tlast; //1
            bufen[1] = 0;
            axis_to_app.write(app_axis_packet);
            meta_to_app = meta_buf[1];
        }
        else{
            if(bufen[0]){
                app_axis_packet.tdata.range(511,16) = axis_buf[1].tdata.range(495,0);
                app_axis_packet.tdata.range(15,0) = axis_buf[0].tdata.range(511,496);
                app_axis_packet.tkeep.range(63,2) = axis_buf[1].tkeep.range(61,0);
                app_axis_packet.tkeep.range(1,0) = axis_buf[0].tkeep.range(63,62);
                if(axis_buf[0].tlast){
                    bufen[0] = axis_buf[0].tkeep[61]; // if more data exists, bufen0 = 1 else bufen0 = 0
                    app_axis_packet.tlast = !axis_buf[0].tkeep[61]; // if more data exists, tlast = 0 else tlast = 1
                }
                else{
                    app_axis_packet.tlast = axis_buf[0].tlast; //0
                }
                bufen[1] = 0;
                axis_to_app.write(app_axis_packet);
                meta_to_app = meta_buf[1];
            }
        }
        last = app_axis_packet.tlast;
    }
}

void app_to_udp(
    hls::stream <app_axis> & axis_from_app,
    hls::stream <udp_axis> & axis_to_udp,

    meta meta_from_app,
    meta & meta_to_udp
) {
// #pragma HLS INLINE
#pragma HLS PIPELINE II=1

#define INIT_APP_TO_UDP 0
#define STREAM_APP_TO_UDP 1
#define END_APP_TO_UDP 2

    static hls::stream <app_axis> app_axis_buf("app_to_udp_axis_int");
    static hls::stream <meta> meta_buf("app_to_udp_meta_int");
#pragma HLS stream variable=app_axis_buf depth=4
#pragma HLS stream variable=meta_buf depth=4

    static app_axis app_axis_packet;
    static udp_axis udp_axis_packet;
    static meta metadata;
    static ap_uint <16> tdata_buf;
    static ap_uint <2> tkeep_buf;
    static int state = INIT_APP_TO_UDP;

    meta_to_udp.local_port=metadata.local_port;
    meta_to_udp.remote_ip=metadata.remote_ip;
    meta_to_udp.remote_port=metadata.remote_port;
    switch(state){
        case INIT_APP_TO_UDP:{
            if(!axis_from_app.empty()){
                // packet buffering
                // no data is in fifo, directly use new data
                if(app_axis_buf.empty()){
                    app_axis_packet = axis_from_app.read();
                    metadata = meta_from_app;
                }
                // use data existing in fifo
                else{
                    app_axis_packet = app_axis_buf.read();
                    app_axis_buf.write(axis_from_app.read());
                    metadata = meta_buf.read();
                    meta_buf.write(meta_from_app);
                }
                // packet processing
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
                axis_to_udp.write(udp_axis_packet);
                meta_to_udp = metadata;
                break;
            }
            else if(!app_axis_buf.empty()){
                app_axis_packet = app_axis_buf.read();
                metadata = meta_buf.read();
                // packet processing
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
                axis_to_udp.write(udp_axis_packet);
                meta_to_udp = metadata;
                break;
            }
            state = INIT_UDP_TO_APP;
            break;
        }
        case STREAM_APP_TO_UDP:{
            if(!axis_from_app.empty()){
                // packet buffering
                // no data is in fifo, directly use new data
                if(app_axis_buf.empty()){
                    app_axis_packet = axis_from_app.read();
                    metadata = meta_from_app;
                }
                // use data existing in fifo
                else{
                    app_axis_packet = app_axis_buf.read();
                    app_axis_buf.write(axis_from_app.read());
                    metadata = meta_buf.read();
                    meta_buf.write(meta_from_app);
                }
                // packet processing
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
                axis_to_udp.write(udp_axis_packet);
                meta_to_udp = metadata;
                break;
            }
            else if(!app_axis_buf.empty()){
                app_axis_packet = app_axis_buf.read();
                metadata = meta_buf.read();
                // packet processing
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
                axis_to_udp.write(udp_axis_packet);
                meta_to_udp = metadata;
                break;
            }
            state = STREAM_UDP_TO_APP;
            break;
        }
        case END_APP_TO_UDP:{
            if(!axis_from_app.empty() && !app_axis_buf.full()){
                app_axis_buf.write(axis_from_app.read());
                meta_buf.write(meta_from_app);
            }
            // packet processing
            udp_axis_packet.tdata.range(511,496) = tdata_buf;
            udp_axis_packet.tdata.range(495,0) = ap_uint <496> (0);
            udp_axis_packet.tkeep.range(63,62) = tkeep_buf;
            udp_axis_packet.tkeep.range(61,0) = ap_uint <62> (0);
            udp_axis_packet.tlast = app_axis_packet.tlast; //1
            axis_to_udp.write(udp_axis_packet);
            meta_to_udp = metadata;
            state = INIT_APP_TO_UDP;
            break;
        }
    }
}

void udp_bridge (
    hls::stream <udp_axis> & axis_from_udp,
    hls::stream <app_axis> & axis_to_app,
    hls::stream <app_axis> & axis_from_app,
    hls::stream <udp_axis> & axis_to_udp,

    meta meta_from_udp,
    meta & meta_to_app,
    meta meta_from_app,
    meta & meta_to_udp
) {

#pragma HLS DATAFLOW

// #pragma HLS STREAM variable=axis_from_udp depth=0
// #pragma HLS STREAM variable=axis_to_app depth=1
// #pragma HLS STREAM variable=axis_from_app depth=1
// #pragma HLS STREAM variable=axis_to_udp depth=1

#pragma HLS RESOURCE core=AXI4Stream variable=axis_from_udp
#pragma HLS RESOURCE core=AXI4Stream variable=axis_to_app
#pragma HLS RESOURCE core=AXI4Stream variable=axis_from_app
#pragma HLS RESOURCE core=AXI4Stream variable=axis_to_udp

#pragma HLS DATA_PACK variable=axis_from_udp
#pragma HLS DATA_PACK variable=axis_to_app
#pragma HLS DATA_PACK variable=axis_from_app
#pragma HLS DATA_PACK variable=axis_to_udp

#pragma HLS INTERFACE ap_none port=meta_from_udp
#pragma HLS INTERFACE ap_none port=meta_to_app
#pragma HLS INTERFACE ap_none port=meta_from_app
#pragma HLS INTERFACE ap_none port=meta_to_udp

#pragma HLS INTERFACE ap_ctrl_none port=return

    udp_to_app(axis_from_udp, axis_to_app, meta_from_udp, meta_to_app);
    app_to_udp(axis_from_app, axis_to_udp, meta_from_app, meta_to_udp);
}
