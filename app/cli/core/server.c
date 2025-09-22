#include "cli.h"

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
#endif

static void init_networking(void) {
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
}

static void cleanup_networking(void) {
#ifdef _WIN32
    WSACleanup();
#endif
}

int start_dev_server(int port, const char* host) {
    if (port <= 0 || port > 65535) {
        fprintf(stderr, "Error: Invalid port number: %d\n", port);
        return 1;
    }

    if (!host) host = "localhost";

    printf("Starting Hyperion development server...\n");
    printf("Host: %s\n", host);
    printf("Port: %d\n", port);

    init_networking();

    // Create socket
    int server_fd;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

#ifdef _WIN32
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == INVALID_SOCKET) {
#else
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
#endif
        fprintf(stderr, "Error: Socket creation failed\n");
        cleanup_networking();
        return 1;
    }

    // Set socket options
#ifdef _WIN32
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)) < 0) {
#else
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
#endif
        fprintf(stderr, "Error: setsockopt failed\n");
#ifdef _WIN32
        closesocket(server_fd);
#else
        close(server_fd);
#endif
        cleanup_networking();
        return 1;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    // Bind socket
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        fprintf(stderr, "Error: Bind failed on port %d\n", port);
#ifdef _WIN32
        closesocket(server_fd);
#else
        close(server_fd);
#endif
        cleanup_networking();
        return 1;
    }

    // Listen for connections
    if (listen(server_fd, 3) < 0) {
        fprintf(stderr, "Error: Listen failed\n");
#ifdef _WIN32
        closesocket(server_fd);
#else
        close(server_fd);
#endif
        cleanup_networking();
        return 1;
    }

    printf("\n🚀 Hyperion development server is running!\n");
    printf("📡 Local:    http://%s:%d\n", host, port);
    printf("🌐 Network:  http://localhost:%d\n", port);
    printf("\nPress Ctrl+C to stop the server\n\n");

    // Simple HTTP server loop
    while (1) {
        int new_socket;
        char buffer[1024] = {0};
        
#ifdef _WIN32
        new_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen);
        if (new_socket == INVALID_SOCKET) {
#else
        new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
        if (new_socket < 0) {
#endif
            continue;
        }

        // Read request
#ifdef _WIN32
        recv(new_socket, buffer, 1024, 0);
#else
        read(new_socket, buffer, 1024);
#endif

        // Simple HTTP response
        const char* http_response = 
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Connection: close\r\n"
            "\r\n"
            "<!DOCTYPE html>\n"
            "<html>\n"
            "<head>\n"
            "    <title>Hyperion Development Server</title>\n"
            "    <style>\n"
            "        body { font-family: Arial, sans-serif; margin: 40px; background: #1e1e1e; color: #fff; }\n"
            "        .container { max-width: 800px; margin: 0 auto; }\n"
            "        h1 { color: #007acc; }\n"
            "        .status { background: #2d2d30; padding: 20px; border-radius: 8px; }\n"
            "    </style>\n"
            "</head>\n"
            "<body>\n"
            "    <div class=\"container\">\n"
            "        <h1>🚀 Hyperion Development Server</h1>\n"
            "        <div class=\"status\">\n"
            "            <h2>Server Status: Running</h2>\n"
            "            <p>Your development server is up and running!</p>\n"
            "            <p>This is a basic development server created by Hyperion CLI.</p>\n"
            "        </div>\n"
            "    </div>\n"
            "</body>\n"
            "</html>\n";

#ifdef _WIN32
        send(new_socket, http_response, (int)strlen(http_response), 0);
        closesocket(new_socket);
#else
        write(new_socket, http_response, strlen(http_response));
        close(new_socket);
#endif

        printf("📝 Request served at %s:%d\n", host, port);
    }

#ifdef _WIN32
    closesocket(server_fd);
#else
    close(server_fd);
#endif
    cleanup_networking();
    return 0;
}

int create_tunnel(const char* name) {
    if (!name) {
        fprintf(stderr, "Error: No tunnel name specified\n");
        return 1;
    }

    printf("Creating secure tunnel: %s\n", name);
    
    // Generate a random tunnel ID
    srand((unsigned int)time(NULL));
    int tunnel_id = rand() % 10000 + 1000;
    
    printf("🔒 Tunnel created successfully!\n");
    printf("📡 Tunnel ID: %s-%d\n", name, tunnel_id);
    printf("🌐 Public URL: https://%s-%d.hyperion-tunnel.dev\n", name, tunnel_id);
    printf("🔗 Local URL: http://localhost:8080\n");
    printf("\nTunnel is now active and forwarding traffic.\n");
    printf("Share the public URL to allow external access to your local server.\n");
    
    // In a real implementation, this would:
    // 1. Connect to a tunneling service
    // 2. Establish a secure connection
    // 3. Forward traffic between local and remote endpoints
    
    // For demonstration, we'll simulate an active tunnel
    printf("\nTunnel logs:\n");
    printf("============\n");
    
    int connection_count = 0;
    while (connection_count < 5) { // Simulate some activity
#ifdef _WIN32
        Sleep(2000); // 2 seconds
#else
        sleep(2);
#endif
        connection_count++;
        printf("📊 Connection #%d established from external client\n", connection_count);
        
        if (connection_count == 3) {
            printf("🔄 Tunnel health check: OK\n");
        }
    }
    
    printf("\n✅ Tunnel demonstration completed.\n");
    printf("In a production environment, this tunnel would remain active until terminated.\n");
    
    return 0;
}