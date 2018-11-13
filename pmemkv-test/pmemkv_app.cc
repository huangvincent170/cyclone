#include <iostream>
#include "pmemkv.h"

#define LOG(msg) std::cout << msg << "\n"

using namespace pmemkv;

int main() {
    LOG("Starting engine");
    KVEngine* kv = KVEngine::Open("kvtree3", "/dev/shm/pmemkv", 1024*1024*1024);
	if(kv == nullptr){
		LOG("Cannot open db");
		exit(1);
	}
    LOG("Putting new key");
    KVStatus s = kv->Put("key1", "value1");
    assert(s == OK && kv->Count() == 1);

    LOG("Reading key back");
    string value;
    s = kv->Get("key1", &value);
    assert(s == OK && value == "value1");

    LOG("Iterating existing keys");
    kv->Put("key2", "value2");
    kv->Put("key3", "value3");
    kv->Each([](void *context, int kb, int vb, const char *k, const char* v) {
        LOG("  visited: " << k);
    });

    LOG("Removing existing key");
    s = kv->Remove("key1");
    assert(s == OK && !kv->Exists("key1"));

    LOG("Stopping engine");
    kv->Close(kv);
    return 0;
}
