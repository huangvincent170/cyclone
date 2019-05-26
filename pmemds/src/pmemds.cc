#include "pmemds.h"
#include "hashmap/concurrent_hash_map.hpp"
#include "tree/btree.h"
#include "hashmap/hashmap.h"


const std::string pmem_home =  "/mnt/pmem1/pmemds";
const long ds_pool_size = 1024*1024; // pool size for the data-structure. Not a fundamental limitation


namespace pmemds{

PMStatus PMLib::open(const string& app_path) {

	engine_map = new std::map<uint16_t ,PMEngine*>();
	//TODO: create dir path
	return OK;
}

PMStatus PMLib::close() {
	// free runtime state
	std::map<uint16_t ,PMEngine*>::iterator it;
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

	PMEngine * PMLib::find_ds(uint16_t id) {
		auto engine = engine_map->find(id);
		return engine->second;
	}

/*
 * The routing function call
 */
void PMLib::exec(pm_rpc_t *req,pm_rpc_t *resp){
		uint8_t ds_type = TYPE_ID(req->meta);
		uint16_t op_id  = OP_ID(req->meta);
		uint16_t ds_id  = DS_ID(req->meta);


	PMEngine *engine = find_ds(ds_id);
		if(engine == nullptr){

		}
	switch(op_id){
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
		std::string str = req->value;
			 engine->exec(op_id,ds_type,std::to_string(ds_id),req->key,str,resp);
	}
}


PMStatus PMLib::create_ds(uint8_t ds_type, uint16_t ds_id){
		PMEngine *engine;
	switch (ds_type){
		case SORTED_BTREE:
			engine = new BTreeEngine(std::to_string(ds_id),ds_pool_size);
			break;
		case HASH_MAP:
			engine = new HashMapEngine(std::to_string(ds_id),ds_pool_size);
			break;
		case VECTOR:
		default:
			LOG_ERROR("Invalid DS type");
	}
	engine_map->insert(std::pair<uint16_t, PMEngine*>(ds_id,engine));
	return OK;
}


	PMStatus PMLib::delete_ds(uint16_t ds_id){
		auto engine = engine_map->find(ds_id);
		if(engine != engine_map->end()){
			engine_map->erase(ds_id);
			return OK;
		}
		LOG_ERROR("data-structure not found");
		return NOT_FOUND;
	}

}
