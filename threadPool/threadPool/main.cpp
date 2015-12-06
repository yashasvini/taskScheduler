//
//  main.cpp
//  threadPool
//
//  Created by yashasvi on 8/21/15.
//  Copyright (c) 2015 yashasvi. All rights reserved.
//

#include <iostream>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <future>


#include <deque>


class lowerSetTime;

using namespace std;
class Task{
public:
    Task(string st,int timeInsecs=0){s=st;set_time(timeInsecs);}
    virtual void run(){
        printf("Hello!! from task and the string is %s \n",s.c_str());
      //  sleep(5);
        
    };
    virtual time_t get_time(){ return time_value;}
    virtual void set_time(int timeInSecs){ time_value = time_t(time(NULL)+timeInSecs);}
    Task(const Task &t){
        printf("Calling copy ctor\n");
        this->s=t.s;
        this->time_value=t.time_value;
    }
    Task& operator=( Task &t){
        printf("calling copy assignment\n");
        Task &temp(t);
        return temp;
    }
    public:
    string s;
    time_t time_value;
};

class Task1:public Task{
    
public:
    Task1(string s,int timeInSecs):Task(s){ set_time(timeInSecs);}
     void run(){
        printf("Hello!! from task1 and the string is %s\n",s.c_str());
         //sleep(10);
        
    };
    void set_time(int timeInSecs){ time_value = time_t(time(NULL)+timeInSecs);}
    time_t get_time(){ return time_value;}
    Task1(const Task1 &t):Task(t){}
 //   Task1& operator==(Task &t):Task(t){}


};

class lowerSetTime{
public:
    bool operator()( Task* t1, Task* t2){
        return (t1->get_time()> t2->get_time());
    }
};

class Worker;

class threadPool{
public:
    threadPool(int size);
    ~threadPool();
    void queueTask(Task *t);
        void lockQueue(){
        queueMutex.lock();
    }
    
 /*   void condiontalWait(){
        cond_variable.wait(queueMutex);
    }*/
    
    void notifyAll(){
        cond_variable.notify_all();
    }
    bool isStop(){return stop;}
    void stopPool(){
        stop = true;
    }
    
private:
    friend class Worker;
    
    priority_queue<Task*,deque<Task*>,lowerSetTime> taskList;
    //deque<Task*> taskList;
    vector<thread> workerThreads;

    int poolSize;
    mutex queueMutex;
    condition_variable cond_variable;
    bool stop;
    time_t myTime;
    
    
};


class Worker{
public:
    Worker(threadPool &s):tp(s){}
    void operator()();
    
private:
    threadPool &tp;
};


void Worker::operator()(){
    while(true){
        printf("\n\n\nHEre the thread id is %lld  and task size is %d \n\n",this_thread::get_id(),tp.taskList.size());
        Task* t=NULL;
        {
            unique_lock<mutex> lock(tp.queueMutex);
            // tp.lockQueue();
            // look for tasks
            while(!tp.isStop() && tp.taskList.empty()){
                    // no task, wait
    
                tp.cond_variable.wait(lock);
            }
            if(tp.isStop()){
                return;
            }
            
            bool erased=false;
            
            /*deque<Task*>::iterator it;
            for(it = tp.taskList.begin();it!=tp.taskList.end();){
                t=*it;
              //printf("WORKER: inside worker task  is:%s\n",t->s.c_str());
                if((t->get_time()-time(NULL))<=0){
                   printf("\npopping task with value %s\n",t->s.c_str());
                 //  tp.taskList.pop_front();
                    tp.taskList.erase(it);
                    erased=true;
                    break;
                }
                else{
                    it++;
                }
            }
          */
            t= tp.taskList.top();
            if((t->get_time()-time(NULL))<=0){
                printf("\n popping task with value %s\n",t->s.c_str());
                tp.taskList.pop();
                erased=true;
            }
            
            //if(it == tp.taskList.end()&& erased==false){
            if(erased==false){
              //  printf("going for a cond wait!\n");
                t = NULL;
                tp.cond_variable.wait_for(lock,std::chrono::seconds(5));
                printf("got up from 5 sec wait\n");
                continue;
            }
        }// release lock
               if(t!=NULL){
            printf("calling run task with value %s\n",t->s.c_str());
            t->run();
        }
    }
}

threadPool::threadPool(int poolSize):stop(false){
    myTime = time(NULL);
    for(int i=0;i<poolSize;i++){
        Worker w(*this);
        workerThreads.push_back(thread(w));
    }
}

threadPool::~threadPool(){
   // stop = true;
    notifyAll();
    for(int i=0;i<poolSize;i++){
        workerThreads[i].join();
    }
    
}

void threadPool::queueTask(Task* t){
    {
        unique_lock<mutex> lock(queueMutex);
       // taskList.push_back(t);
        taskList.push(t);
        deque<Task*>::iterator it;
     /*   for(it = taskList.begin();it!=taskList.end();it++){
            Task* temp=taskList.front();
     //       printf(" \nthe current time is :%lld and task s time is:%lld \n",time(NULL),temp->get_time());
        }*/
        
        //cout<<endl<<"inside queue task, time is :"<<t->get_time()<<endl;
    }// release the lock
    cond_variable.notify_one();
}


int main(int argc, const char * argv[]) {
    threadPool *pool1= new threadPool(3);
    Task* t1(new Task("first task"));
    pool1->queueTask(t1);
    Task* t2(new Task1("second task",15));
  // printf("\n main time before task2 is %lld, task t2 time is %lld and time after that is %lld\n",time(NULL),t2->get_time(),time(NULL));
  //  printf("the time from task t2 is %lld\n",t2->get_time());
 
    Task* t3(new Task1("third task",30));
    Task* t4(new Task1("fourth task",20));
    Task* t5(new Task1("fifth task",5));
    Task* t6(new Task1("sixth task",1));
    Task* t7(new Task1("seventh task",4));
    Task* t8(new Task1("eigth task",6));
    Task* t9(new Task1("ninth task",7));
    pool1->queueTask(t2);
    pool1->queueTask(t3);
    pool1->queueTask(t4);
    pool1->queueTask(t5);
    pool1->queueTask(t6);
    pool1->queueTask(t7);
    pool1->queueTask(t8);
    pool1->queueTask(t9);
    
    sleep(300);

   // pool1->stopPool();
    delete pool1;
       return 0;
}
