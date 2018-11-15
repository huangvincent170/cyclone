#ifndef __LATENCY_TRACER_HPP
#define __LATENCY_TRACER_HPP

enum trace_keys{
	raft=0,
	op_log,
	app_wr,
	app_ro,
	queues,
	MAX_LT_TRACE
};

#if !defined (__LATENCY_TRACER)

#define LT_INIT_RUNTIME()
#define LT_START(key, rpc)
#define LT_END(key, rpc)
#define LT_PRINT() 

#else

#include <stdlib.h>
#include <stdio.h>

#include "clock.hpp"
#include "logging.hpp"


static const char *print_headers[MAX_LT_TRACE] = 
{ "RAFT",
	"OPLOG",
	"APP_WR",
	"APP_RO",
	"QUEUE",
};

enum lt_state{
	start=0,
	stop,
	MAX_LT_STATE
};

extern struct lt_tracer_ ltracer;

#define LT_INIT_RUNTIME() do{ \
														ltracer.init_tracer();\
														}while(0)

#define LT_START(key, rpc) ltracer.start_counter(key, rpc)
#define LT_END(key, rpc) ltracer.end_counter(key, rpc)
#define LT_PRINT() ltracer.print_trace()


typedef struct lt_counter_{
	unsigned long stime;
	unsigned long ttime;
	unsigned long count;
	int state;
}lt_counter_t;



typedef struct lt_tracer_{
	lt_counter_t counters[MAX_LT_TRACE];

	void init_tracer(){
		int i;
		for(i=0; i < MAX_LT_TRACE; i++){
			counters[i].stime = 0;
			counters[i].ttime = 0;
			counters[i].count = 0;
		}
		return;
	}

	int start_counter(enum trace_keys key, struct rpc_st *rpc){
		if(key >= MAX_LT_TRACE){
			exit(1);
		}	
		rpc->timestamp =  rtc_clock::current_time();
		return 0;
	}

	int end_counter(enum trace_keys key, struct rpc_st *rpc){
		if(key >= MAX_LT_TRACE){
			exit(1);
		}	
		unsigned long elapsed_time = rtc_clock::current_time() - rpc->timestamp;
		counters[key].ttime += elapsed_time;
		counters[key].count++;
		return 0;
	}

	void print_trace(){
		int i;
		for(i=0; i < MAX_LT_TRACE;i ++){
			printf("app lt : %lu " , counters[i].count == 0 ? 0 : counters[i].ttime/counters[i].count);
			printf("raft lt : %lu " , counters[i].count == 0 ? 0 : counters[i].ttime/counters[i].count);
		}
		return;
	}

}lt_tracer_t;



#endif

#endif
