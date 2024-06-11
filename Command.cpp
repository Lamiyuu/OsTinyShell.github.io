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
#include "Process.h"
#include "Conversion.h"

#define MAX_CWD_LENS 128
#define MAX_BUFFER_SIZE 64
#define MAX_TOK_BUFSIZE 64
#define TOKEN_DELIMETERS " \t\r\n\a"

using namespace std;

/* Mảng các câu lệnh command được hỗ trợ */
const char *command[] = {
    "cd",     
    "cls",    
    "echo",  
    "exit", 
    "help", 
    "del",
    "dir",
    "mkdir", 
    "date",
    "time",
    "pc",
    "calc",
    "run"
};

/* Mảng các lệnh command*/
int (*activate_command[])(char **) = { 
  &cd,
  &cls,
  &echo,
  &exit,
  &help,
  &del,
  &dir,
  &mk_dir,
  &date,
  &time_cmd,
  &pc,
  &calc,
  &runbat	  
}; 

/**
 * Chạy lệnh với lệnh là arg[0] như cd, dir, exit, help
 **/
int execute(char ** args){
    if (args[0] == NULL){
        return 0;
    }
    for(int i=0; i < size_of_command() ; i++){
        if(strcmp(args[0],command[i]) == 0){ /* Kiểm tra xem người dùng nhập lệnh nào trong tập lệnh */
            return (*activate_command[i])(args);
        }
    }
    printf("Not supported command. Please try again. \n");
    return 0;
}

/**
 * Trả về số câu lệnh trong tập các lệnh 
 **/
int size_of_command(){
    return sizeof(command) / sizeof(char *);
}

//////////////////////////////////////////
////////// Danh sách câu lệnh //////////// 
//////////////////////////////////////////

