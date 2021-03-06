//--------------------------------------------------------------------------------------------------
/**
 * @file command.h
 *
 * Copyright (C) Sierra Wireless Inc.  Use of this work is subject to license.
 */
//--------------------------------------------------------------------------------------------------

#ifndef LEGATO_MKTOOLS_MODEL_COMMAND_H_INCLUDE_GUARD
#define LEGATO_MKTOOLS_MODEL_COMMAND_H_INCLUDE_GUARD


struct Command_t
{
    std::string name;       ///< Name of the command.
    App_t* appPtr;      ///< Pointer to the app that contains the executable.
    std::string exePath;    ///< Absolute path (within app's read-only area) of the exe.

    const parseTree::Command_t* parseTreePtr; ///< Ptr to parse tree object.

    Command_t(const parseTree::Command_t* p): parseTreePtr(p) {}
};


#endif // LEGATO_MKTOOLS_MODEL_COMMAND_H_INCLUDE_GUARD
