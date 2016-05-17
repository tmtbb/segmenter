//  Copyright (c) 2015-2015 The KID Authors. All rights reserved.
//  Created on: 2015年9月14日 Author: kerry


#include "digest_logic.h"
#include "send_comm.h"
#include "digest_cache_manager.h"
#include "basic/basictypes.h"
#include  <string>
#include <list>
#include <iostream>
#include "core/common.h"

#define DEFAULT_CONFIG_PATH     "./plugins/digest/digest_config.xml"

namespace digest_logic {

DigestLogic*
DigestLogic::instance_ = NULL;

DigestLogic::DigestLogic() {
    if (!Init())
        assert(0);
}

DigestLogic::~DigestLogic() {
	digest_logic::DigestCacheEngine::FreeDigestCacheEngine();
	digest_logic::DigestCacheEngine::FreeDigestCacheManager();
}

bool DigestLogic::Init() {
	digest_logic::DigestCacheEngine::GetDigestCacheEngine();
	digest_logic::DigestCacheEngine::GetDigestCacheManager();
    return true;
}

DigestLogic*
DigestLogic::GetInstance() {
    if (instance_ == NULL)
        instance_ = new DigestLogic();
    return instance_;
}



void DigestLogic::FreeInstance() {
    delete instance_;
    instance_ = NULL;
}

/*
bool DigestLogic::InitPython() {
	PyRun_SimpleString("import sys");
	PyRun_SimpleString("sys.path.append('./')");

	py_module_ = PyImport_ImportModule("textrank_manager");
	if (!py_module_) {
		LOG_ERROR("Cant open python file");
		return false;
	}

	py_dict_ =  PyModule_GetDict(py_module_);
	if (!py_dict_) {
		LOG_ERROR("Cant find dictionary");
		return false;
	}

	PrintDict(py_dict_);

	py_analyzer_ = PyDict_GetItemString(py_dict_,"Analyzer");
	if (!py_analyzer_) {
		LOG_ERROR("Cant find TextRank");
		return false;
	}
	return true;


	/*PyRun_SimpleString("import sys");
	PyRun_SimpleString("import os");
	PyRun_SimpleString("sys.path.append('./')");
	PyRun_SimpleString("f=open('zhou.txt','w')");
	PyRun_SimpleString("f.write(os.getcwd())");
	//导入模块
	PyObject* pModule = PyImport_ImportModule("textrank_manager");
	if (!pModule) {
		LOG_ERROR("Cant open python file!/n");
		return -1;
	}

	//模块字典
	PyObject* py_dict = PyModule_GetDict(pModule);
	if (!py_dict) {
		LOG_ERROR("Cant find dictionary");
		return false;
	}

	PrintDict(py_dict);

	PyObject* pClassTextRank = PyDict_GetItemString(py_dict,"Analyzer");
	if (!pClassTextRank) {
		LOG_ERROR("Cant find TextRank");
		return false;
	}

	//实例化
	/*PyObject* pInstanceTextRank = PyInstance_New(pClassTextRank,NULL,NULL);
	if (!pInstanceTextRank) {
		LOG_ERROR("Cant find TextRank instance");
		return false;
	}*/
	/*std::string content = "在这份全球最具价值品牌排行榜中(共100个品牌)，科技行业占据了17席，排名前十的科技公司有五家，除了前三名的苹果、谷歌、微软意外，Facebook排名第五，品牌价值为526亿美元；IBM排名第七>，品牌价值为81.7亿美元。福布斯发布全球最具价值品牌排行榜之后，引来网友议论纷纷。网友称，“没看到中国企业的身影”，“前一百名居然都没有中国品牌”，“几乎清一色高科技企业”。据悉《福布斯》所评出的全球最具价值的100大品牌来自16个国家以及19大行业。来自美国的公司占据榜单的半壁江山，达到了52家，其次是德国(11家)、日本(8家)以及法国(6家)。";

	PyObject* pArgs = PyTuple_New(1);
	PyTuple_SetItem(pArgs, 0, Py_BuildValue("s",content.c_str()));
	PyObject* pRet = PyObject_CallObject(pClassTextRank,pArgs);
	//std::cout<<"11111"<<std::endl;
	LOG_MSG("^^^^^^^^^^^^^^^^^^^^^^");
	LOG_MSG2("len[%d]",PyString_AsString(pRet));
	LOG_MSG("^^^^^^^^^^^^^^^^^^^^^^");

	if (pArgs)
		Py_DECREF(pArgs);
	if (pRet)
		Py_DECREF(pRet);
	if (pClassTextRank)
		Py_DECREF(pClassTextRank);
	if (py_dict)
		Py_DECREF(py_dict);
	if (pModule)
		Py_DECREF (pModule);*/

