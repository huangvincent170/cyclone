#include "pmemds.h"
#include "hashmap/concurrent_hash_map.hpp"
#include "tree/btree.h"
#include "hashmap/hashmap.h"


const std::string pmem_home =  "/mnt/pmem1/pmemds";

namespace pmemds{

PMStatus PMLib::open(const string& app_path) {

	engine_map = new std::map<std::string,PMEngine*>();
	//TODO: create dir path

}

PMStatus PMLib::close() {
	// free runtime state
	std::map<std::string,PMEngine*>::iterator it;
	for(it = engine_map->begin(); it != engine_map->end();it++){
		delete it->second;
		engine_map->erase(it);
	}
	return OK;
}

struct GetCallbackContext {
    PMStatus result;
		std::string* value;
};

	PMEngine * PMLib::find_ds(std::string id) {
		auto engine = engine_map->find(id);
		return engine->second;
	}

/*
 * The routing function call
 */
PMStatus PMLib::exec(uint16_t op_name,uint8_t ds_type, std::string ds_id,std::string in_key,
		std::string in_val, std::string out_val){
	PMEngine *engine = find_ds(ds_id);
		if(engine == nullptr){

		}
	switch(op_name){	
  /* open-close data-structures */	
		case pmemds::OPEN:
			open(pmem_home);
			break;
		case pmemds::CLOSE:
			close();
			break;
	/* handle data-structure creation and delettion */
		case pmemds::CREATE:	
				create_ds(ds_type,ds_id);
				break;
		case pmemds::DELETE:
				delete_ds(ds_id);
				break;
			
	/* handle data-structure local requests*/
		default:
			return engine->exec(op_name,ds_type,ds_id,in_key,in_val,out_val);
	}
}


PMStatus PMLib::create_ds(uint8_t ds_type, std::string ds_id){
		PMEngine *engine;
	switch (ds_id){
		case SORTED_BTREE:
			engine = new BTreeEngine();
			break;
		case HASH_MAP:
			engine = new HashMapEngine();
			break;
		case VECTOR:
		default:
			LOG_ERROR("Invalid DS type");
	}
	engine_map->insert(ds_id, engine)
}


	PMStatus PMLib::delete_ds(std::string ds_id){
		auto engine = engine_map->find(ds_id);
		if(engine != engine_map->end()){
			engine_map->erase(ds_id);
			return OK;
		}
		LOG_ERROR("data-structure not found");
		return NOT_FOUND;
	}

}
