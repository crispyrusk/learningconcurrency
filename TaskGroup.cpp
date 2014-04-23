#include "tbb/task_group.h"
#include <iostream>

// observe parallel watch - so thread and recursion depth
// thread watch to understand how task_group works
using namespace tbb;

/*
http://software.intel.com/en-us/articles/using-tasks-instead-of-threads
Intel TBB and the OpenMP API manage task scheduling through work stealing. 
In work stealing, each thread in the thread pool maintains a local task pool 
that is organized as a deque (double-ended queue). A thread uses its own task 
pool like a stack, pushing new tasks that it spawns onto the top of this stack. 
When a thread finishes executing a task, it first tries to pop a task from 
the top of its local stack. The task on the top of the stack is the newest 
and therefore most likely to access data that is hot in its data cache. If
there are no tasks in its local task pool, however, it attempts to steal 
work from another thread (the victim). When stealing, a thread uses the 
victim’s deque like a queue so that it steals the oldest task from the victim’s 
deque. For recursive algorithms, these older tasks are nodes that are high 
in the task tree and therefore are large chunks of work, often work that is 
not hot in the victim’s data cache. Therefore, work stealing is an effective 
mechanism for balancing load while maintaining cache locality.
*/

// in this example, both the master thread
// and worker threads create several tasks
// so the threads can steal from each other
int Fib(int n) 
{
  if( n<2 ) 
  {
    return n;
  } 
  else 
  {
    int x, y;
    task_group g;
    g.run([&]{x=Fib(n-1);}); // spawn a task
    g.run([&]{y=Fib(n-2);}); // spawn another task
    g.wait();                // wait for both tasks to complete
    return x+y;
  }
}

/*
http://software.intel.com/en-us/blogs/2010/05/07/have-we-made-task_group-a-little-too-easy-to-use
*/

// the main thread holds all the task
// so worker threads have to steal from the main thread
// if the try to steal from another worker thread, it will be unsuccessful
// parallelism is not exploited well in this poorExample
// example when n is large and say we have only 4 threads
// there is one master thread whose task queue contains all the n task
// the three other threads will try to randomly steal from each other 
// and the master thread. So there is only 1/3 hit of getting a task
// so even the system is not well load balanced due to the random
// stealing

// it would be better to use parallel_for in this example

// take-away :
// task_group is a good use-case when task created by
// the master thread (and are stolen by worker threads) create 
// tasks on their own (like in the prev. example), so that the
// worker thread task queues also get filled up
// Another use-case is for tasks that dont create other task
// if the task themselves are very heavy, then other overhead in
// searching (1/3 success in the previous example) and 
// stealing task would become negligible
void poorExample(int n) 
{
  task_group g;
  for(int i=0; i<n; ++i) 
  {
    g.run([&]{
      std::cout<< i << std::endl;
    });
  }
  g.wait();
}



int mainTaskGroup() 
{
  std::cout << Fib(10) << std::endl;
  poorExample(10);
}

