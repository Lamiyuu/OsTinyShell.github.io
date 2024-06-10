#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <windows.h>
#include <time.h>
#include <sys/types.h>
#include <psapi.h>
#include <tlhelp32.h>
#include "Command.h"

#define MAX_CWD_LENS 128
#define MAX_BUFFER_SIZE 64
#define MAX_TOK_BUFSIZE 64
#define TOKEN_DELIMETERS " \t\r\n\a"

using namespace std;

/////////////////////////////////////
////////// Xử lý câu lệnh ///////////
/////////////////////////////////////

/**
 * Sửa lỗi gõ cách câu lệnh
 * VD: C:\Users\Admin> $           cd E:\ 
 * ==> C:\Users\Admin> $ cd E:\
 * 
 **/
void fixCmd(char* cmd){
    int length = strlen(cmd);
    int first;
    for(int i=0; i<length; i++){
        if((cmd)[i] != ' '){
            first = i;
            break;
        }
    }
    int real_len = length - first, i=0;
    // bỏ đi dấu cách đầu dòng nếu có 
    while(real_len--){
        (cmd)[i] = (cmd)[i+first];
        i++;
    }
    (cmd)[length - first] = '\0'; 
}

/**
 * In ra màn hình console đường dẫn 
 * (VD: C:\Users\Admin\.....> $ *phần tiếp này là lệnh*)
 **/
int printConsolePath(wchar_t *cur_directory) {
    // Lấy đường dẫn đầy đủ của file thực thi
    if (GetModuleFileNameW(NULL, cur_directory, MAX_CWD_LENS) == 0) {
        std::wcout << L"Reading of executable path failed.\n";
        return -1;
    }

    // Tách phần thư mục từ đường dẫn đầy đủ
    wchar_t* last_backslash = wcsrchr(cur_directory, L'\\');
    if (last_backslash != NULL) {
        *last_backslash = L'\0'; // Kết thúc chuỗi tại vị trí dấu gạch chéo ngược cuối cùng
    }

    // In phần thư mục và dấu nhắc lệnh
    std::wcout << cur_directory << L"\\> $";   
    return 1;
}

/**
 * Chia câu lệnh thành các đoạn ngăn cách bởi ký tự ngăn (" ","\t","\r","\n","\a")
 * 
 **/
char **separator_line(char *line){
    int bufsize = MAX_TOK_BUFSIZE;
    int position = 0;
    char **tokens = (char**)malloc(bufsize*sizeof(char*));
    char *token;

    if(!tokens){
        cout << "Allocation Failed" << endl; /* Không đủ bộ nhớ cấp phát */
        exit(EXIT_FAILURE);
    }

    token = strtok(line, TOKEN_DELIMETERS); /*Con trỏ trỏ tới args[0] của lệnh cmd VD: cd, dir*/
    while(token != NULL){
        tokens[position] = token; /* Lưu các con trỏ chứa thành phần của lệnh cmd */
        position++; 
        if(position >= bufsize){ /* số thành phần args[i] trong lệnh cmd lớn hơn số bufsize dự tính*/
            bufsize += MAX_TOK_BUFSIZE; 
            tokens = (char**)realloc(tokens, bufsize); 
            if(!tokens){
                cout << "Allocation Failed" << endl; 
                exit(EXIT_FAILURE);
            } 
        }
        token = strtok(NULL,TOKEN_DELIMETERS); /* Trỏ token tới thành phần args tiếp theo trong của cmd*/
    }
    tokens[position] = NULL; /*Kết thúc danh sách mã*/
    return tokens;
}

/**
 * Chuẩn hóa đường dẫn [path], tránh trường hợp như:
 * VD: cd E:\New folder\ => [path] = 'E:\New'
 **/
wchar_t* combinePath(char **args, int start) {
    wchar_t *path = (wchar_t *)calloc(MAX_CWD_LENS, sizeof(wchar_t)); /*Các thành phần không bị ngăn cách*/
    wchar_t *pivot = _wcsdup(L""); /* Dấu ngăn cách */
    
    /* Chuyển đổi args[start] từ char* sang wchar_t* và kết hợp */
    wchar_t wArgs[MAX_CWD_LENS];
    mbstowcs(wArgs, args[start], strlen(args[start]) + 1);
    wcscat(path, wArgs);
    
    int i = start + 1;
    while (args[i] != NULL) {
        wcscat(path, pivot);
        mbstowcs(wArgs, args[i], strlen(args[i]) + 1);
        wcscat(path, wArgs);
        ++i;
    }
    return path;
}

/**
 * Đổi màu chữ 
 * 
 **/
void setColor(char* color) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
    WORD saved_attributes;

    /* Lưu các thuộc tính */
    // GetConsoleScreenBufferInfo(hConsole, &consoleInfo);
    // saved_attributes = consoleInfo.wAttributes;

    if (strcmp(color,"green")==0) {
      SetConsoleTextAttribute(hConsole,FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    }
}