int help(char **args){
    if (args[1] == NULL) {
	printf("Type \"help [command]\" for more information about a specific command.\n");
	printf("Supported commands:\n cd, date, time, dir, cls, echo, del, mkdir, pc, exit\n ");
	printf("Usage:\n\t <command> [option]\n\tEXAMPLE: help cd\n");
	printf("%-30s%s\n%-30s%s", " cd",
	        "Change the current directory. You must write the new directory after this command.",
		" ", "EXAMPLES: \"cd C:/\"\n\n");
	printf("%-30s%s\n%-30s%s", " date",
		"Show today's date.",
		" ", "EXAMPLES: \"date\"\n\n");
	printf("%-30s%s\n%-30s%s", " time",
		"Show current time.",
		" ", "EXAMPLES: \"time\"\n\n");
	printf("%-30s%s\n%-30s%s", " dir",
		"Show all files and folders in the current directory.",
		" ", "EXAMPLES: \"dir\"\n\n");
        printf("%-30s%s\n%-30s%s", " cls",
		"Clear console screen.",
            	" ", "EXAMPLES: \"cls\"\n\n");
        printf("%-30s%s\n%-30s%s", " echo",
		"Print a message on the screen.",
            	" ", "EXAMPLES: \"echo [message]\"\n\n");
        printf("%-30s%s\n%-30s%s", " del",
		"Delete a file or folder in the current directory.",
            	" ", "EXAMPLES: \"del [Folder/Filename]\"\n\n");
        printf("%-30s%s\n%-30s%s", " mkdir",
		"Make a new directory.",
            	" ", "EXAMPLES: \"mkdir[Foldername]\"\n\n");
	printf("%-30s%s\n%-30s%s", " run",
		"Run .bat file only.",
            	" ", "EXAMPLES: \"run [filename.bat]\"\n\n");
        printf("%-30s%s\n%-30s%s\n%-30s%s", " pc",
            	"Process.", " ",
            	"You must enter the options in the 2nd argument, such as fg, bg, all, find, kill, killid, suspend, resume",
            	" ", "EXAMPLES: \"pc bg\"\n\n");
	printf("%-30s%s", " exit", "Exit this tiny shell :((((\n");
    }
    else if(!strcmp(args[1],"cd")) {
        cout << "Change the current directory." << endl;
        cout << "       cd           : Show the current directory" << endl;
        cout << "       cd ..        : Show the parent directory of the current directory" << endl;
        cout << "       cd [path]    : Change the current directory to [path]" << endl;
        cout << "EXAMPLE: cd C:\\Users\\Admin\\ => change current directory to C:\\User\\Admin" << endl;
    }
    else if(!strcmp(args[1],"date")) {
        cout << "Display the current date to screen." << endl;
        cout << "This command does not support any options." << endl;
    }
    else if(!strcmp(args[1],"time")){
        cout << "Diplay the current time to screen." << endl;
        cout << "This command does not support any options." << endl;
    }
    else if(!strcmp(args[1],"dir")){
        cout << "Display the list of files and folder in the directory to the screen." << endl;
        cout << "       dir         : Show the files and folders in the current directory" << endl;
        cout << "       dir [path]  : Show the files and folders in the [path]" << endl;
        cout << "EXAMPLE: \"dir\", \"dir C:\\Users\\Admin\"" << endl;
    }
    else if(!strcmp(args[1],"cls")){
        cout << "Clear all line displaying on the console screen." << endl;
        cout << "This command does not support any options." << endl;
    }
    else if(!strcmp(args[1],"echo")){
        cout << "Print the message on the screen." << endl;
        cout << "This command does not support any options." << endl;
        cout << "EXAMPLE: \"echo Hello World\"" << endl;
        cout << "       => \"Hello World\"" << endl;
    }
    else if(!strcmp(args[1],"del")){
        cout << "Delete file or folder in the current directory." << endl;
        cout << "Recommend: Folder/File name contains no space." << endl;
        cout << "       del [Folder/Filename] : Delete folder/file (Folder/Filename) in the current dirrectory" << endl;
        cout << "EXAMPLE: \"del ABC\"" << endl; 
    }
    else if(!strcmp(args[1],"mkdir")){
        cout << "Make a new directory in the current directory." << endl;
        cout << "Recommend: Foldername contains no space." << endl;
        cout << "       mkdir [Foldername] : Make folder [Foldername] in the current directory" << endl;
        cout << "EXAMPLE: \"mkdir ABC\"" << endl;
    }
    else if (!strcmp(args[1], "run")){
        cout << "Run .bat file only with commands which our shell supports" << endl;
        cout << "       run [Filename.bat] : Run .bat file with commands which our shell supports " << endl;
        cout << "EXAMPLE: \"run command.bat\"" << endl;
    }
    else if(!strcmp(args[1],"pc")){
        cout << "Supported options:" << endl;
        cout << "        all     Show list of all running processes" << endl;
        cout << "        find    Get pid of specific program(s) by name" << endl;
        cout << "        suspend Suspend a program by process id" << endl;
        cout << "        resume  Resume a program by process id" << endl;
        cout << "        kill    Terminate all program by name" << endl;
        cout << "        killid  Terminate a program by process id" << endl;
        cout << "        bg      Run a program in background mode by path to program" << endl;
	    cout << "        fg      Run a program in foregound mode by path to program" << endl;
        cout << "EXAMPLE: \"pc fg C:\\Users\\Admin\\ABC\"" << endl;
    }
    else if(!strcmp(args[1],"exit")){
        cout << "Exit the TinyShell." << endl;
        cout << "This command does not support any options." << endl; 
    }
    return 0;
}

/**
 * Chuyển directory hiện tại sang directory mới
 * Câu lệnh: cd [path] 
 * cd : trả về đường dẫn directory hiện tại 
 * cd .. :trả về đường dẫn directory cha
 * cd [path]: chuyển current working directory sang directory mới
 * 
 **/
