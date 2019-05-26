/*
 * Common interface to our persistent data-structures. 
 * We may change them/subclass
 *
 *
 */
#ifndef __PMEMDS_H
#define __PMEMDS_H

#include <string>
#include "pmemds-common.h"

typedef void(PMEachCallback)(void* context,                // callback function for Each operation
                             int keybytes,
                             int valuebytes,
                             const char* key,
                             const char* value);

typedef void(PMGetCallback)(void* context,                 // callback function for Get operation
                            int valuebytes,
                            const char* value);

struct PMGetCallbackContext {
	PMStatus result;
	std::string* value;
};

#include <string>
#include <map>
#include <libpmemobj++/make_persistent.hpp>
#include <libpmemobj++/make_persistent_array.hpp>
#include <libpmemobj++/persistent_ptr.hpp>
#include <libpmemobj++/pool.hpp>
#include <libpmemobj++/transaction.hpp>

#include "pmemds-common.h"
#include "pmemds_log.h"

using std::string;
using std::to_string;

namespace pmemds {
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
		void exec(pm_rpc_t *req, pm_rpc_t *resp);


	private:
		PMEngine* find_ds(uint16_t id);
		PMStatus create_ds(uint8_t ds_type,uint16_t ds_id);
		PMStatus delete_ds(uint16_t id);

		std::map<uint16_t ,PMEngine*> *engine_map; //name to data-structure mapping

};



const string LAYOUT = "pmemds";                            // pool layout identifier

class PMEngine {                                           // storage engine implementations
  public:

	virtual ~PMEngine();


	PMStatus open(const string& engine,            // open storage engine
                          const string& path,              // path to persistent pool
                          size_t size);                    // size used when creating pool
    PMStatus close();                       // close storage engine

	void exec(uint16_t op_id,uint8_t ds_type, std::string ds_id,unsigned long in_key,
				  std::string& in_val,pm_rpc_t *resp);


	virtual string Engine() = 0;                           // engine identifier
    
    virtual void Exists(const string& key,pm_rpc_t *resp) = 0;        // does key have a value?

    virtual void get(const string& key,pm_rpc_t *resp) = 0;        // append value to string

    virtual void put(const string& key,                // store key and value
                         const string& value,pm_rpc_t *resp) = 0;
    virtual void remove(const string& key,pm_rpc_t *resp) = 0;        // remove value for key
};
}
#endif
