/**
 * @file huffman_tree.cpp
 * Implementation of a Huffman Tree class.
 *
 * @author Chase Geigle
 * @date Summer 2012
 */

#include <algorithm>
#include <iostream>
#include <queue>
#include <utility>

#include "huffman_tree.h"

using namespace std;

HuffmanTree::HuffmanTree(vector<Frequency> frequencies)
{
    std::stable_sort(frequencies.begin(), frequencies.end());
    buildTree(frequencies);
    vector<bool> path;
    buildMap(root_, path);
}

HuffmanTree::HuffmanTree(const HuffmanTree& other)
{
    copy(other);
}

HuffmanTree::HuffmanTree(BinaryFileReader& bfile)
{
    root_ = readTree(bfile);
    vector<bool> path;
    buildMap(root_, path);
}

HuffmanTree::~HuffmanTree()
{
    clear(root_);
}

const HuffmanTree& HuffmanTree::operator=(const HuffmanTree& rhs)
{
    if (this != &rhs) {
        clear(root_);
        copy(rhs);
    }
    return *this;
}

void HuffmanTree::clear(TreeNode* current)
{
    if (current == NULL)
        return;
    clear(current->left);
    clear(current->right);
    delete current;
}

void HuffmanTree::copy(const HuffmanTree& rhs)
{
    root_ = copy(rhs.root_);
}

HuffmanTree::TreeNode* HuffmanTree::copy(const TreeNode* current)
{
    if (current == NULL)
        return NULL;
    TreeNode* node = new TreeNode(current->freq);
    node->left = copy(current->left);
    node->right = copy(current->right);
    return node;
}

HuffmanTree::TreeNode*
HuffmanTree::removeSmallest(queue<TreeNode*>& singleQueue,
                            queue<TreeNode*>& mergeQueue)
{
	if (singleQueue.size() == 0 && mergeQueue.size() == 0) {return NULL;}
	else if (singleQueue.size() == 0) {
		TreeNode * temp = mergeQueue.front();
		mergeQueue.pop();
		return temp;
	}
	else if (mergeQueue.size() == 0) {
		TreeNode * temp = singleQueue.front();
		singleQueue.pop();
		return temp;
	}
	else if (singleQueue.front()->freq.getFrequency() <= mergeQueue.front()->freq.getFrequency()) {
		TreeNode * temp = singleQueue.front();
		singleQueue.pop();
		return temp;
	}
	else {
		TreeNode * temp = mergeQueue.front();
		mergeQueue.pop();
		return temp;
	}
}

void HuffmanTree::buildTree(const vector<Frequency>& frequencies)
{
    queue<TreeNode*> singleQueue; // Queue containing the leaf nodes
    queue<TreeNode*> mergeQueue;  // Queue containing the inner nodes

    for (unsigned i = 0; i < frequencies.size(); i++) {
    	TreeNode * new_node = new TreeNode(frequencies[i]);
    	singleQueue.push(new_node);
    }
    while (singleQueue.size() + mergeQueue.size() != 1) {
    	TreeNode * smallest = removeSmallest(singleQueue, mergeQueue);
    	TreeNode * nextSmallest = removeSmallest(singleQueue, mergeQueue);

    	TreeNode * merged = new TreeNode(smallest->freq.getFrequency() + nextSmallest->freq.getFrequency());
    	merged->left = smallest;
    	merged->right = nextSmallest;
    	mergeQueue.push(merged);
    }
    if (singleQueue.size() == 1) {root_ = singleQueue.front();}
    else {root_ = mergeQueue.front();}
}

string HuffmanTree::decodeFile(BinaryFileReader& bfile)
{
    stringstream ss;
    decode(ss, bfile);
    return ss.str();
}

void HuffmanTree::decode(stringstream& ss, BinaryFileReader& bfile)
{
    TreeNode* current = root_;
    bool cur_bit;
    while (bfile.hasBits()) {
    	cur_bit = bfile.getNextBit();
    	if (cur_bit) {current = current->right;}
    	else {current = current->left;}
    	if (current->right == NULL && current->left == NULL) {
    		ss << current->freq.getCharacter();
    		current = root_;
    	}
    }
}

void HuffmanTree::writeTree(BinaryFileWriter& bfile)
{
    writeTree(root_, bfile);
    bfile.close();
}

void HuffmanTree::writeTree(TreeNode* current, BinaryFileWriter& bfile)
{
	if (current->left == NULL && current->right == NULL) {
		bfile.writeBit(true);
		bfile.writeByte(current->freq.getCharacter());
	}
	else {
		bfile.writeBit(false);
		writeTree(current->left, bfile);
		writeTree(current->right, bfile);
	}
}

