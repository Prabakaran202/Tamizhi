#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

int global_server_fd = -1;

// 🌟 இதுதான் அவுட்புட் பைனரியில் ரன் ஆகப்போகும் ஒரிஜினல் Listen ஃபங்ஷன்!
void tamizhi_rt_listen(int port) {
    global_server_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(global_server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    bind(global_server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(global_server_fd, 3);
    
    printf("\n🌐 [Tamizhi Run-Time] Web Server started on port %d...\n", port);
}

// 🌟 இதுதான் பிரவுசரை கனெக்ட் பண்ணி "வணக்கம்" சொல்லப்போகும் ஃபங்ஷன்!
void tamizhi_rt_accept() {
    if (global_server_fd == -1) return;

    struct sockaddr_in address;
    int addrlen = sizeof(address);
    
    printf("⏳ Waiting for connection in browser...\n");
    int new_socket = accept(global_server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
    
    if (new_socket >= 0) {
        printf("✅ New Client Connected! Sending Response...\n");

        // இதுதான் பிரவுசருக்குப் புரியும் HTTP Response Format!
        char *http_response = 
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/plain; charset=utf-8\r\n"
            "Connection: close\r\n"
            "\r\n"
            "வணக்கம் பிரபா! தமிழி சர்வர் வெற்றிகரமாக இயங்குகிறது! 🔥";
            
        write(new_socket, http_response, strlen(http_response));
        close(new_socket);
    }
}
