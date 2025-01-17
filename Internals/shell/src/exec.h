#ifndef EXEC_H
#define EXEC_H

#include <libc/stdbool.h>

int string_to_arguments(char* str, char** arguments, int max);

int execute_from_args(int argc, const char** argv, const char** envp, int* return_value, bool as_daemon, int to_close);

int command_cd(int argc, const char** argv, const char** envp);

void save_tty_settings();
void load_tty_settings();

bool check_exist(const char* fname);

#endif // EXEC_H