#include <iostream>
#include <string>
#include <cstdlib>

#include "ap_int.h"
#include "hls_stream.h"
#include "ap_utils.h"

#include "udp_bridge.h"

int main(){

    static hls::stream <app_axis> to_app("to_app");
    static hls::stream <app_axis> from_app("from_app");
    static hls::stream <udp_axis> to_udp("to_udp");
    static hls::stream <udp_axis> from_udp("from_app");
    static meta meta_from_udp;
    static meta meta_to_app;
    static meta meta_from_app;
    static meta meta_to_udp;

    int i;
    int j;
    int k;
    int l;
    int send;
    int first;
    int last;

    int done;
    int send_done;
    int cycle_buffer;
    int max_buffer;
    int timer;
    int max_timer;
    int timeout;

    int tdata_good;
    int tkeep_good;
    int tlast_good;
    int remote_port_good;
    int local_port_good;
    int remote_ip_good;
    int good;

    int random_retval = 0;

    // std::srand(23237);
    std::srand(2);

    #define NTESTS 100
    int tkeep_index[NTESTS];
    udp_axis udp_vectors[NTESTS];
    meta meta_vectors[NTESTS];
    app_axis app_results[NTESTS];
    meta meta_app_results[NTESTS];
    udp_axis udp_results[NTESTS];
    meta meta_results[NTESTS];

    meta metadata;

    // construct test cases
    first=1;
    for(i=0; i<NTESTS; i++){
        udp_vectors[i].tdata.range(31,0) = std::rand();
        udp_vectors[i].tdata.range(63,32) = std::rand();
        udp_vectors[i].tdata.range(95,64) = std::rand();
        udp_vectors[i].tdata.range(127,96) = std::rand();
        udp_vectors[i].tdata.range(159,128) = std::rand();
        udp_vectors[i].tdata.range(191,160) = std::rand();
        udp_vectors[i].tdata.range(223,192) = std::rand();
        udp_vectors[i].tdata.range(255,224) = std::rand();
        udp_vectors[i].tdata.range(287,256) = std::rand();
        udp_vectors[i].tdata.range(319,288) = std::rand();
        udp_vectors[i].tdata.range(351,320) = std::rand();
        udp_vectors[i].tdata.range(383,352) = std::rand();
        udp_vectors[i].tdata.range(415,384) = std::rand();
        udp_vectors[i].tdata.range(447,416) = std::rand();
        udp_vectors[i].tdata.range(479,448) = std::rand();
        udp_vectors[i].tdata.range(511,480) = std::rand();

        udp_vectors[i].tlast = (i == NTESTS-1) ? 1 : std::rand() % 2;
        // udp_vectors[i].tlast = 1;

        if(udp_vectors[i].tlast){
            udp_vectors[i].tkeep = 0x0;
            if(first){
                tkeep_index[i] = (std::rand() % 62); // first packet 2 bytes minimum for tdest
            }
            else{
                tkeep_index[i] = (std::rand() % 63); // every other packet 1 byte minimum
            }
            udp_vectors[i].tkeep.range(63, tkeep_index[i]) = 0xFFFFFFFFFFFFFFFF;
        }
        else{
            tkeep_index[i] = 63;
            udp_vectors[i].tkeep = 0xFFFFFFFFFFFFFFFF;
        }

        // axis supports transfer interleaving
        metadata.remote_port = std::rand();
        metadata.local_port = std::rand();
        metadata.remote_ip = std::rand();

        meta_vectors[i] = metadata;

        first = udp_vectors[i].tlast; // first is equal to last iteration's tlast
    }

    i=0;
    j=0;
    done=0;
    cycle_buffer=0;
    max_buffer=3;
    while(!done){
        // write in testcases with 1/3 chance to not send
        send_done = i == NTESTS;
        send = std::rand() % 3;
        if(!send_done && send){
            from_udp.write(udp_vectors[i]);
            meta_from_udp = meta_vectors[i];
            i++;
        }
        // cycle bridge
        udp_bridge(
            from_udp,
            to_app,
            from_app,
            to_udp,
            meta_from_udp,
            meta_to_app,
            meta_from_app,
            meta_to_udp
        );
        // read testcases and metadata as soon as available
        if(!to_app.empty()){
            app_results[j] = to_app.read();
            meta_app_results[j] = meta_to_app;            
            j++;
        }
        if(send_done){
            cycle_buffer++;
        }
        done = send_done && cycle_buffer == max_buffer;
    }

    // loopback results
    k=0;
    l=0;
    done=0;
    timer=0;
    max_timer=1000;
    timeout=0;
    while(!done){
        // write in testcases with 1/3 chance to not send        
        send_done = k == j;
        send = std::rand() % 3;
        if(!send_done && send){
            from_app.write(app_results[k]);
            meta_from_app = meta_app_results[k];
            k++;
        }
        // cycle bridge
        udp_bridge(
            from_udp,
            to_app,
            from_app,
            to_udp,
            meta_from_udp,
            meta_to_app,
            meta_from_app,
            meta_to_udp
        );
        if(!to_udp.empty()){
            udp_results[l] = to_udp.read();
            meta_results[l] = meta_to_udp;            
            l++;
            timer=0; // reset timer
        }
        if(send_done){
            timer++;
            timeout = timer == max_timer;
        }
        done = timeout || (send_done && l == NTESTS);
    }

    if(timeout){
        std::cout << "WARNING: Loopback timed out" << std::endl;
    }

    std::cout << "udp packets sent: " << i << std::endl;
    std::cout << "app packets read: " << j << std::endl;
    std::cout << "app packets sent: " << k << std::endl;
    std::cout << "udp packets read: " << l << std::endl;

    for(k=0; k<NTESTS; k++){
            tdata_good = udp_results[k].tdata.range(511,tkeep_index[k]*8) == udp_vectors[k].tdata.range(511,tkeep_index[k]*8);
            tkeep_good = udp_results[k].tkeep == udp_vectors[k].tkeep;
            tlast_good = udp_results[k].tlast == udp_vectors[k].tlast;
            
            if(k<j){
                std::cout << std::hex;
                std::cout << "tdata " << app_results[k].tdata << std::endl;
                std::cout << "tkeep " << app_results[k].tkeep << std::endl;
                std::cout << "tdest " << app_results[k].tdest << std::endl;
                std::cout << "tlast " << app_results[k].tlast << std::endl;
                std::cout << std::dec;
                std::cout << std::endl;
            }
            
            std::cout << std::hex;
            std::cout << "tdata_good " << tdata_good << " " << udp_results[k].tdata << " " << udp_vectors[k].tdata << std::endl;
            std::cout << "tkeep_good " << tkeep_good << " " << udp_results[k].tkeep << " " << udp_vectors[k].tkeep << std::endl;
            std::cout << "tlast_good " << tlast_good << " " << udp_results[k].tlast << " " << udp_vectors[k].tlast << std::endl;
            std::cout << std::dec;
            std::cout << std::endl;

            remote_port_good = meta_results[k].remote_port == meta_vectors[k].remote_port;
            local_port_good = meta_results[k].local_port == meta_vectors[k].local_port;
            remote_ip_good = meta_results[k].remote_ip == meta_vectors[k].remote_ip;
            
            if(k<j){
                std::cout << std::hex;
                std::cout << "remote_port " << meta_app_results[k].remote_port << std::endl;
                std::cout << "local_port " << meta_app_results[k].local_port << std::endl;
                std::cout << "remote_ip " << meta_app_results[k].remote_ip << std::endl;
                std::cout << std::dec;
                std::cout << std::endl;
            }
            
            std::cout << std::hex;
            std::cout << "remote_port_good " << remote_port_good << " " << meta_results[k].remote_port << " " << meta_vectors[k].remote_port << std::endl;
            std::cout << "local_port_good " << local_port_good << " " << meta_results[k].local_port << " " << meta_vectors[k].local_port << std::endl;
            std::cout << "remote_ip_good " << remote_ip_good << " " << meta_results[k].remote_ip << " " << meta_vectors[k].remote_ip << std::endl;
            std::cout << std::dec;
            std::cout << std::endl;

            good = tdata_good && tkeep_good && tlast_good && remote_port_good && local_port_good && remote_ip_good;
            random_retval = random_retval || (!good);

            std::cout << "Random Test iteration " << k << " results " << good << std::endl;
            std::cout << std::endl;
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
