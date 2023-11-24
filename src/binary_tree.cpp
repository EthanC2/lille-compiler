#include <iostream>
#include <memory>

#include "binary_tree.h"

template <typename T>
Node<T>::Node(T value_)
{
    value = value_;
    left = nullptr;
    right = nullptr;
}

template <typename T>
BinaryTree<T>::BinaryTree()
{
    root = nullptr;
}

template <typename T>
bool BinaryTree<T>::insert(const T &item)
{
    return insert(root, item);
}

template <typename T>
bool BinaryTree<T>::insert(std::unique_ptr<Node<T>> &node, const T &item)
{
    if (node == root && root == nullptr)
    {
	root = std::unique_ptr<Node<T>>(new Node<T>(item));
	return true;
    }

    if (item < node->value)
    {
	if (node->left == nullptr)
	{
	    node->left = std::unique_ptr<Node<T>>(new Node<T>(item));
	    return true;
	}
	else
	{
	    return insert(node->left, item);
	}
    }
    else if (item > node->value)
    {
	if (node->right == nullptr)
	{
	    node->right = std::unique_ptr<Node<T>>(new Node<T>(item));
	    return true;
	}
	else
	{
	    return insert(node->right, item);
	}
    }
    else
    {
	return false;
    }
}

template <typename T>
bool BinaryTree<T>::contains(const T &item)
{
    return contains(root, item);
}

template <typename T>
bool BinaryTree<T>::contains(std::unique_ptr<Node<T>> &node, const T &item)
{
    if (node == nullptr)
    {
	return false;
    }

    if (item < node->value)
    {
	return contains(node->left, item);
    }
    else if (item > node->value)
    {
	return contains(node->right, item);
    }
    else
    {
	return true;
    }
}

template <typename T>
T* BinaryTree<T>::fetch(const T& key)
{
    return fetch(root, key);
}

template <typename T>
T* BinaryTree<T>::fetch(std::unique_ptr<Node<T>> &node, const T &key)
{
    if (node == nullptr)
    {
	return nullptr;
    }

    if (key < node->value)
    {
	return fetch(node->left, key);
    }
    else if (key > node->value)
    {
	return fetch(node->right, key);
    }
    else
    {
	return &node->value;
    }
}

template <typename T>
void BinaryTree<T>::print()
{
    print(root, 0);
}

template <typename T>
void BinaryTree<T>::print(std::unique_ptr<Node<T>> &node, size_t depth)
{
    if (node == nullptr)
    {
	return;
    }
    
    for (size_t i=0; i < depth; ++i)
    {
	std::cout << "  ";
    }
    std::cout << '[' << depth << "] " << node->value << '\n';
    
    print(node->left, depth + 1);
    print(node->right, depth + 1);
}
