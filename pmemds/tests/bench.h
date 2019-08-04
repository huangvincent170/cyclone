#ifndef PMEMDS_BENCH_H
#define PMEMDS_BENCH_H

#include <pthread.h>

typedef struct stats_t_ {
    double start_;
    double finish_;
    double elapsed_time_;
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
    Random rand;         // Has different seeds for different threads
    stats_t stats;
    shared_state_t *shared;

    thread_state_t_(int index)
            : tid(index),
              rand(1000 + index) {
    }
} thread_state_t;

typedef struct thread_arg_t {
    Benchmark *bm;
    SharedState *shared;
    ThreadState *thread;
    void (*worker)(void *engine, thread_arg_t *,thread_state_t *);
}thread_arg_t;

struct Benchmark {

    Benchmark(){
    }

    ~Benchmark() {
    }

    static void ThreadBody(void *v) {
        thread_arg_t *arg = reinterpret_cast<thread_arg_t *>(v);
        shared_state_t *shared = arg->shared;
        thread_state_t *thread = arg->thread;
        {
            pthread_mutex_lock(&shared->mu);
            shared->num_initialized++;
            if (shared->num_initialized >= shared->total) {
                pthread_cond_broadcast(&shared->cv);
            }
            while (!shared->start) {
                pthread_cond_wait(&shared->cv,&shared->mu);
            }
        }

        thread->stats.Start();
        (arg->bm->*(arg->method))(thread);
        thread->stats.Stop();

        {
            pthread_mutex_lock(&shared->mu);
            shared->num_done++;
            if (shared->num_done >= shared->total) {
                pthread_cond_broadcast(&shared->cv);
            }
        }
    }

    void run(int n,void (*bw)(void *engine, thread_arg_t *,thread_state_t *)) {
        shared_state_t shared;
        shared.total = n;
        shared.num_initialized = 0;
        shared.num_done = 0;

        ThreadArg *arg = new ThreadArg[n];
        for (int i = 0; i < n; i++) {
            arg[i].bm = this;
            arg[i].method = method;
            arg[i].shared = &shared;
            arg[i].thread = new ThreadState(i);
            arg[i].thread->shared = &shared;
            g_env->StartThread(ThreadBody, &arg[i]);
        }

        pthread_mutex_lock(&shared.mu);
        while (shared.num_initialized < n) {
            pthread_cond_wait(&shared.cv,&shared.mu);
        }

        shared.start = true;
        pthread_cond_broadcast(&shared.cv);
        while (shared.num_done < n) {
            pthread_cond_wait(&shared.cv);
        }

        pthread_mutex_unlock(&shared.mu);

        for (int i = 1; i < n; i++) {
            arg[0].thread->stats.Merge(arg[i].thread->stats);
        }
        arg[0].thread->stats.Report(name);

        for (int i = 0; i < n; i++) {
            delete arg[i].thread;
        }
        delete[] arg;
    }

};

#endif
