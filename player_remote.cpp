#include <winsock2.h>
#include <windows.h>
#include <vector>
#include <iostream>
#include <string>

#pragma comment(lib, "ws2_32.lib")

// --- CONFIGURATION ---
#define W 640
#define H 480
#define LISTEN_PORT 9999 
#define CONTROL_PORT 9998
#define ROW_SIZE (W * 3)

#define TARGET_IP "127.1.1.0" 

std::vector<char> frame_buffer(W * H * 3, 0); 
SOCKET control_sock;
sockaddr_in target_addr;

void SendMouse(std::string type, int x, int y) {
    float nx = (float)x / W;
    float ny = (float)y / H;

    char packet[50];
    sprintf(packet, "%s %.4f %.4f", type.c_str(), nx, ny);

    sendto(control_sock, packet, strlen(packet), 0, (sockaddr*)&target_addr, sizeof(target_addr));
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch(uMsg) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            BITMAPINFO bmi = {0};
            bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
            bmi.bmiHeader.biWidth = W;
            bmi.bmiHeader.biHeight = -H; // Top-down
            bmi.bmiHeader.biPlanes = 1;
            bmi.bmiHeader.biBitCount = 24;
            bmi.bmiHeader.biCompression = BI_RGB;
            StretchDIBits(hdc, 0, 0, W, H, 0, 0, W, H, frame_buffer.data(), &bmi, DIB_RGB_COLORS, SRCCOPY);
            EndPaint(hwnd, &ps);
            return 0;
        }
        
        // --- MOUSE INPUTS ---
        case WM_MOUSEMOVE: {
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);
            if (x >= 0 && x < W && y >= 0 && y < H) {
                SendMouse("M", x, y);
            }
            return 0;
        }

        case WM_LBUTTONDOWN: {
            SendMouse("LD", 0, 0);
            return 0;
        }

        case WM_LBUTTONUP: {
            SendMouse("LU", 0, 0);
            return 0;
        }

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

DWORD WINAPI NetworkThread(LPVOID lpParam) {
    HWND hwnd = (HWND)lpParam;
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
    
    int buff_size = 1024 * 1024; 
    setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char*)&buff_size, sizeof(buff_size));

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(LISTEN_PORT);
    addr.sin_addr.s_addr = INADDR_ANY;
    bind(sock, (sockaddr*)&addr, sizeof(addr));

    std::vector<char> packet_buffer(10000); 

    while (true) {
        int len = recv(sock, packet_buffer.data(), 10000, 0);
        if (len > 2) {
            unsigned short row_id = 0;
            memcpy(&row_id, &packet_buffer[0], 2);

            if (row_id < H) {
                char* dest = &frame_buffer[row_id * ROW_SIZE];
                char* src = &packet_buffer[2];
                int data_len = len - 2;
                
                if(data_len == ROW_SIZE) {
                    memcpy(dest, src, ROW_SIZE);
                    InvalidateRect(hwnd, NULL, FALSE);
                }
            }
        }
    }
    return 0;
}

int main() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    control_sock = socket(AF_INET, SOCK_DGRAM, 0);
    target_addr.sin_family = AF_INET;
    target_addr.sin_port = htons(CONTROL_PORT);
    target_addr.sin_addr.s_addr = inet_addr(TARGET_IP);

    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = "ControlPlayer";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClass(&wc);
    
    HWND hwnd = CreateWindow("ControlPlayer", "OnePlay C++ Client", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 100, 100, W+20, H+40, NULL, NULL, NULL, NULL);
    
    CreateThread(NULL, 0, NetworkThread, hwnd, 0, NULL);

    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0)) { TranslateMessage(&msg); DispatchMessage(&msg); }
    
    return 0;
}