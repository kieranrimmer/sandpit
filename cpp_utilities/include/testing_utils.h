
#ifndef _TESTING_UTILS_H_
#define _TESTING_UTILS_H_

#include <iostream>
#include <chrono>
#include <ctime>


using std::cout;


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


#endif