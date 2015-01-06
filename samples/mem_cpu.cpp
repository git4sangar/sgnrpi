int TEventHandler::sendMemoryCPUInfo(DiagCpuMemory_Info_t data, int realTime_flag=0){

    int ret = 0;
    stringstream ss;
    time_t rawtime;
    struct tm *timeinfo;
    char buffer[80];
    
    time(&rawtime);
    timeinfo = localtime(&rawtime);

    //char *date_str = asctime(timeinfo);
    strftime(buffer, 80, "%a %b %d %Y %T GMT%z", timeinfo);

    json_t *pJsonPkt = json_object();
    if(realTime_flag == 0 || realTime_flag == 1) {
    	cout << "&& Entering sendMemoryCPUInfo" << endl;
		json_object_set_new( pJsonPkt, "logTime", json_string(buffer) );
		json_object_set_new( pJsonPkt, "StbID", json_string(SystemInfo.StbID.c_str()) );
		json_object_set_new( pJsonPkt, "VMSUA", json_string(SystemInfo.UnitAddress.c_str()) );

		ss.str(""); ss << data.ramTotalMemory <<" MB";
		json_object_set_new( pJsonPkt, "RAM_Total", json_string(ss.str().c_str()) );

		ss.str(""); ss << data.ramTotalMemoryFree <<" MB";
		json_object_set_new( pJsonPkt, "RAM_Free", json_string(ss.str().c_str()) );
		json_object_set_new( pJsonPkt, "GWSN", json_string(SystemInfo.gwSN.c_str()) );
		json_object_set_new( pJsonPkt, "SNO", json_string(SystemInfo.hostSN.c_str()) );

		int iCount = 0;
		stringstream ss1, ss2;
		for(vector<pair<string, int> >::iterator it = data.processMemory.begin();
				it != data.processMemory.end();
				++it, ++iCount) {
			if(iCount > 4) break;
			ss1.str(""); ss1 << "Process_" << iCount;
			ss2.str(""); ss2 << it->first;
			json_object_set_new( pJsonPkt, ss1.str().c_str(), json_string(ss2.str().c_str()) );

			ss1.str(""); ss1 << "Memory_" << iCount;
			ss2.str(""); ss2 << it->second;
			json_object_set_new( pJsonPkt, ss1.str().c_str(), json_string(ss2.str().c_str()) );
		}
		json_object_set_new( pJsonPkt, "EventType", json_string("DIAG_DATA") );
		json_object_set_new( pJsonPkt, "fid", json_string("VZ_MEMORY_INFO") );

		if(realTime_flag == 0) {
			json_object_set_new( pJsonPkt, "Critical", json_string("NO") );
		} else {
			json_object_set_new( pJsonPkt, "Critical", json_string("YES") );
		}
		char *pSzJsonPkt	= json_dumps(pJsonPkt, JSON_PRESERVE_ORDER);
		cout << "&& all memory details accumulated \n" << pSzJsonPkt << endl;
		postDataToSender( pSzJsonPkt, SystemInfo.EventDataServer, SystemInfo.EventDataPort, (realTime_flag != 0) );
		json_decref(pJsonPkt); free(pSzJsonPkt);
    }

    pJsonPkt = json_object();
    if(realTime_flag == 0 || realTime_flag == 2){
		json_object_set_new( pJsonPkt, "logTime", json_string(buffer) );
		json_object_set_new( pJsonPkt, "StbID", json_string(SystemInfo.StbID.c_str()) );
		json_object_set_new( pJsonPkt, "VMSUA", json_string(SystemInfo.UnitAddress.c_str()) );
		json_object_set_new( pJsonPkt, "GWSN", json_string(SystemInfo.gwSN.c_str()) );
		json_object_set_new( pJsonPkt, "SNO", json_string(SystemInfo.hostSN.c_str()) );

	    double cpu = 0;
		int iCount = 0;
		stringstream ss1, ss2;
	    for(vector<pair<string, float> >::iterator it=data.processCPU.begin(); it!=data.processCPU.end(); ++it, ++iCount) {
		    if(iCount < 5) {

		    	ss1.str(""); ss1 << "Process_" << iCount;
		    	ss2.str(""); ss2 << it->first;
		    	json_object_set_new( pJsonPkt, ss1.str().c_str(), json_string(ss2.str().c_str()) );

				ss1.str(""); ss1 << "CPU_" << iCount;
				ss2.str(""); ss2 << it->second;
				json_object_set_new( pJsonPkt, ss1.str().c_str(), json_string(ss2.str().c_str()) );
		    }
		    cpu = cpu + it->second;
	    }
	    ss1.str(""); ss1 << cpu;
	    json_object_set_new(pJsonPkt, "TotalCPU_Utilization", json_string(ss1.str().c_str()) );
	    json_object_set_new(pJsonPkt, "EventType", json_string("DIAG_DATA") );
	    json_object_set_new(pJsonPkt, "fid", json_string("VZ_CPU_INFO") );

	    if(realTime_flag == 0) {
	    	json_object_set_new( pJsonPkt, "Critical", json_string("NO") );
	    } else {
	    	json_object_set_new( pJsonPkt, "Critical", json_string("YES") );
	    }
	    char *pSzJsonPkt	= json_dumps(pJsonPkt, JSON_PRESERVE_ORDER);
	    cout << "&& all CPU details accumulated \n" << pSzJsonPkt << endl;
	    postDataToSender(pSzJsonPkt, SystemInfo.EventDataServer, SystemInfo.EventDataPort, (realTime_flag != 0) );
	    json_decref(pJsonPkt); free(pSzJsonPkt);
    }

    return ret;
}
