#ifndef BINARY_TREE_H_
#define BINARY_TREE_H_

#include <memory>

template <typename T>
struct Node
{
    Node(T value);

    T value;
    std::unique_ptr<Node<T>> left;
    std::unique_ptr<Node<T>> right;
};

template <typename T>
class BinaryTree
{
    public:
	BinaryTree();
	bool insert(const T &item);
	bool contains(const T &item);
	T* fetch(const T& key);
	void print();
	//T& fetch(T key)

    private:
	std::unique_ptr<Node<T>> root;
	bool insert(std::unique_ptr<Node<T>> &node, const T &item);
	bool contains(std::unique_ptr<Node<T>> &node, const T &item);
	T* fetch(std::unique_ptr<Node<T>> &node, const T &key);
	void print(std::unique_ptr<Node<T>> &node, size_t depth);
};

#endif
