/**
 * Eric Willard
 * CS 3800 Operating Systems
 * Mike Gosnell
 * HW 4
 * 
 * @description
 * This program implements a solution to the dining philosophers problem with 
 * a fork master. I used a custom data type to pass metadata to and from 
 * philosophers and the fork master. The philosopher with send a message to
 * the fork master saying that he is hungry. The fork master will then add the
 * philosopher to the queue. Then it will try to give 2 forks to the next 
 * philosopher in the queue. The philosophers will wait until they get a message 
 * from the fork master (data doesn't matter, they just need a message) which 
 * indicates that they have been given the forks and then they can eat. Then 
 * after they eat, they send a message back to the fork master to let it know 
 * that the 2 forks by him are available now.
 */

#include <cstdlib>
#include <iostream>
#include <cerrno>
#include <unistd.h>
#include "mpi.h"
#include "stdlib.h"
#include "stddef.h"
#include "stdio.h"
#include <chrono>
#include <thread>
#include <queue>

void thinkOrEat(long timeToEat);

#define HUNGRY 0
#define DONE 1

struct message {
  int id;
  int status;
};

int main (int argc, char *argv[]) {
  int id;
  int p;
  MPI::Status status;
  int tag = 1;

  MPI::Init (argc, argv);
  //  Get the number of processes.
  p = MPI::COMM_WORLD.Get_size();
  //  Determine the rank of this process.
  id = MPI::COMM_WORLD.Get_rank();
  /* create a type for struct message */
  const int nitems=2;
  int blocklengths[2] = {1,1};
  MPI_Datatype types[2] = {MPI_INT, MPI_INT};
  MPI_Datatype mpi_message_type;
  MPI_Aint offsets[2];

  offsets[0] = offsetof(message, id);
  offsets[1] = offsetof(message, status);

  MPI_Type_create_struct(nitems, blocklengths, offsets, types, &mpi_message_type);
  MPI_Type_commit(&mpi_message_type);
  /* end of creating data type */

  //Safety check - need at least 2 philosophers to make sense
  if (p < 3) {
	    MPI::Finalize();
	    std::cerr << "Need at least 2 philosophers! Try again" << std::endl;
	    return 1; //non-normal exit
  }

  srand(id + time(NULL)); //ensure different seeds...
  if ( id == 0 )
  {
    message msgIn;
    message msgOut; // dummy variable
    int satisfiedPhilosophers = 0;
    bool hungryPhilosophers = true;
    // queue for philosophers waiting for forks
    std::queue<int> waitingForForks;
    // true for available fork, false for unavailable
    // p - 1 because fork master does not get forks
    bool* forks = new bool[p - 1];
    for(int i = 0; i < p - 1; i++) {
      forks[i] = true;
    }
    while (hungryPhilosophers) {
      MPI::COMM_WORLD.Recv(&msgIn, 1, mpi_message_type, MPI::ANY_SOURCE, tag, status);
      std::cout<< "Receving message from Philo " << msgIn.id << " with status " << msgIn.status << std::endl;
      if(msgIn.status == HUNGRY) {
        // if philosopher is hungry, queue it up to prevent starvation
        // subtract offset of 1 so that the forks he is using can be calculated correctly
        // because ombudsman in 0 and the array starts at 0
        waitingForForks.push(msgIn.id - 1);
      } else if (msgIn.status == DONE) {
        // if the philosopher is returning its forks
        satisfiedPhilosophers++;
        msgIn.id--;
        int leftFork = ((msgIn.id - 1) < 0 ? (p - 2) : (msgIn.id - 1));
        int rightFork = msgIn.id;
        if (!forks[leftFork] && !forks[rightFork]) {
          // change values back to true for available
          forks[leftFork] = true;
          forks[rightFork] = true;
        } else {
          // should never get here...
          std::cerr << "For some reason one of the forks is already there." << std::endl;
          std::cerr << "Philo: " << msgIn.id << "| Left Fork : " << forks[leftFork] << ", Right Fork : " << forks[rightFork] << std::endl;
        }
      }
      // if anyone else is left in the queue
      if (!waitingForForks.empty()) {
        int id = waitingForForks.front();
        int leftFork = ((id - 1) < 0 ? (p - 2) : (id - 1));
        int rightFork = id;
        // check if both their forks are there
        if (forks[leftFork] && forks[rightFork]) {
          // remove him from the queue and make his forks unavailable
          waitingForForks.pop();
          forks[leftFork] = false;
          forks[rightFork] = false;
          std::cout << "Sending forks to Philo " << (++id) << std::endl;
          // letting philosoper know he can eat
          MPI::COMM_WORLD.Send(&msgOut, 1, mpi_message_type, id, tag);
        } else {
          std::cout << "One or both of the forks are unavailable for Philo " << (msgIn.id) << ", waiting until fork(s) are available." << std::endl;
        }
      }
      // Each philosopher only eats once, so when they have all eaten, end the program
      if (satisfiedPhilosophers >= (p - 1)) {
        hungryPhilosophers = false;
        std::cout << "All philosophers are satisfied." << std::endl;
      }
    }
    delete [] forks;
  }
  else
  {
    message msgIn; // dummy variable
    message msgOut;
    // think for a random amount of time
    thinkOrEat(rand() % 5001 + 1000);
    msgOut.id = id;
    msgOut.status = HUNGRY;
    std::cout << "Philo " << msgOut.id << " send status of " << msgOut.status << std::endl;
    // Lets ombudsman know he is hungry
    MPI::COMM_WORLD.Send(&msgOut, 1, mpi_message_type, 0, tag);
    // Waiting to receive message from ombudsman
    MPI::COMM_WORLD.Recv(&msgIn, 1, mpi_message_type, MPI::ANY_SOURCE, tag, status);
    // After receiving a message from ombudsman, it knows to eat
    std::cout << "Philo " << id << " received his forks, now is eating" << std::endl;
    // Eat for a random amount of time
    thinkOrEat(rand() % 5001 + 1000);
    // set status to done
    msgOut.status = DONE;
    std::cout << "Philo " << id << " is done eating, letting ombudsman know." << std::endl;
    // let ombudsman know it is done eating
    MPI::COMM_WORLD.Send(&msgOut, 1, mpi_message_type, 0, tag);
  }

  MPI::Finalize();
  return 0;
}

void thinkOrEat(long timeToEat) {
    std::this_thread::sleep_for(std::chrono::milliseconds(timeToEat));
}
