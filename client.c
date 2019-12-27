#include <netinet/in.h>
#include <sys/socket.h>
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <sys/time.h>


char buf[80];
char IP[15] = "127.0.0.1";
int port = 5000;
int etatConnexion;
int passage = 0;

struct timeval t1, t2;
struct timeval result;

struct timeval diff_timeval(struct timeval t2, struct timeval t1, int socket_client)
{
	struct timeval resultTime;

	resultTime.tv_sec = t2.tv_sec - t1.tv_sec;	// Soustraction des secondes
	resultTime.tv_usec = t2.tv_usec - t1.tv_usec; // Soustraction des micro secondes
	// la soustraction des microsecondes peut etre négative
	// Dans ce cas on soustrait 1sec et on rajoute 1000000 microsecondes
	while (resultTime.tv_usec < 0)
	{
		resultTime.tv_usec += 1000000;
		resultTime.tv_sec--;
	}
	if (resultTime.tv_sec > 30)
	{
		printf("Le serveur a prit trop de temps à répondre, tentative de reconnexion. \n");
			close(socket_client);
			main();
	}
	return resultTime;
}

void PersonaliserParamConnection()
{
    printf("Voulez vous modifier l'IP (127.0.0.1 par défaut) ? y/n \n");
    scanf("%s", &buf);
    if (strcmp(buf, "y") == 0)
    {
        printf("Entrez l'IP du serveur :\n");
        scanf("%s", &IP);
    }
    else
    {
        printf("IP = 127.0.0.1\n");
    }
    printf("Voulez vous modifier le port (5000 par défaut) ? y/n \n");
    scanf("%s", &buf);
    if (strcmp(buf, "y") == 0)
    {
        printf("Entrez le port :\n");
        scanf("%d", &port);
    }
    else
    {
        printf("Port = 5000.\n");
    }
}

void verificationEtatConnexion(int etatConnexion, int socket_client)
{
    int i;
    if (etatConnexion == 0)
    {
        printf("Déconnexion du Serveur. \n");
        close(socket_client);

        printf("Tentative de reconnexion dans : \n");
        for (i = 5; i >= 1; i--)
        {
            printf("%d \n", i);
            sleep(1);
        }

        main();
    }
}

void envoieCode(int socket_client)
{
    int code;

    etatConnexion = recv(socket_client, &buf, sizeof(buf), 0);
    verificationEtatConnexion(etatConnexion, socket_client);
    printf("Tour de contrôle : %s\n", buf);

    scanf("%s", &buf);
    send(socket_client, buf, sizeof(buf), 0);
    printf("J'ai envoye [%s] au serveur\n", buf);
    gettimeofday(&t1, NULL);

    code = atoi(buf);

    etatConnexion = recv(socket_client, &buf, sizeof(buf), 0);
    verificationEtatConnexion(etatConnexion, socket_client);
    printf("Tour de contrôle :  %s\n", buf);
    gettimeofday(&t2, NULL);

    result = diff_timeval(t2, t1, socket_client);

    if (code == 1)
    {
        scanf("%s", &buf);
        send(socket_client, buf, sizeof(buf), 0);
        printf("J'ai envoye [%s] au serveur\n", buf);
        gettimeofday(&t1, NULL);


        etatConnexion = recv(socket_client, &buf, sizeof(buf), 0);
        verificationEtatConnexion(etatConnexion, socket_client);
        printf("Tour de contrôle :  %s\n", buf);
        gettimeofday(&t2, NULL);

        result = diff_timeval(t2, t1, socket_client);

    }
    if (code == 3)
    {
        close(socket_client);
        exit(1);
    }
    else if (!(code != 1 || code != 2))
    {
        envoieCode(socket_client);
    }
}

int main()
{

    int socket_client;            //On definit des entier qui prendront les sockets
    struct sockaddr_in serv_addr; //On definit une adresse pour le serveur

    /* PROPRIETE DU SERVEUR */
    if (passage == 0)
    {
        PersonaliserParamConnection();
        passage = passage + 1;
    }
    //On initialise l'adresse du serveur
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(IP);
    serv_addr.sin_port = htons(port);
    memset(&serv_addr.sin_zero, 0, sizeof(serv_addr.sin_zero));

    socket_client = socket(PF_INET, SOCK_STREAM, 0);

    /* LE SOCKETS */

    //On connecte le client au serveur
    //connect(socket_client, (struct sockaddr *)&serv_addr, sizeof serv_addr);

    if ((connect(socket_client, (struct sockaddr *)&serv_addr, sizeof serv_addr)) < 0)
    {
        printf("Serveur non atteignable.\n");
        exit(1);
    }

    /* LA CONNEXION EST ETABLIE */
    printf("Je me suis connecté au serveur %s sur son port %d\n",
           inet_ntoa(serv_addr.sin_addr), ntohs(serv_addr.sin_port));

    /* ENVOIE DU CODE A LA TOUR DE CONTROLE */
    etatConnexion = recv(socket_client, &buf, sizeof(buf), 0);
    verificationEtatConnexion(etatConnexion, socket_client);
    printf("Tour de contrôle : %s\n", buf);

    do
    {
        scanf("%s", &buf);
        send(socket_client, buf, sizeof(buf), 0);
        printf("J'ai envoye [%s] au serveur\n", buf);
        gettimeofday(&t1, NULL);

        etatConnexion = recv(socket_client, &buf, sizeof(buf), 0);
        verificationEtatConnexion(etatConnexion, socket_client);
        printf("Tour de contrôle : %s\n", buf);
        gettimeofday(&t2, NULL);

        result = diff_timeval(t2, t1, socket_client);


    } while ((strcmp(buf, "Votre id à été recconu.")) != 0);
    //(strcmp(buf,"Votre id à été recconu."))==0

    while (1)
    {
        envoieCode(socket_client);
    }

    close(socket_client);
}
