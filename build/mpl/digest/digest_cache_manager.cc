//  Copyright (c) 2015-2015 The MNLP Authors. All rights reserved.
//  Created on: 2016年5月15日 Author: kerry

#include "digest_cache_manager.h"
#include "ptl_comm_head.h"
#include "ptl_packet.h"
#include "send_comm.h"

namespace digest_logic {

DigestCacheManager* DigestCacheEngine::digest_mgr_ = NULL;
DigestCacheEngine* DigestCacheEngine::digest_engine_ = NULL;

DigestCacheManager::DigestCacheManager() {
	digest_cache_ = new DigestCache();
	Init();
}

DigestCacheManager::~DigestCacheManager() {
	DeinitThreadrw(lock_);
	if (py_module_)
		Py_DECREF(py_module_);
	if (py_dict_)
		Py_DECREF(py_dict_);
	if (py_analyzer_)
		Py_DECREF(py_analyzer_);
}

void DigestCacheManager::Init() {
	InitThreadrw(&lock_);
	InitPython();
	TestDigest();
}

void DigestCacheManager::TestDigest() {
	std::string digest_info = "上交所在2012年鼓励上市公司每年的现金分红比例不少于30%，但随着中国经济的收缩和融资成本的上升，很多上市公司的现金流愈趋紧张，敢于大比例分红的企业屈指可数，不过，汽车板块无疑是个例外。截至5月12日，从各大车企发布的年度财务报告来看，虽然2015年我国汽车市场增幅比上年同期有所减缓，但九成上市车企净利润均实现不同程度的涨 幅，排名前三位的车企为上汽集团、长安汽车以及长城汽车，净利润额分别为296.51亿元、99.53亿元和80.4亿元。";
	PyObject* pArgs = PyTuple_New(1);
	LOG_MSG2("[full text]: %s",digest_info.c_str());
	PyTuple_SetItem(pArgs, 0, Py_BuildValue("s",digest_info.c_str()));
	PyObject* result = PyObject_CallObject(py_analyzer_,pArgs);
	if (result == NULL)
		return;
	std::string digest = PyString_AsString(result);
}

bool DigestCacheManager::InitPython() {
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
}

bool DigestCacheManager::PrintDict(PyObject* obj) {
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

void DigestCacheManager::SetDigestInfo(const int socket,
		digest_logic::ArticleUnit& unit) {
	base_logic::WLockGd lk(lock_);
	DIGESTINFOS_MAP::iterator it =
			digest_cache_->digest_infos_map_.find(socket);
	if (it == digest_cache_->digest_infos_map_.end()) {
		DIGESTINFO_MAP info_map;
		digest_logic::DigestInfo digest;
		digest.set_id(unit.article_id());
		digest.set_article(unit);
		info_map[unit.article_id()] = digest;
		digest_cache_->digest_infos_map_[socket] = info_map;
	}else {
		DIGESTINFO_MAP info_map = it->second;
		DIGESTINFO_MAP::iterator itr = info_map.find(unit.article_id());
		if (itr != info_map.end()) {
			digest_logic::DigestInfo digest = itr->second;
			digest.set_article(unit);
			info_map[unit.article_id()] = digest;
		}else{
			digest_logic::DigestInfo digest;
			digest.set_id(unit.article_id());
			digest.set_article(unit);
			info_map[unit.article_id()] = digest;
		}
		digest_cache_->digest_infos_map_[socket] = info_map;
	}
}


void DigestCacheManager::SendDigest(const int socket, const int32 article_id) {
	std::string digest;
	CreateDigest(socket,article_id,digest);
	if(digest.empty()){
		LOG_ERROR2("digest null article_id %d",article_id);
		digest = "具体查看内容详情";
		//return;
	}
	struct ArticleResultDigest digest_result;
	MAKE_HEAD(digest_result, ARTICLE_RESULT_DIGEST, 0, 0, 0,0);
	digest_result.article_identifies = article_id;
	digest_result.digest = digest;
	send_message(socket, &digest_result);
}

void DigestCacheManager::CreateDigest(const int socket,
		const int32 article_id, std::string& digest) {
	base_logic::WLockGd lk(lock_);
	DIGESTINFOS_MAP::iterator it =
			digest_cache_->digest_infos_map_.find(socket);
	if (it == digest_cache_->digest_infos_map_.end()) {
		LOG_ERROR2(" socket error not article %d",socket);
		return;
	}


	DIGESTINFO_MAP info_map = it->second;
	DIGESTINFO_MAP::iterator itr = info_map.find(article_id);
	if (itr == info_map.end()) {
		LOG_ERROR2(" article_id error not article %d",article_id);
		return;
	}
	digest_logic::DigestInfo digest_info = itr->second;

	//删除节点值
	info_map.erase(itr);

	digest_info.CreatFullText();
	PyObject* pArgs = PyTuple_New(1);
	//LOG_MSG2("[full text]: %s",digest_info.FullText().c_str());
	PyTuple_SetItem(pArgs, 0, Py_BuildValue("s",digest_info.FullText().c_str()));
	PyObject* result = PyObject_CallObject(py_analyzer_,pArgs);
	if (result == NULL)
		return;
	digest = PyString_AsString(result);
}

}
