#include "Ptree.hpp"

TreeNode::TreeNode(TreeNode *parent, unsigned num_threads) : no_of_threads(num_threads){
    this->parent = parent; 
    flags = new bool[no_of_threads];
    for(unsigned i=0; i<no_of_threads; i++){
        flags[i] = false;
    }
}

TreeNode::~TreeNode(){
    delete [] flags;
}

void TreeNode::lock(unsigned tid) {
    flags[tid] = true;
    victim = tid;

    //spin until my flag is unset and I am not victim
    while(isAnotherFlag(tid) && isVictim(tid)){};
  //std::cout << "Lock obtained by thread " << tid << std::endl;
}

void TreeNode::unlock(unsigned tid) {
  flags[tid] = false;
  //std::cout << "Lock released by thread " << tid << std::endl;
}

bool TreeNode::isVictim(unsigned tid) {
    return victim == tid;
}

bool TreeNode::isAnotherFlag(unsigned tid) {
    for(unsigned i = 0; i < no_of_threads; i++){
        if(flags[i] && (i != tid))
            return true;
    }
    return false;
}

TournamentTree::TournamentTree(unsigned num_threads) : no_of_threads(num_threads){
    if(is_2pown(no_of_threads))
    {
        root = new TreeNode(nullptr, no_of_threads);
        std::vector<TreeNode*> initList;
        initList.push_back(root);
        leaves = growTree(initList);
    }
    else{
        throw std::invalid_argument("Number of threads must be power of 2");
    }
}

TournamentTree::~TournamentTree(){
    leaves.clear();
}

void TournamentTree::lock(unsigned tid){
    TreeNode *currentNode = leafLockForThread(tid);
    while(currentNode != nullptr){
        currentNode->lock(tid);
        currentNode = currentNode->parent;
    }
}

void TournamentTree::unlock(unsigned tid){
    TreeNode *currentNode = leafLockForThread(tid);
    while(currentNode != nullptr){
        currentNode->unlock(tid);
        currentNode = currentNode->parent;
    }
}

TreeNode* TournamentTree::leafLockForThread(unsigned tid){
    return leaves[int(tid/2)];
}

std::vector<TreeNode*> TournamentTree::growTree(std::vector<TreeNode*> leaves){
    if(leaves.size() == no_of_threads/2)
        return leaves;

    std::vector<TreeNode*> currentLeaves;
    for(TreeNode* node : leaves){
        node->leftChild = new TreeNode(node, no_of_threads);
        node->rightChild = new TreeNode(node, no_of_threads);
        currentLeaves.push_back(node->leftChild);
        currentLeaves.push_back(node->rightChild);
    }

    return growTree(currentLeaves);
}

bool TournamentTree::is_2pown(unsigned nthreads){    
    while (nthreads > 1) { // Keep dividing by 2 until there is either zero or no modulo
        if (nthreads % 2 != 0) {return false;}
        nthreads = nthreads/ 2;
    }
    
    return true;
}