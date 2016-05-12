#ifndef	__RELRANK_DICT_HEADER__
#define	__RELRANK_DICT_HEADER__

#include "ngram_dict.h"
#include "largetrie/trie_map.h"

#define PCRE2_CODE_UNIT_WIDTH 32
#include "pcre2.h"

#define FIELD_COUNT 16


namespace Relrank
{

typedef struct {
    float vals[FIELD_COUNT];
} VALSET;

class NGramDictManage
{
public:
	NGramDictManage();
    ~NGramDictManage();


    /*
     * Retrieve all sub strings of pword, get ids and lengths of these strings
     * pword    input string 
     * sz       length of pword 
     * pids     output ids, a N x M matrix, where N == sz, and M == max
     * plens    output length, a N x M matrix, where N == sz, and M == max
     * max      max amount of possible ids at specific position
     */
    int     get_sub_ids(uint16_t* pword, uint32_t sz, float pvals[][QSS_MAX_ELEM_COUNT], int plens[][QSS_MAX_ELEM_COUNT]);
    /*
    float   get_seg_ids(const uint16_t* pword, uint32_t sz, int *segs, float *prob);
    float   get_seg_ids(const wchar_t* pword, uint32_t sz, int *segs, float *prob);
    */
    float   get_segs(const char* buf, uint32_t sz, char* segbuf, uint32_t bufsz);
    float   get_segs(const uint16_t* buf, uint32_t sz, uint16_t* segbuf, uint32_t bufsz);
    float   get_segs(const wchar_t* buf, uint32_t sz, wchar_t* segbuf, uint32_t bufsz, uint32_t hardbreak);
    float   get_segs_ex(const char* buf, uint32_t sz, char**& psegbuf, int*& ptype, float*& pfreq, uint32_t& cnt);
    float   get_segs_ex(const wchar_t* buf, uint32_t sz, wchar_t**& psegbuf, int*& ptype, float*& pfreq, uint32_t& cnt, uint32_t hardbreak);

    bool    verify_core(const char* rawfile);
    bool    verify_ngram(TrieMap<uint16_t>* coredict, const char* rawfile);

    bool    build_core(const char* rawfile);
    bool    build_ngram(TrieMap<uint16_t>* coredict, const char* rawfile);
    
    bool    load_word_ngram (const char* file_prefix);
    bool    load_char_ngram (const char* file_prefix);
    
    bool    load_word_core  (const char* file_prefix);
    bool    load_char_core  (const char* file_prefix, const char* dfa_file);

    bool    dump_word_ngram (const char* file_prefix);
    bool    dump_char_ngram (const char* file_prefix);
    bool    dump_core(const char* file_prefix);
    bool    dump_ngram(const char* file_prefix);

    //void    finalize();

private:
    NGramDict<uint16_t>*  _char_ngram;
    NGramDict<uint32_t, VALSET, FIELD_COUNT>*  _word_ngram;
    pcre2_code* _splitter_re;
    bool _loaded;
};

}

#endif
