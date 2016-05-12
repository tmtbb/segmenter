#include <string>

#include "relrank_dict.h"
#include "largetrie/utility.h"
#include "largetrie/transcode.h"
//#include "atom_recognizer.h"
//#include "re2/re2.h"

#define PCRE2_CODE_UNIT_WIDTH 8
#include "pcre2.h"

int main(int agrs,char *argv[]) {
	std::string dict_core = "dict_20150526_web2.core";
	std::string dict_dfa = "dfa_file_qss_yk2.map";
	std::string word = "程序员，宇宙中读书最多的群体之一！23号世界读书日，我们联合@图灵教育 为大家准备了各个领域的9本好书， 转发+书名，抽9位";
	Relrank::NGramDictManage dict;
	dict.load_char_core(dict_core.c_str(), dict_dfa.c_str());
	long cnt = 0;
	long reserved = 0;
	char** res;
	int* type;
	float* freq;
	uint32_t bufsz;
	float sc = dict.get_segs_ex(word.c_str(), word.length(), res, type, freq,
			bufsz);
    for (uint32_t i = 0; i < bufsz; i ++) {
        printf("%s\t%d\t%.2f\t", res[i], type[i], freq[i]);
        delete [] res[i];
    }
    printf("%f\n", sc);
    delete [] res;
    delete [] type;
    delete [] freq;
    cnt ++;
    if (cnt % 10000 == 0)
        fprintf(stderr, "%ld out of %ld\n", reserved, cnt);

}
