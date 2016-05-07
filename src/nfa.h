#ifndef __QSS_SEGMENTER_NFA_H__
#define __QSS_SEGMENTER_NFA_H__
#include <map>
#include <set>
#include "common_def.h"
#include "regular_expression_types.h"

namespace qss {
namespace segmenter {

// the stack of NfaTable, used to do NFA generation
typedef std::stack<NfaTable *> NfaTableStack;

class Dfa;
class Nfa {
 public:
  friend class Dfa;
  Nfa();
  ~Nfa();
  // clear the member variable and free memory
  void Clear();
  // clear the NfaTable stack: nfa_stack_;
  // don't free the memory which the nfa table pointer point to
  void ClearStack();
  // generate NFA from suffix expression
  NfaTable *GenerateNfaTable(const TokenizedExpression &suffix_expression);
  // set the type of the final node, return the old type
  static INT32 SetFinalNodeType(NfaTable *table, INT32 type);
  // push one NfaTable to stack
  void PushStack(NfaTable* const table);
  // generate final Nfa by the nfa_stack_;and store it at the top of nfa_stack_
  // return the final nfa table
  const NfaTable *GenerateFinalNfaTable();
  // for debug, output the NfaTable at the top of nfa_stack_
  void Output();
 private:
  // build basic NfaTable for one character
  NfaTable *GenerateOneCharacterNfa(UINT16 symbol);
  // calculate concatenation NFA table of the top 2 NFA table in nfa_stack_ 
  // and push the result back to nfa_stack_;  operator AB
  void Concatenate();
  // calculate union NFA table of the top 2 NFA table in nfa_stack_ 
  // and push the result back to nfa_stack_; operator A|B
  void Union();
  // calculate the superset NFA table of the top NFA table in nfa_stack_
  // and push the result back to nfa_stack_; operator A*
  void Superset();
  // calculate the zero or one time NFA table of top NFA table in nfa_stack_
  // and push the result back to nfa_stack_; operator A?
  void ZeroOrOne();
  // calculate the one or more times NFA table of the top table in nfa_stack_
  // and push the result back to nfa_stack_; operator A+
  void OneOrMore();
  // get the stack size of nfa_stack_
  INT32 GetSizeOfStack();
  // get the top NFA table in nfa_stack_
  const NfaTable *GetTopNfa();
  // get the nfa_nodes_
  const std::vector<NfaNode *> &nfa_node();
  //store the map relationship of variable to the NFA of its regular expression
  NfaTableStack nfa_stack_;
  // store all NFA nodes
  std::vector<NfaNode *> nfa_nodes_;
  // symbol set, store the symbols appear in NFA
  std::set<UINT16> symbols_;
};
}
}
#endif
