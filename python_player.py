import socket
import struct
import pygame

HOST_IP_FOR_CONTROLS = "10.219.193.23" 

LISTEN_IP = "0.0.0.0"
VIDEO_PORT = 9999
CONTROL_PORT = 9998
WIDTH = 640
HEIGHT = 480
ROW_SIZE = WIDTH * 3 

pygame.init()
screen = pygame.display.set_mode((WIDTH, HEIGHT))
pygame.display.set_caption(f"Connected to {HOST_IP_FOR_CONTROLS}")

video_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
video_sock.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, 1024 * 1024)
video_sock.bind((LISTEN_IP, VIDEO_PORT))
video_sock.settimeout(0.001)

control_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

def send_mouse(cmd, x, y):
    
    nx = x / WIDTH
    ny = y / HEIGHT
    
    message = f"{cmd} {nx:.4f} {ny:.4f}"
    control_sock.sendto(message.encode(), (HOST_IP_FOR_CONTROLS, CONTROL_PORT))

print(f"Client Started. Video Port: {VIDEO_PORT}, Control Target: {HOST_IP_FOR_CONTROLS}")

running = True
clock = pygame.time.Clock()

while running:

    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False
        
        elif event.type == pygame.MOUSEMOTION:
            x, y = event.pos

            if 0 <= x < WIDTH and 0 <= y < HEIGHT:
                send_mouse("M", x, y)
                
        elif event.type == pygame.MOUSEBUTTONDOWN:
            if event.button == 1:
                send_mouse("LD", 0, 0)
                
        elif event.type == pygame.MOUSEBUTTONUP:
            if event.button == 1:
                send_mouse("LU", 0, 0)

    try:
        while True:
            try:
                data, addr = video_sock.recvfrom(4096)
                if len(data) > 2:
                    row_id = struct.unpack('H', data[:2])[0]
                    if row_id < HEIGHT and len(data) - 2 == ROW_SIZE:
                        pixel_data = data[2:]
                        row_surf = pygame.image.fromstring(pixel_data, (WIDTH, 1), 'RGB')
                        screen.blit(row_surf, (0, row_id))
            except socket.timeout:
                break
            except BlockingIOError:
                break
    except Exception:
        pass

    pygame.display.flip()
    clock.tick(60)

pygame.quit()