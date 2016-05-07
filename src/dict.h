#ifndef	__QSS_SEGMENTER_DICT_H__
#define	__QSS_SEGMENTER_DICT_H__

#include "segbase.h"
#include "index_map.h"
#include "index_multimap.h"
#include "datrie.h"
#include "ngram_dict.h"

namespace qss {
namespace segmenter {

#define SMOOTH_SIMPLE   1
#define SMOOTH_KATZ     2
#define SMOOTH_POS      3

//unicode dictionary
class	Dict
{
    public:
        static Dict* get_instance();

        int     matchAll        (const uint16_t *psrc, int srclen, word_t **pwords, short* pwordlen, word_t* lwords, int wordlen = 0);
        int     find            (const uint16_t *psrc, int srclen, word_t &word_t);
        int     matchLongest    (const uint16_t *psrc, int srclen, word_t &word_t);
        int     findAmbiguity   (const uint16_t *psrc, int srclen);
        bool    word_has_pos    (int id, uint32_t pos);

        float   get_cond_prob   (word_t &w_1, word_t &w_0, bool &bdef);

        float   get_cond_prob   (int id_2, int id_1, int id_0, bool &bdef);
        float   get_cond_prob   (int id_1, int id_0, bool &bdef);
        float   get_cond_prob   (int id_0, bool &bdef);

    protected:
        Dict();
        ~Dict();

        xbb::TrieMap<uint16_t>*      _core;
        xbb::IndexMap<info_t>*       _info;
        xbb::IndexMultiMap<pos_t>*   _post;
        NGramDict<uint32_t>*    _ngram;

        int                     _max_wordlen;
        int                     _smoothing_model;
        int                     _disambi_algo;

		float *_prob_coefficient; //used to adjust the  word prob

        float   _pos_smoothing_prob   (word_t &w_1, word_t &w_0, bool &bdef);
        float   _katz_smoothing_prob  (word_t &w_1, word_t &w_0);
        float   _simple_smoothing_prob(word_t &w_1, word_t &w_0);
        float   _pos_prob             (word_t &w_1, word_t &w_0);
        void    _fill_word            (word_t &word, int len, int id);

        static Dict *_instance;

        class GarbageClear {
            public:
                ~GarbageClear();
        };
        static GarbageClear garbage_clear;
};
}
}
#endif
