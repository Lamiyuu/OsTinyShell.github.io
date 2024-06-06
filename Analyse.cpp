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
//			"     Ta Ton" -> "Ta Tona Ton"
    while(real_len--){
        (cmd)[i] = (cmd)[i+first];
        i++;
    }
    (cmd)[length - first] = '\0'; //"Ta Tona Ton" -> "Ta Ton"
}

/**
 * In ra màn hình console đường dẫn 
 * (VD: C:\Users\Admin\.....> $ *phần tiếp này là lệnh*)
 **/
int printPrompt(wchar_t *cur_directory){
    if(GetCurrentDirectoryW(MAX_CWD_LENS,cur_directory) == 0){
        cout << "Reading of current working directory failed.\n";
        return -1;
    }
    cout << cur_directory << "> $";   
    return 1;
}

/**
 * Chia câu lệnh thành các đoạn ngăn cách bởi ký tự ngăn (" ","\t","\r","\n","\a")
 * 
 **/
char **separate_line(char *line){
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
        // cout << tokens[position] << endl;
        position++; 
        if(position >= bufsize){ /* số thành phần args[i] trong lệnh cmd lớn hơn số bufsize dự tính*/
            bufsize += MAX_TOK_BUFSIZE; /* Tăng số bufsize */
            tokens = (char**)realloc(tokens, bufsize); /* Cấp phát thêm bộ nhớ cho tokens */
            if(!tokens){
                cout << "Allocation Failed" << endl; /* Không đủ bộ nhớ cấp phát */
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
 * Đổi màu chữ (Có thể có hoặc k cơ bản là cũng k có ảnh hưởng gì chỉ là trang trí )
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