#pragma once 

#include <string>
#include <iostream>

std::string get_current_timestamp();

#define DEBUG_MSG(msg) std::cout << "[DEBUG] [" << get_current_timestamp() << "] " << msg << std::endl;
