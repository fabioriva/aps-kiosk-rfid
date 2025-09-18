#include "global.h"
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
//--------------------------------------------------------------------------------------------------
void ComponiPost(char *content) {
	int i;
	char tempstr[10], str_uid[25], str_data[25];
	str_uid[0]=0;
	for (i = 0; i < BloccoDatiIn.Struttura.DimUidTessera; i++)
	{
		sprintf(tempstr, "%02X", BloccoDatiIn.Struttura.Uid[i]);
		strcat(str_uid, tempstr);
	}
	str_data[0]=0;
	for (i = 0; i < DIM_CODICE_TESSERA; i++)
	{
			sprintf(tempstr, "%02X", BloccoDatiIn.Struttura.CodiceTessera[i]);
			strcat(str_data, tempstr);
	}
	sprintf(content, "{ \"uid\": \"%s\", \"data\": \"%s\" }", str_uid, str_data);
	}

int http_post(const char* url, uint16_t portno, const char* content) {
	int sockfd, n;
	struct sockaddr_in serv_addr;
	struct hostent* server;

	// Estrai host e percorso dall'URL
	char hostname[256];
	char path[256];
	sscanf(url, "http://%[^/]%s", hostname, path);

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		perror("Errore apertura socket");
		return 1;
	}

	printf("Hostname: %s\n", hostname);
	server = gethostbyname(hostname);
	if (server == NULL) {
		fprintf(stderr, "Errore, host sconosciuto\n");
		close(sockfd);
		return 1;
	}

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	memcpy((char*)server->h_addr, (char*)&serv_addr.sin_addr.s_addr, server->h_length);
	serv_addr.sin_port = htons(uint16_t(portno));

	if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
		perror("Errore connessione");
		close(sockfd);
		return 1;
	}

	// Costruisci la richiesta HTTP POST
	char request[4096];
	sprintf(request, "POST %s HTTP/1.1\r\nHost: %s\r\nContent-Type: application/json\r\nContent-Length: %ld\r\n\r\n%s\r\n",
		path, hostname, strlen(content), content);
	printf("%s\n", request);
	// Invia la richiesta
	n = write(sockfd, request, strlen(request));
	if (n < 0) {
		perror("Errore scrittura socket");
		close(sockfd);
		return 1;
	}

	// Leggi la risposta (semplificata)
	char response[4096];
	n = read(sockfd, response, 4095);
	if (n < 0) {
		perror("Errore lettura socket");
		close(sockfd);
		return 1;
	}

	response[n] = '\0';
	printf("%s\n", response); // Stampa la risposta

	close(sockfd);
	return 0;
}
//--------------------------------------------------------------------------------------------------

int http_get(const char* host, uint16_t port, const char* data) {
	int sockfd, n;
	struct sockaddr_in serv_addr;
	struct hostent* server;
	char request[4096];

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		perror("Errore apertura socket");
		return 1;
	}

	server = gethostbyname(host);
	if (server == NULL) {
		fprintf(stderr, "Errore, host sconosciuto\n");
		close(sockfd);
		return 1;
	}

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	memcpy((char*)server->h_addr, (char*)&serv_addr.sin_addr.s_addr, server->h_length);
	serv_addr.sin_port = htons(port);

	if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
		perror("Errore connessione");
		close(sockfd);
		return 1;
	}

	sprintf(request, "GET /?%s HTTP/1.1\r\nHost: %s:%d\r\n\r\n", data, host, port);
	//sprintf(request, "PUT /data HTTP/1.1\r\nHost: %s:%d\r\nContent-Type: text/plain\r\nContent-Length: %ld\r\n\r\n%s", host, port, strlen(data), data);
	//sprintf(request, "%s", data);

	n = write(sockfd, request, strlen(request));
	if (n < 0) {
		perror("Errore scrittura socket");
		close(sockfd);
		return 1;
	}

	char response[4096];
	n = read(sockfd, response, 4095);
	if (n < 0) {
		perror("Errore lettura socket");
		close(sockfd);
		return 1;
	}

	response[n] = '\0';
	printf("%s\n", response);

	close(sockfd);
	return 0;
}
//--------------------------------------------------------------------------------------------------
