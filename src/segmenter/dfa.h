#ifndef __QSS_SEGMENTER_DFA_H__
#define __QSS_SEGMENTER_DFA_H__
#include <set>
#include <vector>
#include "nfa.h"

namespace qss {
namespace segmenter {


// store one NFA node 
struct DfaNode {
  INT32 node_id;
  // -1: original node;  >=1:final node
  // (the number is the ID of final variable in regular expression)
  INT32 type;
  //the indexes of nodes could be arrived from this node by one edge
  std::map<UINT16 /*symbol*/,INT32 /*succeed node id*/> succeed_nodes;
};

// store one DFA machine, the nodes' instances are stored in Dfa::nodes_
// the first node is the start node
// there are one or more final node for each NfaTable
//typedef std::deque<DfaNode *> DfaTable;

class Dfa {
 public:
  Dfa();
  ~Dfa();
  // clear the member variables
  void Clear();
  // init from one Nfa
  void InitFromNfa(const Nfa &nfa);
  // init from serialization file of dfa
  void InitFromFile(const char *dfa_file);
  // store the content of dfa to a file
  void Serialization(const char *dfa_file);  
  // store the content of dfa to a file(the file will be used for mmap load)
  void StoreToMmap(const char *dfa_file);
  // init from mmap file of dfa
  void InitFromMmapFile(const char *dfa_file);
  // max length match, return the match positon and the type matched
  // @param buffer UINT16 buffer to be scaned
  // @param len length of UINT16 buffer
  // @param result memory should be allocated outside this function, store the match result
  // Return: length of result
  // it is thread-safe to call this method of one instance in multiple thread
  // if the instance don't be changed at the same time
  INT32 GreedyMatch(const UINT16 *buffer, INT32 len,
                         std::vector<RegularMatchResult> *result) const;
  // for the sam type, longest match, if diff type output the pre-matched result
  INT32 LongestAnyMatch(const UINT16 *buffer, INT32 len,
                         std::vector<RegularMatchResult> *result) const;
  // for the sam type, longest match, if diff type output the pre-matched result
  INT32 LongestAnyMatch(const UINT16 *buffer, INT32 len,
                         RegularMatchResult *result_buf, INT32 buf_len) const;
  // all possible match.
  INT32 AnyMatch(const UINT16 *buffer, INT32 len,
                         std::vector<RegularMatchResult> *result) const;
  INT32 MatchInMmapPattern(const UINT16 *buffer, INT32 len, float* pvals, int* plens, int bufsize) const;
  INT32 MatchInMmapPattern(const wchar_t *buffer, INT32 len, float* pvals, int* plens, int bufsize) const;

 private:
  float GetScore(INT32 type, int len) const;
  // if the instance don't be changed at the same time, use map_
  INT32 GreedyMatchInMmapPattern(const UINT16 *buffer, INT32 len,
                         std::vector<RegularMatchResult> *result) const;
  // all possible match. use map_
  INT32 LongestAnyMatchInMmapPattern(const UINT16 *buffer, INT32 len,
                         std::vector<RegularMatchResult> *result) const;
  // all possible match. use map_
  INT32 LongestAnyMatchInMmapPattern(const UINT16 *buffer, INT32 len,
                         RegularMatchResult *result_buf, INT32 buf_len) const;
  // if the instance don't be changed at the same time, use nodes_
  INT32 GreedyMatchInStdPattern(const UINT16 *buffer, INT32 len,
                         std::vector<RegularMatchResult> *result) const;
  // all possible match. use nodes_
  INT32 LongestAnyMatchInStdPattern(const UINT16 *buffer, INT32 len,
                         std::vector<RegularMatchResult> *result) const;
  // all possible match. use nodes_
  INT32 LongestAnyMatchInStdPattern(const UINT16 *buffer, INT32 len,
                         RegularMatchResult *result_buf, INT32 buf_len) const;
  // all possible match. use map_
  INT32 AnyMatchInMmapPattern(const UINT16 *buffer, INT32 len,
                         std::vector<RegularMatchResult> *result) const;
  // all possible match. use nodes_
  INT32 AnyMatchInStdPattern(const UINT16 *buffer, INT32 len,
                         std::vector<RegularMatchResult> *result) const;
  // get the epsilon closure of the input NFA set; expand the input set as result
  // @param nfa_set the IDs of the set nodes. memory should be allocated outside this function
  // Return: length of the nfa_nodes after calculate
  INT32 EpsilonClosure(const Nfa &nfa, std::set<INT32> *nfa_set);
  // @param nfa the nfa instance
  // @param start_set the IDs of the set started from
  // @param symbol the terminal on path
  // @param move_set memory should be allocated outside this function, store Move(start_set, symbol)
  // Return: length of move_set
  INT32 Move(const Nfa &nfa, const std::set<INT32> &start_set,
               UINT16 symbol, std::set<INT32> *move_set);
  // create a new DfaNode and store its pointer in nodes_; if the node has existed, just return its ID
  // Return: the new DfaNode's id(the index in nodes_)
  INT32 CreateDfaNode(const Nfa &nfa, const std::set<INT32> *nfa_set);
  void ClearIdMap();
  void ClearMmap();
  // clear the std::map pattern member variables
  void ClearStdMemory();
  /////////////////////////// std::map pattern //////////////////
  std::vector<DfaNode *> nodes_; //store all nodes pointer of dfa the first node is the start node
  std::set<INT32> accept_types_; //the types could be accepted
  // the map relation from NFA nodes' IDs to one DFA node ID
  std::map<std::string, INT32> state_exists_;
  // the map relation from DFA ID to relative NFA nodes' IDs
  std::map<INT32, std::set<INT32> * > id_map_;

  bool mmap_pattern_; //the DFA is using mmap_pattern or std::map pattern
  //////////////////////////////mmap pattern///////////////////
  int fd_map_;
  INT32 *map_;
  UINT32 map_size_;
  INT32 node_count_;
  INT32 *node_type_;
  INT32 *level1_;
  INT32 *level2_;
};
}
}
#endif
