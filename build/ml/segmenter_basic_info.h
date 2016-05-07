//  Copyright (c) 2015-2015 The KID Authors. All rights reserved.
//  Created on: 2016年5月1日 Author: kerry

#ifndef ML_SEGMENTER_BASIC_INFO
#define ML_SEGMENTER_BASIC_INFO
#include <string>
#include <map>

#include "basic/basictypes.h"
#include "logic/base_values.h"

namespace segmenter_logic {

class SegmentWord {
public:
	explicit SegmentWord();
	explicit SegmentWord(const char* word,const int32 type,
			const double idf);

	~SegmentWord() {
		if (data_ != NULL) {
			data_->Release();
		}
	}

	SegmentWord(const SegmentWord& segment_word);
	SegmentWord& operator = (const SegmentWord& segment_word);

	class Data {
	public:
		Data()
		:refcount_(1)
		,type_(0)
		,idf_(0.00){}
		Data(const std::string& word, const int32 type, const double idf)
		:refcount_(1)
		,type_(type)
		,idf_(idf){}

	public:
		std::string word_;
		int32       type_;
		double      idf_;
		void AddRef() { refcount_++;}
		void Release() {if (!--refcount_) delete this;}
	private:
		int         refcount_;
	};

	Data*           data_;
};

}
#endif
