#include "atom_recognizer.h"
//#include "config.h"
#include "exception.h"
//#include "dict.h"
#include "type_const.h"
//#include "alnum_path.h"
#include <cstdio>
#include <iostream>

namespace qss {
namespace segmenter {

//#include "transcode.h"
//#include <iostream>

AtomRecognizer* AtomRecognizer::_instance = NULL;
AtomRecognizer::GarbageClear AtomRecognizer::garbage_clear;

AtomRecognizer::GarbageClear::~GarbageClear() {
  if (AtomRecognizer::_instance) {
    delete AtomRecognizer::_instance;
    AtomRecognizer::_instance = NULL;
  }
}

// singleton pattern
AtomRecognizer* AtomRecognizer::get_instance() {
  // need double lock check if there are multi-thread call
  if (_instance == NULL) {
    _instance = new AtomRecognizer();
  }
  return _instance;
}

AtomRecognizer::AtomRecognizer() {
  const char* dfa_file = "../data/dfa_file_qss.map";
  _max_wordlen = 32;
  //int max_sentence_len = pcfg->readint("segment", "max_sentence_len", 1024);
  //_match_buf = 0;
  //_malloc_buffer(max_sentence_len);
  try {
    _dfa = new Dfa;
    //_dfa->InitFromFile(dfa_file);
    _dfa->InitFromMmapFile(dfa_file);
  } catch (RegularExpressionException &e) {
    fprintf(stderr, "error load: %s\n", dfa_file);
  }
}

AtomRecognizer::~AtomRecognizer() {
  if (_dfa) {
    delete _dfa;
    _dfa = 0;
  }
  //if (_match_buf) {
  //  delete [] _match_buf;
  //}
}

/*
bool AtomRecognizer::_malloc_buffer(int size) {
  if (_match_buf_capacity >= size) return true;
  if (_match_buf)
    delete [] _match_buf;
  _match_buf = new qss::RegularMatchResult[size];
  _match_buf_capacity = size;
  return true;
}
*/

/*
float AtomRecognizer::AdjustProb(int pos_id, float pos_p) {
  float p = pos_p * 3.33;
  if (p < -16.0)    p = -16.0;
  if (p > -9.9)     p = -9.9;
  return p;
}
*/


int AtomRecognizer::matchAll(const uint16_t *psrc, int srclen) {
  std::vector<RegularMatchResult> match_result;
  //int word_num = _dfa->GreedyMatch(psrc, srclen, &match_result);
  int word_num = _dfa->LongestAnyMatch(psrc, srclen, &match_result);
  for (int i = 0; i < match_result.size(); i ++) {
      std::cout<<match_result[i].begin<<" "<<match_result[i].end<<" "<<match_result[i].type<<std::endl;
  }
  /*
  int count = FillMatchResult(match_result, pwords, pwordlen, lwords);
  word_num = _small_dfa->LongestAnyMatch(psrc, srclen, &match_result);
  count += FillMatchResult(match_result, pwords, pwordlen, lwords);
  return count;
  */
  return word_num;
}

/*
int AtomRecognizer::matchAll(const uint16_t *psrc, int srclen, word_t **pwords, short *pwordlen, word_t* lwords) {
  _malloc_buffer(srclen);
  int word_num = _dfa->LongestAnyMatch(psrc, srclen, _match_buf, _match_buf_capacity);
  int count = FillMatchResult(_match_buf, word_num, pwords, pwordlen, lwords);
  word_num = _small_dfa->LongestAnyMatch(psrc, srclen, _match_buf, _match_buf_capacity);
  count += FillMatchResult(_match_buf, word_num, pwords, pwordlen, lwords);
  return count;
}
*/

/*
int AtomRecognizer::FillMatchResult(std::vector<RegularMatchResult> &match_result, word_t **pwords, short *pwordlen, word_t* lwords) {
  int word_num =match_result.size();
  int count = 0;
  for (int i=0; i<word_num; ++i) {
    int word_len = match_result[i].end - match_result[i].begin;
    PosTag_t word_pos = ConvertTypeToPos(match_result[i].type);
    //float p = -18;
    //if (word_pos == POS_Q_SHU && psrc[match_result[i].begin] < 0xFF) p = -15;
    int pos_wordid = get_wordid_of_pos(word_pos);
    bool bdef = false;
    float p = Dict::get_instance()->get_cond_prob(pos_wordid, bdef);
    p = AdjustProb(pos_wordid, p);
    // split the too len word into short candidate
    if (word_len > _max_wordlen) {
      int offset = match_result[i].begin;
      int sub_word_len = _max_wordlen;
      while (offset < match_result[i].end) {
        int end = offset + sub_word_len;
        if (end >  match_result[i].end) {
          sub_word_len = match_result[i].end - offset;
          end = match_result[i].end;
        }
        if (pwordlen[offset] < sub_word_len) {
          pwordlen[offset] = sub_word_len;
        }
        pwords[offset][sub_word_len-1].set_online_word(sub_word_len, pos_wordid, word_pos, p, match_result[i].type, ol_src_const::atom_recognizer);
        // adjuster the ol_prob of eng and eng_number
        if ((match_result[i].type & k_type_alphabet) || (match_result[i].type & k_type_eng_number)) {
          AlnumPather::degradeWeight(&(pwords[offset][sub_word_len-1]));
        }
        if ((word_pos & POS_Q_SHU) == POS_Q_SHU) {
          if (lwords[offset].len <= pwords[offset][sub_word_len-1].len) {
            lwords[offset] = pwords[offset][sub_word_len-1]; //the pwords[offset][sub_word_len-1] has been updated
          }
        }
        ++count;
        offset = end;
      }
    }
    else {
      int offset = match_result[i].begin;
      if (pwordlen[offset] < word_len) {
        pwordlen[offset] = word_len;
      }
      pwords[offset][word_len-1].set_online_word(word_len, pos_wordid, word_pos, p, match_result[i].type, ol_src_const::atom_recognizer);
      // adjuster the ol_prob of eng and eng_number
      if ((match_result[i].type & k_type_alphabet) || (match_result[i].type & k_type_eng_number)) {
        AlnumPather::degradeWeight(&(pwords[offset][word_len-1]));
      }
    // insert all number word into lwords
      if ((word_pos & POS_Q_SHU) == POS_Q_SHU) {
        if (lwords[offset].len <= pwords[offset][word_len-1].len) {
          lwords[offset] = pwords[offset][word_len-1];
        }
      }
      ++count;
    }
  }
  return count;
}
*/

/*
int AtomRecognizer::FillMatchResult(const qss::RegularMatchResult *match_result, int word_num,
  	                     word_t **pwords, short *pwordlen, word_t* lwords) {
  //int word_num =match_result.size();
  int count = 0;
  for (int i=0; i<word_num; ++i) {
    int word_len = match_result[i].end - match_result[i].begin;
    PosTag_t word_pos = ConvertTypeToPos(match_result[i].type);
    //float p = -18;
    //if (word_pos == POS_Q_SHU && psrc[match_result[i].begin] < 0xFF) p = -15;
    int pos_wordid = get_wordid_of_pos(word_pos);
    bool bdef = false;
    float p = Dict::get_instance()->get_cond_prob(pos_wordid, bdef);
    p = AdjustProb(pos_wordid, p);
    // split the too len word into short candidate
    if (word_len > _max_wordlen) {
      int offset = match_result[i].begin;
      int sub_word_len = _max_wordlen;
      while (offset < match_result[i].end) {
        int end = offset + sub_word_len;
        if (end >  match_result[i].end) {
          sub_word_len = match_result[i].end - offset;
          end = match_result[i].end;
        }
        if (pwordlen[offset] < sub_word_len)
          pwordlen[offset] = sub_word_len;
        pwords[offset][sub_word_len-1].set_online_word(sub_word_len, pos_wordid, word_pos, p, match_result[i].type);
        if (word_pos & POS_Q_SHU) {
          if (lwords[offset].len <= pwords[offset][sub_word_len-1].len) {
            lwords[offset] = pwords[offset][sub_word_len-1]; //the pwords[offset][sub_word_len-1] has been updated
          }
        }
        ++count;
        offset = end;
      }
    }
    else {
      int offset = match_result[i].begin;
      if (pwordlen[offset] < word_len)
        pwordlen[offset] = word_len;
      pwords[offset][word_len-1].set_online_word(word_len, pos_wordid, word_pos, p, match_result[i].type);
    // insert all number word into lwords
      if (word_pos & POS_Q_SHU) {
        if (lwords[offset].len <= pwords[offset][word_len-1].len) {
          lwords[offset] = pwords[offset][word_len-1];
        }
      }
      ++count;
    }
  }
  return count;
}
*/

/*
// greedy match(only return the longest matched word) all possible 
// and insert all number word into lwords
int AtomRecognizer::matchAll(const uint16_t *psrc, int srclen, word_t **pwords, short *pwordlen, word_t* lwords) {
  std::vector<qss::RegularMatchResult> match_result;
  //int word_num = _dfa->GreedyMatch(psrc, srclen, &match_result);
  int word_num = _dfa->AnyMatch(psrc, srclen, &match_result);
  int count = 0;
  for (int i=0; i<word_num; ++i) {
    int word_len = match_result[i].end - match_result[i].begin;
    PosTag_t word_pos = ConvertTypeToPos(match_result[i].type);
    //float p = -18;
    //if (word_pos == POS_Q_SHU && psrc[match_result[i].begin] < 0xFF) p = -15;
    int pos_wordid = get_wordid_of_pos(word_pos);
    bool bdef = false;
    float p = Dict::get_instance()->get_cond_prob(pos_wordid, bdef);
    p = AdjustProb(pos_wordid, p);
    //if (word_pos == POS_Q_SHU && psrc[match_result[i].begin] < 0xFF) p = -9;

//    if (word_len > 1 && word_pos == POS_Q_SHU && psrc[match_result[i].begin] < 0xFF && psrc[match_result[i].end-1] > 0xFF)
//        p -= 10;
    // split the too len word into short candidate
    if (word_len > _max_wordlen) {
      int offset = match_result[i].begin;
      int sub_word_len = _max_wordlen;
      while (offset < match_result[i].end) {
        int end = offset + sub_word_len;
        if (end >  match_result[i].end) {
          sub_word_len = match_result[i].end - offset;
          end = match_result[i].end;
        }
        if (pwordlen[offset] < sub_word_len)
          pwordlen[offset] = sub_word_len;
        pwords[offset][sub_word_len-1].set_online_word(sub_word_len, pos_wordid, word_pos, p, match_result[i].type);
        if (word_pos & POS_Q_SHU) {
          if (lwords[offset].len <= pwords[offset][sub_word_len-1].len) {
            lwords[offset] = pwords[offset][sub_word_len-1]; //the pwords[offset][sub_word_len-1] has been updated
          }
        }
        ++count;
        offset = end;
      }
    }
    else {
      int offset = match_result[i].begin;
      if (pwordlen[offset] < word_len)
        pwordlen[offset] = word_len;
      pwords[offset][word_len-1].set_online_word(word_len, pos_wordid, word_pos, p, match_result[i].type);
    // insert all number word into lwords
      if (word_pos & POS_Q_SHU) {
        if (lwords[offset].len <= pwords[offset][word_len-1].len) {
          lwords[offset] = pwords[offset][word_len-1];
        }
      }
    ///////////for debug
    //char output_buf[1024];
    //int out_len = convFromucs2(const_cast<uint16_t *>(psrc+match_result[i].begin), word_len, output_buf, 1024, qsrch_code_utf8);
    //output_buf[out_len] = 0;
    //std::cout << output_buf << "\tType:" << match_result[i].type << std::endl;
    ///////////end for debug
      ++count;
    }
  }
  return count;
}
*/

/*
PosTag_t AtomRecognizer::ConvertTypeToPos(int type) {

  if (type & k_type_alphabet) {
    return POS_S_STR;
  }
  else if (type & k_type_digit_number) {
    return POS_Q_SHU;
  }
  else if (type & k_type_eng_number) {
    return POS_S_MIX;
  }
  else if (type & k_type_punctuation) {
    return POS_S_PUNC;
  }
  else if (type & k_type_date) {
    return POS_Q_TIME;
  }
  else if (type & k_type_time) {
    return POS_Q_TIME;
  }
  else if (type & k_type_telephone) {
    return POS_S_STR;
  }
  else if (type & k_type_email) {
    return POS_S_STR;
  }
  else if (type & k_type_ip) {
    return POS_S_STR;
  }
  else if (type & k_type_chinese_numeric_str) {
    return POS_S_STR;
  }
  else if (type & k_type_special_word) {
    return POS_N;
  }
  else if (type & k_type_chinese_number) {
    return POS_Q_SHU;
  }
  else if (type & k_type_version_number) {
    return POS_S_STR;
  }
  else if (type & k_type_percent) {
    return POS_Q_SHU;
  }
  else if (type & k_type_non_english) {
    return POS_S_STR;
  }
  else if (type & k_type_non_eng_number) {
    return POS_S_MIX;
  }
  
  return POS_S_STR;
}
*/
}
}

