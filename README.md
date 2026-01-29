🚀 Win32 UDP Low-Latency Streamer
A custom, high-performance remote desktop protocol built from scratch in C++ using raw Win32 APIs and UDP sockets. Designed to understand and minimize network latency at the packet level.

🛠️ Tech Stack
Language: C++ (No 3rd party frameworks)

Networking: Raw UDP Sockets (winsock2)

Graphics: Win32 GDI & Bitmap Manipulation

Optimization: Custom Delta Compression & Multithreading

⚡ Why I Built This
Most remote desktop tools (Zoom, TeamViewer) rely on TCP or heavy encoders like H.264. I wanted to understand the "metal" of networking:

Manual Packet Sequencing: Handling UDP packet loss and reordering without TCP's overhead.

Memory Management: Zero-allocation ring buffers to prevent heap fragmentation.

Concurrency: Separating capture and network threads to ensure 60 FPS responsiveness.

🔧 How to Build
No external dependencies required (Static linking).

1. Clone the repo

Bash

git clone https://github.com/Slashin3/remote-desktop.git

2. Compile (MinGW/G++)

Bash

### ⚙️ Configuration
Before compiling, you need to set the IP address of the computer running the **Streamer**.

1. Open `player_remote.cpp` and `streamer_remote.cpp`.
2. Find the line:
   ```cpp
   #define TARGET_IP =  "127.0.0.1" // Change this to your ip.

# Compile the Streamer (Host)
g++ streamer_remote.cpp -o streamer_remote.exe -lgdi32 -lws2_32 -static

# Compile the Player (Client)
g++ player_remote.cpp -o player_remote.exe -lgdi32 -lws2_32 -static



🚧 Current Limitations
Uses CPU-bound GDI capture (Planning migration to DirectX/DXGI).

No NAT Traversal (Works on LAN or VPN).

1080p requires high bandwidth (Optimization in progress).
