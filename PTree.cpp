#include "Ptree.hpp"

TreeNode::TreeNode(TreeNode *parent, unsigned nthreads){
    this -> nthreads = nthreads;
    this->parent = parent; 
    this -> flags = new bool[nthreads];
    for(unsigned i=0; i<nthreads; i++){flags[i] = false;} //initialize not-interested flags
}

TreeNode::~TreeNode(){delete [] flags;} //destructor

void TreeNode::lock(unsigned tid) {
    flags[tid] = true; //I am interested
    this -> victim = tid; //you go first
    while(you_first(tid) && (this->victim) == tid  ){}; //spin until my flag is unset and I am not victim
}

void TreeNode::unlock(unsigned tid) {flags[tid] = false;} /*not interested anymore*/


bool TreeNode::you_first(unsigned tid) {
    for(unsigned i = 0; i < nthreads; i++){ 
        if(flags[i] && (i != tid)) {return true;} } //if anyone else interested, let it first
    return false; }


TournamentTree::TournamentTree(unsigned num_threads){
    assert( is_2pown(num_threads) );
    this -> nthreads = num_threads;
    rootnode = new TreeNode(nullptr, nthreads);
    std::vector<TreeNode*> init_node;
    init_node.push_back(rootnode);
    leaves = build_TournamentTree(init_node); //initialize the nodes of the tree
}

TournamentTree::~TournamentTree(){leaves.clear();}

void TournamentTree::lock(unsigned tid){
    TreeNode *leaf = startLeaf(tid);
    while(leaf != nullptr){ //compete until I reach the root
        leaf->lock(tid); //acquire lock of this leaf
        leaf = leaf->parent; //move up if I acquired lock otherwise sit on this leaf
    }
}

void TournamentTree::unlock(unsigned tid){
    TreeNode *leaf = startLeaf(tid);
    while(leaf != nullptr){ //release from top down to root
        leaf->unlock(tid);
        leaf = leaf->parent;
    }
}

TreeNode* TournamentTree::startLeaf(unsigned thread){return leaves[int(thread/2)]; } //give starting leaf of thread

std::vector<TreeNode*> TournamentTree::build_TournamentTree(std::vector<TreeNode*> leaves){
    if(leaves.size() == nthreads/2) {return leaves;}

    std::vector<TreeNode*> tree;
    for(TreeNode* node : leaves){
        node->leftChild = new TreeNode(node, nthreads); //add left Child
        node->rightChild = new TreeNode(node, nthreads); //add right child
        tree.push_back(node->leftChild);
        tree.push_back(node->rightChild);
    }

    return build_TournamentTree(tree);
    //return tree;
}

bool TournamentTree::is_2pown(unsigned n){  //algorithm only works for nthreads = 2^k, k integer
    while (n > 1) { // Keep dividing by 2 until there is either zero or no modulo
        if (n % 2 != 0) {return false;}
        n = n/ 2;
    }   
    return true;
}