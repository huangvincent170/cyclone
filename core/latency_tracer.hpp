#ifndef __LATENCY_TRACER_HPP
#define __LATENCY_TRACER_HPP

enum trcekey{
	trcekey_raft=0,
	trcekey_oplog,
	trcekey_app_wr,
	trcekey_app_ro,
	trcekey_other,
	MAX_LT_TRACE
};

#if !defined (__LATENCY_TRACER)

#define LT_INIT_RUNTIME()
#define LT_START(key, rpc)
#define LT_PAUSE(key, rpc)
#define LT_END(key, rpc)

#define LT_LOCAL_START_BATCH(key, count)
#define LT_LOCAL_END_BATCH(key, count)

#define LT_PRINT() 

#else

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include<sys/time.h>

static const char *print_headers[MAX_LT_TRACE] = 
{ "RAFT",
	"OPLOG",
	"APP_WR",
	"APP_RO",
	"OTHER",
};

enum lt_state{
	start=0,
	stop,
	MAX_LT_STATE
};

typedef struct lt_counter_{
	unsigned long ttime; //total time
	unsigned long stime; // start time for local counters
	unsigned long count;
	unsigned int batch_count; // keeping track of how many message when batched timing
	int state;
}lt_counter_t;

extern struct lt_tracer_ ltracer;

static unsigned long tracer_current_time(){
    struct timeval tm; 
    gettimeofday(&tm, NULL);
    return tm.tv_sec*1000000 + tm.tv_usec;
}

static uint64_t get_cpu_freq(void)
{
    FILE *fd;
    uint64_t freq = 0;
    float freqf = 0;
    char *line = NULL;
    size_t len = 0;

    fd = fopen("/proc/cpuinfo", "r");
    if (!fd) {
        fprintf(stderr, "failed to get cpu frequecy\n");
        perror(NULL);
        return freq;
    }

    while (getline(&line, &len, fd) != EOF) {
        if (sscanf(line, "cpu MHz\t: %f", &freqf) == 1) {
            freqf = freqf * 1000000UL;
            freq = (uint64_t)freqf;
            break;
        }
    }

    fclose(fd);
    return freq;
}

static uint64_t read_tsc(void)
{
    uint32_t a, d;
    __asm __volatile("rdtsc" : "=a" (a), "=d" (d));
    return ((uint64_t) a) | (((uint64_t) d) << 32);
}




#define LT_INIT_RUNTIME() do{ \
														ltracer.init_tracer();\
														}while(0)

#define LT_START(key, rpc) ltracer.start_counter(key, rpc)
#define LT_PAUSE(key, rpc) ltracer.pause_counter(key, rpc)
#define LT_END(key, rpc) ltracer.end_counter(key, rpc)

#define LT_LOCAL_START_BATCH(key, count)  ltracer.start_local_counter(key,count)
#define LT_LOCAL_END_BATCH(key, count)  ltracer.end_local_counter(key,count)

#define LT_PRINT() ltracer.print_trace()


typedef struct lt_tracer_{
	lt_counter_t counters[MAX_LT_TRACE];

	void init_tracer(){
		int i;
		for(i=0; i < MAX_LT_TRACE; i++){
			counters[i].ttime = 0;
			counters[i].stime = 0;
			counters[i].count = 0;
			counters[i].batch_count = 1;
		}
		return;
	}

	int start_counter(enum trcekey key, struct rpc_st *rpc){
		if(key >= MAX_LT_TRACE){
			printf("invalid counter key\n");
			exit(1);
		}	
		rpc->timestamp =  tracer_current_time();
		return 0;
	}

	int pause_counter(enum trcekey key, struct rpc_st *rpc){
		if(key >= MAX_LT_TRACE){
			printf("invalid counter key\n");
			exit(1);
		}	
		unsigned long elapsed_time = tracer_current_time() - rpc->timestamp;
		counters[key].ttime += elapsed_time;
		return 0;
	}


	int end_counter(enum trcekey key, struct rpc_st *rpc){
		if(key >= MAX_LT_TRACE){
			printf("invalid counter key\n");
			exit(1);
		}	
		unsigned long elapsed_time = tracer_current_time() - rpc->timestamp;
		counters[key].ttime += elapsed_time;
		counters[key].count++;
		return 0;
	}

	int start_local_counter(enum trcekey key, int count){
		if(key >= MAX_LT_TRACE){
			printf("invalid counter key\n");
			exit(1);
		}	
		counters[key].stime =  tracer_current_time();
		return 0;
	}

	int end_local_counter(enum trcekey key, int count){
		if(key >= MAX_LT_TRACE){
			printf("invalid counter key\n");
			exit(1);
		}	
		unsigned long elapsed_time = tracer_current_time() - counters[key].stime;
		counters[key].ttime += elapsed_time;
		counters[key].count += count;
		return 0;
	}


	void print_trace(){
		float time[MAX_LT_TRACE];
		int i;
		for(i=0; i < MAX_LT_TRACE;i ++){
			time[i] = (counters[i].count == 0) ? 0 : 
				counters[i].ttime/((float)counters[i].count);
		}
			printf( "%s : %.5f  "
					"%s : %.5f  "
					"%s : %.5f  "
					"%s : %.5f  "
					"%s : %.5f  "
					"\n" , 
					print_headers[0], time[0],
					print_headers[1], time[1],
					print_headers[2], time[2],
					print_headers[3], time[3],
					print_headers[4], time[4]); 

			printf( "%s : %lu , %lu  "
					"%s : %lu , %lu  "
					"%s : %lu , %lu  "
					"%s : %lu , %lu  "
					"%s : %lu , %lu  "
					"\n" , 
					print_headers[0], counters[0].ttime,counters[0].count,
					print_headers[1], counters[1].ttime,counters[1].count,
					print_headers[2], counters[2].ttime,counters[2].count,
					print_headers[3], counters[3].ttime,counters[3].count,
					print_headers[4], counters[4].ttime,counters[4].count);

		init_tracer();
		return;
	}

}lt_tracer_t;



#endif
#endif
