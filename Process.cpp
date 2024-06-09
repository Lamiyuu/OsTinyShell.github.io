#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <windows.h>
#include <time.h>
#include <sys/types.h>
#include <psapi.h>
#include <tlhelp32.h>
#include <unordered_map>
#include <string>
#include "Analyse.h"
#include "Command.h"
#include "Conversion.h"
using namespace std;

#define MAX_CWD_LENS 128
#define MAX_BUFFER_SIZE 64
#define MAX_TOK_BUFSIZE 64
#define TOKEN_DELIMETERS " \t\r\n\a"

HANDLE hForeProcess;
std::unordered_map<DWORD, std::string> processStates;

///////////////////////////////////
//////// Xử lí tiến trình /////////
///////////////////////////////////

/**
 * Đón tín hiệu ngắt Ctrl + C
 **/
void sighandler(int signum) {
    /**
     * Đón tín hiệu ngắt Ctrl + C
     **/
    // printf("Caught signal %d, coming out...\n", signum);
    if (hForeProcess != NULL) {
        TerminateProcess(hForeProcess, 0);
        hForeProcess = NULL;
    }
    exit(1);
}

/**
 * In ra các tiến trình đang hoạt động
 * In ra màn hình tên tiến trình, Process ID, Parent PID
 * Câu lệnh: pc all
 * 
 **/
int getProcessListAll() {
    HANDLE hProcessSnap;
    PROCESSENTRY32 pe32; // Cấu trúc của tiến trình khi được gọi snapshot

    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); // Chụp lại các tiến trình
    // Nếu hProcessSnap trả về lỗi return 0
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        cout << "ERROR: CreateToolhelp32Snapshot Fail " << GetLastError() << endl;
        return 0;
    }

    pe32.dwSize = sizeof(PROCESSENTRY32);
    // Kiểm tra thằng đầu tiên
    if (!Process32First(hProcessSnap, &pe32)) {
        // Nếu lỗi in ra...
        cout << "ERROR: Process32First Fail " << GetLastError() << endl;
        return 0;
    }
    printf("%-50s%-20s%-20s\n", "Process Name", "Process ID", "Parent Process ID");
	printf("%-50s%-20s%-20s\n", "----------------------------------", "----------", "-----------");    
    
    do {
    	printf("%-50s%-20d%-20d\n", pe32.szExeFile, pe32.th32ProcessID, pe32.th32ParentProcessID);
    } while (Process32Next(hProcessSnap, &pe32)); CloseHandle(hProcessSnap);
    return 1;
}

/**
 * Tìm tiến trình bằng tên
 * In ra màn hình tên tiến trình, Process ID, Parent PID
 * Câu lệnh pc find [name_process]
 * 
 **/
int findProcessID(wchar_t *name_process) {
    HANDLE hProcessSnap;
    PROCESSENTRY32W pe32; // Cấu trúc của tiến trình khi được gọi snapshot

    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); // Chụp lại các tiến trình
    // Nếu trả về lỗi return 0
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        return 0;
    }

    pe32.dwSize = sizeof(PROCESSENTRY32W);
    
    // Kiểm tra thằng đầu tiên
    if (!Process32FirstW(hProcessSnap, &pe32)) {
        CloseHandle(hProcessSnap);
        return 0;
    }
    wprintf(L"%-50ls%-20ls%-20ls\n", L"Process Name", L"Process ID", L"Parent Process ID");
    wprintf(L"%-50ls%-20ls%-20ls\n", L"-----------------", L"-----------------", L"----------------------");

    do {
        if (wcscmp(name_process, pe32.szExeFile) == 0) {
            // Nếu pe32.szExeFile trùng với tên tiến trình thì in ra
            wprintf(L"%-50ls%-20d%-20d\n", pe32.szExeFile, pe32.th32ProcessID, pe32.th32ParentProcessID);
        }
    } while (Process32NextW(hProcessSnap, &pe32));
    CloseHandle(hProcessSnap);
    return 1;
}

