//  Copyright (c) 2016-2016 The segmenter Authors. All rights reserved.
//  Created on: 2016年5月1日 Author: kerry

#ifndef SEGMENTER_ML_LOGIC_
#define SEGMENTER_ML_LOGIC_

#include "ptl_comm_head.h"
#include "ptl_packet.h"
#include "core/common.h"
#include "basic/basictypes.h"
#include "basic/scoped_ptr.h"
#include "relrank_dict.h"


namespace segmenter_logic {

class SegmenterManager {
 public:
	SegmenterManager();
	virtual ~SegmenterManager();
 public:
	static SegmenterManager* GetInstance();
	static void FreeInstance();
 private:
	static SegmenterManager*  instance_;

 public:
	bool Init();
	bool LoadCharCore(const char* file_prefix, const char* dfa_file);

	float GetSegsEx(const char* buf, uint32_t sz, char**& psegbuf, int*& ptype,
			float*& pfreq, uint32_t& cnt);

	float GetSegsEx(const wchar_t* buf, uint32_t sz, wchar_t**& psegbuf,
			int*& ptype, float*& pfreq, uint32_t& cnt, uint32_t hardbreak);

 private:
	Relrank::NGramDictManage  dict_manager_;
};

class Segmenterlogic {
 public:
    Segmenterlogic();
    virtual ~Segmenterlogic();

 private:
    static Segmenterlogic    *instance_;

 public:
    static Segmenterlogic *GetInstance();
    static void FreeInstance();

    bool OnSegmenterConnect(struct server *srv, const int socket);

    bool OnSegmenterMessage(struct server *srv, const int socket,
            const void *msg, const int len);

    bool OnSegmenterClose(struct server *srv, const int socket);

    bool OnBroadcastConnect(struct server *srv, const int socket,
            const void *data, const int len);

    bool OnBroadcastMessage(struct server *srv, const int socket,
            const void *msg, const int len);

    bool OnBroadcastClose(struct server *srv, const int socket);

    bool OnIniTimer(struct server *srv);

    bool OnTimeout(struct server *srv, char* id, int opcode, int time);


 private:
    bool Init();
    bool OnSegmentWord(struct server* srv, int socket,
            struct PacketHead *packet, const void *msg = NULL,
            int32 len = 0);
 private:
    SegmenterManager*         dict_manager_;
};

}  // namespace segmenter_logic

#endif

