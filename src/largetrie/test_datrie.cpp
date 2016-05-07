#include <string.h>
#include "utility.h"
#include "trie_utility.h"
#include "trie_map.h"
#include "datrie.h"
#include "bufferTrie.h"
#include "transcode.h"

void usage()
{
    printf("test -b|-t dictfile inputfile \n");
    printf("-b for build dict\n");
    printf("-t for test\n");
    printf("-s for special test\n");
    printf("input file format:\n");
    printf("word\tid\n");

    exit(1);
}

int main(int argc, char**argv)
{
    if (argc < 4)
    {
        usage();
    }

    if (strcmp(argv[1], "-b") == 0)
    {
        FILE *ifs = fopen(argv[3], "r");
        char        buf[1024]; uint16_t    wbuf[1024];
        int pstart[10]; int plen[10];

        printf("start build trie\n");
        xbb::daTrie<uint16_t> wdatrie(8);
        while (fgets(buf, 1024, ifs))
        {
            int n = strlen(buf);
            while (isspace(buf[n-1])) buf[--n] = 0;

            n = split(buf, '\t', pstart, plen);
            if (n != 2) { printf("err: %s\n", buf); continue; }

            int len = convToucs2(buf+pstart[0], plen[0], wbuf, 1024, qsrch_code_utf8);
            wbuf[len] = 0;
            int nid = atoi(buf+pstart[1]);
            //fprintf(stderr, "%s: %d\n", buf, nid);

            wdatrie.insert(wbuf, len, nid);
        }
        fclose(ifs);
        printf("finished build trie\n");

        printf("start build double array trie\n");
        //wdatrie.set_convType(xbb::__g_daTrie_conv_useTab);
        wdatrie.build();
        printf("finished build double array trie\n");
        sprintf(buf, "%s.dict", argv[2]);
        wdatrie.save(buf);
    }
    else if (strcmp(argv[1], "-t") == 0)
    {
        char        buf[1024];
        uint16_t    wbuf[1024];
        sprintf(buf, "%s.dict", argv[2]);
        xbb::TrieMap<uint16_t>* triemap = xbb::TrieUtility<uint16_t>::dynamic_load(buf);

        if (triemap == NULL) {
            fprintf(stderr, "error load triemap\n");
            return -1;
        }
        //printf("hole rate: %f\n", wdatrie.hole_rate());


        int pstart[10]; int plen[10];

        FILE *ifs = fopen(argv[3], "r");
        while (fgets(buf, 1024, ifs))
        {
            int n = strlen(buf);
            while (isspace(buf[n-1])) buf[--n] = 0;

            n = split(buf, '\t', pstart, plen);
            int len = convToucs2(buf+pstart[0], plen[0], wbuf, 1024, qsrch_code_utf8);
            wbuf[len] = 0;
            int nid = atoi(buf+pstart[1]);
            int oldid = triemap->find(wbuf);
            if (nid != oldid) fprintf(stderr, "Fail!  %s: %d\t%d\n", buf, nid, oldid);
            //else              fprintf(stdout, "OK!  %s: %d\t%d\n", buf, nid, oldid);
        }
    }
    else if (strcmp(argv[1], "-s") == 0)
    {
        char        buf[1024];
        sprintf(buf, "%s.dict", argv[2]);
        xbb::TrieMap<uint16_t>* triemap = xbb::TrieUtility<uint16_t>::dynamic_load(buf);

        if (triemap == NULL) {
            fprintf(stderr, "error load triemap\n");
            return -1;
        }

        uint16_t    wbuf[64], suffix[64];   int pids[64];
        wbuf[0] = 'a';  wbuf[1] = 'b';  wbuf[2] = 'c';
        suffix[0]   = 'd';  suffix[1]   = 'y';  suffix[2]   = 'e';  suffix[3]   = 'z';  suffix[4]   = 'f'; suffix[5]    = 'x';
        int n   = triemap->suffix_nmatch(wbuf, 3, suffix, 6, pids);
        for (int i = 0; i < 6; i ++)
        {
            printf("%c: %d\n", suffix[i], pids[i]);
        }
    }
    else
    {
        usage();
    }

}