int cd(char **args) {
    if(args[1] == NULL) { /* Nếu chỉ gõ lệnh cd */
        system("cd"); /* Dùng luôn lệnh cd có sẵn của Windows (Chơi bẩn nhưng kệ) */
        cout << endl;
        return EXIT_SUCCESS;
    }
    /* Nếu cd [path] */
    else {
        wchar_t* path = combinePath(args, 1); /* Chuẩn hóa path */
        if(SetCurrentDirectoryW(path) == FALSE) { /* Tìm đường dẫn nếu có */
            fwprintf(stdout, L"Not able to set current working directory\n");
        }
        free(path);
    }
    return 0;
}

/**
 * Liệt kê các folder, file trong directory 
 * Câu lệnh: dir [path] 
 * 
 **/
int dir(char **args) {
    wchar_t *cur_dir = (wchar_t *)malloc(MAX_CWD_LENS * sizeof(wchar_t));
    GetCurrentDirectoryW(MAX_CWD_LENS, cur_dir);
    WIN32_FIND_DATAW data;
    FILETIME time_lastwrite;
    SYSTEMTIME convert_time;
    string date;
    string time;
    wchar_t *char_date = (wchar_t *)calloc(15, sizeof(wchar_t));
    wchar_t *char_time = (wchar_t *)calloc(15, sizeof(wchar_t));
    const wchar_t add[] = L"\\*";
    wchar_t *path = (wchar_t *)malloc(MAX_CWD_LENS * sizeof(wchar_t));
    if (args[1] == NULL) {
        wcscpy(path, cur_dir);
        wcscat(path, L"\\*");
    } else {
        path = combinePath(args, 1);
        wcscat(path, L"\\*");
    }
    HANDLE han = FindFirstFileW(path, &data);
    wprintf(L"%-15ls%-15ls%-15ls%-15ls\n", L"Date", L"Time", L"Type", L"Name");
    wprintf(L"=========================================================================\n");
    if (han != INVALID_HANDLE_VALUE) {
        do {
            time_lastwrite = data.ftLastWriteTime;
            FileTimeToSystemTime(&time_lastwrite, &convert_time);

            date = to_string(convert_time.wDay) + '/' + to_string(convert_time.wMonth) + '/' + to_string(convert_time.wYear);
            time = to_string(convert_time.wHour + 7) + ':' + to_string(convert_time.wMinute);
            mbstowcs(char_date, date.c_str(), date.size() + 1);
            mbstowcs(char_time, time.c_str(), time.size() + 1);

            if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                if (wcscmp(data.cFileName, L".") != 0 && wcscmp(data.cFileName, L"..") != 0) {
                    wprintf(L"%-15ls%-15ls%-15ls%-15ls\n", char_date, char_time, L"<FOLDER>", data.cFileName);
                }
            } else {
                wprintf(L"%-15ls%-15ls%-15ls%-15ls\n", char_date, char_time, L"<FILE>", data.cFileName);
            }
        } while (FindNextFileW(han, &data) != 0);
        FindClose(han);
        free(char_time);
        free(char_date);
        free(cur_dir);
        free(path);
        return EXIT_SUCCESS;
    } else {
        free(char_time);
        free(char_date);
        free(cur_dir);
        free(path);
        return EXIT_FAILURE;
    }
}

/**
 * Tạo folder trong directory hiện tại
 * Câu lệnh: mkdir [foldername]
 * 
 **/
int mk_dir(char **args){
    if(args[1] == NULL){ /* Chỉ gõ lệnh mkdir */
        cout << "ERROR: Command mk_dir need filename" << endl;
        cout << "Command: mkdir [filename]" << endl;
        cout << "Recommend: filename should not have any space" << endl;
        return 0;
    }
    mkdir(args[1]); /* Lệnh mkdir tạo folder có sẵn */
    return 0;
}

/**
 * In ra message đi kèm với echo
 * Câu lệnh: echo [message]
 *  
 **/
int echo(char **args){
    if(args[1] == NULL){
        cout << "ERROR: Echo + [message]" << endl;
        return 0;
    }
    int i=0;
    while(args[++i] != NULL){
        for(int j=0; j<strlen(args[i]); j++){
            cout << args[i][j];
        }
        cout << " ";
    }
    cout << endl;
    return 0;
}

