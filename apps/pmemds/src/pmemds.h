

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

#include "pmemds-common.h"

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

enum dstypes{
	SORTED_BTREE = 0,
	HASH_MAP,
	PRIORITY_QUEUE,
	VECTOR
};



/* forward declare */
class PMEngine;

/* the entry point to all data-structure manipulations. PMLib interprets the 
 * incoming request and route the operation to appropriate data-structure
 * runtime
 */
class PMLib{

	public:
		PMStatus open(const string& app);
		PMStatus close();
		PMStatus exec(uint16_t op_name,uint8_t ds_type, std::string ds_id,std::string in_key,
					  std::string in_val, std::string out_val);


	private:
		PMEngine* find_ds(std::string id);
		PMStatus create_ds(uint8_t ds_type,std::string ds_id);
		PMStatus delete_ds(std::string id);

		std::map<std::string,PMEngine*> *engine_map; //name to data-structure mapping

};



const string LAYOUT = "pmemds";                            // pool layout identifier

class PMEngine {                                           // storage engine implementations
  public:
	PMStatus open(const string& engine,            // open storage engine
                          const string& path,              // path to persistent pool
                          size_t size);                    // size used when creating pool
    PMStatus close();                       // close storage engine

	PMStatus exec(uint16_t op_name,uint8_t ds_type, std::string ds_id,std::string in_key,
				  std::string in_val, std::string out_val);


	virtual string Engine() = 0;                           // engine identifier
    
    virtual PMStatus Exists(const string& key) = 0;        // does key have a value?

    inline void get(const string& key,                     // pass value to callback
                    PMGetCallback* callback) {
        get(nullptr, key, callback);
    }
    virtual void get(void* context,                        // pass value to callback with context
                     const string& key,
                     PMGetCallback* callback) = 0;
    PMStatus get(const string& key, string& value);        // append value to string

    virtual PMStatus put(const string& key,                // store key and value
                         const string& value) = 0;
    virtual PMStatus remove(const string& key) = 0;        // remove value for key
};
}
#endif
