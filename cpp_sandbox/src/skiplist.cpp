
#include <iostream>
#include <vector>
#include <cassert>
#include <ctime>

using std::cout;
using std::vector;

using flags_t=char;

#define _ASSUMED_CACHE_LINE_SIZE_ 64
#define _IS_LEAF_FLAG_ 0x01

// For a tree-based index in C++, assuming it is best to fit nodes into cache lines, is it better to: (a) use polymorphism / burn space on a vtable pointer, (b) use some sort of flag to check the node type at runtime, (c) something else?



constexpr size_t ASSUMED_CACHE_LINE_SIZE = (size_t) _ASSUMED_CACHE_LINE_SIZE_;
constexpr flags_t IS_LEAF_FLAG = (flags_t) _IS_LEAF_FLAG_;

template<typename KeyType, typename DataType>
struct BaseNode {
  static_assert( sizeof(void *) == sizeof(DataType), "DataType type must be pointer-sized" );
  virtual DataType getData() = 0;
  virtual BaseNode *getNext() = 0;
  virtual ~BaseNode() = default;
};

template<typename KeyType, typename DataType>
struct InternalNode: public BaseNode<KeyType, DataType> {
  static_assert( sizeof(void *) == sizeof(DataType), "DataType type must be pointer-sized" );
  constexpr static size_t max_keyptr_pairs = (sizeof(KeyType) + sizeof(BaseNode<KeyType, DataType> *)) / (ASSUMED_CACHE_LINE_SIZE - sizeof(BaseNode<KeyType, DataType>));
  constexpr static size_t keyptr_pairs = max_keyptr_pairs > 0 ? max_keyptr_pairs : 1;
  static_assert(keyptr_pairs > 0, "Array is large enough");
  BaseNode<KeyType, DataType>* next[keyptr_pairs];
  DataType getData() override {
    return nullptr;
  }
  BaseNode<KeyType, DataType> *getNext() override {
    return next[0];
  }
  explicit InternalNode(BaseNode<KeyType, DataType> *_next) : next{_next} {}
};

template<typename KeyType, typename DataType>
struct LeafNode: public BaseNode<KeyType, DataType> {
  static_assert( sizeof(void *) == sizeof(DataType), "DataType type must be pointer-sized" );
  constexpr static size_t max_keyval_pairs = (sizeof(KeyType) + sizeof(DataType)) / (ASSUMED_CACHE_LINE_SIZE - sizeof(BaseNode<KeyType, DataType>));
  constexpr static size_t keyval_pairs = max_keyval_pairs > 0 ? max_keyval_pairs : 1;
  static_assert(keyval_pairs > 0, "Array is large enough");
  DataType data[keyval_pairs];
  DataType getData() override {
    return data[0];
  }
  BaseNode<KeyType, DataType> *getNext() override {
    return nullptr;
  }
  explicit LeafNode(DataType _data) : data{_data} {}
};

template<typename KeyType, typename DataType>
struct RuntimeCheckNode {
  constexpr static size_t max_keyval_pairs = (sizeof(KeyType) + sizeof(DataType)) / (ASSUMED_CACHE_LINE_SIZE - sizeof(flags_t));
  constexpr static size_t keyval_pairs = max_keyval_pairs > 0 ? max_keyval_pairs : 1;
  static_assert(keyval_pairs > 0, "Array is large enough");
  void* data[keyval_pairs];
  flags_t flags;
  bool isLeafNode() const {
    return flags & (flags_t) IS_LEAF_FLAG;
  }
  DataType getData() {
    return isLeafNode() ? reinterpret_cast<DataType>(data[0]) : nullptr;
  }
  RuntimeCheckNode *getNext() {
    return isLeafNode() ? nullptr : reinterpret_cast<RuntimeCheckNode*>(data[0]);
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

template<typename K, typename T>
vector< std::unique_ptr< BaseNode<K, T> > > generatePolymorphicVec(T datum, size_t vecSize = 10) {
  vector< std::unique_ptr< BaseNode<K, T> > > nodeVec;
  nodeVec.push_back(std::make_unique< LeafNode<K, T> >( LeafNode<K, T>(datum) ));
  size_t i = 0;
  while(i < vecSize) {
    nodeVec.push_back(std::make_unique< InternalNode<K, T> >(InternalNode<K, T>( nodeVec[nodeVec.size() - 1].get() ) ));
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
    vector< std::unique_ptr< BaseNode<int, int*> > > nodeVec = generatePolymorphicVec<int, int*>(&datum, vecSize);
    //
    cout << "Time to search Polymorphic nodes\n";
    cout << "InternalNode<int, int*>::max_keyval_pairs = " << InternalNode<int, int*>::max_keyptr_pairs << "\n";
    cout << "LeafNode<int, int*>::max_keyval_pairs = " << LeafNode<int, int*>::max_keyval_pairs << "\n";
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

template<typename K, typename T>
vector< std::unique_ptr< RuntimeCheckNode<K, T> > > generateRuntimeVec(T datum, size_t vecSize = 10) {
  vector< std::unique_ptr< RuntimeCheckNode<K, T> > > nodeVec;
  nodeVec.push_back(std::make_unique< RuntimeCheckNode<K, T> >(RuntimeCheckNode<K, T>{ .data = {datum}, .flags =  IS_LEAF_FLAG  }));
  size_t i = 0;
  while(i < vecSize) {
    nodeVec.push_back(std::make_unique< RuntimeCheckNode<K, T> >(RuntimeCheckNode<K, T>{.data = {nodeVec[nodeVec.size() - 1].get()}, .flags = '\0' }));
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
    vector< std::unique_ptr< RuntimeCheckNode<int, int*> > > nodeVec = generateRuntimeVec<int, int*>(&datum, vecSize);
    cout << "RuntimeCheckNode<int, int*>::max_keyval_pairs = " << RuntimeCheckNode<int, int*>::max_keyval_pairs << "\n";
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

