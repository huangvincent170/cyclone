#include "pmemds.h"
#include "hashmap/concurrent_hash_map.hpp"

namespace pmemds{

PMLib* PMLib::open(const string& engine, const string& path, const size_t size) {

}

void PMLib::close(const std::string& engine) {

}

struct GetCallbackContext {
    PMStatus result;
		std::string* value;
};

/*
 * The routing function call
 */
PMStatus PMLib::exec(uint16_t op_name,uint8_t ds_type, std::string ds_id,std::string in_key,
		std::string in_val, std::string out_val){
	PMEngine *engine = find_ds(ds_id);
	switch(op_name){	
  /* open-close data-structures */	
		case pmemds::OPEN:
			engine->open();	
			break;
		case pmemds::CLOSE:
			engine->close();
			break;
	/* handle data-structure creation and delettion */
		case pmemds::CREATE:	
				engine->insert();
				break;
		case pmemds::DELETE:
				engine->erase();
				break;
			
	/* handle reads request */
		case pmemds::GET:
			engine->get();
			break;
		case pmemds::PUT:
			engine->put();
			break;
		default:
			
	}
}


PMStatus PMLib::create_ds(){

}


PMStatus PMLib::delete_ds(){


}


PMStatus PMLib::get(const string& key, string* value) {
}

PMStatus PMLib::put(const string& key, string* value) {
}

}
