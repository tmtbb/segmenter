//  Copyright (c) 2015-2015 The MNLP Authors. All rights reserved.
//  Created on: 2016年5月15日 Author: kerry

#include "digest_cache_manager.h"

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
		digest_logic::DigestInfo) {

}

void DigestCacheManager::CreateDigest(const int socket,const int32 article_id) {

}

}
