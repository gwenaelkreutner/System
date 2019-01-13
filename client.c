#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>

#define PORT 4000
#define MAX_SIZE 1000

void removeN(char *msg);
void continuer(void);
int reservation(char alpha[]);
int compteurPlaceLibre (char recvBuff[]);
void affichage(char recvBuff[], char alpha[]);
void red ();
void green ();
void reset ();
void magenta ();




int main()
{
    int fdSocket, nPlace;
    struct sockaddr_in serv_addr;
    char sendBuff[MAX_SIZE], recvBuff[MAX_SIZE];
    char alpha[10] = {'A','B','C','D','E','F','G','H','I','J'};
    char argument;

    if((fdSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        printf("erreur socket\n");

    bzero((char *) &serv_addr, sizeof (serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr.sin_port = htons(PORT);

    if (connect(fdSocket, (struct sockaddr *) &serv_addr, sizeof (serv_addr)) < 0)
    {
        printf("connection echoué\n");
        return -1;
    }

    printf("Connection établi\n");

    /*
    //Boucle tchat simple
    while(fgets(sendBuff, MAX_SIZE , stdin)!=NULL)
                {
                  removeN(sendBuff);
                  send(fdSocket, sendBuff, strlen(sendBuff) + 1,0);

                      if((nbCarartere = recv(fdSocket,recvBuff,MAX_SIZE,0)) == 0)
                       printf("Error");
                      else if (nbCarartere > 1) // Il y a des caracteres, la chaine n'est pas vide
                        printf("%s\n", recvBuff);

                   bzero(recvBuff,MAX_SIZE);
                }

                close(fdSocket);
                return 0;
    */

    int choix;

    do
    {
        printf("Menu\n\n");
        printf("1. Reserver une place\n");
        printf("2. Afficher les places disponibles\n");
        printf("3. Annuler votre réservation\n");
        printf("4. Fermer le programme\n");
        scanf("%d",&choix);
        getchar();

        switch(choix)
        {
            case 1:

                printf("-- Reservation --\n\n");
                strcpy(sendBuff, "reservation");
                removeN(sendBuff);
                send(fdSocket, sendBuff, strlen(sendBuff) + 1,0);

                nPlace = reservation(alpha);
                sprintf(sendBuff, "%d", nPlace);
                removeN(sendBuff);
                send(fdSocket, sendBuff, strlen(sendBuff) + 1,0);

                printf("Veuillez saisir votre nom : ");
                fgets(sendBuff, MAX_SIZE , stdin);
                removeN(sendBuff);
                send(fdSocket, sendBuff, strlen(sendBuff) + 1,0);

                printf("Veuillez saisir prenom : ");
                fgets(sendBuff, MAX_SIZE , stdin);
                removeN(sendBuff);
                send(fdSocket, sendBuff, strlen(sendBuff) + 1,0);

                recv(fdSocket,recvBuff,MAX_SIZE,0);
                magenta();
                printf("%s\n", recvBuff);
                reset();
                bzero(recvBuff,MAX_SIZE);

                continuer();

                break;
            case 2:
                printf("-- Affichage --\n\n");

                int nbPlaceLibre, nPlace;

                strcpy(sendBuff, "place");
                send(fdSocket, sendBuff, strlen(sendBuff) + 1,0);

                recv(fdSocket,recvBuff,MAX_SIZE,0);
                affichage(recvBuff, alpha);
                nbPlaceLibre = compteurPlaceLibre(recvBuff);
                printf("\n\nIl reste %d places libre dans la salle\n", nbPlaceLibre);

                continuer();
                break;
            case 3:
                printf("-- Annulation --\n\n");

                strcpy(sendBuff, "annulation");
                removeN(sendBuff);
                send(fdSocket, sendBuff, strlen(sendBuff) + 1,0);

                printf("Veuillez saisir le numero de ticket : ");
                fgets(sendBuff, MAX_SIZE , stdin);
                removeN(sendBuff);
                send(fdSocket, sendBuff, strlen(sendBuff) + 1,0);

                printf("Veuillez saisir votre nom : ");
                fgets(sendBuff, MAX_SIZE , stdin);
                removeN(sendBuff);
                send(fdSocket, sendBuff, strlen(sendBuff) + 1,0);

                recv(fdSocket,recvBuff,MAX_SIZE,0);
                magenta();
                printf("%s\n", recvBuff);
                reset();
                bzero(recvBuff,MAX_SIZE);

                continuer();
                break;
            case 4:
                printf("Fermer le programme\n");
                exit(0);
                break;

            default:
                printf("Mauvais choix!\n");
                break;
        }

    } while (choix != 4);
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

void affichage(char recvBuff[], char alpha[])
{
    int nbPlaceTotal, colonne, compteur = 0;
    nbPlaceTotal=strlen(recvBuff);
    printf("nombre de place au total : %d\n\n", nbPlaceTotal);

    for (colonne = 0; colonne < 10; colonne++) {
        if (colonne==0) {
          printf("   |");
        }
        printf(" %c |", alpha[colonne]);
    }
    for (int i = 0; i < nbPlaceTotal; ++i)
    {
        if (recvBuff[i]=='0'){
            recvBuff[i]='O';
        }
        else{
            recvBuff[i]='X';
        }
        //retour a la ligne toutes les 10 valeurs ajoutées
        if (i % 10 == 0) {
            compteur++;
            printf("\n--------------------------------------------\n%-3d|",compteur);
        }
        if (recvBuff[i]=='O'){
            green();
            printf(" %c", recvBuff[i]);
            reset();
            printf(" |");
        }
        else {
            red();
            printf(" %c", recvBuff[i]);
            reset();
            printf(" |");
        }
    }
}

// Enleve le dernier char de la chaine ( \n ou \0 )
void removeN(char *msg)
{
    int l = strlen(msg) - 1;
    if (msg[l] == '\n')
        msg[l] = '\0';
}

int compteurPlaceLibre (char recvBuff[])
{
  int compteur = 0;

  for (int i=0; i<100; ++i){
    if (recvBuff[i]=='0' || recvBuff[i]=='O') {
        compteur++;
    }
  }
  return(compteur);
}

int reservation(char alpha[])
{
    // retourne un entier entre 1 et 100 en saisisant la colonne et la rangée
    int rang = 0, nPlace = 0, nColonne;
    char colonne;
    printf("Veuillez saisir le placement de votre siege ( rang -> colonne )\n");
    do {
       printf("Veuillez saisir le rang ( 1 - 10 )\n");
       scanf("%d", &rang);
       getchar();
    } while (rang<1 || rang>10);

    do {
    printf("Veuillez saisir la colonne en majuscule ( A - J )\n");
    scanf("%c", &colonne);
        for (int i = 0; i < 10; i++)
        {
            if (alpha[i]==colonne)
            {
                nColonne = i+1;
            }
        }
    } while (nColonne<1 || nColonne>10);

    getchar();

    printf("Votre place -> rang:  %d  colonne : %c\n", rang, colonne);

    nPlace = ((rang-1)*10)+nColonne;
    return(nPlace);
}

void continuer(void)
{
    int ch;
    printf("\n\nAppuyer sur ENTREE pour continuer.");
    while ((ch = getchar()) != '\n' && ch != EOF);

    return;
}
