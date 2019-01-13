#include <stdio.h>
#include <string.h>    //strlen
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h> //inet_addr
#include <unistd.h>    //write
#include <pthread.h> //for threading
#include <errno.h>

#define PORT 4000
#define NBPLACE 100
#define TAILLEMAX 50

typedef struct ticket
{
	char nom[TAILLEMAX];
	char prenom[TAILLEMAX];
	long nTicket;
} Ticket;

typedef struct salleConcert
{
	Ticket salleConcert[NBPLACE];
	pthread_mutex_t mutexLock;
} SalleConcert;

//Compteur client connecté
typedef struct nbClient
{
	int client;
	pthread_mutex_t mutexLock;
} NbClient;

typedef struct socketClient
{
	int socket;
	struct sockaddr_in address;
} SocketClient;

void *threadClient(void *);
void envoiSalle(char msg[]);
int obtenirIndexParNTicket(long nTicket);
long reserveTicket(char *nom, char *prenom, int nPlace);
int annulationTicket(long nbDossier, char *nom);
void red ();
void green ();
void magenta ();
void reset ();

SalleConcert salleConcert;
NbClient nbClient;
SocketClient *socketClient;

int main(int argc , char *argv[])
{
	int sock_d_server, sock_d_client, client_new_sock;
	socklen_t len;
	struct sockaddr_in server, client;
	char msg[150];

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	server.sin_port = htons(PORT);

	// Creation d'un serveur socket
	if ((sock_d_server = socket(AF_INET , SOCK_STREAM , 0)) == -1)
	{
		printf("Socket incorrect");
		return 1;
	}
	printf("Socket crée\n");

	// Binding a socket server
	if (bind(sock_d_server,(struct sockaddr *)&server ,sizeof(server)) == -1)
	{
		perror("erreur de bind");
		return 1;
	}

	// Listen
	if (listen(sock_d_server , 3) != 0)
	{
		perror("erreur de listen");
		return 1;
	}
	printf("Port connecté\n");

	// Accept and incoming connection
	puts("En attente d'une connection...\n");
	// Pour pouvoir recevoir plusieur client
	while ( client_new_sock = accept(sock_d_server,(struct sockaddr*) &client, &len))
	{
		pthread_t thread_1;

		if (client_new_sock < 0)
		{
			perror("Erreur de accept");
			return 1;
		}

		green();
		printf("Client connecté - %s:%d\n",
        inet_ntoa(client.sin_addr),
        ntohs(client.sin_port));
        reset();

		socketClient = malloc(sizeof(SocketClient));
		socketClient->socket = client_new_sock;
		socketClient->address = client;

		pthread_mutex_init(&nbClient.mutexLock, NULL);

		// Compteur client
		pthread_mutex_lock(&nbClient.mutexLock);
			nbClient.client++;
			magenta();
			printf("Nombre de client connecté au serveur : %d\n", nbClient.client);
			reset();
		pthread_mutex_unlock(&nbClient.mutexLock);

    	pthread_mutex_init(&salleConcert.mutexLock, NULL);

		if(pthread_create(&thread_1, NULL , threadClient, (void*) socketClient) < 0)
		{
			perror("Erreur du thread");
			return 1;
		}
	}

	return 0;
}

