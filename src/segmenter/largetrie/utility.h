#ifndef __RELRANK_UTILITY_HEADER__
#define __RELRANK_UTILITY_HEADER__
#include <string>
#include <vector>
#include <stdint.h>
#include <string.h>

using namespace std;

namespace Relrank 
{

int split(const string& line, const string& delims, vector<string>& tokens);
int split(const char* line, char c, int *pstart, int *plen);
int split(const char* line, const char* end, char c, int *pstart, int *plen);
int split(const uint16_t* line, const uint16_t* end, uint16_t c, int *pstart, int *plen);

}
#endif
