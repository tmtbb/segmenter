#include "utility.h"

namespace Relrank
{

int split(const string& line, const string& delims, vector<string>& tokens)
{
    string::size_type begin, end;
    begin = line.find_first_not_of(delims);
    int n       = 0;
    while (begin != string::npos)
    {   
        end     = line.find_first_of(delims, begin);
        if (end == string::npos) 
            end = line.length();

        n ++;
        tokens.push_back(line.substr(begin, (end-begin)));
        begin   = line.find_first_not_of(delims, end);
    }   

    return n;
}


int split(const char* line, const char* end, char c, int *pstart, int *plen)
{
    int i = 0;
    const char *pl = line;
    const char *pr = (const char*)memchr(pl, c, end - pl);
    while (pl != end)
    {
        if (pr == NULL) 
        {
            pr = end;
            pstart[i] = pl - line;
            plen  [i] = pr - pl;
            i ++;
            break;
        }
        else// if (pr > pl)
        {
            pstart[i] = pl - line;
            plen  [i] = pr - pl;
            i ++;
        }
        pl = pr + 1;
        pr = (const char*)memchr(pl, c, end - pl);
    }

    return i;
}

int split(const uint16_t* line, const uint16_t* end, uint16_t c, int *pstart, int *plen)
{
    int i = 0;
    int s = 0, e = 0;

    while((line + e) < end) {
        if(*(line + e) == c) {
            pstart[i] = s;
            plen  [i] = e - s;
            s = e + 1;
            i++;
        }
        e++;
    }
    return i;
}

int split(const char* line, char c, int *pstart, int *plen)
{
    int i = 0;
    const char *pl = line;
    const char *pr = strchr(pl, c);
    while (pl != '\0')
    {
        if (pr == NULL) 
        {
            pr = line + strlen(line);
            pstart[i] = pl - line;
            plen  [i] = pr - pl;
            i ++;
            break;
        }
        else// if (pr > pl)
        {
            pstart[i] = pl - line;
            plen  [i] = pr - pl;
            i ++;
        }
        pl = pr + 1;
        pr = strchr(pl, c);
    }

    return i;
}

}
