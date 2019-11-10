/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#pragma once

#include "base/core/types.h"
#include "base/io/base_file.h"

#include <unistd.h>

namespace Base {
namespace Process {

pid_t getpid();
/**
        pid_t pid = fork();
        if(pid == 0) {
                // child process
        } else if(pid > 0) {
                // parent process
        } else {
                // error: fork failed
        }
**/

pid_t fork();

//! argv Null terminated list of null terminated strings.
//!		First should, by convention, point to path.
int exec(const char *path, char *const argv[]);

pid_t spawn(const char *path, char *const argv[]);

pid_t spawn2(const char *path, char *const argv[], FileHandle &std_out);

} // namespace Process
} // namespace Base
