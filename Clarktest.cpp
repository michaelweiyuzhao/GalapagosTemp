#include <iostream>
#include "ap_int.h"
#include "hls_stream.h"
#include "ap_utils.h"

#include "udp_bridge.h"
#include "udp.dat"
using namespace std;
int _main(){

	static udp_axis udp_resultvec;
	static app_axis app_resultvec;
	static hls::stream <app_axis> to_app;
	static hls::stream <app_axis> from_app;
	static hls::stream <udp_axis> to_udp;
	static hls::stream <udp_axis> from_udp;
	static meta metadata;

	int i;
	for (i=0; i<4; i++){
		from_udp.write(udp_testvec[i]);
		cout << "----------------------------------------------------" << endl;
		cout << "udp input packet #" << 0 << " : " << endl;
		cout << "\tdata : 0x" << setfill('0') << setw(128) << udp_testvec[i].tdata.to_string(16).replace(0,2,"") << endl;
		cout << "\tkeep : 0x" << setfill('0') << setw(16) << udp_testvec[i].tkeep.to_string(16).replace(0,2,"") << endl;
		cout << "\tlast : 0x" << setfill('0') << setw(1) << udp_testvec[i].tlast.to_string(16).replace(0,2,"") << endl;
		cout << endl;
	}
	for (i=0; i<7; i++){
		udp_bridge(from_udp,to_app,from_app,to_udp,metadata,metadata,metadata,metadata);
	}
	while (!to_app.empty()){
		app_resultvec = to_app.read();
		from_app.write(app_resultvec);
		cout << "----------------------------------------------------" << endl;
		cout << "app output packet #" << 0 << " : " << endl;
		cout << "\tdata : 0x" << setfill('0') << setw(128) << app_resultvec.tdata.to_string(16).replace(0,2,"") << endl;
		cout << "\tkeep : 0x" << setfill('0') << setw(16) << app_resultvec.tkeep.to_string(16).replace(0,2,"") << endl;
		cout << "\tdest : 0x" << setfill('0') << setw(4) << app_resultvec.tdest.to_string(16).replace(0,2,"") << endl;
		cout << "\tlast : 0x" << setfill('0') << setw(1) << app_resultvec.tlast.to_string(16).replace(0,2,"") << endl;
		cout << endl;
	}
	for (i=0; i<7; i++){
		udp_bridge(from_udp,to_app,from_app,to_udp,metadata,metadata,metadata,metadata);
	}
	while (!to_udp.empty()){
		udp_resultvec = to_udp.read();
		cout << "----------------------------------------------------" << endl;
		cout << "udp output packet #" << 0 << " : " << endl;
		cout << "\tdata : 0x" << setfill('0') << setw(128) << udp_resultvec.tdata.to_string(16).replace(0,2,"") << endl;
		cout << "\tkeep : 0x" << setfill('0') << setw(16) << udp_resultvec.tkeep.to_string(16).replace(0,2,"") << endl;
		cout << "\tlast : 0x" << setfill('0') << setw(1) << udp_resultvec.tlast.to_string(16).replace(0,2,"") << endl;
		cout << endl;
	}

	return 0;
}
