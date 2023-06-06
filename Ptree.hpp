#include <cstddef>
#include <atomic>   
#include <iostream>
#include <vector>
#include <omp.h>
#include <algorithm>

class TreeNode {
private:
  const unsigned no_of_threads;
  volatile bool *flags;
  std::atomic_uint victim;
  bool isVictim(unsigned tid);
  bool isAnotherFlag(unsigned tid);

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
    const unsigned no_of_threads;
    TreeNode* leafLockForThread(unsigned tid);
    std::vector<TreeNode*> growTree(std::vector<TreeNode*> leaves);
    bool is_2pown(unsigned threads);

public:
    TournamentTree(unsigned num_threads);
    ~TournamentTree();
    TreeNode *root;
    std::vector<TreeNode*> leaves;
    void lock(unsigned tid);
    void unlock(unsigned tid);
};