/**
 * Clear toàn màn hình console
 * Câu lệnh: cls
 **/
int cls(char **args){ 
    if(strcmp(args[0],"cls") == 0){
        HANDLE hConsole; 
        hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        SMALL_RECT scrollRect;
        COORD scrollTarget;
        CHAR_INFO fill;

        // Get the number of character cells in the current buffer.
        if (!GetConsoleScreenBufferInfo(hConsole, &csbi))
        {
            return 0;
        }

        // Scroll the rectangle of the entire buffer.
        scrollRect.Left = 0;
        scrollRect.Top = 0;
        scrollRect.Right = csbi.dwSize.X;
        scrollRect.Bottom = csbi.dwSize.Y;

        // Scroll it upwards off the top of the buffer with a magnitude of the entire height.
        scrollTarget.X = 0;
        scrollTarget.Y = (SHORT)(0 - csbi.dwSize.Y);

        // Fill with empty spaces with the buffer's default text attribute.
        fill.Char.UnicodeChar = TEXT(' ');
        fill.Attributes = csbi.wAttributes;

        // Do the scroll
        ScrollConsoleScreenBuffer(hConsole, &scrollRect, NULL, scrollTarget, &fill);

        // Move the cursor to the top left corner too.
        csbi.dwCursorPosition.X = 0;
        csbi.dwCursorPosition.Y = 0;

        SetConsoleCursorPosition(hConsole, csbi.dwCursorPosition);
    }
    return 0;
}

/**
 * In ra màn hình ngày hiện tại
 * Câu lệnh: date
 * 
 **/
int date(char **args){
    if (args[1] != NULL)
	{
		cout << "Command \"date\" does not support any option !\n";
		return EXIT_FAILURE;
	}
	time_t t = time(0);
	struct tm * now = localtime(&t);
	cout << "Current Date: " << now->tm_mday << '/'
		<< (now->tm_mon + 1) << '/'
		<< (now->tm_year + 1900)
		<< endl;
	return 0;
}

/**
 * In ra màn hình thời gian (giờ:phút:giây) hiện tại
 * Câu lệnh: time
 * 
 */
int time_cmd(char **args){
    if (args[1] != NULL)
	{
		cout << "Command \"time\" does not support any option !\n";
		return EXIT_FAILURE;
	}
	time_t t = time(0);
	struct tm * now = localtime(&t);
	cout << "Current time: " << now->tm_hour << ':' << now->tm_min << ':' << now->tm_sec << endl;
	return 0;
}

/**
 * Delete file hoặc folder
 * Câu lệnh: del path
 *  file/folder
 **/
int del(char **args){
    if(args[1] == NULL){
        cout << "ERROR: Command 'del' need path" << endl;
        cout << "Command: del [path]" << endl;
        return 0;
    }
    else if (args[2] != NULL){
        cout << "ERROR: Command 'del' cannot delete more than 1 file" << endl;
        cout << "Command: del [path]" << endl;
        return 0;
    }
    if(remove(args[1]) == -1 && rmdir(args[1]) == -1){ /* Nếu arg[1] là file thì xóa file, folder thì xóa folder. Nếu k có thì in chỗ dưới */
        cout << "ERROR: Unable to find file to be deleted. Try again" << endl;
        cout << "Command: del [path]" << endl;
        return 0;
    }
    return 0;
}

/**
 * Thoát chương trình
 * Câu lệnh: exit
 * 
 **/
int exit(char **args) {
    wchar_t* term = (wchar_t*)malloc(64 * sizeof(wchar_t));
    if (args[1] != NULL) {
        term = combinePath(args, 1);
        wcout << L"ERROR: Term " << term << L" is not recognized for EXIT" << endl;
        wcout << L"Command: exit" << endl;
        return 0;
    }
    /* Trả 1 về cho biết stop trong main */
    return 1;
}
/**
 * Các lệnh với tiến trình
 * Câu lệnh: pc (Process)
 * 
 **/
