#ifndef BINARY_TREE_H_
#define BINARY_TREE_H_

#include <memory>
#include "id_table.h"

class IdTableEntry;

struct Node
{
    Node(const IdTableEntry &entry_);

    IdTableEntry entry;
    std::unique_ptr<Node> left;
    std::unique_ptr<Node> right;
};

class BinaryTree
{
    public:
	BinaryTree();
	bool insert(const IdTableEntry &entry);
	bool contains(const IdTableEntry &entry);
	IdTableEntry* fetch(const IdTableEntry &entry);
	void print();

    private:
	std::unique_ptr<Node> root;
	bool insert(std::unique_ptr<Node> &node, const IdTableEntry &entry);
	bool contains(std::unique_ptr<Node> &node, const IdTableEntry &entry);
	IdTableEntry* fetch(std::unique_ptr<Node> &node, const IdTableEntry &key);
	void print(std::unique_ptr<Node> &node, size_t depth);
};

#endif
