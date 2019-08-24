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



class string_view {
public:
	string_view() noexcept;
	string_view(const char *data, size_t size);
	string_view(const std::string &s);
	string_view(const char *data);

	string_view(const string_view &rhs) noexcept = default;
	string_view &operator=(const string_view &rhs) noexcept = default;

	const char *data() const noexcept;
	std::size_t size() const noexcept;

	/**
	 * Compares this string_view with other. Works in the same way as
	 * std::basic_string::compare.
	 *
	 * @return 0 if both character sequences compare equal,
	 *         positive value if this is lexicographically greater than other,
	 *         negative value if this is lexicographically less than other.
	 */
	int compare(const string_view &other) noexcept;

private:
	const char *_data;
	std::size_t _size;
};

inline string_view::string_view() noexcept : _data(""), _size(0)
{
}

inline string_view::string_view(const char *data, size_t size) : _data(data), _size(size)
{
}

inline string_view::string_view(const std::string &s) : _data(s.c_str()), _size(s.size())
{
}

inline string_view::string_view(const char *data)
		: _data(data), _size(std::char_traits<char>::length(data))
{
}

inline const char *string_view::data() const noexcept
{
	return _data;
}

inline std::size_t string_view::size() const noexcept
{
	return _size;
}

inline int string_view::compare(const string_view &other) noexcept
{
	int ret = std::char_traits<char>::compare(data(), other.data(),
											  std::min(size(), other.size()));
	if (ret != 0)
		return ret;
	if (size() < other.size())
		return -1;
	if (size() > other.size())
		return 1;
	return 0;
}


/* forward declare */
class PMEngine;

/* the entry point to all data-structure manipulations. PMLib interprets the 
 * incoming request and route the operation to appropriate data-structure
 * runtime
 */
class PMLib{

	public:
		PMLib(const std::string path){
			this->pmem_path = path;
		}

		PMLib(const std::string path,uint8_t npartitions){
			this->pmem_path = path;
			this->npartitions = npartitions;
		}

		int open(const string& app);
		int close();
		void exec(uint8_t thread_id, pm_rpc_t *req, pm_rpc_t *resp);


	private:
		std::string pmem_path;
		uint8_t npartitions;

        PMEngine* find_ds(uint16_t id);
		int create_ds(uint8_t ds_type,uint16_t ds_id,uint8_t npartitions);
        int remove_ds(uint8_t ds_type,uint16_t ds_id);
		int close_ds(uint8_t ds_type,uint16_t ds_id);

		/// vote benchmark specific
		void vote_topk(pm_rpc_t *request, pm_rpc_t *response);

		std::map<uint16_t ,PMEngine*> *engine_map; /// TBD: name to data-structure mapping. Make this an array

};



const string LAYOUT = "pmemds";                            // pool layout identifier

class PMEngine {                                           // storage engine implementations
public:

    PMEngine(){};
	virtual ~PMEngine(){};

	virtual void exec(uint8_t thread_id, uint16_t op_id,uint8_t ds_type, std::string ds_id,
				  pm_rpc_t *req, pm_rpc_t *resp)=0;

	virtual string engine() =0;                           // engine identifier

};
}
#endif
