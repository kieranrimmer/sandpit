
#include <iostream>
#include <vector>
#include <cassert>
#include <ctime>
#include <algorithm>

using std::cout;
using std::vector;

using flags_t=char;

#define _ASSUMED_CACHE_LINE_SIZE_ 64
#define _IS_LEAF_FLAG_ 0x01
#define _TEST_ARR_SIZE_ 10
// For a tree-based index in C++, assuming it is best to fit nodes into cache lines, is it better to: (a) use polymorphism / burn space on a vtable pointer, (b) use some sort of flag to check the node type at runtime, (c) something else?

// CRUDE PACKING TESTS

constexpr size_t TEST_ARR_SIZE = (size_t) _TEST_ARR_SIZE_;

struct serial_obj {
  int64_t int_64_val;
  int32_t int_32_val;
};

struct parallel_arrays_container {
  int64_t int_64_val_arr[TEST_ARR_SIZE];
  int32_t int_32_val_arr[TEST_ARR_SIZE];
};

struct serial_object_array_container {
  serial_obj serial_obj_arr[TEST_ARR_SIZE];
};

void reportPackingScenarios() {
  cout << "\n#### Packing Scenarios:\n";
  cout << "sizeof(parallel_arrays_container) = " << sizeof(parallel_arrays_container) << "\n";
  cout << "sizeof(serial_object_array_container) = " << sizeof(serial_object_array_container) << "\n";
  cout << "\n####\n";
}

//

constexpr size_t ASSUMED_CACHE_LINE_SIZE = (size_t) _ASSUMED_CACHE_LINE_SIZE_;
constexpr flags_t IS_LEAF_FLAG = (flags_t) _IS_LEAF_FLAG_;

template<typename KeyType, typename DataType>
struct BaseNode {
  static_assert( sizeof(void *) == sizeof(DataType), "DataType type must be pointer-sized" );
  virtual DataType getData() = 0;
  virtual void autoIncrementKeys() {}
  virtual BaseNode *getNext(KeyType key) = 0;
  virtual ~BaseNode() = default;

  // constexpr static std::less<KeyType> KeyComparatorLess = std::less<KeyType>();
  // constexpr static std::equal_to<KeyType> KeyComparatorEq = std::equal_to<KeyType>();

};

const std::less<> KeyComparatorLess = std::less<>();

template<typename KeyType, typename DataType>
struct InternalNode: public BaseNode<KeyType, DataType> {
  static_assert( sizeof(void *) == sizeof(DataType), "DataType type must be pointer-sized" );
  constexpr static size_t max_keyptr_pairs = 
    (ASSUMED_CACHE_LINE_SIZE - sizeof(BaseNode<KeyType, DataType>)) /
    (sizeof(KeyType) + sizeof(BaseNode<KeyType, DataType> *));
  constexpr static size_t keyptr_pairs = max_keyptr_pairs > 0 ? max_keyptr_pairs : 1;
  static_assert(keyptr_pairs > 0, "Array is large enough");

  // Larger type first for optimal layout
  BaseNode<KeyType, DataType>* next[keyptr_pairs];
  KeyType keys[keyptr_pairs];

  DataType getData() override {
    return nullptr;
  }
  void autoIncrementKeys() override {
    for(int i=0;i<keyptr_pairs;++i) {
      keys[i]=keys[i-1]+1;
      next[i]=next[0];
    }
  }
  size_t scanKeyArr(KeyType key) const {
    size_t i = 0;
    while(KeyComparatorLess(keys[i], key) && i < keyptr_pairs)
      ++i;
    return i;
  }
  BaseNode<KeyType, DataType> *getNext(KeyType key) override {
    return next[scanKeyArr(key)];
  }
  explicit InternalNode(BaseNode<KeyType, DataType> *_next) : next{_next} {}
  explicit InternalNode(BaseNode<KeyType, DataType> *_next, KeyType _key) : next{_next}, keys{_key} {}
};

template<typename KeyType, typename DataType>
struct LeafNode: public BaseNode<KeyType, DataType> {
  
  static_assert( sizeof(void *) == sizeof(DataType), "DataType type must be pointer-sized" );
  constexpr static size_t max_keyval_pairs = 
    (ASSUMED_CACHE_LINE_SIZE - sizeof(BaseNode<KeyType, DataType>)) /
    (sizeof(KeyType) + sizeof(DataType));
  constexpr static size_t keyval_pairs = max_keyval_pairs > 0 ? max_keyval_pairs : 1;
  static_assert(keyval_pairs > 0, "Array is large enough");
  
  // Larger type first for optimal layout
  DataType data[keyval_pairs];
  KeyType keys[keyval_pairs];

  DataType getData() override {
    return data[0];
  }
  BaseNode<KeyType, DataType> *getNext(KeyType key) override {
    return nullptr;
  }
  explicit LeafNode(DataType _data) : data{_data} {}
  explicit LeafNode(DataType _data, KeyType _key) : data{_data}, keys{_key} {}
};

template<typename KeyType, typename DataType>
struct RuntimeCheckNode {
  constexpr static size_t max_keyval_pairs = 
    (ASSUMED_CACHE_LINE_SIZE - sizeof(flags_t)) /
    (sizeof(KeyType) + 
      sizeof(DataType)
    );
  constexpr static size_t keyval_pairs = max_keyval_pairs > 0 ? max_keyval_pairs : 1;
  static_assert(keyval_pairs > 0, "Array is large enough");

