#include <iostream>
//#include <boost/algorithm/string.hpp>
#include <vector>
#include <string>

#include "relrank_dict.h"
#include "largetrie/utility.h"
#include "largetrie/transcode.h"
//#include "atom_recognizer.h"
//#include "re2/re2.h"

#define PCRE2_CODE_UNIT_WIDTH 8
#include "pcre2.h"

const char *me = NULL;

void usage() {
    printf("%s -c{build core}  word_id.file input.file output.file\n", me);
    printf("%s -n{build ngram}  word_id.file input.file output.file\n", me);
    printf("%s -s{search} core.file ngram.file word1 word2\n", me);
}

int main(int args, char *argv[]) {

    /*
    int errornumber;
    PCRE2_SIZE erroroffset;
    pcre2_code *re = pcre2_compile((PCRE2_SPTR)"[()（）《》\\[\\]【】\"\"“”、,，。：;；？!！|]+|[-.~=_]{2,}|\\.[ ]+|-[ ]+|[ ]+-|(?<![ .0-9a-z-])[ ]+|[ ]+(?![ .0-9a-z-])|(.)\\1{5,}", 
            PCRE2_ZERO_TERMINATED, PCRE2_UTF, &errornumber, &erroroffset, NULL);
    pcre2_match_data *match_data = pcre2_match_data_create_from_pattern(re, NULL);

    char buff[10 * 1024 * 1024];
    gets(buff);

    size_t totallen = strlen(buff);
    int rc;
    int start = 0;
    int len = 0;
    int skip = 0;
    do {
        rc = pcre2_match(re, (PCRE2_SPTR)buff + start, totallen - start, 0, 0, match_data, NULL);
        if (rc == PCRE2_ERROR_NOMATCH) {
            len = totallen - start;
            skip = 0;
        }
        else if (rc > 0) {
            PCRE2_SIZE *ovector = pcre2_get_ovector_pointer(match_data);
            len = (int)ovector[0];
            skip = (int)ovector[1] - (int)ovector[0];
        }
        else {
            return 1;
        }
        printf("start at offset %d, len %d, skip %d, result: %s\n", start, len, skip, string(buff + start, len).c_str());
        start += len + skip;
    } while (rc > 0);
    pcre2_match_data_free(match_data);
    pcre2_code_free(re);
    return 0;
    */

    me = argv[0];
    printf("me %s",me);
    if(args < 2) {
        usage();
        return 1;
    }
    /*
    if (argv[1][0] == '-' && argv[1][1] == 'd') {
        qss::segmenter::AtomRecognizer* atom = qss::segmenter::AtomRecognizer::get_instance();
        std::string line;
        while (std::getline(std::cin, line)) {
            int sz = line.length();
            uint16_t wbuf[sz * 2];
            int len = convToucs2(line.c_str(), sz, wbuf, sz * 2, qsrch_code_utf8);
            atom->matchAll(wbuf, sz);
        }
    }
    else*/ if (argv[1][0] == '-' && argv[1][1] == 'c') {

        Relrank::NGramDictManage dict;

        dict.build_core(argv[3]);
        dict.dump_core(argv[4]);
        dict.verify_core(argv[3]);

    } else if (argv[1][0] == '-' && argv[1][1] == 'v' && argv[1][2] == 'c') {

        Relrank::NGramDictManage dict;
        dict.load_char_core(argv[3], argv[4]);
        dict.verify_core(argv[5]);

    } else if (argv[1][0] == '-' && argv[1][1] == 't') {
        Relrank::NGramDictManage dict;
        dict.load_char_core(argv[2], argv[3]);
        std::string line;
        long cnt = 0;
        long reserved = 0;
        while (std::getline(std::cin, line)) {
            /*
            vector<string> flds;
            boost::split(flds, line, boost::is_any_of("\t"));
            long n = atol(flds[1].c_str());
            if (n < 20)
                continue;
            //std::cout<<flds[0]<<"\t"<<n<<std::endl;
            int sz = flds[0].length();
            uint16_t wbuf[sz * 2];
            int len = convToucs2(flds[0].c_str(), sz, wbuf, sz * 2, qsrch_code_utf8);
            int *segs = new int[len];
            dict.get_seg_ids(wbuf, len, segs);
            if (segs[0] == len) {
                reserved ++;
                std::cout<<line<<std::endl;
            }
            delete [] segs;
            */
            /*
            uint32_t bufsz = line.length() * 8;
            char* res = new char[bufsz];
            */
            char** res;
            int* type;
            float* freq;
            uint32_t bufsz;
            float sc = dict.get_segs_ex(line.c_str(), line.length(), res, type, freq, bufsz);
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
        /*
           int sz = strlen(argv[3]);
           uint16_t wbuf[sz * 2];
           int len = convToucs2(argv[3], sz, wbuf, sz * 2, qsrch_code_utf8);
           dict.get_seg_ids(wbuf, len);
         */


    } 
    return 1;
}



