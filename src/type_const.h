#ifndef __QSS_SEGMENTER_TYPE_CONST_HEAD__
#define __QSS_SEGMENTER_TYPE_CONST_HEAD__

namespace qss {
namespace segmenter {

typedef int  k_type_t;
const k_type_t k_type_unknown = 0;
const k_type_t k_type_alphabet = 1;
const k_type_t k_type_digit_number = 2;
const k_type_t k_type_eng_number = 4;
const k_type_t k_type_punctuation = 8;
const k_type_t k_type_date = 16;
const k_type_t k_type_time = 32;
const k_type_t k_type_telephone = 64;
const k_type_t k_type_email = 128;
const k_type_t k_type_ip = 256;
const k_type_t k_type_chinese_numeric_str = 512;
const k_type_t k_type_special_word = 1024; //小时，分钟，世纪等可能出现在原子词中的普通词条
const k_type_t k_type_chinese_number = 2048;
const k_type_t k_type_version_number = 4096;
const k_type_t k_type_percent = 8192;
const k_type_t k_type_non_english = 16384;
const k_type_t k_type_non_eng_number = 32768;
}
}
#endif
