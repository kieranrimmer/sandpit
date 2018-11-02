
#include <iostream>
#include <atomic>
#include <thread>
#include <chrono>
#include <sstream>

using std::cout;
using std::stringstream;
using std::string;


std::atomic_bool simpleLock(false);

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


void lockRaceTestAndSet() {
  std::thread threadOne(testAndSetTest, 1);
  std::thread threadTwo(testAndSetTest, 2);
  std::thread threadThree(testAndSetTest, 55);
  threadOne.join();
  threadTwo.join();
  threadThree.join();

}

int main() {
  lockRaceTestAndSet();
}
