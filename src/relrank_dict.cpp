#include "relrank_dict.h"
#include "largetrie/utility.h"
#include "largetrie/transcode.h"

using namespace Relrank;

NGramDictManage::NGramDictManage()
{
    _char_ngram = NULL;
    _splitter_re = NULL;
    _loaded = false;
    _char_ngram  = new NGramDict<uint16_t>();
    int errornumber;
    PCRE2_SIZE erroroffset;
    _splitter_re = pcre2_compile((PCRE2_SPTR)L"[()（）《》\\[\\]【】\"\"“”、,，。：;；？!！|]+|[-.~=_]{2,}|\\.[ ]+|-[ ]+|[ ]+-|(?<![ .0-9a-z-])[ ]+|[ ]+(?![ .0-9a-z-])|(.)\\1{5,}", 
            PCRE2_ZERO_TERMINATED, PCRE2_UTF, &errornumber, &erroroffset, NULL);
}

NGramDictManage::~NGramDictManage()
{
    if (_char_ngram)
        delete  _char_ngram;
    _char_ngram = NULL;
    if (_splitter_re)
        pcre2_code_free(_splitter_re);
    _splitter_re = NULL;
}

int NGramDictManage::get_sub_ids(uint16_t* wbuf, uint32_t sz, float pvals[][QSS_MAX_ELEM_COUNT], int plens[][QSS_MAX_ELEM_COUNT])
{
    _char_ngram->get_sub_ids(wbuf, sz, pvals, plens);
    return sz;
}

float NGramDictManage::get_segs_ex(const wchar_t* buf, uint32_t sz, wchar_t**& psegbuf, int*& ptype, float*& pfreq, uint32_t& cnt, uint32_t hardbreak)
{
    uint32_t lastcnt = 0;
    cnt = 0;
    psegbuf = NULL;
    ptype = NULL;
    pfreq = NULL;
    float ret = 0.0;


    pcre2_match_data *match_data = pcre2_match_data_create_from_pattern(_splitter_re, NULL);
    int rc;
    uint32_t start = 0;
    uint32_t len = 0;
    uint32_t skip = 0;


    do {
        rc = pcre2_match(_splitter_re, (PCRE2_SPTR)buf + start, sz - start, 0, 0, match_data, NULL);
        if (rc == PCRE2_ERROR_NOMATCH) {
            len = sz - start;
            skip = 0;
        }
        else if (rc > 0) {
            PCRE2_SIZE *ovector = pcre2_get_ovector_pointer(match_data);
            len = (uint32_t)ovector[0];
            skip = (uint32_t)ovector[1] - (uint32_t)ovector[0];
        }
        else {
            pcre2_match_data_free(match_data);
            return 0.0;
        }
        //fprintf(stderr, "start at offset %d, len %d, skip %d\n", start, len, skip);
        if (len > 0) {
            if (len > 1000) len = 1000; //hardbreak
            int *segs = new int[len];
            float *prob = new float[len];
            int *type = new int[len];
            for (uint32_t i = 0; i < len; i++) prob[i] = 0.0;
            ret += _char_ngram->get_seg_ids(buf + start, len, segs, prob, type, 0);
            for (uint32_t i = 0; i < len; i += segs[i])
                cnt ++;
            psegbuf = (wchar_t**)realloc(psegbuf, cnt * sizeof(wchar_t*));
            ptype = (int*)realloc(ptype, cnt * sizeof(int));
            pfreq = (float*)realloc(pfreq, cnt * sizeof(float));
            for (uint32_t i = 0; i < len; i += segs[i]) {
                psegbuf[lastcnt] = new wchar_t[segs[i] + 1];
                memcpy(psegbuf[lastcnt], buf + start + i, segs[i] * sizeof(wchar_t));
                psegbuf[lastcnt][segs[i]] = '\0';
                ptype[lastcnt] = type[i];
                pfreq[lastcnt] = (i + segs[i] < len) ? prob[i] - prob[i + segs[i]] : prob[i];
                lastcnt ++;
            }
            delete [] segs;
            delete [] prob;
            delete [] type;
        }
        if (skip > 0) {
            cnt ++;
            psegbuf = (wchar_t**)realloc(psegbuf, cnt * sizeof(wchar_t*));
            ptype = (int*)realloc(ptype, cnt * sizeof(int));
            pfreq = (float*)realloc(pfreq, cnt * sizeof(float));
            psegbuf[lastcnt] = new wchar_t[skip + 1];
            memcpy(psegbuf[lastcnt], buf + start + len, skip * sizeof(wchar_t));
            psegbuf[lastcnt][skip] = '\0';
            ptype[lastcnt] = 0;
            pfreq[lastcnt] = 0;
            lastcnt ++;
        }
        start += len + skip;
    } while (rc > 0);
    pcre2_match_data_free(match_data);

    return ret;
}

