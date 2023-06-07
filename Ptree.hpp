#include <cstddef>
#include <atomic>   
#include <iostream>
#include <vector>
#include <omp.h>
#include <algorithm>
#include <cassert>

class TreeNode {
private:
  unsigned nthreads;
  volatile bool *flags;
  std::atomic_uint victim;
  bool you_first(unsigned tid);

public:
  TreeNode *leftChild;
  TreeNode *rightChild;
  TreeNode *parent;

  TreeNode(TreeNode *par, unsigned num_threads);
  ~TreeNode();
  void lock(unsigned tid);
  void unlock(unsigned tid);


};

class TournamentTree{
private:
    unsigned nthreads;
    TreeNode* startLeaf(unsigned tid);
    std::vector<TreeNode*> build_TournamentTree(std::vector<TreeNode*> leaves);
    bool is_2pown(unsigned threads);

public:
    TournamentTree(unsigned num_threads);
    ~TournamentTree();
    TreeNode *rootnode;
    std::vector<TreeNode*> leaves;
    void lock(unsigned tid);
    void unlock(unsigned tid);
};