#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/time.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <postgresql/libpq-fe.h>
//compilation avec -lpq
char connexion[100] = "host=localhost port=5432 dbname=aerow user=aerowAdmin password=admin";

char IP[15] = "127.0.0.1";
int port = 5000;
char buf[80];

int etatConnexion;
int passage = 0;

struct timeval t1, t2;
struct timeval result;

struct timeval diff_timeval(struct timeval t2, struct timeval t1, int socket_dial, int socket_ecoute )
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
		printf("Le client a prit trop de temps à répondre.\n");
			close(socket_dial);
			close(socket_ecoute);
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

void verificationEtatConnexion(int etatConnexion, int socket_dial, int socket_ecoute)
{
	if (etatConnexion == 0)
	{
		printf("Déconnexion du client. \n");
		close(socket_dial);
		close(socket_ecoute);
		main();
	}
}

int verificationID(int id)
{
	printf("\n");
	PGconn *conn = PQconnectdb(connexion);
	char requete[100];

	if (PQstatus(conn) == CONNECTION_BAD)
	{
		fprintf(stderr, "Echec de la connexion: %s\n",
				PQerrorMessage(conn));
		PQfinish(conn);
		printf("\n");
	}
	snprintf(requete, 100, "SELECT id_plane FROM plane WHERE id_plane='%d'\n", id);
	PGresult *res = PQexec(conn, requete);

	int rows = PQntuples(res);
	int columns = PQnfields(res);

	if (PQresultStatus(res) == PGRES_TUPLES_OK && rows != NULL && columns != NULL)
	{
		//printf("%s \n", PQgetvalue(res, 0, 0));
		PQclear(res);
		PQfinish(conn);
		return id;
	}
	return 0;
	PQclear(res);
	PQfinish(conn);
	printf("\n");
}

void trainPlane(int id_plane, int train)
{
	printf("\n");
	char requete[100];
	PGconn *conn = PQconnectdb(connexion);

	if (PQstatus(conn) == CONNECTION_BAD)
	{

		fprintf(stderr, "Echec de la connexion: %s\n",
				PQerrorMessage(conn));
		PQfinish(conn);
		exit(1);
	}

	if (train == 1)
	{
		snprintf(requete, 100, "UPDATE plane SET train='true' WHERE id_plane='%d'\n", id_plane);
	}
	else
	{
		snprintf(requete, 100, "UPDATE plane SET train='false' WHERE id_plane='%d'\n", id_plane);
	}

	PQexec(conn, requete);

	printf("Modification éffectuées\n");

	PQfinish(conn);
	printf("\n");
}

void urgence(int id_plane)
{
	printf("\n");
	char requete[100];
	PGconn *conn = PQconnectdb(connexion);

	if (PQstatus(conn) == CONNECTION_BAD)
	{

		fprintf(stderr, "Echec de la connexion: %s\n",
				PQerrorMessage(conn));
		PQfinish(conn);
		exit(1);
	}

	snprintf(requete, 100, "UPDATE plane SET etaturgence='true' WHERE id_plane='%d'\n", id_plane);

	PQexec(conn, requete);

	printf("Modification éffectuées\n");

	PQfinish(conn);
	printf("\n");
}

