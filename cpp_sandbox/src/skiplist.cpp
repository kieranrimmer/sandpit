
#include <iostream>
#include <vector>
#include <cassert>
#include <ctime>

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
  virtual ~BaseNode() = default;
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
  explicit InternalNode(BaseNode<DataType> *_next) : next{_next} {}
};

template<typename DataType>
struct LeafNode: public BaseNode<DataType> {
  DataType data;
  DataType getData() override {
    return data;
  }
  BaseNode<DataType> *getNext() override {
    return nullptr;
  }
  explicit LeafNode(DataType _data) : data{_data} {}
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
  ~RuntimeCheckNode() { 
    // cout << "Destroying!!!\n\n"; 
  }
  // RuntimeCheckNode(void* _data, char _flags) : data{_data}, flags{_flags} {}
};

template<typename T>
vector< std::unique_ptr< BaseNode<T> > > generatePolymorphicVec(T datum, size_t vecSize = 10) {
  vector< std::unique_ptr< BaseNode<T> > > nodeVec;
  nodeVec.push_back(std::make_unique< LeafNode<T> >( LeafNode<T>(datum) ));
  size_t i = 0;
  while(i < vecSize) {
    nodeVec.push_back(std::make_unique< InternalNode<T> >(InternalNode<T>( nodeVec[nodeVec.size() - 1].get() ) ));
    // cout << "Added node #" << i << "\n";
    ++i;
  }
  return nodeVec;
}

void polymorphicTest(size_t vecSize) {
  int datum = 2252;
  size_t i = 0;
  size_t j = 0;
  {
    vector< std::unique_ptr< BaseNode<int*> > > nodeVec = generatePolymorphicVec(&datum, vecSize);
    //
    cout << "Time to search Polymorphic nodes\n";
    // CLOCK SETUP BLOCK
    std::clock_t start;
    double duration;
    start = std::clock();
    // END CLOCK SETUP BLOCK
    auto curNode = nodeVec[nodeVec.size() - 1].get();
    while (curNode->getData() ==  nullptr) {
      //cout << "Searching...\n";
      curNode = curNode->getNext();
    }
    // cout << "Found datum = " << *curNode->getData() << "\n";
    assert(*curNode->getData() == datum);
    // CLOCK READ BLOCK
    duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
    // END CLOCK READ BLOCK
    // CLOCK REPORT BLOCK
    cout << __func__ << " duration: " << duration << '\n';
    // END CLOCK REPORT BLOCK
    cout << "Polymorphic Test passed!!!\n";
  }
  cout << "Test completed!!!\n";
}

template<typename T>
vector< std::unique_ptr< RuntimeCheckNode<T> > > generateRuntimeVec(T datum, size_t vecSize = 10) {
  vector< std::unique_ptr< RuntimeCheckNode<T> > > nodeVec;
  nodeVec.push_back(std::make_unique< RuntimeCheckNode<T> >(RuntimeCheckNode<T>{ .data = datum, .flags =  IS_LEAF_FLAG  }));
  size_t i = 0;
  while(i < vecSize) {
    nodeVec.push_back(std::make_unique< RuntimeCheckNode<int*> >(RuntimeCheckNode<int*>{.data = nodeVec[nodeVec.size() - 1].get(), .flags = '\0' }));
    // cout << "Added node #" << i << "\n";
    ++i;
  }
  return nodeVec;
}


void runtimeTest(size_t vecSize) {
  int datum = 2252;
  size_t i = 0;
  size_t j = 0;
  {
    vector< std::unique_ptr< RuntimeCheckNode<int*> > > nodeVec = generateRuntimeVec(&datum, vecSize);
    //
    cout << "Time to search Runtime Checkable nodes\n";
    // CLOCK SETUP BLOCK
    std::clock_t start;
    double duration;
    start = std::clock();
    // END CLOCK SETUP BLOCK
    auto curNode = nodeVec[nodeVec.size() - 1].get();
    while (!curNode->isLeafNode()) {
      // cout << "Searching...\n";
      curNode = curNode->getNext();
    }
    // cout << "Found datum = " << *curNode->getData() << "\n";
    assert(*curNode->getData() == datum);
    // CLOCK READ BLOCK
    duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
    // END CLOCK READ BLOCK
    // CLOCK REPORT BLOCK
    cout << __func__ << " duration: " << duration << '\n';
    // END CLOCK REPORT BLOCK
    cout << "Test passed!!!\n";
  }
  cout << "Test completed!!!\n";
}

int main() {
  runtimeTest(1e3);
  polymorphicTest(1e3);
}

