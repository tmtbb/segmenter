//  Copyright (c) 2015-2015 The MNLP Authors. All rights reserved.
//  Created on: 2016年5月15日 Author: kerry
#ifndef DIGEST_CACHE_MANAGER_H_
#define DIGEST_CACHE_MANAGER_H_
#include <map>
#include <list>
#include "Python.h"
#include "digest_basic_info.h"
#include "basic/basictypes.h"
#include "logic/logic_comm.h"
#include "thread/base_thread_handler.h"
#include "thread/base_thread_lock.h"

typedef std::map<int32,digest_logic::DigestInfo> DIGESTINFO_MAP; //id - digestinfo
typedef std::map<int,DIGESTINFO_MAP> DIGESTINFOS_MAP; //socket - DIGESTINFO

namespace digest_logic {

class DigestCache {
 public:
	DIGESTINFOS_MAP        digest_infos_map_;
};

class DigestCacheManager {
 public:
	DigestCacheManager();
	virtual ~DigestCacheManager();

	void SetDigestInfo(const int socket,digest_logic::ArticleUnit& unit);

	void SendDigest(const int socket, const int32 article_id);

 private:
	void CreateDigest(const int socket,const int32 article_id,
			 std::string& digest);

	void TestDigest();


 private:
	void Init();
	bool InitPython();
	bool PrintDict(PyObject* obj);
 private:
	struct threadrw_t*    lock_;
	DigestCache*          digest_cache_;

 private:
    PyObject*                 py_module_;
    PyObject*                 py_dict_;
    PyObject*                 py_analyzer_;
};


class DigestCacheEngine {
 private:
    static DigestCacheManager    *digest_mgr_;
    static DigestCacheEngine     *digest_engine_;

    DigestCacheEngine() {}
    virtual ~DigestCacheEngine() {}
 public:
    static DigestCacheManager* GetDigestCacheManager() {
        if (digest_mgr_ == NULL)
        	digest_mgr_ = new DigestCacheManager();
        return digest_mgr_;
    }

    static DigestCacheEngine* GetDigestCacheEngine() {
        if (digest_engine_ == NULL)
        	digest_engine_ = new DigestCacheEngine();
        return digest_engine_;
    }

    static void FreeDigestCacheManager() {
    	delete digest_mgr_;
    }

    static void FreeDigestCacheEngine() {
    	delete digest_engine_;
    }
};
}
#endif
