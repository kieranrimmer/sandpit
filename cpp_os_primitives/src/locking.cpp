
#include <iostream>
#include <atomic>
#include <thread>
#include <chrono>
#include <sstream>
#include <cassert>
#include <ctime>

using std::cout;
using std::stringstream;
using std::string;

// Globals for CAS lock primitive testing
std::atomic_bool simpleLock(false);
bool cas_bool(false);


// template<typename F, typename ... Args>
// void TestWrapper(F& fn, ) {
  

// }

template<typename F1, typename ... Args>
auto testWrapperBasic(F1 f1, Args && ... args) 
-> decltype(f1(args...)) {
  // CLOCK SETUP BLOCK
  std::clock_t start;
  double duration;
  start = std::clock();
  // END CLOCK SETUP BLOCK
  f1(args...);
  // CLOCK READ BLOCK
  duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
  // END CLOCK READ BLOCK
  // CLOCK REPORT BLOCK
  cout << __func__ << " duration: " << duration << '\n';
  // END CLOCK REPORT BLOCK
  cout << "Test passed!!!\n"; 
}



void testAndSetTest(int arg) {
  stringstream sThread;
  std::this_thread::sleep_for(std::chrono::milliseconds(10 / arg));
  bool falsey = false;
  uint64_t wait_count = 0;
  sThread << "#######\n\nThread with arg = " << arg << " invoked testAndSetTest() \n";
  while (!simpleLock.compare_exchange_weak(falsey, true)) {
    falsey = false;
    ++wait_count;
  }
  sThread << "Thread with arg = " << arg << " underwent busy waiting " << wait_count << " times\n";


  sThread << "Thread with arg = " << arg << " obtained Test And Set lock!!\n";
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  sThread << "Thread with arg = " << arg << " ready to release Test And Set lock!!!\nShall Exit Now!!!\n\n";
  cout << sThread.str();
  simpleLock.store(false);
}

void builtinTestAndSetTest(int arg) {
  stringstream sThread;
  std::this_thread::sleep_for(std::chrono::milliseconds(10 / arg));
  bool falsey = false;
  uint64_t wait_count = 0;
  sThread << "#######\n\nThread with arg = " << arg << " invoked builtinTestAndSetTest() \n";
  while (!__sync_bool_compare_and_swap(&cas_bool, false, true)) {
    // falsey = false;
    ++wait_count;
  }
  sThread << "Thread with arg = " << arg << " underwent busy waiting " << wait_count << " times\n";


  sThread << "Thread with arg = " << arg << " obtained Test And Set lock!!\n";
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  sThread << "Thread with arg = " << arg << " ready to release Test And Set lock!!!\nShall Exit Now!!!\n\n";
  cout << sThread.str();
  __atomic_store_n(&cas_bool, false, __ATOMIC_SEQ_CST);
}


void lockRaceTestAndSet() {

  

  std::thread threadOne(testAndSetTest, 1);
  std::thread threadTwo(testAndSetTest, 2);
  std::thread threadThree(testAndSetTest, 55);
  threadOne.join();
  threadTwo.join();
  threadThree.join();

}

void lockRaceBuiltinTestAndSet() {
  std::thread threadOne(builtinTestAndSetTest, 1);
  std::thread threadTwo(builtinTestAndSetTest, 2);
  std::thread threadThree(builtinTestAndSetTest, 55);
  threadOne.join();
  threadTwo.join();
  threadThree.join();

}

int main() {

  // lockRaceTestAndSet();
  // lockRaceBuiltinTestAndSet();

  testWrapperBasic(lockRaceTestAndSet);
  testWrapperBasic(lockRaceBuiltinTestAndSet);

  

}
