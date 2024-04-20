#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#define PORT 22114


int main() {
    // Se definesc structurile pt adresa serverului si a clientului, initializare descriptor socket pt "accept"
    int accept_socket;
    int true = 1;
    struct sockaddr_in server_address, their_addr;
    socklen_t addr_size = sizeof(their_addr);

    // creare socket server cu parametrii: domeniu: AF_INET(pt. IPV4), tip: SOCK_STREAM Este asociat cu TCP (Transmission Control Protocol).
    //Asigură o conexiune fiabilă și orientată pe șir de octeți.
    //Datele sunt trimise sub forma unui șir de octeți și sunt garantate că ajung la destinație în ordinea în care au fost trimise.
    int server_socket= socket(AF_INET, SOCK_STREAM, 0);

    if (server_socket < 0) {
        perror("Eroare creare socket\n");
        exit(EXIT_FAILURE);
    }
    else printf("Socket creat cu succes\n");


    memset (&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);

    // Setarea adresei IPv4 în structura sockaddr_in.
    // Funcția inet_pton convertește o adresă IPv4 în format text ("46.243.115.196") într-o reprezentare binară și o salvează în câmpul sin_addr.s_addr al structurii server_address.
    // Funcția returneaza o valoare mai mică ca 0 în caz de eroare, 0 pt ip invalid si 1 pt succes.
    if(inet_pton(AF_INET, "46.243.115.196", &server_address.sin_addr.s_addr) <= 0) {
        printf("Eroare la setarea adresei ipv4\n");
        exit(1);
    }

    //server_socket este descriptorul de fișier pentru socket returnat de socket().
    //(struct sockaddr *)&server_address este un pointer la o structură sockaddr care conține informații despre adresa.
    //sizeof(server_address) este lungimea în octeți a adresei.
    //Daca bind() returneaza -1 seteaza errno(nu folosim errno), se iese din program.
    if(bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        printf("Eroare la bind socket\n");
        exit(1);
    }
    else {
        printf("Bind OK\n");
    }

    // listen() contine descriptorul de fisier pt socket server_socket si lungimea cozii de asteptare. Returneaza -1 in caz de eroare.
    if(listen(server_socket, 5) == 1) {
        printf("Eroare asteptare client\n");
        exit(1);
    }
    printf("Asteptare client...\n");


    while(1){
        // accept() returneaza -1 si seteaza errno in caz de eroare.
        // Argumentul server_socket este un socket care a fost creat cu socket(), a fost legat la o adresă locală cu bind(), și ascultă pentru conexiuni după un listen().
        // Argumentul (struct sockaddr*)&their_addr este un pointer către o structură sockaddr.
        accept_socket = accept(server_socket, (struct sockaddr*)&their_addr, &addr_size);

        //Daca accept() returneaza -1 se iese din program
        if(accept_socket == -1) {
            printf("Conexiunea nu a fost acceptata\n");
            exit(1);
        }

        else {
            printf("Conexiune acceptata\n");
        }

        char buffer[1024];

        while(1) {
            //accept_socket descriptorul de fișier pentru socket de la care se va citi informația.
            //bufer[]: Este un pointer către bufferul în care se vor stoca datele citite.
            //sizeof(buffer) lungime buffer
            //flag-urile sunt setate la 0
            ssize_t bytes_received = recv(accept_socket, buffer, sizeof buffer, 0);

            // daca recv() returneaza -1 se iese din program, daca returneaza 0 clientul a inchis conexiunea
            if(bytes_received == -1) {
                printf("Eroare la primirea datelor\n");
            } else if(bytes_received == 0) {
                printf("Conexiunea inchisa de catre client\n");
                break;

	    //se compara primele 3 caractere ale buffer-ului pt a verifica daca comanda primita este cea corespunzatoare
            } else {
                if(buffer[0] == '0' && buffer[1] == '8' && buffer[2] == '#'){
                    printf("Comanda 08# a fost primita\n");
                    
		    // se creeaza socketul pt site-ul de pe care se vor prelua datele
                    const char* ipv6_address_str = "2001:470:0:503::2";
                    int client_socket= socket(AF_INET6, SOCK_STREAM, 0);

                    if (client_socket < 0) {
                        perror("Eroare creare socket\n");
                        exit(EXIT_FAILURE);
                    }
                    else {
                        printf("Socket client creat cu succes\n");
                    } 
                    
		    //se definesc in structura parametrii serverului ipv6 de unde se vor prelua datele
		    //se foloseste portul 80 (default pt http) 
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

                    //se realizeaza conexiunea cu serverul ipv6
                    if (connect(client_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) {
                        perror("Conectare esuata\n");
                        close(client_socket);
                        exit(EXIT_FAILURE);
                    }
                    else printf("Conexiune realizata cu succes\n");

                    //pointer către un șir de caractere constant care contine o comandă HTTP de tip GET utilizând versiunea 1.0 a protocolului HTTP
                    //În cadrul protocolului HTTP, doua caract newline consecutive ("\r\n\r\n") marchează sfârșitul antetului HTTP și începutul conținutului efectiv al cererii sau răspunsului.
                    const char *get_request = "GET / HTTP/1.0\r\n\r\n";

                    //client_socket este descriptorul de fișier pentru socket către care se vor trimite datele.
                    //get_request este un pointer catre datele pe care se trimit.
                    //strlen(get_request) este lungimea datelor din bufferul get_request în octeți.
                    //flags este setat la 0 pentru operațiuni normale
                    if(send(client_socket, get_request, strlen(get_request),0) == -1) {
                        printf("Nu s-a reusit trimiterea request-ului\n");
                        close(client_socket);
                        exit(1);
                    }

                    char buff[1024];
                    int bytes_rec;
                    FILE *fp;
                    fp = fopen("output.html", "w");

		    //bucla scrie in fisierul "output.html" datele primite de la site si in acelasi timp le trimite catre client 
                    while((bytes_rec = recv(client_socket, buff, sizeof(buff)-1,0))>0) {
                        buff[bytes_rec] = '\0';
                        //  printf("\n%s",buff);

                        fputs(buff, fp);
                        send(accept_socket,buff,bytes_rec,0);
                    }
                    fclose(fp);

                    //daca recv() returneaza -1 se iese din program
                    if(bytes_rec == -1) {
                        printf("Primire esuata\n");
                        exit(1);
                    }
		    
		    //daca recv() returneaza 0 inseamna ca site-ul a terminat de transmis datele (acesta inchide conexiunea)
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

