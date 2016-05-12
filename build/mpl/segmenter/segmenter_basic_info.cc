//  Copyright (c) 2015-2015 The KID Authors. All rights reserved.
//  Created on: 2016年5月1日 Author: kerry
#include "segmenter_basic_info.h"

namespace segmenter_logic {

SegmentWord::SegmentWord(){
	data_ = new Data();
}

SegmentWord::SegmentWord(const char* word, const int32 type,
		const double idf) {
	data_ = new Data(word, type, idf);
}

SegmentWord::SegmentWord(const SegmentWord& segment_word)
:data_(segment_word.data_){
	if(data_!=NULL){
		data_->AddRef();
	}
}

SegmentWord& SegmentWord::operator =(const SegmentWord& segment_word){
	if(segment_word.data_!=NULL){
		segment_word.data_->AddRef();
	}

	if(data_!=NULL){
		data_->Release();
	}
	data_ = segment_word.data_;
	return (*this);
}

}
