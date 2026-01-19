#include <winsock2.h>
#include <windows.h>
#include <iostream>
#include <vector>
#include <cstring>
#include <stdio.h>

#pragma comment(lib, "ws2_32.lib")

#define TARGET_IP "127.1.1.0" 

#define W 640
#define H 480
#define VIDEO_PORT 9999
#define CONTROL_PORT 9998
#define ROW_SIZE (W * 3)

DWORD WINAPI ControlThread(LPVOID lpParam) {
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(CONTROL_PORT);
    addr.sin_addr.s_addr = INADDR_ANY;
    
    if (bind(sock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        std::cout << "Error:control port bind failed" << CONTROL_PORT << "\n";
        return 0;
    }

    std::cout << " Listening for Mouse on Port " << CONTROL_PORT << "...\n";
    
    char buffer[1024];
    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);

    while(true) {
        int len = recv(sock, buffer, 1024, 0);
        if(len > 0) {
            buffer[len] = '\0';
            
            char type[5];
            float nx, ny;
        
            sscanf(buffer, "%s %f %f", type, &nx, &ny);

            if (strcmp(type, "M") == 0) {
                
                SetCursorPos((int)(nx * screenW), (int)(ny * screenH));
            }
            else if (strcmp(type, "LD") == 0) {
                
                mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
            }
            else if (strcmp(type, "LU") == 0) {
               
                mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
            }
        }
    }
    return 0;
}
// ------------------------------------------------

int main() {
   
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    
    CreateThread(NULL, 0, ControlThread, NULL, 0, NULL);

    SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
    sockaddr_in target;
    target.sin_family = AF_INET;
    target.sin_port = htons(VIDEO_PORT);
    target.sin_addr.s_addr = inet_addr(TARGET_IP);

    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);
    HDC hScreen = GetDC(NULL);
    HDC hMemory = CreateCompatibleDC(hScreen);
    HBITMAP hBitmap = CreateCompatibleBitmap(hScreen, W, H);
    SelectObject(hMemory, hBitmap);

    std::vector<char> current_frame(W * H * 3);
    std::vector<char> previous_frame(W * H * 3, 0);
    
    BITMAPINFO bmi = {0};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = W;
    bmi.bmiHeader.biHeight = -H; 
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 24;
    bmi.bmiHeader.biCompression = BI_RGB;

    std::cout << "🚀 Streamer Started. Handing over control...\n";

    while (true) {
        SetStretchBltMode(hMemory, COLORONCOLOR);
        StretchBlt(hMemory, 0, 0, W, H, hScreen, 0, 0, screenW, screenH, SRCCOPY);
        GetDIBits(hMemory, hBitmap, 0, H, current_frame.data(), &bmi, DIB_RGB_COLORS);

        for (int row = 0; row < H; row++) {
            char* curr_ptr = &current_frame[row * ROW_SIZE];
            char* prev_ptr = &previous_frame[row * ROW_SIZE];
            
            if (std::memcmp(curr_ptr, prev_ptr, ROW_SIZE) != 0) {
                std::vector<char> packet(2 + ROW_SIZE);
                unsigned short row_id = (unsigned short)row;
                memcpy(&packet[0], &row_id, 2);
                memcpy(&packet[2], curr_ptr, ROW_SIZE);
                
                sendto(sock, packet.data(), packet.size(), 0, (sockaddr*)&target, sizeof(target));
                memcpy(prev_ptr, curr_ptr, ROW_SIZE);
            }
        }
        Sleep(15); 
    }
    return 0;
}