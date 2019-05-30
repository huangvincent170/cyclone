#ifndef PMEMDS_PMEMDS_LOG_H
#define PMEMDS_PMEMDS_LOG_H

#ifdef DEBUG
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define LOG_INFO(msg) std::cout << "[info]  " << __FILENAME__ <<" "+ __LINE__ + " " << msg << "\n"
#define LOG_DEBUG(msg) std::cout << "[debug]  " << __FILENAME__ <<" "+ __LINE__ + " " << msg << "\n"
#define LOG_ERROR(msg) std::cout << "[error]  " << __FILENAME__ <<" "+ __LINE__ + " " << msg << "\n"

#else

#define LOG_INFO(msg)
#define LOG_DEBUG(msg)
#define LOG_ERROR(msg)

#endif

#endif //PMEMDS_PMEMDS_LOG_H
