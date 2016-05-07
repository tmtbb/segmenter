//  Copyright (c) 2015-2015 The KID Authors. All rights reserved.
//  Created on: 2015年9月14日 Author: kerry


#include "segmenter_logic.h"
#include "relrank_dict.h"
#include "send_comm.h"
#include "basic/basictypes.h"
#include  <string>
#include <list>
#include "core/common.h"

#define DEFAULT_CONFIG_PATH     "./plugins/segmenter/segmenter_config.xml"

namespace segmenter_logic {

SegmenterManager*
SegmenterManager::instance_ = NULL;

SegmenterManager::SegmenterManager() {
	if (!Init())
		assert(0);
}

SegmenterManager::~SegmenterManager() {
}

bool SegmenterManager::Init() {
	return true;
}

SegmenterManager*
SegmenterManager::GetInstance(){
	if (instance_ == NULL)
		instance_ = new SegmenterManager();

	return instance_;
}

void SegmenterManager::FreeInstance() {
	delete instance_;
}

bool SegmenterManager::LoadCharCore(const char* file_prefix,
		const char* dfa_file) {
	return dict_manager_.load_char_core(file_prefix, dfa_file);
}
float SegmenterManager::GetSegsEx(const char* buf, uint32_t sz,
		char**& psegbuf, int*& ptype, float*& pfreq, uint32_t& cnt){
	return dict_manager_.get_segs_ex(buf, sz, psegbuf, ptype, pfreq, cnt);
}

float SegmenterManager::GetSegsEx(const wchar_t* buf, uint32_t sz, wchar_t**& psegbuf,
		int*& ptype, float*& pfreq, uint32_t& cnt, uint32_t hardbreak) {
	return dict_manager_.get_segs_ex(buf, sz, psegbuf, ptype, pfreq, cnt,
			hardbreak);
}

Segmenterlogic*
Segmenterlogic::instance_ = NULL;

Segmenterlogic::Segmenterlogic() {
    if (!Init())
        assert(0);
}

Segmenterlogic::~Segmenterlogic() {
	SegmenterManager::FreeInstance();
}

bool Segmenterlogic::Init() {
	std::string dict_core = "dict_20150526_web2.core";
	std::string dict_dfa = "dfa_file_qss_yk2.map";
	dict_manager_ = SegmenterManager::GetInstance();
	dict_manager_->LoadCharCore(dict_core.c_str(), dict_dfa.c_str());
    return true;
}

Segmenterlogic*
Segmenterlogic::GetInstance() {
    if (instance_ == NULL)
        instance_ = new Segmenterlogic();
    return instance_;
}



void Segmenterlogic::FreeInstance() {
    delete instance_;
    instance_ = NULL;
}

bool Segmenterlogic::OnSegmenterConnect(struct server *srv, const int socket) {
    return true;
}



bool Segmenterlogic::OnSegmenterMessage(struct server *srv, const int socket,
        const void *msg, const int len) {
    bool r = false;
    struct PacketHead* packet = NULL;
    if (srv == NULL || socket < 0 || msg == NULL
            || len < PACKET_HEAD_LENGTH)
        return false;

    if (!ptl::PacketProsess::UnpackStream(msg, len, &packet)) {
        //LOG_ERROR2("UnpackStream Error socket %d", socket);
        //net::PacketProsess::HexEncode(msg, len);
        return false;
    }

    assert(packet);
    //LOG_MSG("dump packet packet");
    //net::PacketProsess::DumpPacket(packet);
    switch (packet->operate_code) {
      case WORD_SEGMENTER: {
    	OnSegmentWord(srv,socket,packet);
    	break;
    }
        default:
            break;
    }
    ptl::PacketProsess::DeletePacket(msg, len, packet);
    return true;
}

bool Segmenterlogic::OnSegmenterClose(struct server *srv, const int socket) {
    return true;
}



bool Segmenterlogic::OnBroadcastConnect(struct server *srv, const int socket,
        const void *msg, const int len) {
    return true;
}

bool Segmenterlogic::OnBroadcastMessage(struct server *srv, const int socket,
        const void *msg, const int len) {
    return true;
}

bool Segmenterlogic::OnBroadcastClose(struct server *srv, const int socket) {
    return true;
}

bool Segmenterlogic::OnIniTimer(struct server *srv) {
    return true;
}

bool Segmenterlogic::OnTimeout(struct server *srv, char *id,
        int opcode, int time) {
    switch (opcode) {
     default:
        break;
    }
    return true;
}

bool Segmenterlogic::OnSegmentWord(struct server* srv, int socket,
            struct PacketHead *packet, const void *msg,
            int32 len) {
	struct SegmenterWord* segmenter_word =
			(struct SegmenterWord*)packet;

	char** res;
	int* type;
	float* req;
	uint32 bufsz;
	int32 base_num = 5;

	float sc = dict_manager_->GetSegsEx(segmenter_word->content.c_str(),
			segmenter_word->content.length(), res, type, req, bufsz);

	struct WordResult word_result;
	MAKE_HEAD(word_result, WORD_RESULT, 0, 0, 0, 0);

	for (uint32 i = 0; i < bufsz; i++) {
		struct WordUnit* unit = new struct WordUnit;
        memset(unit->word, '\0', WORD_SIZE);
        memcpy(unit->word, res[i],
               (WORD_SIZE - 1) < strlen(res[i]) ?
               (WORD_SIZE - 1) : strlen(res[i]));

        unit->utype = type[i];
        word_result.list.push_back(unit);

        if (word_result.list.size() % base_num == 0 &&
        		word_result.list.size() != 0) {
        	//发送
        	send_message(socket, &word_result);
        	//清理
        	ptl::PacketProsess::ClearWordList((struct PacketHead*)&word_result);

        }
	}

	//剩余分词
	if(word_result.list.size() > 0) {
		send_message(socket, &word_result);
		ptl::PacketProsess::ClearWordList((struct PacketHead*)&word_result);
	}

	return true;
}
}
