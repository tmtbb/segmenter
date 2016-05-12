#ifndef __QSS_SEGMENTER_SEG_BASE_H__
#define __QSS_SEGMENTER_SEG_BASE_H__

#include <stdint.h>
#include <vector>
#include <string>
#include "config.h"
#include "pos_const.h"
#include "reserved_word.h"
#include "transcode.h"
#include "word_t.h"

namespace qss {
namespace segmenter {

#define ONLINE_WORD_ID      0x80000000
#define WORD_GRAN_LONG      1
#define WORD_GRAN_SHORT     2
#define WORD_GRAN_COMPOUND  3

using std::basic_string;

extern int  g_verbose;
typedef     basic_string<uint16_t> u16string;
int         getWords        (const uint16_t *psrc, int srclen, int *pword, int *ptype, std::vector<u16string > &words);
int         get_unicode_buf (Config *pcfg, const char* sect, const char* key, uint16_t* &param);
uint16_t    get_unicode_char(Config *pcfg, const char* sect, const char* key);
bool        is_reserved_word(int wordid);
//int         insert_word     (word_t *buf, int depth, const word_t &word);
void        debug_info      (const uint16_t *wbuf, int len, word_t **matrix, short* pwordlen, int rows);
void        debug_result    (const uint16_t *wbuf, int len, word_t *words);
bool        is_string       (const uint16_t *psrc, int srclen);
// convert a Hex string to be a uint_16 number, len is no more than 4
uint16_t HexStrToUINT16(const char *str, int len);
}
}
#endif
