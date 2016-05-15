//  Copyright (c) 2015-2015 The MNLP Authors. All rights reserved.
//  Created on: 2016年5月15日 Author: kerry

#include "basic/basictypes.h"
#include "logic/base_values.h"
#include <list>

namespace  digest_logic {

class ArticleUnit {
 public:
	ArticleUnit();
	ArticleUnit(const ArticleUnit& article_unit);
	ArticleUnit& operator = (const ArticleUnit& article_unit);

	~ArticleUnit() {
		if (data_ != NULL){
			data_->Release();
		}
	}

	void set_id(const int32 id) {data_->id_ = id;}
	void set_article_id(const int32 article_id) {
		data_->article_id_ = article_id;
	}

	void set_content(const std::string& content) {
		data_->article_content_ = content;
	}

	const int32 id () const { return data_->id_;}
	const int32 article_id () const {return data_->article_id_;}
	const std::string& article_content() const {return data_->article_content_;}

 private:
	class Data {
	  public:
		Data()
	  	  :refcount_(1)
	  	  , id_(0)
	  	  , article_id_(0) {}

	  public:
		int32          id_;
		int32          article_id_;
		std::string    article_content_;

		void AddRef() {__sync_fetch_and_add(&refcount_, 1);}
		void Release() {__sync_fetch_and_sub(&refcount_, 1);
        	if (!refcount_)delete this;
		}
	  private:
		int            refcount_;
	};

	Data*             data_;
};


class DigestInfo {
 public:
	DigestInfo();
	DigestInfo(const DigestInfo& digest_info);

	DigestInfo& operator = (const DigestInfo& digest_info);

	~DigestInfo() {
		if (data_ != NULL){
			data_->Release();
		}
	}

	void set_id(const int32 id) {data_->id_ = id;}
	void set_article(const ArticleUnit& unit) {
		data_->list_.push_back(unit);
	}

 private:
	class Data {
	 public:
		int32                    id_;
		std::list<ArticleUnit>  list_;
		void AddRef() {__sync_fetch_and_add(&refcount_, 1);}
		void Release() {__sync_fetch_and_sub(&refcount_, 1);
        	if (!refcount_)delete this;
		}
	  private:
		int            refcount_;
	};

	Data*                        data_;
};

}
