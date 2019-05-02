

/*
 * Common interface to our persistent data-structures. 
 * We may change them/subclass
 *
 *
 */


#ifndef __PMEMDS_H
#define __PMEMDS_H

typedef enum {                                             // status enumeration
    FAILED = -1,                                           // operation failed
    NOT_FOUND = 0,                                         // key not located
    OK = 1                                                 // successful completion
} PMStatus;

typedef void(PMEachCallback)(void* context,                // callback function for Each operation
                             int keybytes,
                             int valuebytes,
                             const char* key,
                             const char* value);

typedef void(PMGetCallback)(void* context,                 // callback function for Get operation
                            int valuebytes,
                            const char* value);
#include <string>
#include <map>
#include <libpmemobj++/make_persistent.hpp>
#include <libpmemobj++/make_persistent_array.hpp>
#include <libpmemobj++/persistent_ptr.hpp>
#include <libpmemobj++/pool.hpp>
#include <libpmemobj++/transaction.hpp>

using std::string;
using std::to_string;

namespace pmemds {
enum opnames{
	  OPEN=0,
		CLOSE,
		CREATE,
		DELETE,
		GET,
		PUT
};



/* the entry point to all data-structure manipulations. PMLib interprets the 
 * incoming request and route the operation to appropriate data-structure
 * runtime
 */
class PMLib{

	public:
		PMLib* open(const string& engine, const string& path, const size_t size);
		void close(const std::string& engine);
		PMStatus exec(std::string op_name, std::string ds_name, std::string in_key, 
				std::string in_val, std::string out_val);
		PMStatus create_ds(std::string id);
		PMStatus delete_ds(std::string id);
	  PMStatus get(const std::string& key, std::string& value);	
	  PMStatus put(const std::string& key, std::string& value);	

	private:
		std::map<std::string,PMEngine*> engine_map; //name to data-structure mapping

};



const string LAYOUT = "pmemds";                            // pool layout identifier

class PMEngine {                                           // storage engine implementations
  public:
    static PMEngine* Open(const string& engine,            // open storage engine
                          const string& path,              // path to persistent pool
                          size_t size);                    // size used when creating pool
    static void Close(PMEngine* kv);                       // close storage engine

    virtual string Engine() = 0;                           // engine identifier
    
    virtual PMStatus Exists(const string& key) = 0;        // does key have a value?

    inline void Get(const string& key,                     // pass value to callback
                    PMGetCallback* callback) {
        Get(nullptr, key, callback);
    }
    virtual void Get(void* context,                        // pass value to callback with context
                     const string& key,
                     PMGetCallback* callback) = 0;
    PMStatus Get(const string& key, string* value);        // append value to string

    virtual PMStatus Put(const string& key,                // store key and value
                         const string& value) = 0;
    virtual PMStatus Remove(const string& key) = 0;        // remove value for key
};
}
#endif
