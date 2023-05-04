#include <unistd.h>
#include <assert.h>
# include <stdio.h>
#include <sys/time.h>
#include <csignal>
#include <iostream>

#include "uthreads.h" // TODO: If you get an error of "undefined reference to..." try to replace it with #include "uthreads.cpp"
//#include "uthreads.cpp"

# define FAILURE (-1)
# define SUCCESS 0

using namespace std;


int MAIN_THREAD = 0;
int QUANTOM = 1000000;

void send_sigalarm(){
    kill(getpid(),SIGVTALRM);
}


void kill_yourself_entry_point(){
    uthread_terminate(uthread_get_tid());
}

void test_init(){
    uthread_init(QUANTOM);
    assert(uthread_get_tid() == MAIN_THREAD);
    printf("Passed Init Test!\n");
}

void entry_point_spawn(){
    assert(uthread_get_tid() == 1);
    uthread_terminate(uthread_get_tid());
}

void test_spawn(){
    uthread_spawn(entry_point_spawn);
    assert(uthread_get_tid()==MAIN_THREAD);
    send_sigalarm();
    printf("Passed Spawn Test!\n");
}

void entry_point_timer(){
    timeval start;
    timeval end;
    gettimeofday(&start, nullptr);

    while (true){
        gettimeofday(&end, nullptr);
        if(end.tv_sec - start.tv_sec > (QUANTOM / 1000000) + 1){
            cout << "stack in a loop"<< endl;
            uthread_terminate(uthread_get_tid());
        }
    }
}

void test_timer(){
    uthread_spawn(entry_point_timer);
    send_sigalarm();
    assert(uthread_get_tid() == 0);
    uthread_terminate(1);
    printf("Passed Timer Test!\n");
}

void test_spawn_too_many(){
    for (int i=1;i<=99;++i){
        assert(uthread_spawn(entry_point_timer) == i);
    }
    assert(uthread_spawn(entry_point_timer) == -1);
    for (int i=1;i<=99;++i){
        assert(uthread_terminate(i) == SUCCESS);
    }
    printf("Passed spawn_too_many Test!\n");
}

void test_spawn_thread_gets_minimal_tid(){
    // jump to 1 -> 1 will kill himself -> jump to 2 -> 2 will kill himself -> jump to 3 -> 3 will create new thread -> check that new thread_id==1
    // 3 will kill himself
    uthread_spawn(kill_yourself_entry_point); // tid 1
    uthread_spawn(kill_yourself_entry_point); // tid 2
    uthread_spawn(kill_yourself_entry_point); // tid 3
    send_sigalarm();  // jump to tid 1
    assert(uthread_spawn(entry_point_spawn) == 1); // should create new thread with tid 1
    send_sigalarm();  // jump to tid 1
    printf("Passed gets minimal tid Test!\n");
}

void block_entry_point(){
    assert(uthread_get_tid() == 2);
    uthread_terminate(2);
}

void test_block(){
    uthread_spawn(block_entry_point); // tid 1
    uthread_spawn(block_entry_point); // tid 2
    uthread_block(1);
    send_sigalarm();
    assert(uthread_get_tid() == 0);
    uthread_terminate(1);
    printf("Passed Block Test!\n");
}

void resume_entry_point(){
    assert(uthread_get_tid() == 2);
    assert(uthread_resume(1) == SUCCESS);
    uthread_terminate(2);
}

void test_resume(){
    uthread_spawn(kill_yourself_entry_point); // tid 1
    uthread_spawn(resume_entry_point); // tid 2
    uthread_block(1);
    send_sigalarm(); // should jump to resume_entry_point should jump to tid 2
    assert(uthread_get_tid() == 0);
    send_sigalarm(); // should jump to 1 and terminate it self
    printf("Passed Resume Test!\n");
}

void sleep_entrypoint(){
    assert(uthread_get_tid() == 1);
    uthread_sleep(2);
    assert(uthread_get_quantums(1) == 2);
    uthread_terminate(1);
}

void test_sleep(){
    uthread_spawn(sleep_entrypoint); // tid 1
    uthread_spawn(kill_yourself_entry_point); // tid 2
    send_sigalarm(); //  should jump to 1 and it will send itself to sleep
    send_sigalarm(); // wait for sleeping time to pass
    send_sigalarm(); // jump back to thread 1 -> it will terminate itself
    assert(uthread_get_tid() == 0);
    printf("Passed Basic Sleep Test!\n");
}

void test_block_sleeping_thread(){
    // blocking a sleeping thread, after the sleeping period passed, it should still be blocked

    uthread_spawn(sleep_entrypoint); // tid 1
    send_sigalarm(); // jump to tid 1 -> tid1 will go to sleep -> go back here
    uthread_block(1);
    send_sigalarm(); // wait for sleeping time to pass
    send_sigalarm(); // wait for sleeping time to pass
    assert(uthread_get_quantums(1) == 1); // check that tid 1 doesn't run more than once
    uthread_resume(1);
    assert(uthread_get_tid() == 0);
    printf("Passed Block Sleeping thread Test!\n");
}

void test_send_main_thread_to_sleep(){
    assert(uthread_sleep(2) == FAILURE);
    printf("Passed Send Main Thread To Sleep Test!\n");
}

int main(){
    test_init();
    test_timer();
    test_spawn();
    test_spawn_thread_gets_minimal_tid();
    test_spawn_too_many();
    test_block();
    test_resume();
    test_sleep();
    test_block_sleeping_thread();
    test_send_main_thread_to_sleep();
    cout << "There should be 2 library error messages" << endl;
    uthread_terminate(0);
}



