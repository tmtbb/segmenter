//  Copyright (c) 2015-2015 The MNLP Authors. All rights reserved.
//  Created on: 2016年5月15日 Author: kerry

#include "digest_basic_info.h"

namespace digest_logic {

DigestInfo::DigestInfo() {
	data_ = new Data();
}

DigestInfo::DigestInfo(const DigestInfo& digest_info)
:data_(digest_info.data_) {
	if (data_ != NULL){
		data_->AddRef();
	}
}

DigestInfo& DigestInfo::operator =(
		const DigestInfo& digest_info) {
	if (digest_info.data_ != NULL) {
		digest_info.data_->AddRef();
	}

	if (data_ != NULL) {
		data_->Release();
	}

	data_ = digest_info.data_;
	//data_->list_ = digest_info.data_->list_;
	return (*this);
}

void DigestInfo::CreatFullText() {
	data_->full_text_ = "";
	while (data_->list_.size() > 0) {
		ArticleUnit unit =  data_->list_.front();
		data_->list_.pop_front();
		data_->full_text_.append(unit.article_content());
	}
}

ArticleUnit::ArticleUnit() {
	data_ = new Data();
}

ArticleUnit::ArticleUnit(const ArticleUnit& article_unit)
:data_(article_unit.data_) {
	if (data_ != NULL) {
		data_->AddRef();
	}
}

ArticleUnit& ArticleUnit::operator =(const ArticleUnit& article_unit) {
	if (article_unit.data_ != NULL) {
		article_unit.data_->AddRef();
	}

	if (data_ != NULL) {
		data_->Release();
	}

	data_ = article_unit.data_;
	return (*this);
}


}
