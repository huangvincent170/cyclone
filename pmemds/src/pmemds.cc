#include "pmemds.h"
#include "hashmap/concurrent_hash_map.hpp"
#include "tree/btree.h"
#include "hashmap/hashmap.h"


const std::string pmem_home =  "/mnt/pmem1/pmemds";
const long ds_pool_size = 1024*1024; // pool size for the data-structure. Not a fundamental limitation


namespace pmemds{

PMStatus PMLib::open(const string& app_path) {

	engine_map = new std::map<std::string,PMEngine*>();
	//TODO: create dir path
	return OK;
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
void PMLib::exec(uint16_t op_name,uint8_t ds_type, std::string ds_id,std::string in_key,
		std::string in_val, pm_rpc_t *resp){
	PMEngine *engine = find_ds(ds_id);
		if(engine == nullptr){

		}
	switch(op_name){	
  /* open-close data-structures */	
		case OPEN:
			open(pmem_home);
			break;
		case CLOSE:
			close();
			break;
	/* handle data-structure creation and delettion */
		case CREATE:
				create_ds(ds_type,ds_id);
				break;
		case DELETE:
				delete_ds(ds_id);
				break;
			
	/* handle data-structure local requests*/
		default:
			 engine->exec(op_name,ds_type,ds_id,in_key,in_val,resp);
	}
}


PMStatus PMLib::create_ds(uint8_t ds_type, std::string ds_id){
		PMEngine *engine;
	switch (ds_type){
		case SORTED_BTREE:
			engine = new BTreeEngine(ds_id,ds_pool_size);
			break;
		case HASH_MAP:
			engine = new HashMapEngine(ds_id,ds_pool_size);
			break;
		case VECTOR:
		default:
			LOG_ERROR("Invalid DS type");
	}
	engine_map->insert(std::pair<std::string, PMEngine*>(ds_id,engine));
	return OK;
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
