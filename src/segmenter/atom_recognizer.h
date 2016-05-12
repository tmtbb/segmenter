#ifndef __QSS_SEGMENTER_ATOM_RECOGNIZER_H__
#define __QSS_SEGMENTER_ATOM_RECOGNIZER_H__

//#include "segbase.h"
#include "dfa.h"
#include "regular_expression_types.h"
#include <vector>

namespace qss {
namespace segmenter {

class AtomRecognizer {
 public:
  static AtomRecognizer* get_instance();
  //int matchAll(const uint16_t *psrc, int srclen, word_t **pwords, short *pwordlen, word_t* lwords);
  int matchAll(const uint16_t *psrc, int srclen);

 private:
  AtomRecognizer();
  virtual ~AtomRecognizer();
  //bool _malloc_buffer(int size);
  //int FillMatchResult(std::vector<RegularMatchResult> &match_result, word_t **pwords, short *pwordlen, word_t* lwords);
  //int FillMatchResult(const qss::RegularMatchResult *match_result, int match_result_count,
  //	                     word_t **pwords, short *pwordlen, word_t* lwords);
  // convert the type in DFA match result to be the Pos(defined in postag.h)
  //static PosTag_t ConvertTypeToPos(int type);
  //static float AdjustProb(int pos_id, float pos_p);
  int _max_wordlen;
  Dfa* _dfa;
  //qss::RegularMatchResult *_match_buf;
  //int _match_buf_capacity;
  static AtomRecognizer *_instance;
  // delete _instance while the program exit
  class GarbageClear {
   public:
   	~GarbageClear();
  };
  static GarbageClear garbage_clear;
};
}
}
#endif

