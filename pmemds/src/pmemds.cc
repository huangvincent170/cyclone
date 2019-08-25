#include "pmemds.h"
#include "pmemds_log.h"
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
 *  vote benchmarks' get top-K stored prodecure
 */
void PMLib::vote_topk(pm_rpc_t *request, pm_rpc_t *response){
	struct vote_payload_st vpayload[MAX_VOTE_PAYLOAD];
 	uint16_t  sharded_hashmap = 0;
 	uint16_t  sharded_pq = 1;  /// TBD: these are request params

    PMEngine *hm_temp = find_ds(sharded_hashmap);
    PMEngine *pq_temp = find_ds(sharded_pq);

	ShardedHashMapEngine * hashmap_engine = reinterpret_cast<ShardedHashMapEngine *>(hm_temp);
	ShardedPriorityQueueEngine * pq_engine = reinterpret_cast<ShardedPriorityQueueEngine *>(pq_temp);

		using hashmap_t = pmem::obj::experimental::concurrent_hash_map<unsigned long, pstring<16>>;

		int fill_index = 0;
		int size;
		hashmap_t::accessor result;
		for(int i=0; i < this->npartitions; i++) {

			hashmap_t *hm = (hashmap_t *)hashmap_engine->engine(i);
			persistent_priority_queue *pq = (persistent_priority_queue *) pq_engine->engine(i);
			pq->read_topK(&vpayload[fill_index],&size);
			// fill the titles next
			if(size != 0){
				for(int j = 0 ; j < size; j++){
					bool found = hm->find(result,vpayload[j].idx);
					if(!found){
						LOG_ERROR("key not found");
						return;
					}
					snprintf(vpayload[j].art,16, "%s",result->second.c_str());

				}
                fill_index += size;
			}
		}
}


/*
 * The routing function call
 */
void PMLib::exec(uint8_t thread_id, pm_rpc_t *req,pm_rpc_t *resp){
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
				if(!create_ds(ds_type,ds_id,this->npartitions)){
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
        case GET_TOPK:
            vote_topk(req,resp);
            break;
	/* handle data-structure local requests*/
		default:
			PMEngine *engine = find_ds(ds_id);
			//LOG_DEBUG("data-structure type : " + engine->engine());
			if(engine == nullptr){
				LOG_ERROR("cannot locate data-structure");
				SET_STATUS(resp->meta,NOT_FOUND);
				return;
			}
			engine->exec(thread_id, op_id,ds_type,std::to_string(ds_id),req,resp);
	}
}


int PMLib::create_ds(uint8_t ds_type, uint16_t ds_id, uint8_t npartitions){
		PMEngine *engine = nullptr;
        std::string path = pmem_path + "/" + std::to_string(ds_id);
        //std::string path = pmem_path;
	switch (ds_type){
		case SORTED_BTREE:
			engine = new BTreeEngine( path ,ds_pool_size);
			break;
		case CONCURRENT_HASHMAP:
			engine = new HashMapEngine(path,ds_pool_size);
			break;
		case SHARDED_HASHMAP:
			engine = new ShardedHashMapEngine(path,ds_pool_size,npartitions);
			break;
		case VECTOR:
            break;
        case SHARDED_PRIORITY_QUEUE:
            engine = new ShardedPriorityQueueEngine(path,ds_pool_size,npartitions);
            break;
		default:
			LOG_ERROR("Invalid DS type");
			exit(-1);
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

}
