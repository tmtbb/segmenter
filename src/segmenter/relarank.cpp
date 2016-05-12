#include "ngram_dict_manage.h"

int main(int argc, char* argv[]) {
    NGramDictManage dict(2, 5);
    xbb::TrieMap<uint16_t>* piddict = xbb::TrieUtility<uint16_t>::dynamic_load(argv[2]);

}
