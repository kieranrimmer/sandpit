
#include <iostream>

using std::cout;

// For a tree-based index in C++, assuming it is best to fit nodes into cache lines, is it better to: (a) use polymorphism / burn space on a vtable pointer, (b) use some sort of flag to check the node type at runtime, (c) something else?

#define _IS_LEAF_FLAG_ 0x01

const char IS_LEAF_FLAG = (char) _IS_LEAF_FLAG_;

template<typename DataType>
struct BaseNode {
  public:
  virtual DataType getData() = 0;
  virtual BaseNode *getNext() = 0;
};

template<typename DataType>
struct InternalNode: public BaseNode<DataType> {
  BaseNode<DataType> *next;
  DataType getData() override {
    return nullptr;
  }
  BaseNode<DataType> *getNext() override {
    return next;
  }
};

template<typename DataType>
struct LeafNode: public BaseNode<DataType> {
  DataType *data;
  DataType getData() override {
    return data;
  }
  BaseNode<DataType> *getNext() override {
    return nullptr;
  }
};

template<typename DataType>
struct RuntimeCheckNode {
  void *data;
  char flags;
  bool isLeafNode() const {
    return flags & (char) IS_LEAF_FLAG;
  }
  DataType getData() {
    return isLeafNode() ? reinterpret_cast<DataType>(data) : nullptr;
  }
  RuntimeCheckNode *getNext() {
    return isLeafNode() ? nullptr : reinterpret_cast<RuntimeCheckNode*>(data);
  }
};

void runtimeTest() {
  int datum = 2252;
  auto curNode = new RuntimeCheckNode<int*>{ .data = &datum, .flags = IS_LEAF_FLAG };
  cout << "Added intital node at: " << &curNode << "\n"; 
  for(int i; i < 10; ++i) {
    curNode = new RuntimeCheckNode<int*>{ .data = curNode, .flags = '\0' };
  }
  //
  cout << "Time to search Runtime Checkable nodes\n";
  while (!curNode->isLeafNode()) {
    cout << "Searching...\n";
    curNode = curNode->getNext();
  }
  cout << "Found datum = " << *curNode->getData() << "\n";
  
}

int main() {
  runtimeTest();
}

