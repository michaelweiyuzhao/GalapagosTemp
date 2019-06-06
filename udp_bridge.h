#pragma once

#ifndef __UDP_BRIDGE_H__
#define __UDP_BRIDGE_H__

#include "ap_int.h"
#include "hls_stream.h"
#include "ap_utils.h"

struct udp_axis{
    ap_uint <512> tdata;
    ap_uint <64> tkeep;
    ap_uint <1> tlast;
};

struct app_axis{
    ap_uint <512> tdata;
    ap_uint <1> tlast;
    ap_uint <16> tdest;
    ap_uint <64> tkeep;
};

struct meta{
    ap_uint <16> remote_port;
    ap_uint <16> local_port;
    ap_uint <32> remote_ip;
};

ap_uint <512> reverse_endian_512_tdata(ap_uint <512> X);
ap_uint <64> reverse_endian_512_tkeep(ap_uint <64> X);

void udp_to_app(
    hls::stream <udp_axis> & from_udp,
    hls::stream <app_axis> & to_app
);

void app_to_udp(
    hls::stream <app_axis> & from_app,
    hls::stream <udp_axis> & to_udp
);

void udp_bridge (
    hls::stream <app_axis> & to_app,
    hls::stream <app_axis> & from_app,
    hls::stream <udp_axis> & to_udp,
    hls::stream <udp_axis> & from_udp
    // ,

    // hls::stream <meta> & meta_in,
    // hls::stream <meta> & meta_out
);

#endif // __UDP_BRIDGE_H__
