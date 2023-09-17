// scim.hpp
//
// main header file for the Sawcon Image Manipulator
// This file was written for the Sawcon Image Manipulator
// This version of the header was written for SCIM Alpha 1.3
// 
// Written: Sunday 13th August 2023
// Last Updated: Wednesday 13th September 2023
// 
// Written by Gabriel Jickells

#pragma once
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <ctime>

namespace scim {

    using byte = unsigned char;
    using word = unsigned short;
    using dword = unsigned int;

    size_t min(size_t a, size_t b) {
        return a < b ? a : b;
    }

    #include "fat.hpp"
    
    #include "mode.hpp"

}