float NGramDictManage::get_segs(const wchar_t* buf, uint32_t sz, wchar_t* segbuf, uint32_t bufsz, uint32_t hardbreak)
{
    /*
    for (uint32_t i = 0; i < sz; i ++)
        fprintf(stderr, "orig: %d\t%d\t%ld\n", i, buf[i], sizeof(wchar_t));
        */
    int *segs = new int[sz];
    float *prob = new float[sz];
    int p = 0;
    float ret = 0.0;
    for (uint32_t n = 0; n < sz; n += hardbreak) {
        uint32_t sz2 = (hardbreak < sz - n ? hardbreak : sz - n);
        for (uint32_t i = 0; i < sz2; i++) prob[i] = 0.0;
        ret += _char_ngram->get_seg_ids(buf + n, sz2, segs, prob, 0);
        for (uint32_t i = 0; i < sz2; i += segs[i]) {
            if (p > 0) {
                segbuf[p] = '\1';
                p ++;
            }
            memcpy(segbuf + p, buf + n + i, segs[i] * sizeof(wchar_t));
            p += segs[i];
        }
    }
    segbuf[p] = '\0';
    delete [] segs;
    delete [] prob;
    return ret;

    /*
    int *segs = new int[sz];
    float *prob = new float[sz];
    int p = 0;
    float ret = 0.0;


    pcre2_match_data *match_data = pcre2_match_data_create_from_pattern(_splitter_re, NULL);
    int rc;
    uint32_t start = 0;
    uint32_t len = 0;
    uint32_t skip = 0;


    do {
        for (uint32_t i = 0; i < sz; i++) prob[i] = 0.0;
        rc = pcre2_match(_splitter_re, (PCRE2_SPTR)buf + start, sz - start, 0, 0, match_data, NULL);
        if (rc == PCRE2_ERROR_NOMATCH) {
            len = sz - start;
            skip = 0;
        }
        else if (rc > 0) {
            PCRE2_SIZE *ovector = pcre2_get_ovector_pointer(match_data);
            len = (uint32_t)ovector[0];
            skip = (uint32_t)ovector[1] - (uint32_t)ovector[0];
        }
        else {
            return 0.0;
        }
        //fprintf(stderr, "start at offset %d, len %d, skip %d\n", start, len, skip);
        //fprintf(stderr, "aa %c\n", (char)*(buf + start));
        if (len > 0) {
            if (len > 1000) len = 1000; //hardbreak
            ret += _char_ngram->get_seg_ids(buf + start, len, segs, prob, 0);
            for (uint32_t i = 0; i < len; i += segs[i]) {
                if (p > 0) {
                    segbuf[p] = '\1';
                    p ++;
                }
                //fprintf(stderr, "%d\t%d\t%d\n", i, p, segs[i]);
                memcpy(segbuf + p, buf + start + i, segs[i] * sizeof(wchar_t));
                p += segs[i];
            }
        }
        start += len + skip;
    } while (rc > 0);
    pcre2_match_data_free(match_data);


    segbuf[p] = '\0';
    delete [] segs;
    delete [] prob;
    return ret;
    */
}

float NGramDictManage::get_segs_ex(const char* buf, uint32_t sz, char**& psegbuf, int*& ptype, float*& pfreq, uint32_t& cnt)
{
    uint16_t wbuf[sz * 2];
    int len = convToucs2(buf, sz, wbuf, sz * 2, qsrch_code_utf8);
    //fprintf(stderr, "len: %d\n", len);
    int *segs = new int[len];
    float *prob = new float[len];
    int *type = new int[len];
    for (int i = 0; i < len; i++) prob[i] = 0.0;
    float ret = _char_ngram->get_seg_ids(wbuf, len, segs, prob, type, 0);
    cnt = 0;
    for (int i = 0; i < len; i += segs[i])
        cnt ++;
    psegbuf = new char*[cnt];
    ptype = new int[cnt];
    pfreq = new float[cnt];
    int n = 0;
    for (int i = 0; i < len; i += segs[i]) {
        psegbuf[n] = new char[segs[i] * 3 + 1];
        int olen = convFromucs2(wbuf + i, segs[i], psegbuf[n], segs[i] * 3 + 1, qsrch_code_utf8);
        psegbuf[n][olen] = '\0';
        ptype[n] = type[i];
        //pfreq[n] = prob[i];
        pfreq[n] = (i + segs[i]) < len ? prob[i] - prob[i + segs[i]] : prob[i];
        n ++;
    }
    delete [] segs;
    delete [] prob;
    delete [] type;
    return ret;
}

