/* Assane Thiao 14/03/2025 */

#include "network.h"  /* Inclure le fichier d'en-tête personnalisé */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 1024

int main(int argc, char* argv[]) {
    SOCKET sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    int port = 8085;

    if (argc == 2) {
        port = atoi(argv[1]);
    }

    initNetwork();

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        printf("Erreur lors de la creation du socket\n");
        cleanNetwork();
        return EXIT_FAILURE;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");  /* Adresse localhost */

    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        printf("Erreur lors de la connexion au serveur\n");
        closesocket(sock);
        cleanNetwork();
        return EXIT_FAILURE;
    }

    printf("Connecte au serveur sur le port %d\n", port);

    while (1) {
        printf("Entrez un message a envoyer au serveur : ");
        fgets(buffer, sizeof(buffer), stdin);
        buffer[strcspn(buffer, "\n")] = 0;  /* Enlever le '\n' à la fin du message */

        if (send(sock, buffer, strlen(buffer), 0) == SOCKET_ERROR) {
            printf("Erreur lors de l'envoi du message\n");
            closesocket(sock);
            cleanNetwork();
            return EXIT_FAILURE;
        }

        printf("Message envoye au serveur : %s\n", buffer);

        if (strcmp(buffer, "CLIENT:QUIT") == 0) {
            break;
        }

        int recv_len = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (recv_len > 0) {
            buffer[recv_len] = '\0';  /* Ajouter le caractère de fin de chaîne */
            printf("Reponse du serveur : %s\n", buffer);
        }
        else {
            printf("Erreur ou fermeture de la connexion du serveur\n");
            break;
        }
    }

    closesocket(sock);
    cleanNetwork();
    return 0;
}
