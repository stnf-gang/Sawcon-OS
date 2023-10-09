// mode.hpp
//
// utilities for specifying modes in SCIM
// This file was written as part of the Sawcon Image Manipulator
// This version of the header was written for SCIM Alpha 1.4
//
// Written: Sunday 27th August 2023
// Last Updated: Saturday 7th August 2023
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
    "serialise",
    "serialize",                // alternate spelling for the serialise mode
    NULL                        // end of list
};

enum MODES {
    M_INVALID = 0,
    M_LIST,
    M_READ,
    M_DELETE,
    M_WRITE,
    M_SERIALISE,
    M_SERIALIZE,                // alternate spelling for serialise
};