	/*return true;
}*/

/*
bool DigestLogic::PrintDict(PyObject* obj) {
	if (!PyDict_Check(obj))
		return false;
	PyObject *k, *keys;
    keys = PyDict_Keys(obj);
    for (int i = 0; i < PyList_GET_SIZE(keys); i++) {
        k = PyList_GET_ITEM(keys, i);
        char* c_name = PyString_AsString(k);
        LOG_MSG2("%s", c_name);
    }
    return true;
}
*/

bool DigestLogic::OnDigestConnect(struct server *srv, const int socket) {
    return true;
}



bool DigestLogic::OnDigestMessage(struct server *srv, const int socket,
        const void *msg, const int len) {
    bool r = false;
    struct PacketHead* packet = NULL;
    if (srv == NULL || socket < 0 || msg == NULL
            || len < PACKET_HEAD_LENGTH)
        return false;

    if (!ptl::PacketProsess::UnpackStream(msg, len, &packet)) {
        LOG_ERROR2("UnpackStream Error socket %d", socket);
        ptl::PacketProsess::HexEncode(msg, len);
        return false;
    }

    assert(packet);
    LOG_MSG("dump packet packet");
    ptl::PacketProsess::DumpPacket(packet);
    switch (packet->operate_code) {
      case ARTICLE_DIGEST_UNIT: {
    	OnArticleDigestUnit(srv,socket,packet);
    	break;
    }
      case ARTICLE_DIGEST_END : {
    	  OnArticleDigestEnd(srv,socket,packet);
    	  break;
      }
        default:
            break;
    }
    ptl::PacketProsess::DeletePacket(msg, len, packet);
    return true;
}

bool DigestLogic::OnDigestClose(struct server *srv, const int socket) {
    return true;
}



bool DigestLogic::OnBroadcastConnect(struct server *srv, const int socket,
        const void *msg, const int len) {
    return true;
}

bool DigestLogic::OnBroadcastMessage(struct server *srv, const int socket,
        const void *msg, const int len) {
    return true;
}

bool DigestLogic::OnBroadcastClose(struct server *srv, const int socket) {
    return true;
}

bool DigestLogic::OnIniTimer(struct server *srv) {
    return true;
}

bool DigestLogic::OnTimeout(struct server *srv, char *id,
        int opcode, int time) {
    switch (opcode) {
     default:
        break;
    }
    return true;
}

bool DigestLogic::OnArticleDigestUnit(struct server* srv, int socket,
            struct PacketHead *packet, const void *msg,
            int32 len) {
	struct ArticleDigestUnit* article = (struct ArticleDigestUnit*)(packet);
	digest_logic::ArticleUnit unit;
	unit.set_article_id(article->article_identifies);
	unit.set_content(article->article_unit);
	digest_logic::DigestCacheEngine::GetDigestCacheManager()->SetDigestInfo(
			socket, unit);
	return true;
}

bool DigestLogic::OnArticleDigestEnd(struct server* srv, int socket,
                struct PacketHead *packet, const void *msg,
                int32 len) {
	struct ArticleDigestEnd* end = (struct ArticleDigestEnd*)(packet);
	digest_logic::DigestCacheEngine::GetDigestCacheManager()->SendDigest(
			socket,end->article_identifies);
	return true;
}


}