int pc(char **args) {
    if (args[1] == NULL) {
        cout << "ERROR: Too few argument" << endl;
        return 0;
    }
    if (strcmp(args[1], "all") == 0) {
        if (getProcessListAll()) {
            return 0;
        }
        return 1;
    }
    if (strcmp(args[1], "find") == 0){
		if (args[2] == NULL)
		{
			printf("ERROR: Too few arguments\n");
			return 0;
		}
		// Tìm ID Process
		if (findProcessID(convertToWideChar(args[2])))
			return 0;
        return 1;
	}

    if (strcmp(args[1], "kill") == 0) {
        if (args[2] == NULL) {
            cout << "ERROR: Too few argument" << endl;
            return 0;
        }

        if (killProcess(convertToWideChar(args[2]))) {
            return 0;
        }
        return 1;
    }

    if (strcmp(args[1], "killid") == 0) {
        if (args[2] == NULL) {
            cout << "ERROR: Too few argument" << endl;
            return 0;
        }
        DWORD process_id = atoi(args[2]);
        if (killProcessID(process_id)) {
            return 0;
        }
        return 1;
    }

    if (strcmp(args[1], "suspend") == 0) {
        if (args[2] == NULL) {
            cout << "ERROR: Too few argument" << endl;
            return 0;
        }
        DWORD process_id = atoi(args[2]);
        suspendProcess(process_id);
        return 0;
    }

    if (strcmp(args[1], "resume") == 0) {
        if (args[2] == NULL) {
            cout << "ERROR: Too few argument" << endl;
            return 0;
        }
        DWORD process_id = atoi(args[2]);
        resumeProcess(process_id);
        return 0;
    }

    if (strcmp(args[1], "bg") == 0 || strcmp(args[1], "fg") == 0) {
        if(args[2] == NULL) {
            cout << "ERROR: Too few argument" << endl;
            return 0;
        } else {
            createNewProcess(args);
        }
        return 0;
    } 
    if(strcmp(args[1], "bgTofg") == 0){
        if(args[2] == NULL) {
            cout << "ERROR: Too few argument" << endl;
            return 0;
        } else{
            DWORD process_id = atoi(args[2]);
            switchProcessMode(process_id);
            return 0;
        }
    }
    else {
        cout << "ERROR: Too few argument" << endl;
        return 0;
    }
}

int calc(char **args){
    system("calc");
    return 0;
}

//////////////////////////////////////////
//////// Dành riêng cho file .bat ////////
//////////////////////////////////////////

/**
 * Kiểm tra xem câu lệnh có được hỗ trợ trong shell không
 * 
 **/
bool cmdCheck(char **args){
    if (args[0] == NULL){
        return 0;
    }
    for(int i=0; i < size_of_command() ; i++){
        if(strcmp(args[0],command[i]) == 0){ /* Kiểm tra xem người dùng nhập lệnh nào trong tập lệnh */
            return true;
        } 
    }
    return false;
}

/**
 * Chạy riêng cho file .bat  
 * Câu lệnh: run [Filename.bat]
 * 
 **/
int runbat(char **args){
    wchar_t w[255], a[255];
    wchar_t *run_file = combinePath(args, 1); // Chuyển đổi kiểu dữ liệu và ghép đường dẫn
    FILE *f = _wfopen(run_file, L"rt"); // Sử dụng _wfopen thay vì fopen để mở file wchar_t
    if(f == NULL) {
        wprintf(L"\nLoi doc file.\n");
        return 0;
    }
    else {
        while(!feof(f)) {
            fgetws(w, 255, f); // Sử dụng fgetws thay vì fgets để đọc dữ liệu wchar_t
            wcscpy(a, w);
            char **arg = separator_line(convertToChar(a)); // Chuyển đổi kiểu dữ liệu của dữ liệu đọc được
            printf("");
            if(cmdCheck(arg)) {
                int stop = execute(arg);
            }
            else {
                _wsystem(w); // Sử dụng _wsystem thay vì system để chạy lệnh wchar_t
            }
        }
        fclose(f);
    }
    return 0;
}