void traitementCode(int socket_dial, int socket_ecoute, int id)
{
	int code;
	strcpy(buf, "Envoyez votre code 1:train 2:urgence 3:déconnexion\n");
	send(socket_dial, buf, sizeof(buf), 0);
	gettimeofday(&t1, NULL);
	printf("Attente de la réception du code\n");

	strcpy(buf, "");

	etatConnexion = recv(socket_dial, &buf, sizeof(buf), 0);
	verificationEtatConnexion(etatConnexion, socket_dial, socket_ecoute);
	gettimeofday(&t2, NULL);
	code = atoi(buf);
	result = diff_timeval(t2, t1, socket_dial, socket_ecoute);

	printf("J'ai recu [%d] du client\n", code);

	if (code == 1)
	{
		strcpy(buf, "Code 1 reçu, pret à réceptionner l'état de votre train d'atterissage.\n");
		send(socket_dial, buf, sizeof(buf), 0);
		gettimeofday(&t1, NULL);

		etatConnexion = recv(socket_dial, &buf, sizeof(buf), 0);
		verificationEtatConnexion(etatConnexion, socket_dial, socket_ecoute);
		printf("Avion : %s\n", buf);

		gettimeofday(&t2, NULL);
		result = diff_timeval(t2, t1, socket_dial, socket_ecoute);

		if (strcmp(buf, "true") == 0)
		{
			printf("Je rentre dans le true");
			trainPlane(id, 1);

		strcpy(buf, "L'état de votre train d'atterissage à été mis à jour.\n");
		send(socket_dial, buf, sizeof(buf), 0);

		}
		else if (strcmp(buf, "false") == 0)
		{
			printf("Je rentre dans le false");
			trainPlane(id, 0);
			
			strcpy(buf, "L'état de votre train d'atterissage à été mis à jour.\n");
			send(socket_dial, buf, sizeof(buf), 0);
		}else{
			strcpy(buf, "Aucune donnée mise à jour. true/false erreur.\n");
			send(socket_dial, buf, sizeof(buf), 0);
			printf("Aucune donnée mise à jour. Erreur au true/false.\n");

		}
	}
	else if (code == 2)
	{
		strcpy(buf, "Code 2 reçu, Signalement à l'administrateur !\n");
		send(socket_dial, buf, sizeof(buf), 0);
		urgence(id);
	}
	else if (code == 3)
	{
		strcpy(buf, "Code 3 reçu, déconnexion !\n");
		send(socket_dial, buf, sizeof(buf), 0);
		close(socket_dial);
		close(socket_ecoute);
		exit(1);
	}
	else
	{
		strcpy(buf, "Erreur dans le code envoyé.");
		send(socket_dial, buf, sizeof(buf), 0);
		traitementCode(socket_dial, socket_ecoute, id);
	}
}

int main()
{
	int code;
	int id;
	int socket_ecoute, socket_dial, cli_len; //On definit des entier qui prendront les sockets
	struct sockaddr_in serv_addr;			 //On definit une adresse pour le serveur
	struct sockaddr_in cli_addr;			 //On definit une adresse pour le serveur

	/******* LE SERVEUR ********/
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

	printf("Serveur lancé | IP: %s | port: %d\n",
		   inet_ntoa(serv_addr.sin_addr), ntohs(serv_addr.sin_port));

	/******** LES SOCKETS ********/

	//On initialise le socket d'écoute
	socket_ecoute = socket(PF_INET, SOCK_STREAM, 0);
	//On connecte le socket d'écoute au serveur
	//bind(socket_ecoute, (struct sockaddr *)&serv_addr, sizeof serv_addr);

	if ((bind(socket_ecoute, (struct sockaddr *)&serv_addr, sizeof serv_addr)) < 0)
	{
		printf("Port occupé, veuillez lancer le serveur avec un port différent. \n");
		exit(1);
	}

	//Il ecoute jusqu'à 1 client
	listen(socket_ecoute, 1);
	//On lui donne la taille des données d'adresse
	cli_len = sizeof(cli_addr);

	//Socket de dialogue initialisée dès que la connexion est faite.
	socket_dial = accept(socket_ecoute, (struct sockaddr *)&cli_addr, &cli_len); //boucle infinie en attente d'un client

	/********* LA CONNEXION EST ETABLIE **********/

	printf("Le client d'adresse IP %s s'est connecté depuis son port %d\n",
		   inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));

	/* Reception DU CODE DE L'AVION */
	strcpy(buf, "Bonjour, quel est votre id ?");
	send(socket_dial, buf, sizeof(buf), 0);
	gettimeofday(&t1, NULL);

	etatConnexion = recv(socket_dial, &buf, sizeof(buf), 0);
	verificationEtatConnexion(etatConnexion, socket_dial, socket_ecoute);
	printf("Avion: %s\n", buf);
	gettimeofday(&t2, NULL);
	id = atoi(buf);

	result = diff_timeval(t2, t1, socket_dial, socket_ecoute);

	while (id == 0 || id != verificationID(id))
	{
		strcpy(buf, "Erreur dans le id, veuillez rentrer le bon id.");
		etatConnexion = send(socket_dial, buf, sizeof(buf), 0);
		gettimeofday(&t1, NULL);


		etatConnexion = recv(socket_dial, &buf, sizeof(buf), 0);
		verificationEtatConnexion(etatConnexion, socket_dial, socket_ecoute);
		printf("L'avion n'a pas pu s'identifer : %s\n", buf);
		id = atoi(buf);
		gettimeofday(&t2, NULL);

		result = diff_timeval(t2, t1, socket_dial, socket_ecoute);

	}
	printf("Avion %d recconnu. \n", id);

	strcpy(buf, "Votre id à été recconu.");
	send(socket_dial, buf, sizeof(buf), 0);

	while (1)
	{
		traitementCode(socket_dial, socket_ecoute, id);
	}

	close(socket_dial);
	close(socket_ecoute);
}
