#ifndef PMEMDS_BENCH_H
#define PMEMDS_BENCH_H

#include <pthread.h>
#include "random.h"
#include "../../core/clock.hpp"

typedef struct stats_t_ {
    double start_;
    double finish_;
    int64_t nops_;
} stats_t;

// State shared by all concurrent executions of the same benchmark.
typedef struct shared_state_t_ {
    pthread_mutex_t mu;
    pthread_cond_t cv;
    int total;
    // Each thread goes through the following states:
    //    (1) initializing
    //    (2) waiting for others to be initialized
    //    (3) running
    //    (4) done
    int num_initialized;
    int num_done;
    bool start;
}shared_state_t;

// Per-thread state for concurrent executions of the same benchmark.
typedef struct thread_state_t_ {
    int tid;             // 0..n-1 when running in n threads
    leveldb::Random rand;         // Has different seeds for different threads
    stats_t stats;
    shared_state_t *shared;

    thread_state_t_(int index)
            : tid(index),
              rand(1000 + index) {
    }
} thread_state_t;

typedef struct thread_arg_t {
    shared_state_t *shared;
    thread_state_t *thread;
    void *engine;
    void (*worker)(void *engine,thread_state_t *);
}thread_arg_t;


void merge(stats_t &prime, const stats_t &other) {
    prime.nops_ += other.nops_;
    if (other.start_ < prime.start_) prime.start_ = other.start_;
    if (other.finish_ > prime.finish_) prime.finish_ = other.finish_;
}


void report(stats_t &merged_stats) {
    unsigned long elapsed_time = merged_stats.finish_ - merged_stats.start_;
    fprintf(stdout, "stats : %.0f ops/sec\n",
            (double)(merged_stats.nops_ * 1e+6)/ elapsed_time);

    fflush(stdout);
}



struct Benchmark {

    Benchmark(){
    }

    ~Benchmark() {
    }

    static void* thread_body(void *v) {
        thread_arg_t *arg = reinterpret_cast<thread_arg_t *>(v);
        shared_state_t *shared = arg->shared;
        thread_state_t *thread = arg->thread;

            pthread_mutex_lock(&shared->mu);
            shared->num_initialized++;
            if (shared->num_initialized >= shared->total) {
                pthread_cond_broadcast(&shared->cv);
            }
            while (!shared->start) {
                pthread_cond_wait(&shared->cv,&shared->mu);
            }
            pthread_mutex_unlock(&shared->mu);


            thread->stats.nops_ = 0;
            /* real work happens here */
            thread->stats.start_ = rtc_clock::current_time();
            arg->worker(arg->engine, thread);
            thread->stats.finish_ = rtc_clock::current_time();


            pthread_mutex_lock(&shared->mu);
            shared->num_done++;
            if (shared->num_done >= shared->total) {
                pthread_cond_broadcast(&shared->cv);
            }
            pthread_mutex_unlock(&shared->mu);


        pthread_exit(NULL);
    }

    void run(int n, void *engine, void (*worker)(void *engine,thread_state_t *)) {
        shared_state_t shared;
        shared.total = n;
        shared.num_initialized = 0;
        shared.num_done = 0;
        shared.start = false;


        thread_arg_t *arg = new thread_arg_t[n];
        pthread_t *threads = new pthread_t[n];

        /* Initialize mutex and condition variable objects */
        pthread_mutex_init(&shared.mu, NULL);
        pthread_cond_init (&shared.cv, NULL);

        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

        for (int i = 0; i < n; i++) {
            arg[i].worker = worker;
            arg[i].engine = engine;
            arg[i].shared = &shared;
            arg[i].thread = new thread_state_t(i);
            arg[i].thread->shared = &shared;
            pthread_create(&threads[i],&attr,thread_body, (void*)(arg + i));
        }

        pthread_mutex_lock(&shared.mu);
        while (shared.num_initialized < n) {
            pthread_cond_wait(&shared.cv,&shared.mu);
        }

        shared.start = true;
        pthread_cond_broadcast(&shared.cv);

        while (shared.num_done < n) {
            pthread_cond_wait(&shared.cv,&shared.mu);
        }
        pthread_mutex_unlock(&shared.mu);


        pthread_mutex_destroy(&shared.mu);
        pthread_cond_destroy(&shared.cv);

        for (int i = 1; i < n; i++) {
            merge(arg[0].thread->stats,arg[i].thread->stats);
        }


        report(arg[0].thread->stats);

        for (int i = 0; i < n; i++) {
            delete arg[i].thread;
        }
        delete[] threads;
        delete[] arg;
    }

};
#endif
