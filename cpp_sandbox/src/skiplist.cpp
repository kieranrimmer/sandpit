
#include <iostream>
#include <vector>
#include <cassert>

using std::cout;
using std::vector;

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
  // RuntimeCheckNode() = default;
  RuntimeCheckNode(const RuntimeCheckNode &other) = default;
  RuntimeCheckNode(RuntimeCheckNode &&other) = default;
  RuntimeCheckNode &operator=(const RuntimeCheckNode &other) = default;
  RuntimeCheckNode &operator=(RuntimeCheckNode &&other) = default;
  ~RuntimeCheckNode() { cout << "Destroying!!!\n\n"; }
  // RuntimeCheckNode(void* _data, char _flags) : data{_data}, flags{_flags} {}
};



void runtimeTest() {
  int datum = 2252;
  size_t vecSize = 10;
  size_t i = 0;
  size_t j = 0;
  {
    vector< std::unique_ptr< RuntimeCheckNode<int*> > > nodeVec;
    nodeVec.push_back(std::make_unique< RuntimeCheckNode<int*> >(RuntimeCheckNode<int*>{ .data = &datum, .flags =  IS_LEAF_FLAG  }));
    cout << "Added intital node at " << nodeVec.size() << ": " << &nodeVec[nodeVec.size() - 1] << "\n"; 
    
    
    while(i < vecSize) {
      nodeVec.push_back(std::make_unique< RuntimeCheckNode<int*> >(RuntimeCheckNode<int*>{.data = nodeVec[nodeVec.size() - 1].get(), .flags = '\0' }));
      cout << "Added node at " << nodeVec.size() << ": " << &nodeVec[nodeVec.size() - 1] << "\n";
      ++i;
    }
    for(auto &elem: nodeVec) {
          std::cout << "entry " << j << " = " << *((int *) elem->data) << "\n";
          ++j;
    }
    //
    cout << "Time to search Runtime Checkable nodes\n";
    auto curNode = nodeVec[nodeVec.size() - 1].get();
    while (!curNode->isLeafNode()) {
      cout << "Searching...\n";
      curNode = curNode->getNext();
    }
    cout << "Found datum = " << *curNode->getData() << "\n";
    assert(*curNode->getData() == datum);
    cout << "Test passed!!!\n";
  }
  cout << "Test completed!!!\n";

  
}

int main() {
  runtimeTest();
}

