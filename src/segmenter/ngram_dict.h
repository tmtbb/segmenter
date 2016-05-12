#ifndef __RELRANK_NGRAM_DICT_TEMPLATE_HEADER__
#define __RELRANK_NGRAM_DICT_TEMPLATE_HEADER__

#include "largetrie/index_map.h"
#include "largetrie/trie_utility.h"
#include "dfa.h"
#include <math.h>
#include <map>
#include <string.h>

#define UNIGRAM_DEFAULT_ID  0x0001
#define BIGRAM_DEFAULT_ID   0x0002
#define TRIGRAM_DEFAULT_ID  0x0003

#define COMPILE_MODE_NONE       0
#define COMPILE_MODE_KATZ       1
#define COMPILE_MODE_SIMPLE     2

#define TOTAL_LOG_P         29.0
#define STOPWORD_LOG_P         27.0

//#define DICT_FORMAT_DATRIE      1
#define DICT_FORMAT_BUFFERTRIE  2

#define DEFAULT_SENT_BEG        '^'
#define DEFAULT_SENT_END        '$'


//using Relrank::TrieNode;
//using Relrank::TrieMap;
//using Relrank::IndexMap;

namespace Relrank
{

template <class _TP, class _VAL = float, int _NVAL = 1>
class NGramDict
{

    public:
        NGramDict()
        {
            _dict      = new Relrank::BufferTrie<_TP>;
            _dfa       = new qss::segmenter::Dfa;
            memset(_stopwords, 0, sizeof(float) * 65536);
            wchar_t *sws = L" \x3000()（）《》[]【】\"\"“”、,，。:：;；?？!！的是";
            for (size_t i = 0; i < wcslen(sws); i ++) {
                //printf("%d\n", sws[i]);
                _stopwords[sws[i]] = STOPWORD_LOG_P;
            }
        }

        ~NGramDict()
        {
            delete  _dict;
            delete  _dfa;
        }

        bool insert_core(const _TP *pword, int len, int child_size, float value)
        {
            _dict->insert_sorted(pword, len, child_size, value);
            return true;
        }

        bool insert_core(const _TP *pword, int len, int child_size)
        {
            _dict->insert_sorted(pword, len, child_size);
            return true;
        }

        bool dump_core(const char* dict_file)
        {
            char buf[64];
            sprintf(buf, "%s.core", dict_file);
            _dict->save(buf);
            return true;
        }

        bool load_core(const char* dict_file, const char* dfa_file, bool bReadOnly = true)
        {
            char buf[4096];

            delete _dict;
            _dict = NULL;

            sprintf(buf, "%s", dict_file);
            _dict = Relrank::TrieUtility<_TP>::dynamic_load(buf);
            if (_dict == NULL) return false;

            _dfa->InitFromMmapFile(dfa_file);

            return true;
        }

        float get_id_value(const _TP *pword, int ngram)
        {
            return _dict->nget(pword, ngram);
        }

        float length_weight(const wchar_t* pword, uint32_t start, uint32_t len)
        {
            /*
            float lenprob[] = {0.0, 
                -6.02707,
                -3.85838,
                -1.8,
                0,
                -1.8,
                -3.85838,
                -6.02707,
                -8.27063,
                -10.5716,
                -12.9193,
                -15.3067,
                -17.7285,
                -20.1808,
                -22.6607,
                -25.1654,
                -27.6931,
                -30.242,
                -32.8105,
                -35.3975,
                -38.0018,
                -40.6225,
                -43.2586,
                -45.9094,
                -48.5742
            };
            */
            uint32_t rlen = 0;
            for (uint32_t i = 0; i < len; i ++) {
                if (pword[start + i] < 128)
                    rlen ++;
                else
                    rlen += 3;
            }
            /*
            uint32_t lenscore = abs((int)rlen - 3) + 1;
            return sqrt(lenscore);
            */
            //printf("len: %d\t%f\n", rlen, lenprob[rlen]);
            //return -TOTAL_LOG_P + lenprob[rlen];
            return -TOTAL_LOG_P;
            //return (abs((int)rlen - 4) * -2.0) - TOTAL_LOG_P;
        }

        float length_weight(const _TP* pword, uint32_t start, uint32_t len)
        {
            /*
            float lenprob[] = {0.0, 
                -6.02707,
                -3.85838,
                -1.8,
                0,
                -1.8,
                -3.85838,
                -6.02707,
                -8.27063,
                -10.5716,
                -12.9193,
                -15.3067,
                -17.7285,
                -20.1808,
                -22.6607,
                -25.1654,
                -27.6931,
                -30.242,
                -32.8105,
                -35.3975,
                -38.0018,
                -40.6225,
                -43.2586,
                -45.9094,
                -48.5742
            };
            */
            /*
            uint32_t rlen = 0;
            for (uint32_t i = 0; i < len; i ++) {
                if (pword[start + i] < 128)
                    rlen ++;
                else
                    rlen += 3;
            }
            */
            /*
            uint32_t lenscore = abs((int)rlen - 3) + 1;
            return sqrt(lenscore);
            */
            //printf("len: %d\t%f\n", rlen, lenprob[rlen]);
            //return -TOTAL_LOG_P + lenprob[rlen];
            return -TOTAL_LOG_P;
            //return (abs((int)rlen - 4) * -2.0) - TOTAL_LOG_P;
        }

        float get_seg_ids(const wchar_t* pword, uint32_t sz, int *segs, float *prob, uint32_t start)
        {
            return get_seg_ids(pword, sz, segs, prob, NULL, start);
        }

