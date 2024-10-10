// disk.cc - simulation of a concurrent disk scheduler
// Ross And Ryan

#include <fstream>
#include <iostream>
#include <list>
#include <queue>
#include "thread.h"

#define MAX_TRACK_VALUE 999
using namespace std;

int disk_tracks[5][2] = {
        {53, 785},
        {914,350},
        {827, 567},
        {302,230},
        {631,11}};

// Class allowing all request info to be stored in one queue
class Request {

private:
    int requester_;
    int track_;

public:
    Request(int requester, int track) : requester_(requester), track_(track) {}

    int getRequester() const { return requester_; }
    int getTrack() const { return track_; }

    // Added comparison operator to work with list search
    bool operator==(const Request& rhs) const {
        return requester_ == rhs.requester_ && track_ == rhs.track_;
    }
};

// Class that simulates the disk itself. Hold the queue and executes requests. Used by scheduler to execute requests.
class Disk{

private:
    int current_loc = 0;
    const int queue_size;
    list<Request> requestQueue;
    queue<int> waitingQueue;

public:
    int readers_active = 0;
    const unsigned queue_edit_lock = 1;
    // As all thread cvs are positive before turned to unsigned
    const unsigned request_ready_cond = 0;

    Disk(int inputted_queue_size): queue_size(inputted_queue_size){}

    bool addRequestToQueue(int thread_num, int track) {
        //Add request to request waiting queue
        waitingQueue.push(thread_num);
        // Wait loop due to mesa semantics
        while((int)requestQueue.size() >= queue_size){
            // Wait for request to be on top of waiting queue
            // Only ever odd numbers used for waiting_queue cv
            // Evens are used for requestQueue cv
            // Excludes zero which is request_ready_cond
            // See Hilbert's Hotel. Allows for max number of threads defined by u_integer type without collision
            thread_wait(queue_edit_lock, 1 + 2 * (unsigned) thread_num );
        }
        //execute request
        cout << "requester " << thread_num << " track " << track << endl;
        // Add request to scheduler queue
        requestQueue.emplace_back(thread_num, track);
        // Remove from scheduler queue
        waitingQueue.pop();
        return true;
    }

    bool fulfillRequest(){
        //Wait while queue is not full or equal to number of threads
        while ((int) requestQueue.size() < readers_active && (int) requestQueue.size() < queue_size) {
            thread_wait(queue_edit_lock, request_ready_cond);
        }
        //Solve condition where all threads exit while waiting
        if(!requestQueue.size() && !readers_active){
            return false;
        }
        // Initialize default values for search
        auto closest_request = requestQueue.begin();
        // Use max track value as no distance will be greater
        int smallestDistance = MAX_TRACK_VALUE;
        // Search for closest
        for (auto iter = requestQueue.begin(); iter != requestQueue.end(); ++iter) {
            int distance = abs(iter->getTrack() - current_loc);
            if (distance < smallestDistance) {
                closest_request = iter;
                smallestDistance = distance;
            }
        }
        // Service the request
	    current_loc = closest_request->getTrack();
	    cout << "service requester " << closest_request->getRequester() << " track " << closest_request->getTrack() << endl;
        requestQueue.erase(closest_request);

        // Wake thread on top of waiting queue to add request to request q
        if(!waitingQueue.empty()) {
            thread_signal(queue_edit_lock,   1 + 2 * (unsigned)waitingQueue.front() );
        }
        // Wake requester thread
        thread_signal(queue_edit_lock,   2 + 2 * (unsigned) closest_request->getRequester());
        thread_unlock(queue_edit_lock);
        return true;
    }

};

// Struct to make initializing threads easier. Holds all parameters for reader thread init.
struct readerParams{
	Disk* disk_addr;
	int thread_num;
	int filename;
};

// Thread that reads files and submits requests to scheduler
void readFile(void* params ){
    // Unpack parameters
    auto* p = static_cast<readerParams*>(params);
    unsigned int lock_id = p->disk_addr->queue_edit_lock;
    // Open file

    thread_lock(lock_id);
    // Iterate through file and make requests
    for(long unsigned int j =0; j < 2; j++){
        // have disk add request
	    p->disk_addr->addRequestToQueue(p->thread_num, disk_tracks[p->filename][j]);
        // tell scheduler there is another request
        thread_signal(lock_id, p->disk_addr->request_ready_cond);
        // Wait till request is fulfilled
        thread_wait(lock_id,  2 + 2 * (unsigned) p->thread_num);
    }
    // Begin exiting thread
    p->disk_addr->readers_active--;
    // Tel scheduler that it may be possible to run, as thread num changed
    thread_signal(lock_id, p->disk_addr->request_ready_cond);
    delete p;
    thread_unlock(lock_id);
}

// Initializes reader threads then starts servicing jobs submitted by readers
void scheduler(void* argv){
    //start_preemptions(false, true, 0);
    // unpack arguments
    //auto** arguments = static_cast<char**>(argv);
    int queue_size = 3;
    // Create object simulating disk stipulating the size of the queue
    Disk outDisk(queue_size);

    // Start threads for each file stipulated
     for (int i = 0; i < 5; i++) {
         //Create a parameter object and load
        auto* params = new readerParams{&outDisk, i,i};
        // start file thread
        thread_create(readFile, params);
        thread_lock(outDisk.queue_edit_lock);
        // Increment threads active
        outDisk.readers_active++;
        thread_unlock(outDisk.queue_edit_lock);
    }

    // Begin scheduling while there are active readers
    while (outDisk.readers_active > 0) {
        thread_lock(outDisk.queue_edit_lock);
        if(outDisk.readers_active > 0){
            outDisk.fulfillRequest();
        }
        thread_unlock(outDisk.queue_edit_lock);
    }

    
}

// Basic help
void print_usage(){
    cout << "Usage: ./disk [queue size] [file 1] [file 2] ...";
    exit(1);
}

int main(int argc, char** argv) {
    // Check for right number of commands

    // Start scheduler

    if (thread_libinit((thread_startfunc_t) scheduler, (void*) argv )) {
        cerr << "thread_libinit failed" << endl;
        return 1;
    }
    return 0; 
}
