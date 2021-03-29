#include "cyclone_ucp.hpp"
#include <iostream>
#include <ucp/api/ucp.h>

void testfunc() {
    ucp_init(0, 0, 0);
    std::cout << "\n\nTEST SUCCESSFUL\n\n";
}