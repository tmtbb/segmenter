#ifndef __QSS_SEGMENTER_ALNUM_PATH_H__
#define __QSS_SEGMENTER_ALNUM_PATH_H__

#include "segbase.h"

namespace qss {
namespace segmenter {

class Dict;
class AlnumPather
{
 public:
    AlnumPather();
    ~AlnumPather();

    bool clear(const uint16_t * psrc, int srclen, word_t *lwords, word_t** pmatrix, short* pwordlen,  word_t *swords);
	bool combine(const uint16_t * psrc, int srclen, word_t *lwords, word_t** pmatrix, short* pwordlen, word_t *swords);

    static  int degradeWeight(word_t* pw);
 private:
	bool _isNumber(uint16_t c);
	bool _isNumberWord(const uint16_t* psrc , int srclen);
        bool _isAlnumWord(const uint16_t* psrc , int srclen);
    bool _isAlpha(uint16_t c);
	bool _isDot(uint16_t c);
	int _max_word_len;
        bool  _is_clear;
};
}
}
#endif 
