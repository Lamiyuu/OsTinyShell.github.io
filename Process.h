#pragma once

#ifndef _PROCESS_H
#define _PROCESS_H

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <windows.h>
#include <time.h>
#include <sys/types.h>
#include <psapi.h>
#include <tlhelp32.h>
#include "Analyse.h"
#include <string> 
#include <vector>
#define MAX_CWD_LENS 128
#define MAX_BUFFER_SIZE 64
#define MAX_TOK_BUFSIZE 64
#define TOKEN_DELIMETERS " \t\r\n\a"

/* Các câu lệnh liên quan đến tiến trình */
void sighandler(int signum);
int findProcessID(wchar_t *name_process);
int getProcessListAll();
int killProcessID(DWORD process_id);
int killProcess(wchar_t *name_process);
int suspendProcess(DWORD process_id);
int resumeProcess(DWORD process_id);
int createNewProcess(char **args);
void switchProcessMode(DWORD processId);
#endif