void *threadClient(void *sock_d_server)
{
	// Get the socket descriptor
	SocketClient *sock = (SocketClient *)sock_d_server;
	int catch;

	int choix = 0;
	int stepChoix = 0;

	long nTicket;
	int nbPlace = 0;
	char nom[200];
	char prenom[200];

	char sendBuff[1000], clientMessage[2000];

	// Boucle sur le recv et send
	while((catch=recv(sock->socket, clientMessage, 2000, 0)) > 0)
	{
		strcpy(sendBuff, ""); // Remise à 0 du message a envoyer
		// Affichage du message du client avec le nombre de case occupé dans la char
		printf("%d | Message recu :%s \n",catch, clientMessage);

		switch (choix)
		{
			case 0:
			{
				if (strcmp(clientMessage, "place") == 0)
					envoiSalle(sendBuff);
				else if (strcmp(clientMessage, "reservation") == 0)
					choix = 1;
				else if (strcmp(clientMessage, "annulation") == 0)
					choix = 2;
				break;
			}
			case 1:
			{
				switch (stepChoix++)
				{
					// Get les messages recus
					case 0: nbPlace = strtol(clientMessage, NULL, 10); break;
					case 1: strcpy(nom, clientMessage); break;
					case 2:
					{
						strcpy(prenom, clientMessage);
						// Fin des infos recu donc traitement
						nTicket = reserveTicket(nom, prenom, nbPlace);
						if (nTicket == -1)
							sprintf(sendBuff, "La place pour %s %s est déjà pris", nom, prenom);
						else sprintf(sendBuff, "Le numéro est %ld pour %s %s", nTicket, nom, prenom);
								// Affichage de la reservation du client avec son adresse IP
								green();
								printf("Client - %s:%d a réservé pour %s %s n°Ticket : %ld\n",
						        inet_ntoa(sock->address.sin_addr),
						        ntohs(sock->address.sin_port), nom, prenom, nTicket);
						        reset();
						choix = 0;
						stepChoix = 0;
						break;
					}
				}
				break;
			}
			case 2:
			{
				switch (stepChoix++)
				{
					// Get les messages recus
					case 0: nTicket = strtol(clientMessage, NULL, 10); break;
					case 1:
					{
						int res = 0;
						strcpy(nom, clientMessage);
						// Fin des infos recu donc traitement
						res = annulationTicket(nTicket, nom);
						if (res == 0){
							sprintf(sendBuff, "Votre ticket n°%ld a bien été annulé M.%s", nTicket, nom);
							red();
							printf("Client - %s:%d a annulé une place pour %s\n",
						    inet_ntoa(sock->address.sin_addr),
						    ntohs(sock->address.sin_port), nom);
						    reset();
						}
						else strcpy(sendBuff, "Le numéro de ticket ou le nom est invalide");

						choix = 0;
						stepChoix = 0;
						break;
					}
				}
				break;
			}
		}

		if (strlen(sendBuff)>0)
			send(sock->socket, sendBuff, strlen(sendBuff) + 1,0);
	}

	red();
	printf("Client déconnecté - %s:%d\n",
        inet_ntoa(sock->address.sin_addr),
		ntohs(sock->address.sin_port));
	close(sock->socket);
	reset();
	// Compteur décrémenté lors de la deconnection d'un client
	pthread_mutex_lock(&nbClient.mutexLock);
		nbClient.client--;
		magenta();
		printf("Nombre de client connecté au serveur : %d\n", nbClient.client);
		reset();
	pthread_mutex_unlock(&nbClient.mutexLock);

	pthread_exit(0);
}

void red (){
    printf("\033[1;31m");
}

void green (){
    printf("\033[1;32m");
}

void reset (){
    printf("\033[0m");
}

void magenta (){
    printf("\033[1;35m");
}

// Envoie d'une chaine de 100 caractères de 0 et de 1 ( 0 : place libre )
void envoiSalle(char msg[])
{
	int i = 0;
	strcpy(msg, "");
	for (i = 0; i < NBPLACE; i++)
		strcat(msg, salleConcert.salleConcert[i].nTicket == 0 ? "0":"1");
}

int obtenirIndexParNTicket(long nTicket)
{
	int i = 0;
	for (i = 0; i < NBPLACE; ++i)
		if (salleConcert.salleConcert[i].nTicket == nTicket)
			return i;

	return -1;
}

long reserveTicket(char *nom, char *prenom, int nPlace)
{
	int nTicket = -1;
	pthread_mutex_lock(&salleConcert.mutexLock);
	if (salleConcert.salleConcert[nPlace - 1].nTicket == 0)
	{
		strcpy(salleConcert.salleConcert[nPlace - 1].nom, nom);
		strcpy(salleConcert.salleConcert[nPlace - 1].prenom, prenom);
		nTicket = 1000000000 + rand() % 1000000000;
		salleConcert.salleConcert[nPlace - 1].nTicket = nTicket;
	}
	pthread_mutex_unlock(&salleConcert.mutexLock);
	// Retourne -1 si la place est déjà prise sinon retourne un nombre aleatoire de 10 chiffres
	return nTicket;
}

int annulationTicket(long nTicket, char *nom)
{
	int index = obtenirIndexParNTicket(nTicket);
	pthread_mutex_lock(&salleConcert.mutexLock);
	if (index >= 0)
	{
		if (strcmp(salleConcert.salleConcert[index].nom, nom) == 0)
		{
			strcpy(salleConcert.salleConcert[index].nom, "");
			strcpy(salleConcert.salleConcert[index].prenom, "");
			salleConcert.salleConcert[index].nTicket = 0;
			return 0; // C'est bon
		}
		return -1; // Ticket existe mais le nom ne correspond pas au nom inscrit sur le ticket
	}
	pthread_mutex_unlock(&salleConcert.mutexLock);
	return -2; // Le ticket n'existe pas
}