/**
 * Đóng tiến trình bằng Process ID 
 * Câu lệnh: pc kill [process_id]
 * 
 **/

int killProcessID(DWORD process_id) {
    // Mở tiến trình đang chạy có Process ID là...
    HANDLE hprocess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, process_id);
    // Nếu hProcess trả về NULL thì báo lỗi
    if (hprocess == NULL) {
        cout << "ERROR: Failed!" << endl;
        return 1;
    }
    // Đóng tiến trình hProcess
    if (!TerminateProcess(hprocess, 0)) {
        return 0;
    }
    return 1;
}

/**
 * Đóng tất cả tiến trình có tên là name_process
 * Câu lệnh pc kill [Name_Process]
 * 
 * */
int killProcess(wchar_t *name_process) {
    HANDLE hProcessSnap;
    PROCESSENTRY32W pe32; // Cấu trúc của tiến trình khi được gọi snapshot

    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); // Chụp lại các tiến trình
    // Nếu trả về lỗi return 0
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        return 0;
    }

    pe32.dwSize = sizeof(PROCESSENTRY32W);
    
    // Kiểm tra thằng đầu tiên
    if (!Process32FirstW(hProcessSnap, &pe32)) {
        CloseHandle(hProcessSnap);
        return 0;
    }
    do {
        if (wcscmp(name_process, pe32.szExeFile) == 0) {
            killProcessID(pe32.th32ProcessID);
        }
    } while (Process32NextW(hProcessSnap, &pe32));
    CloseHandle(hProcessSnap);
    return 1;
}

/**
 * Đình chỉ một tiến trình đang thực hiện 
 * Câu lệnh pc suspend [process_id]
 * 
 **/
int suspendProcess(DWORD process_id) {
    // Chụp lại tất cả các luồng 
    HANDLE hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    THREADENTRY32 th32; // Cấu trúc của luồng khi được gọi snapshot
    HANDLE hthread;
    // Kiểm tra xem hThreadSnap có lỗi không nếu có thì in ra lỗi    
    if (hThreadSnap == INVALID_HANDLE_VALUE) { 
	cout << "ERROR: CreateToolhelp32Snapshot" << GetLastError();
	return 0;
    }
    th32.dwSize = sizeof(THREADENTRY32);
    // Kiểm tra thông tin của luồng đầu tiên
    if (!Thread32First(hThreadSnap, &th32)) {
	cout << "Thread32First Fail " <<  GetLastError(); // Nếu lỗi in ra lỗi
	CloseHandle(hThreadSnap);          // Đóng Handle snapshot
	return 0;
    }
    // Duyệt các luồng khác
    do {
    // Kiểm tra xem các luồng này có thuộc tiến trình cần dừng không
	if (th32.th32OwnerProcessID == process_id) {
	    hthread = OpenThread(THREAD_ALL_ACCESS, FALSE, th32.th32ThreadID); // Mở một luồng đang chạy
	    // Đình chỉ luồng đó
            if (SuspendThread(hthread) == -1) {
		return 0;
	    }
	}
    } while (Thread32Next(hThreadSnap, &th32));	CloseHandle(hThreadSnap);
	return 1;
}

/**
 * Tiếp tục một tiến trình bị đình chỉ
 * Câu lệnh pc resume [process_id]
 * 
 **/
