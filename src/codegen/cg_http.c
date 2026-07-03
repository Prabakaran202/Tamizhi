#include "codegen_bridge.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>

// குளோபல் சாக்கெட் டிஸ்கிரிப்டர்
int global_server_fd = -1;

void tamizhi_gen_socket_listen(int port) {
    // 1. சாக்கெட் உருவாக்குதல்
    global_server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (global_server_fd == -1) {
        fprintf(stderr, "[HTTP Error] சாக்கெட்டை உருவாக்க முடியவில்லை!\n");
        return;
    }

    // சாக்கெட் ஆப்ஷன்ஸ் (Port-ஐ மீண்டும் பயன்படுத்த அனுமதித்தல்)
    int opt = 1;
    setsockopt(global_server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    // 2. Bind செய்தல்
    if (bind(global_server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        fprintf(stderr, "[HTTP Error] Bind தோல்வி!\n");
        return;
    }

    // 3. Listen செய்தல்
    if (listen(global_server_fd, 3) < 0) {
        fprintf(stderr, "[HTTP Error] Listen தோல்வி!\n");
        return;
    }
    
    printf("[HTTP System] போர்ட் %d-ல் சர்வர் இயங்குகிறது...\n", port);
}

void tamizhi_gen_socket_accept() {
    if (global_server_fd == -1) {
        fprintf(stderr, "[HTTP Error] சர்வர் இன்னும் தொடங்கப்படவில்லை!\n");
        return;
    }

    struct sockaddr_in address;
    int addrlen = sizeof(address);
    
    // 4. Accept செய்தல் (கிளையண்ட் கனெக்ஷனுக்காக காத்திருக்கும்)
    int new_socket = accept(global_server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
    
    if (new_socket < 0) {
        fprintf(stderr, "[HTTP Error] Accept தோல்வி!\n");
        return;
    }
    
    printf("[HTTP System] புதிய கிளையண்ட் இணைக்கப்பட்டார்!\n");
    // இங்கே கிளையண்டிற்கு ரெஸ்பான்ஸ் அனுப்பும் லாஜிக்கை அடுத்து சேர்க்கலாம்
    close(new_socket);
}