  // static std::less<KeyType> KeyComparatorLess = std::less<KeyType>();
  // static std::equal_to<KeyType> KeyComparatorEq = std::equal_to<KeyType>();

  // Larger type first for optimal layout
  void* data[keyval_pairs];
  KeyType keys[keyval_pairs];

  flags_t flags;
  bool isLeafNode() const {
    return flags & (flags_t) IS_LEAF_FLAG;
  }
  DataType getData() {
    return isLeafNode() ? reinterpret_cast<DataType>(data[0]) : nullptr;
  }
  size_t scanKeyArr(KeyType key) const {
    size_t i = 0;
    while(KeyComparatorLess(keys[i], key) && i < keyval_pairs)
      ++i;
    return i;
  }
  RuntimeCheckNode *getNext(KeyType key) const {
    return isLeafNode() ? nullptr 
            : reinterpret_cast<RuntimeCheckNode*>(data[scanKeyArr(key)]);
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
vector< std::unique_ptr< BaseNode<K, T> > > generatePolymorphicVec(K key, T datum, size_t vecSize = 10) {
  vector< std::unique_ptr< BaseNode<K, T> > > nodeVec;
  nodeVec.push_back(std::make_unique< LeafNode<K, T> >( LeafNode<K, T>(datum, key) ));
  size_t i = 0;
  while(i < vecSize) {
    auto toInsert = std::make_unique< InternalNode<K, T> >(InternalNode<K, T>( nodeVec[nodeVec.size() - 1].get(), key ) );
    toInsert->autoIncrementKeys();
    nodeVec.push_back(std::move(toInsert));
    // cout << "Added node #" << i << "\n";
    ++i;
  }
  return nodeVec;
}

void polymorphicTest(size_t vecSize) {
  int datum = 2252;
  size_t i = 0;
  size_t j = 0;
  int minKey = 1;
  int maxKey = minKey + InternalNode<int, int*>::keyptr_pairs - 1;
  {
    vector< std::unique_ptr< BaseNode<int, int*> > > nodeVec = generatePolymorphicVec<int, int*>(minKey, &datum, vecSize);
    for (auto& elem: nodeVec) {
      for(int i=0; i < InternalNode<int, int*>::keyptr_pairs; ++i) {
        // elem->keys[i] = minKey + i;
      }
    }
    //
    cout << "Time to search Polymorphic nodes\n";
    cout << "\n#### InternalNode metadata:\n";
    cout << "InternalNode<int, int*>::max_keyval_pairs = " << InternalNode<int, int*>::max_keyptr_pairs << "\n";
    cout << "sizeof(InternalNode<int, int*>)" << sizeof(InternalNode<int, int*>) << "\n";
    cout << "\n#### LeafNode metadata:\n";
    cout << "LeafNode<int, int*>::max_keyval_pairs = " << LeafNode<int, int*>::max_keyval_pairs << "\n";
    cout << "sizeof(LeafNode<int, int*>)" << sizeof(LeafNode<int, int*>) << "\n";
    cout << "###\n";
    // CLOCK SETUP BLOCK
    std::clock_t start;
    double duration;
    start = std::clock();
    // END CLOCK SETUP BLOCK
    auto curNode = nodeVec[nodeVec.size() - 1].get();
    while (curNode->getData() ==  nullptr) {
      //cout << "Searching...\n";
      curNode = curNode->getNext(maxKey);
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
vector< std::unique_ptr< RuntimeCheckNode<K, T> > > generateRuntimeVec(K key, T datum, size_t vecSize = 10) {
  vector< std::unique_ptr< RuntimeCheckNode<K, T> > > nodeVec;
  nodeVec.push_back(std::make_unique< RuntimeCheckNode<K, T> >(RuntimeCheckNode<K, T>{ .data = {datum}, .keys = {key}, .flags =  IS_LEAF_FLAG  }));
  size_t i = 0;
  while(i < vecSize) {
    nodeVec.push_back(std::make_unique< RuntimeCheckNode<K, T> >(RuntimeCheckNode<K, T>{ .data = {nodeVec[nodeVec.size() - 1].get()}, .keys = {key}, .flags = '\0' }));
    // cout << "Added node #" << i << "\n";
    ++i;
  }
  return nodeVec;
}


void runtimeTest(size_t vecSize) {
  int datum = 2252;
  size_t i = 0;
  size_t j = 0;
  int minKey = 1;
  int maxKey = minKey + RuntimeCheckNode<int, int*>::keyval_pairs -1;
  {
    vector< std::unique_ptr< RuntimeCheckNode<int, int*> > > nodeVec = generateRuntimeVec<int, int*>(1, &datum, vecSize);
    for (auto& elem: nodeVec) {
      for(int i=0; i < RuntimeCheckNode<int, int*>::keyval_pairs; ++i) {
        elem->keys[i] = minKey + i;
        elem->data[i] = elem->data[0];
      }
    }
    cout << "\n### RuntimeCheckNode metadata:\n";
    cout << "RuntimeCheckNode<int, int*>::max_keyval_pairs = " << RuntimeCheckNode<int, int*>::max_keyval_pairs << "\n";
    cout << "sizeof(RuntimeCheckNode<int, int*>)" << sizeof(RuntimeCheckNode<int, int*>) << "\n";
    cout << "###\n";
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
      curNode = curNode->getNext(maxKey);
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
  runtimeTest(1e6);
  polymorphicTest(1e6);
  reportPackingScenarios();
}