HuffmanTree::TreeNode* HuffmanTree::readTree(BinaryFileReader& bfile)
{
	TreeNode * new_node;
	if (!bfile.hasBits()) {
		return NULL;
	}
	else if (bfile.getNextBit()) {
		Frequency new_freq(bfile.getNextByte(), 0);// = new Frequency(bfile.getNextByte(), 0); 
		new_node = new TreeNode(new_freq);
	}
	else {
		Frequency new_freq(0);// = new Frequency(0);
		new_node = new TreeNode(new_freq);
		new_node->left = readTree(bfile);
		new_node->right = readTree(bfile);
	}
    return new_node;
}

void HuffmanTree::buildMap(TreeNode* current, vector<bool>& path)
{
    // Base case: leaf node.
    if (current->left == NULL && current->right == NULL) {
        bitsMap_[current->freq.getCharacter()] = path;
        return;
    }

    // Move left
    path.push_back(false);
    buildMap(current->left, path);
    path.pop_back();

    // Move right
    path.push_back(true);
    buildMap(current->right, path);
    path.pop_back();
}

void HuffmanTree::printInOrder() const
{
    printInOrder(root_);
    cout << endl;
}

void HuffmanTree::printInOrder(const TreeNode* current) const
{
    if (current == NULL)
        return;
    printInOrder(current->left);
    cout << current->freq.getCharacter() << ":" << current->freq.getFrequency()
         << " ";
    printInOrder(current->right);
}

void HuffmanTree::writeToFile(const string& data, BinaryFileWriter& bfile)
{
    for (auto it = data.begin(); it != data.end(); ++it)
        writeToFile(*it, bfile);
}

void HuffmanTree::writeToFile(char c, BinaryFileWriter& bfile)
{
    vector<bool> bits = getBitsForChar(c);
    for (auto it = bits.begin(); it != bits.end(); ++it)
        bfile.writeBit(*it);
}

vector<bool> HuffmanTree::getBitsForChar(char c)
{
    return bitsMap_[c];
}

// class for generic printing

template <typename TreeNode>
class HuffmanTreeNodeDescriptor
    : public GenericNodeDescriptor<HuffmanTreeNodeDescriptor<TreeNode>>
{
  public:
    HuffmanTreeNodeDescriptor(const TreeNode* root) : subRoot_(root)
    { /* nothing */
    }

    string key() const
    {
        std::stringstream ss;
        char ch = subRoot_->freq.getCharacter();
        int freq = subRoot_->freq.getFrequency();

        // print the sum of the two child frequencies
        if (ch == '\0')
            ss << freq;
        // print the leaf containing a character and its count
        else {
            if (ch == '\n')
                ss << "\\n";
            else
                ss << ch;
            ss << ":" << freq;
        }
        return ss.str();
    }

    bool isNull() const
    {
        return subRoot_ == NULL;
    }
    HuffmanTreeNodeDescriptor left() const
    {
        return HuffmanTreeNodeDescriptor(subRoot_->left);
    }
    HuffmanTreeNodeDescriptor right() const
    {
        return HuffmanTreeNodeDescriptor(subRoot_->right);
    }

  private:
    const TreeNode* subRoot_;
};

int HuffmanTree::height(const TreeNode* subRoot) const
{
    if (subRoot == NULL)
        return -1;
    return 1 + std::max(height(subRoot->left), height(subRoot->right));
}

void HuffmanTree::print(std::ostream& out) const
{
    int h = height(root_);
    if (h > _max_print_height) {
        out << "Tree is too big to print. Try with a small file (e.g. "
               "data/small.txt)"
            << endl;
        return;
    }

    printTree(HuffmanTreeNodeDescriptor<TreeNode>(root_), out);
}

Frequency
HuffmanTree::testRemoveSmallest(vector<Frequency> single,
                                vector<Frequency> merge)
{
    queue<TreeNode*> singleQueue;
    queue<TreeNode*> mergeQueue;
    for (auto it = single.begin(); it != single.end(); ++it) {
         singleQueue.push(new TreeNode(*it));
    }
    for (auto it = merge.begin(); it != merge.end(); ++it) {
         mergeQueue.push(new TreeNode(*it));
    }

    TreeNode* testNode = removeSmallest(singleQueue, mergeQueue);
    Frequency testFreq = testNode->freq;

    delete testNode;
    while (!singleQueue.empty()) {
        delete singleQueue.front();
        singleQueue.pop();
    }
    while (!mergeQueue.empty()) {
        delete mergeQueue.front();
        mergeQueue.pop();
    }

    return testFreq;
}
