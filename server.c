#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 22114

int main() {
    int accept_socket;
    int true = 1;
    struct sockaddr_in server_address, their_addr;
    socklen_t addr_size = sizeof(their_addr);

    // Create a socket for the server
    int server_socket= socket(AF_INET, SOCK_STREAM, 0);

    if (server_socket < 0) {
        perror("Eroare creare socket\n");
        exit(EXIT_FAILURE);
    }
    else printf("Socket creat cu succes\n");

    memset (&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);

    // Set the IPv4 address in the server structure
    if(inet_pton(AF_INET, "46.243.115.196", &server_address.sin_addr.s_addr) <= 0) {
        printf("Eroare la setarea adresei ipv4\n");
        exit(1);
    }

    // Bind the socket to the specified port
    if(bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        printf("Eroare la bind socket\n");
        exit(1);
    }
    else {
        printf("Bind OK\n");
    }

    // Listen for incoming connections
    if(listen(server_socket, 5) == 1) {
        printf("Eroare asteptare client\n");
        exit(1);
    }
    printf("Asteptare client...\n");

    // Accept incoming connections and handle them
    while(1){
        accept_socket = accept(server_socket, (struct sockaddr*)&their_addr, &addr_size);

        if(accept_socket == -1) {
            printf("Conexiunea nu a fost acceptata\n");
            exit(1);
        }
        else {
            printf("Conexiune acceptata\n");
        }

        char buffer[1024];

        // Receive data from the client
        while(1) {
            ssize_t bytes_received = recv(accept_socket, buffer, sizeof buffer, 0);

            if(bytes_received == -1) {
                printf("Eroare la primirea datelor\n");
            } else if(bytes_received == 0) {
                printf("Conexiunea inchisa de catre client\n");
                break;
            } else {
                // Process the received data
                if(buffer[0] == '0' && buffer[1] == '8' && buffer[2] == '#'){
                    printf("Comanda 08# a fost primita\n");

                    // Create a socket for the client
                    const char* ipv6_address_str = "2001:470:0:503::2";
                    int client_socket= socket(AF_INET6, SOCK_STREAM, 0);

                    if (client_socket < 0) {
                        perror("Eroare creare socket\n");
                        exit(EXIT_FAILURE);
                    }
                    else {
                        printf("Socket client creat cu succes\n");
                    } 

                    // Set the IPv6 address in the server structure
                    struct sockaddr_in6 server_address;
                    memset(&server_address, 0, sizeof(server_address));
                    server_address.sin6_family = AF_INET6;
                    server_address.sin6_port = htons(80);
                    int val_inet =  inet_pton(AF_INET6, ipv6_address_str , &server_address.sin6_addr);

                    if(val_inet==0){
                        printf("Sursa nu e un string valid\n");
                        exit(1);
                    }
                    if(val_inet==-1){
                        printf("Argument AF necunoscut\n");
                        exit(1);
                    }

                    // Connect to the IPv6 server
                    if (connect(client_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) {
                        perror("Conectare esuata\n");
                        close(client_socket);
                        exit(EXIT_FAILURE);
                    }
                    else printf("Conexiune realizata cu succes\n");

                    // Send a GET request to the server
                    const char *get_request = "GET / HTTP/1.0\r\n\r\n";
                    if(send(client_socket, get_request, strlen(get_request),0) == -1) {
                        printf("Nu s-a reusit trimiterea request-ului\n");
                        close(client_socket);
                        exit(1);
                    }

                    // Receive data from the server and send it to the client
                    char buff[1024];
                    int bytes_rec;
                    FILE *fp;
                    fp = fopen("output.html", "w");

                    while((bytes_rec = recv(client_socket, buff, sizeof(buff)-1,0))>0) {
                        buff[bytes_rec] = '\0';

                        // Write received data to the output file
                        fputs(buff, fp);

                        // Send received data back to the client
                        send(accept_socket, buff, bytes_rec, 0);
                    }
                    fclose(fp);

                    // Check for any errors during data reception
                    if(bytes_rec == -1) {
                        printf("Primire esuata\n");
                        exit(1);
                    }
		    
		             // Check if the reception was successful
                    if(bytes_rec == 0) {
                        printf("Primirea realizata cu succes\n");
                    }
                    close(client_socket);

                    printf("Date primite de la site\n");
		            printf("Date scrise in fisier\n");
		            printf("Date transmise catre client cu succes\n");

                }
                else {
                    printf("Comanda neimplementata\n");
                }
            }
        }

    close(accept_socket);
    printf("Conexiune inchisa\n");
    }

    close(server_socket);
    printf("Server socket inchis\n");
}