int resumeProcess(DWORD process_id) {
    // Chụp lại tất cả các luồng 
    HANDLE hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    THREADENTRY32 th32; // Cấu trúc của luồng khi được gọi snapshot
    HANDLE hthread;
    // Kiểm tra xem hThreadSnap có lỗi không nếu có thì in ra lỗi
    if (hThreadSnap == INVALID_HANDLE_VALUE) { 
	cout << "ERROR: CreateToolhelp32Snapshot" << GetLastError();
	return 0;
    }
    th32.dwSize = sizeof(THREADENTRY32);
    // Kiểm tra thông tin của luồng đầu tiên
    if (!Thread32First(hThreadSnap, &th32)) {
	cout << "Thread32First Fail " <<  GetLastError(); // Nếu lỗi in ra lỗi
	CloseHandle(hThreadSnap);          // Đóng Handle snapshot
	return 0;
    }
    // Duyệt các luồng khác
    do {
        // Kiểm tra xem các luồng này có thuộc tiến trình cần dừng không
	if (th32.th32OwnerProcessID == process_id) {
	    hthread = OpenThread(THREAD_ALL_ACCESS, FALSE, th32.th32ThreadID); // Mở một luồng đang chạy
            // Đình chỉ luồng đó
            if (ResumeThread(hthread) == -1) {
	        return 0;
	    }
	}
    } while (Thread32Next(hThreadSnap, &th32)); CloseHandle(hThreadSnap);
    return 1;
}

/**
 * Tạo một tiến trình con 
 * Câu lệnh: pc bg [name_process/path](background mode)
 *           pc fg [name_process/path](foreground mode)
 * 
 **/
int createNewProcess(char **args) {
    int wait_time;
    if (strcmp(args[1], "bg") == 0) {
        wait_time = 0;
    } else {
        wait_time = INFINITE;
    }

    wchar_t *run_file = combinePath(args, 2);

    STARTUPINFOW si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = (strcmp(args[1], "bg") == 0) ? SW_HIDE : SW_SHOW;

    wchar_t* title = convertToWideChar(args[1]);
    si.lpTitle = title;

    ZeroMemory(&pi, sizeof(pi));

    DWORD creationFlags = CREATE_NEW_CONSOLE; // Tạo cửa sổ cmd mới

    if (!CreateProcessW(NULL, run_file, NULL, NULL, FALSE, creationFlags, NULL, NULL, &si, &pi)) {
        int error = GetLastError();
        if (error == 2)
            std::wcout << L"The batch file or execute file '" << run_file << L"' is not found." << std::endl;
        else
            std::wcout << L"Can't run this file" << std::endl;
        free(run_file);
        delete[] title;
        return 1;
    }

    if (strcmp(args[1], "fg") == 0) {
        hForeProcess = pi.hProcess;
        processStates[pi.dwProcessId] = "fg";
    } else {
        processStates[pi.dwProcessId] = "bg";
    }

    if (wait_time != 0) {
        WaitForSingleObject(pi.hProcess, wait_time);
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    free(run_file);
    delete[] title;

    return 1;
}
HWND GetProcessWindow(DWORD processId) {
    HWND hwnd = NULL;
    do {
        hwnd = FindWindowEx(NULL, hwnd, NULL, NULL);
        DWORD pid = 0;
        GetWindowThreadProcessId(hwnd, &pid);
        if (pid == processId) {
            return hwnd;
        }
    } while (hwnd != NULL);
    return NULL;
}

void switchProcessMode(DWORD processId) {
    auto it = processStates.find(processId);
    if (it != processStates.end()) {
        HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
        if (!hProcess) {
            std::cerr << "Failed to open process with ID " << processId << std::endl;
            return;
        }

        HWND hwnd = GetProcessWindow(processId);
        if (!hwnd) {
            std::cerr << "Failed to find window for process with ID " << processId << std::endl;
            CloseHandle(hProcess);
            return;
        }

        if (it->second == "bg") {
            // Chuyển từ chế độ nền sang tiền cảnh
            ShowWindow(hwnd, SW_SHOW);
            processStates[processId] = "fg";
            hForeProcess = hProcess;
        } else {
            // Chuyển từ chế độ tiền cảnh sang nền
            ShowWindow(hwnd, SW_HIDE);
            processStates[processId] = "bg";
            hForeProcess = NULL;
        }

        CloseHandle(hProcess);
    } else {
        std::cerr << "No process found with ID " << processId << std::endl;
    }
}
