#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

int global_server_fd = -1;

// 🌟 1. அவுட்புட் பைனரியில் ரன் ஆகப்போகும் ஒரிஜினல் Listen ஃபங்ஷன்
void tamizhi_rt_listen(double port_num) {
    int port = (int)port_num; // Double-ஐ Int ஆக மாற்றுகிறோம்
    
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
    fflush(stdout);
}

// 🌟 2. பிரவுசரை கனெக்ட் செய்யும் ஃபங்ஷன் 
double tamizhi_rt_accept() {
    if (global_server_fd == -1) return -1.0;

    struct sockaddr_in address;
    int addrlen = sizeof(address);

    printf("⏳ Waiting for connection in browser...\n");
    int new_socket = accept(global_server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);

    if (new_socket >= 0) {
        printf("✅ New Client Connected! Processing Response...\n");
        fflush(stdout);
        return (double)new_socket; // தமிழி 'Num'-க்காக Double ஆக ரிட்டர்ன் செய்கிறோம்
    }
    
    return -1.0;
}

// 🌟 3. தமிழி ரன்டைம்: முழுமையான டைனமிக் HTTP ரெஸ்பான்ஸ் ஃபங்ஷன்
void tamizhi_rt_send_response(double client_socket_num, double status_code_num, const char* content) {
    int client_socket = (int)client_socket_num; // Double-ஐ Int ஆக மாற்றுகிறோம்
    int status_code = (int)status_code_num;     // Double-ஐ Int ஆக மாற்றுகிறோம்

    if (client_socket < 0) return;

    char header[2048];
    char final_response[8192];
    const char* status_text = "OK";

    // HTTP Status Codes முழுமையான பட்டியல்
    switch(status_code) {
        // 1xx – Informational
        case 100: status_text = "Continue"; break;
        case 101: status_text = "Switching Protocols"; break;
        
        // 2xx – Success
        case 200: status_text = "OK"; break;
        case 201: status_text = "Created"; break;
        case 202: status_text = "Accepted"; break;
        case 204: status_text = "No Content"; break;
        
        // 3xx – Redirection
        case 301: status_text = "Moved Permanently"; break;
        case 302: status_text = "Found"; break;
        case 304: status_text = "Not Modified"; break;
        case 307: status_text = "Temporary Redirect"; break;
        case 308: status_text = "Permanent Redirect"; break;
        
        // 4xx – Client Errors
        case 400: status_text = "Bad Request"; break;
        case 401: status_text = "Unauthorized"; break;
        case 403: status_text = "Forbidden"; break;
        case 404: status_text = "Not Found"; break;
        case 405: status_text = "Method Not Allowed"; break;
        case 408: status_text = "Request Timeout"; break;
        case 409: status_text = "Conflict"; break;
        case 413: status_text = "Payload Too Large"; break;
        case 415: status_text = "Unsupported Media Type"; break;
        case 429: status_text = "Too Many Requests"; break;
        
        // 5xx – Server Errors
        case 500: status_text = "Internal Server Error"; break;
        case 501: status_text = "Not Implemented"; break;
        case 502: status_text = "Bad Gateway"; break;
        case 503: status_text = "Service Unavailable"; break;
        case 504: status_text = "Gateway Timeout"; break;
        
        // Default (Fallback)
        default:  status_text = "OK"; break;
    }

    // 204 (No Content) -க்கு Body தேவையில்லை, Header மட்டும் அனுப்ப வேண்டும்
    if (status_code == 204) {
        snprintf(final_response, sizeof(final_response), 
                 "HTTP/1.1 %d %s\r\n"
                 "Connection: close\r\n\r\n", 
                 status_code, status_text);
    } else {
        // மற்ற கோடுகளுக்கு Content மற்றும் Content-Length சேர்த்து அனுப்புதல்
        snprintf(header, sizeof(header), 
                 "HTTP/1.1 %d %s\r\n"
                 "Content-Type: text/html; charset=UTF-8\r\n"
                 "Content-Length: %zu\r\n"
                 "Connection: close\r\n\r\n", 
                 status_code, status_text, strlen(content));

        snprintf(final_response, sizeof(final_response), "%s%s", header, content);
    }

    // Client-க்கு டேட்டாவை அனுப்புதல்
    send(client_socket, final_response, strlen(final_response), 0);
    
    // ரெஸ்பான்ஸ் அனுப்பிய பிறகு கனெக்‌ஷனை Close செய்தால்தான் பிரவுசரில் லோடிங் நிற்கும்
    close(client_socket); 
}

// 🌟 4. HTML ஃபைலை நேரடியாக அனுப்பும் புதிய ஃபங்ஷன்
void tamizhi_rt_send_file(double client_socket_num, double status_code_num, const char* file_path) {
    int client_socket = (int)client_socket_num;

    if (client_socket < 0) return;

    // 1. ஃபைலை ஓபன் செய்கிறோம்
    FILE *file = fopen(file_path, "rb");
    if (file == NULL) {
        // ஃபைல் இல்லை என்றால் 404 Error அனுப்புகிறோம்
        tamizhi_rt_send_response(client_socket_num, 404.0, "<h1>404 - File Not Found</h1><p>The requested HTML file is missing.</p>");
        return;
    }

    // 2. ஃபைலின் சைஸ் (Size) எவ்வளவு என்று கண்டுபிடிக்கிறோம்
    fseek(file, 0, SEEK_END);
    long fsize = ftell(file);
    fseek(file, 0, SEEK_SET);

    // 3. மெமரியை ஒதுக்கி ஃபைலை முழுமையாகப் படிக்கிறோம்
    char *file_content = malloc(fsize + 1);
    fread(file_content, 1, fsize, file);
    fclose(file);
    file_content[fsize] = '\0'; // Null-terminate செய்கிறோம்

    // 4. நாம் ஏற்கனவே எழுதிய send_response மூலமாகவே பிரவுசருக்கு அனுப்புகிறோம்
    tamizhi_rt_send_response(client_socket_num, status_code_num, file_content);

    // 5. மெமரியை க்ளீன் செய்கிறோம் (Memory Leak வராமல் தடுக்க)
    free(file_content);
}