float NGramDictManage::get_segs(const char* buf, uint32_t sz, char* segbuf, uint32_t bufsz)
{
    uint16_t wbuf[sz * 2];
    int len = convToucs2(buf, sz, wbuf, sz * 2, qsrch_code_utf8);
    //fprintf(stderr, "len: %d\n", len);
    int *segs = new int[len];
    float *prob = new float[sz];
    for (uint32_t i = 0; i < sz; i++) prob[i] = 0.0;
    float ret = _char_ngram->get_seg_ids(wbuf, len, segs, prob, 0);
    int p = 0;
    for (int i = 0; i < len; i += segs[i]) {
        assert((int)bufsz > p);
        if (p > 0) {
            segbuf[p] = '\1';
            p ++;
        }
        int append = convFromucs2(wbuf + i, segs[i], segbuf + p, bufsz, qsrch_code_utf8);
        //printf("%d\t%d\n", segs[i], append);
        p += append;
    }
    segbuf[p] = '\0';
    delete [] segs;
    delete [] prob;
    return ret;
}

/*
float NGramDictManage::get_seg_ids(const wchar_t* pword, uint32_t sz, int *segs, float *prob)
{
    float ret = _char_ngram->get_seg_ids(pword, sz, segs, prob, 0);
    return ret;
}

float NGramDictManage::get_seg_ids(const uint16_t* pword, uint32_t sz, int *segs, float *prob)
{
    float ret = _char_ngram->get_seg_ids(pword, sz, segs, prob, 0);
    return ret;
}
*/


bool NGramDictManage::verify_core(const char* rawfile)
{
    int pstart[10]; int plen[10]; char buf[1024];
    printf("load raw file: %s for verifying ngram dict\n", rawfile);
    FILE *ifs = fopen(rawfile, "r");
    if (ifs == NULL) return false;
    int ln = 0;
    uint32_t w = 0, r = 0;
    while (fgets(buf, 1024, ifs))
    {   
        ln ++;
        if (ln % 1000000 == 0) {
            printf("processing line# %d, %.1f%%\n", ln, (double)ln / 9594435.690);
        }
        int n = strlen(buf);
        while (isspace(buf[n-1])) buf[--n] = 0;
        n = Relrank::split(buf, '\t', pstart, plen);

        if ((buf+pstart[2])[0] == 'p') {
            uint16_t wbuf[1024];
            int len;
            len = convToucs2(buf + pstart[1], plen[1], wbuf, 1024, qsrch_code_utf8);
            wbuf[len] = 0;
            long cnt = strtol(buf + pstart[3], NULL, 10);
            float logp = log(cnt);
            float value = _char_ngram->get_id_value(wbuf, len);
            if (logp != value) {
                printf("Faulty: %s %f should be %f\n", string(buf + pstart[1], plen[1]).c_str(), value, logp);
                w ++;
            }
            else {
                r ++;
            }
        }
    }   
    fclose(ifs);
    printf("W: %d, r: %d\n", w, r);

    return true;
}


bool NGramDictManage::build_core(const char* rawfile)
{
    int pstart[10]; int plen[10]; char buf[1024]; uint16_t wbuf[1024];
    printf("load raw file: %s for building core dict\n", rawfile);
    FILE *ifs = fopen(rawfile, "r");
    if (ifs == NULL) return false;
    int ln = 0;
    while (fgets(buf, 1024, ifs))
    {   
        ln ++;
        if (ln % 10000000 == 0) {
            printf("processing line# %d\n", ln);
        }
        int n = strlen(buf);
        while (isspace(buf[n-1])) buf[--n] = 0;
        n = Relrank::split(buf, '\t', pstart, plen);

        int len = convToucs2(buf + pstart[1], plen[1], wbuf, 1024, qsrch_code_utf8);
        wbuf[len] = 0;
        int ngram = strtol(buf + pstart[0], NULL, 10);
        if ((buf+pstart[2])[0] == 'c') {
            int child_size = strtol(buf + pstart[2] + 1, NULL, 10);
            _char_ngram->insert_core(wbuf, ngram, child_size);
        }
        else if ((buf+pstart[2])[0] == 'p') {
            long cnt = strtol(buf + pstart[3], NULL, 10);
            float logp = log(cnt);
            int child_size = strtol(buf + pstart[2] + 1, NULL, 10);
            _char_ngram->insert_core(wbuf, ngram, child_size, logp);
        }
        else if ((buf+pstart[2])[0] == 'd') {
            _char_ngram->insert_core(wbuf, ngram, 0);
        }
    }   
    fclose(ifs);

    return true;
}

bool NGramDictManage::dump_core(const char* file_prefix)
{
    return _char_ngram->dump_core(file_prefix);
}

bool NGramDictManage::load_char_core (const char* file_prefix, const char* dfa_file)
{
    if (_loaded)
        return true;
    _loaded = true;
    fprintf(stderr, "Loading %s, %s\n", file_prefix, dfa_file);
    return _char_ngram->load_core(file_prefix, dfa_file);
}
