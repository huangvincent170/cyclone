#ifndef PMEMDS_TEST_COMMON_HPP
#define PMEMDS_TEST_COMMON_HPP

#include <dirent.h>

#include "boost/filesystem.hpp"

const std::string pmem_path = "/dev/shm/pmemds_test";


static void setup_pmem(std::string path){
    boost::filesystem::path dir(path);
    if(!boost::filesystem::exists(path)) {
        if (boost::filesystem::create_directory(dir)) {
            std::cerr << "Directory Created: " << path << std::endl;
        } else {
            std::cerr << "Directory creation failed" << path << std::endl;
        }
    }else{ //clean up the content
        namespace fs= boost::filesystem;
        fs::path path_to_remove(path);
        for (fs::directory_iterator end_dir_it, it(path_to_remove); it!=end_dir_it; ++it) {
            fs::remove_all(it->path());
        }
    }
}

#endif //PMEMDS_TEST_COMMON_HPP
