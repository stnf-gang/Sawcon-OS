// mode.hpp
//
// utilities for specifying modes in SCIM
// This file was written as part of the Sawcon Image Manipulator
// This version of the header was written for SCIM Alpha 1.3
//
// Written: Sunday 27th August 2023
// Last Updated: Wednesday 30th August 2023
//
// Written by Gabriel Jickells

enum Indexes {
    MODE_INDEX = 1,
};                  // defines the index into argv that certain arguments are expected to be

const char *ValidModes[] {
    "list",
    "read",
    "delete",
    "write",
    NULL                        // end of list
};

enum MODES {
    M_INVALID = 0,
    M_LIST,
    M_READ,
    M_DELETE,
    M_WRITE,
};
