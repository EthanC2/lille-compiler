#include <iostream>
#include <memory>

#include "binary_tree.h"
#include "id_table.h"

Node::Node(const IdTableEntry &entry_)
{
    entry = entry_;
    left = nullptr;
    right = nullptr;
}

BinaryTree::BinaryTree()
{
    root = nullptr;
}

bool BinaryTree::insert(const IdTableEntry &entry)
{
    return insert(root, entry);
}

bool BinaryTree::insert(std::unique_ptr<Node> &node, const IdTableEntry &entry)
{
    if (node == root && root == nullptr)
    {
	root = std::unique_ptr<Node>(new Node(entry));
	return true;
    }

    if (entry < node->entry)
    {
	if (node->left == nullptr)
	{
	    node->left = std::unique_ptr<Node>(new Node(entry));
	    return true;
	}
	else
	{
	    return insert(node->left, entry);
	}
    }
    else if (entry > node->entry)
    {
	if (node->right == nullptr)
	{
	    node->right = std::unique_ptr<Node>(new Node(entry));
	    return true;
	}
	else
	{
	    return insert(node->right, entry);
	}
    }
    else
    {
	return false;
    }
}

bool BinaryTree::contains(const IdTableEntry &entry)
{
    return contains(root, entry);
}

bool BinaryTree::contains(std::unique_ptr<Node> &node, const IdTableEntry &entry)
{
    if (node == nullptr)
    {
	return false;
    }

    if (entry < node->entry)
    {
	return contains(node->left, entry);
    }
    else if (entry > node->entry)
    {
	return contains(node->right, entry);
    }
    else
    {
	return true;
    }
}

IdTableEntry* BinaryTree::fetch(const IdTableEntry& key)
{
    return fetch(root, key);
}

IdTableEntry* BinaryTree::fetch(std::unique_ptr<Node> &node, const IdTableEntry &key)
{
    if (node == nullptr)
    {
	return nullptr;
    }

    if (key < node->entry)
    {
	return fetch(node->left, key);
    }
    else if (key > node->entry)
    {
	return fetch(node->right, key);
    }
    else
    {
	return &node->entry;
    }
}

void BinaryTree::print()
{
    print(root, 0);
}

void BinaryTree::print(std::unique_ptr<Node> &node, size_t depth)
{
    if (node == nullptr)
    {
	return;
    }
    
    for (size_t i=0; i < depth; ++i)
    {
	std::cout << "  ";
    }
    //std::cout << '[' << depth << "] " << node->entry << '\n';
    
    print(node->left, depth + 1);
    print(node->right, depth + 1);
}
