// A sample test program for the thread library.

#include <iostream>
#include <cstdlib>
#include <stdint.h>
#include "thread.h"

using namespace std;

int g = 0; // global (shared by all threads)

void loop(void* a) {
    char* id = (char*) a;
    cout << "loop called with id " << id << endl;

    for (int i = 0; i < 5; i++, g++) {
      cout << id << ":\t" << i << "\t" << g << endl;
      if (thread_yield()) {
          cout << "thread_yield failed\n";
          exit(1);
      }
    }
}

void parent(void* a) {
    int arg = (intptr_t) a;
    cout << "parent called with arg " << arg << endl;

    if (thread_create((thread_startfunc_t) loop, (void*) "child thread")) {
      cout << "thread_create failed\n";
      exit(1);
    }

    loop((void*) "parent thread");
}

int main() {
    //start_preemptions(false, true, 2);
    if (thread_libinit((thread_startfunc_t) parent, (void*) 100)) {
      cout << "thread_libinit failed\n";
      exit(1);
    }
}
