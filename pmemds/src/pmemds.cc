#include "pmemds.h"
#include "pmemds_log.h"
#include "hashmap/concurrent_hash_map.hpp"
#include "tree/btree.h"
#include "hashmap/hashmap.h"
#include "priority_queue/priority_queue.h"

const unsigned long ds_pool_size = 1024*1024*1024*1;

namespace pmemds{

int PMLib::open(const string& app_path) {

	engine_map = new std::map<uint16_t ,PMEngine*>();
	//TODO: create dir path
	return OK;
}

int PMLib::close() {
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

	//LOG_DEBUG("data-structure type : " + std::to_string(ds_type) + " operation id : " + std::to_string(op_id) + " data-structure id : " + std::to_string(ds_id));
	switch(op_id){
  /* open-close data-structures */	
		case OPEN:
			if(!open(pmem_path)){
                    SET_STATUS(resp->meta,OK);
                }else{
                    SET_STATUS(resp->meta,FAILED);
                }
			break;
		case CLOSE:
			if(!close()){
                    SET_STATUS(resp->meta,OK);
                }else{
                    SET_STATUS(resp->meta,FAILED);
                }
			break;
	/* handle data-structure creation and deletion */
		case CREATE_DS:
				if(!create_ds(ds_type,ds_id)){
                    SET_STATUS(resp->meta,OK);
                }else{
                    SET_STATUS(resp->meta,FAILED);
                }
				break;
        case CLOSE_DS:
            if(!close_ds(ds_type,ds_id)){
                SET_STATUS(resp->meta,OK);
            }else{
                SET_STATUS(resp->meta, FAILED);
            }
            break;
		case REMOVE_DS:
                if(!remove_ds(ds_type,ds_id)){
                    SET_STATUS(resp->meta,OK);
                }else{
                    SET_STATUS(resp->meta,FAILED);
                }
				break;

	/* vote benchmark related operations */
		case PUT_ART:
			put_art(req,resp);
			break;
		case GET_ART:
			get_art(req,resp);
			break;
		case VOTE_UP_ART:
			vote_up(req,resp);
			break;
		case VOTE_DOWN_ART:
			vote_down(req,resp);
			break;

	/* handle data-structure local requests*/
		default:
			PMEngine *engine = find_ds(ds_id);
			LOG_DEBUG("data-structure type : " + engine->engine());
			if(engine == nullptr){
				LOG_ERROR("cannot locate data-structure");
				SET_STATUS(resp->meta,NOT_FOUND);
				return;
			}
			engine->exec(op_id,ds_type,std::to_string(ds_id),req->key,req,resp);
	}
}


int PMLib::create_ds(uint8_t ds_type, uint16_t ds_id){
		PMEngine *engine = nullptr;
        std::string path = pmem_path + "/" + std::to_string(ds_id);
        //std::string path = pmem_path;
	switch (ds_type){
		case SORTED_BTREE:
			engine = new BTreeEngine( path ,ds_pool_size);
			break;
		case HASH_MAP:
			engine = new HashMapEngine(path,ds_pool_size);
			break;
		case VECTOR:
            break;
        case PRIORITY_QUEUE:
            engine = new priority_queue(path,ds_pool_size);
		default:
			LOG_ERROR("Invalid DS type");
	}
    if(engine != nullptr) {
        engine_map->insert(std::pair<uint16_t, PMEngine *>(ds_id, engine));
        return 0;
    }
    return -1;
}



    int PMLib::close_ds(uint8_t ds_type, uint16_t ds_id){
        auto engine = engine_map->find(ds_id);
        if(engine != engine_map->end()){
            delete engine->second;
            engine_map->erase(ds_id); // take out from volatile map
            return 0;
        }
        LOG_ERROR("data-structure not found");
        return -1;
    }

	int PMLib::remove_ds(uint8_t ds_type, uint16_t ds_id){
        std::string path = pmem_path + "/" + std::to_string(ds_id);
        return std::remove(path.c_str());
	}


	/* vote benchmark specific operations */
	 void PMLib::put_art(pm_rpc_t *req, pm_rpc_t *res){ //TODO: get ds_id from the request
		uint16_t ds_id  = DS_ID(req->meta);
		auto art = engine_map->find(0);
		auto vote = engine_map->find(1);
		if(art == engine_map->end() || vote == engine_map->end()){
			LOG_ERROR("vote benchmark data-structures not found");
			SET_STATUS(resp->meta,FAILED);

		}
		art->put(req->key, req->value);
		vote->put(req->key, "0");
		SET_STATUS(resp->meta,OK);
	}


	void PMLib::get_art(pm_rpc_t *req, pm_rpc_t *res){
		uint16_t ds_id  = DS_ID(req->meta);
		auto art = engine_map->find(0);
		auto vote = engine_map->find(1);
		if(art == engine_map->end() || vote == engine_map->end()){
			LOG_ERROR("vote benchmark data-structures not found");
			SET_STATUS(resp->meta,FAILED);
		}
		hashmap_type *art_ds = art->get_ds();
		hashmap_type *vote_ds = art->get_ds();

		std::string art_str = art_ds->get(req->key);
		std::string vote_str = vote_ds->get(req->key);

		std::ret_str = art_str + ',' + vote_str;

		resp->key = req->key;
		snprintf(resp->value,MAX_VALUE,"%s", ret_str.c_str());
		SET_STATUS(resp->meta,OK);
	}

	/* do we really need up_vote and down_vote functions? */

	void PMLib::vote_up(pm_rpc_t *req, pm_rpc_t *res){
		uint16_t ds_id  = DS_ID(req->meta);
		auto vote = engine_map->find(ds_id);
		if(art == engine_map->end() || vote == engine_map->end()){
			LOG_ERROR("vote benchmark data-structures not found");
			SET_STATUS(resp->meta,FAILED);
		}
		hashmap_type *vote_ds = art->get_ds();
		std::string vote_str = vote_ds->get(req->key);

		std::ret_str = std::to_string(std::stoi(vote_str) + 1);

		resp->key = req->key;
		snprintf(resp->value,MAX_VALUE,"%s", ret_str.c_str());
		SET_STATUS(resp->meta,OK);
	}

	void PMLib::vote_down(pm_rpc_t *req, pm_rpc_t *res){
		uint16_t ds_id  = DS_ID(req->meta);
		auto vote = engine_map->find(ds_id);
		if(art == engine_map->end() || vote == engine_map->end()){
			LOG_ERROR("vote benchmark data-structures not found");
			SET_STATUS(resp->meta,FAILED);
		}
		hashmap_type *vote_ds = art->get_ds();
		std::string vote_str = vote_ds->get(req->key);

		std::ret_str = std::to_string(std::stoi(vote_str) + 1);

		resp->key = req->key;
		snprintf(resp->value,MAX_VALUE,"%s", ret_str.c_str());
		SET_STATUS(resp->meta,OK);
	}



}
