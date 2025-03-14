/* Assane Thiao 14/03/2025 */

#include "network.h"  /* Inclure le fichier d'en-tête personnalisé */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PORT 8085 
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 2

int main() {
    SOCKET sock, client1 = -1, client2 = -1;
    struct sockaddr_in server_addr, client_addr;
    fd_set readfds;
    int addr_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];

    /* Initialisation du réseau */
    initNetwork();

    /* Création du socket */
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        printf("Erreur lors de la creation du socket\n");
        cleanNetwork();
        return EXIT_FAILURE;
    }

    /* Configuration de l'adresse du serveur */
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    /* Liaison du socket */
    if (bind(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        printf("Erreur lors de la liaison du socket\n");
        closesocket(sock);
        cleanNetwork();
        return EXIT_FAILURE;
    }

    /* Mise en écoute du socket */
    if (listen(sock, MAX_CLIENTS) == SOCKET_ERROR) {
        printf("Erreur lors de la mise en ecoute\n");
        closesocket(sock);
        cleanNetwork();
        return EXIT_FAILURE;
    }
    printf("\n");
    printf(" ********************************************************************************\n");
    printf("|                                                                                |\n");
    printf("|  Port par defaut c'etait 8080, mais ce port est utilise par httpd.exe,         |\n");
    printf("|  et sur la machine de l'ecole on a pas les droits pour stopper ce processus.   |\n");
    printf("|                                                                                |\n");
    printf(" ********************************************************************************\n");
    printf("\n");
    printf("Serveur en ecoute sur le port %d...\n", PORT);

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(sock, &readfds);
        if (client1 != -1) FD_SET(client1, &readfds);
        if (client2 != -1) FD_SET(client2, &readfds);

        int max_sock = sock;
        if (client1 > max_sock) max_sock = client1;
        if (client2 > max_sock) max_sock = client2;

        /* Utilisation de select pour gérer plusieurs clients simultanément */
        if (select(max_sock + 1, &readfds, NULL, NULL, NULL) < 0) {
            printf("Erreur lors de l'utilisation de select\n");
            break;
        }

        /* Gestion des connexions entrantes */
        if (FD_ISSET(sock, &readfds)) {
            SOCKET new_client = accept(sock, (struct sockaddr*)&client_addr, &addr_len);
            if (new_client == INVALID_SOCKET) {
                printf("Erreur lors de l'acceptation du client\n");
                continue;
            }

            if (client1 == -1) {
                client1 = new_client;
                printf("Client 1 connecte\n");
            }
            else if (client2 == -1) {
                client2 = new_client;
                printf("Client 2 connecte\n");
            }
            else {
                printf("Nombre maximal de clients atteint\n");
                closesocket(new_client);
            }
        }

        /* Gestion des données reçues des clients */
        SOCKET clients[] = { client1, client2 };
        int i;
        for (i = 0; i < MAX_CLIENTS; i++) {
            SOCKET client = clients[i];
            if (client != -1 && FD_ISSET(client, &readfds)) {
                int recv_len = recv(client, buffer, BUFFER_SIZE - 1, 0);
                if (recv_len <= 0) {
                    printf("Client deconnecte\n");
                    closesocket(client);
                    if (client == client1) client1 = -1;
                    if (client == client2) client2 = -1;
                    continue;
                }

                buffer[recv_len] = '\0';
                printf("Message recu : %s\n", buffer);

                /* Si le message reçu est "SERVER:QUIT", on ferme tous les sockets et on arrête le serveur */
                if (strcmp(buffer, "SERVER:QUIT") == 0) {
                    printf("Fermeture du serveur\n");
                    closesocket(client1);
                    closesocket(client2);
                    closesocket(sock);
                    cleanNetwork();
                    return 0;
                }
                /* Si le message reçu est "CLIENT:QUIT", on ferme la connexion du client correspondant */
                else if (strcmp(buffer, "CLIENT:QUIT") == 0) {
                    printf("Fermeture de la connexion client\n");
                    closesocket(client);
                    if (client == client1) client1 = -1;
                    if (client == client2) client2 = -1;
                }
                else {
                    /* Renvoi du même message reçu en réponse */
                    send(client, buffer, strlen(buffer), 0);
                }
            }
        }
    }

    closesocket(sock);
    cleanNetwork();
    return 0;
}