        float get_seg_ids(const wchar_t* pword, uint32_t sz, int *segs, float *prob, int *type, uint32_t start)
        {
            //fprintf(stderr, "multi_nmatch: %d, %d\n", start, (int)(sz - start));
            //fwprintf(stderr, L"============ %s\t%d\n", pword, sz);
            if (start == sz)
                return 0.0;
            if (prob[start] != 0.0)
                return prob[start];
            float *pvals = new float[100];
            int *plens = new int[100];
            int cnt = _dict->multi_nmatch(pword + start, (int)(sz - start), pvals, plens, 100);
            int cnt2 = _dfa->MatchInMmapPattern(pword + start, (int)(sz - start), pvals + cnt, plens + cnt, 100 - cnt);
            //fprintf(stderr, "sz: %d, start: %d, cnt: %d, cnt2: %d\n", sz, start, cnt, cnt2);
            if (cnt + cnt2 <= 0) {
                pvals[0] = _stopwords[pword[start]] - TOTAL_LOG_P;
                plens[0] = 1;
                cnt = 1;
            }
            float max = 0.0;
            for (int i = 0; i < cnt + cnt2; i ++) {
                //assert(pvals[i] < 0.0);
                if (plens[i] == 1 && _stopwords[pword[start]] > 0.0)
                    pvals[i] = _stopwords[pword[start]];
                float p = pvals[i];
                //p -= TOTAL_LOG_P;
                p += length_weight(pword, start, plens[i]);
                //fprintf(stderr, "matched: %d out of %d\t%f\t%f\t%d\t%d\t%f\n", start, sz, pvals[i], p, plens[i], pword[start], _stopwords[pword[start]]);
                float rightside = get_seg_ids(pword, sz, segs, prob, type, start + plens[i]);
                //assert(p < 0.0);
                rightside += p;
                if (max == 0.0 || rightside > max) {
                    max = rightside;
                    segs[start] = plens[i];
                    if (type) {
                        type[start] = 0;
                        if (i < cnt) {
                            type[start] |= 1;
                        }
                        else {
                            for (int j = 0; j < cnt; j ++) {
                                if (plens[i] == plens[j])
                                    type[start] |= 1;
                            }
                            type[start] |= 2;
                        }
                    }
                }
                //fprintf(stderr, "matched: %f\t%d\t%f\t%d\n", rightside, plens[i], max, segs[0]);
            }
            delete [] pvals;
            delete [] plens;
            prob[start] = max;
            return max;
        }

        float get_seg_ids(const _TP* pword, uint32_t sz, int *segs, float *prob, uint32_t start)
        {
            return get_seg_ids(pword, sz, segs, prob, NULL, start);
        }

        float get_seg_ids(const _TP* pword, uint32_t sz, int *segs, float *prob, int *type, uint32_t start)
        {
            if (start == sz)
                return 0.0;
            if (prob[start] != 0.0)
                return prob[start];
            float pvals[100];
            int plens[100];
            int cnt = _dict->multi_nmatch(pword + start, (int)(sz - start), pvals, plens, 100);
            int cnt2 = _dfa->MatchInMmapPattern(pword + start, (int)(sz - start), pvals + cnt, plens + cnt, 100 - cnt);
            if (cnt + cnt2 <= 0) {
                pvals[0] = _stopwords[pword[start]] - TOTAL_LOG_P;
                plens[0] = 1;
                cnt = 1;
            }
            float max = 0.0;
            for (int i = 0; i < cnt + cnt2; i ++) {
                //assert(pvals[i] < 0.0);
                if (plens[i] == 1 && _stopwords[pword[start]] > 0.0)
                    pvals[i] = _stopwords[pword[start]];
                float p = pvals[i];
                //p -= TOTAL_LOG_P;
                p += length_weight(pword, start, plens[i]);
                float rightside = get_seg_ids(pword, sz, segs, prob, type, start + plens[i]);
                //assert(p < 0.0);
                rightside += p;
                if (max == 0.0 || rightside > max) {
                    max = rightside;
                    segs[start] = plens[i];
                    if (type) {
                        type[start] = 0;
                        if (i < cnt) {
                            type[start] |= 1;
                        }
                        else {
                            for (int j = 0; j < cnt; j ++) {
                                if (plens[i] == plens[j])
                                    type[start] |= 1;
                            }
                            type[start] |= 2;
                        }
                    }
                }
                //printf("matched: %f\t%d\t%f\t%d\n", rightside, plens[i], max, segs[0]);
            }
            prob[start] = max;
            return max;
        }

        bool get_sub_ids(const _TP* pword, uint32_t sz, float pvals[][QSS_MAX_ELEM_COUNT], int plens[][QSS_MAX_ELEM_COUNT])
        {
            for (uint32_t i = 0; i < sz; i ++) {
                //int cnt = _dict->multi_nmatch(pword + i, (int)sz - i, pids[i], plens[i], QSS_MAX_ELEM_COUNT);
                int cnt = _dict->multi_nmatch(pword + i, (int)sz - i, pvals[i], plens[i], QSS_MAX_ELEM_COUNT);
                printf("found: %d\n", i);
                //pids[i][cnt] = 0;
                pvals[i][cnt] = 0;
                plens[i][cnt] = 0;
            }
            return true;
        }

    private:
        float               _stopwords[65536];

        Relrank::TrieMap<_TP>*       _dict;
        qss::segmenter::Dfa* _dfa;

    private:

};

}

#endif
