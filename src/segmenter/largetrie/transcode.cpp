#include "transcode.h"

#define DEFAULT_CHAR    0xEEEE
size_t utf16ToUtf8(const uint16_t* pu16, size_t ilen, char* pu8, size_t olen);
size_t utf8ToUtf16(const char* pu8, size_t ilen, uint16_t* pu16, size_t olen);

int convToucs2(const char *__psrc, int __nsrc, uint16_t *__pdst, int __ndst, qsrch_code_t __code)
{
    if (__code == qsrch_code_utf8)
    {
        return utf8ToUtf16(__psrc, __nsrc, __pdst, __ndst);
    }
    else
    {
        return 0;
    }
}

int convFromucs2(const uint16_t *__psrc, int __nsrc, char *__pdst, int __ndst, qsrch_code_t __code)
{
    if (__code == qsrch_code_utf8)
    {
        return utf16ToUtf8(__psrc, __nsrc, __pdst, __ndst);
    }
    else
    {
        return 0;
    }
}

size_t utf16ToUtf8(const uint16_t* pu16, char* pu8)
{
  size_t n = 0;
  if (pu16[0] <= 0x007F)
  {
    pu8[0] = (pu16[0] & 0x7F);
    n = 1;
  }
  else if (pu16[0] >= 0x0080 &&  pu16[0] <= 0x07FF)
  {
    pu8[1] = (0x80 | (pu16[0] & 0x003F));
    pu8[0] = (0xC0 | ((pu16[0] >> 6) & 0x001F));
    n = 2;
  }
  else if (pu16[0] >= 0x0800)
  {
    pu8[2] = (0x80 | (pu16[0] & 0x003F));
    pu8[1] = (0x80 | ((pu16[0] >> 6) & 0x003F));
    pu8[0] = (0xE0 | ((pu16[0] >> 12) & 0x000F));
    n = 3;
  }

  return n;
}

size_t utf16ToUtf8(const uint16_t* pu16, size_t ilen, char* pu8, size_t olen)
{
  int offset = 0;
  size_t sz = 0;
  for (size_t i = 0; i < ilen && offset < static_cast<int>(olen) - 3; i++) {
    sz = utf16ToUtf8(pu16 + i, pu8 + offset);
    offset += sz;
  }
  
  pu8[offset] = '\0';
  return offset;
}

size_t utf8ToUtf16(const char* pu8, uint16_t* pu16)
{
  size_t n = 0;
  if ((pu8[0] & 0xF0) == 0xE0)
  {
    if ((pu8[1] & 0xC0) == 0x80 &&
        (pu8[2] & 0xC0) == 0x80)
    {
      pu16[0] = (((pu8[0] & 0x0F) << 4) | ((pu8[1] & 0x3C) >> 2));
      pu16[0] <<= 8;
      pu16[0] |= (((pu8[1] & 0x03) << 6) | (pu8[2] & 0x3F));
    }
    else
    {
      pu16[0] = DEFAULT_CHAR;
    }
    n = 3;
  } 
  else if ((pu8[0] & 0xE0) == 0xC0)
  {
    if( (pu8[1] & 0xC0) == 0x80) 
    {
      pu16[0] = ((pu8[0] & 0x1C) >> 2);
      pu16[0] <<= 8;
      pu16[0] |= (((pu8[0] & 0x03) << 6) | (pu8[1] & 0x3F));
    }
    else
    {
      pu16[0] = DEFAULT_CHAR;
    }
    n = 2;
  } 
  else if ((pu8[0] & 0x80) == 0x00) 
  {
    pu16[0] = pu8[0];
    n = 1;
  }

  return n;
}

size_t utf8ToUtf16(const char* pu8, size_t ilen, uint16_t* pu16, size_t olen)
{
  int offset = 0;
  size_t sz = 0;
  for (size_t i = 0; i < ilen && offset < static_cast<int>(olen); offset ++)
  {
    sz = utf8ToUtf16(pu8 + i, pu16 + offset);
    i += sz;
    if (sz == 0) {
      // failed
      // assert(sz != 0);
      break;
    }
  }
//  pu16[offset] = '\0';

  return offset;
}

