#include <iostream>
#include <chrono>
#include <thread>

void countdown(int total_seconds){
    while (total_seconds >= 0) {
        int hours = total_seconds / 3600;
        int minutes = (total_seconds % 3600) / 60;
        int seconds = total_seconds % 60;

        std::cout << "\rTime remaining: " 
                  << hours << " hours " 
                  << minutes << " minutes " 
                  << seconds << " seconds " << std::flush;

        std::this_thread::sleep_for(std::chrono::seconds(1));
        --total_seconds;
    }
    std::cout << "\nTime's up!" << std::endl;
}

int main() {
    int time = 300;
    countdown(time);
    return 0;
}