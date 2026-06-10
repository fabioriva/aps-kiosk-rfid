#include "global.h"
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
//--------------------------------------------------------------------------------------------------
void ComponiPost(char* content, size_t max_content_len) {
	int i;
	char tempstr[10];
	char str_uid[64];   // Dimensione aumentata per sicurezza (es. supporta UID fino a 31 byte)
	char str_data[64];  // Dimensione aumentata per sicurezza

	if (content == NULL || max_content_len == 0) return;

	// 1. Composizione sicura dell'UID
	str_uid[0] = '\0';
	// Protezione: interrompi se DimUidTessera è troppo grande per il nostro buffer str_uid
	int max_uid_loops = BloccoDatiIn.Struttura.DimUidTessera;
	if (max_uid_loops > 31) max_uid_loops = 31; // 31 * 2 = 62 caratteri + \0

	for (i = 0; i < max_uid_loops; i++)
	{
		snprintf(tempstr, sizeof(tempstr), "%02X", BloccoDatiIn.Struttura.Uid[i]);
		strcat(str_uid, tempstr);
	}

	// 2. Composizione sicura del Codice Tessera
	str_data[0] = '\0';
	// Protezione: interrompi se DIM_CODICE_TESSERA è troppo grande per str_data
	int max_data_loops = DIM_CODICE_TESSERA;
	if (max_data_loops > 31) max_data_loops = 31;

	for (i = 0; i < max_data_loops; i++)
	{
		snprintf(tempstr, sizeof(tempstr), "%02X", BloccoDatiIn.Struttura.CodiceTessera[i]);
		strcat(str_data, tempstr);
	}

	// 3. Scrittura sicura nel buffer finale (impedisce l'overflow di content)
	snprintf(content, max_content_len, "{ \"uid\": \"%s\", \"data\": \"%s\" }", str_uid, str_data);
}

int http_post(const char* url, uint16_t portno, const char* content) {
	int sockfd, n;
	struct sockaddr_in serv_addr;
	struct hostent* server;

	// Controllo di sicurezza iniziale per evitare crash con puntatori nulli
	if (url == NULL || content == NULL) {
		LOG_E((char*)"Errore: url o content sono NULL");
		return 1;
	}

	// Estrai host e percorso dall'URL
	char hostname[256];
	char path[256];
	// Inizializza i buffer a zero per sicurezza
	memset(hostname, 0, sizeof(hostname));
	memset(path, 0, sizeof(path));

	// Limita la lettura a 255 caratteri per evitare buffer overflow
	if (sscanf(url, "http://%255[^/]%255s", hostname, path) < 1) {
		LOG_E((char*)"Errore parsing URL");
		return 1;
	}

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		LOG_E((char*)"Errore apertura socket");
		return 1;
	}

	LOG_I((char*)"Hostname:");
	LOG_I(hostname);

	server = gethostbyname(hostname);
	if (server == NULL) {
		LOG_E((char*)"Errore, host sconosciuto");
		close(sockfd);
		return 1;
	}

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;

	// CORRETTO: Invertito l'ordine di memcpy per copiare DA server A serv_addr
	memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
	serv_addr.sin_port = htons(portno);

	if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
		LOG_E((char*)"Errore connessione");
		close(sockfd);
		return 1;
	}

	// Calcola la dimensione dinamica della richiesta per evitare overflow
	// 200 byte stimati per gli header + la lunghezza del contenuto
	size_t req_size = strlen(path) + strlen(hostname) + strlen(content) + 200;
	char* request = (char*)malloc(req_size);
	if (request == NULL) {
		LOG_E((char*)"Errore allocazione memoria richiesta");
		close(sockfd);
		return 1;
	}

	// Costruisci la richiesta in modo sicuro
	snprintf(request, req_size, "POST %s HTTP/1.1\r\nHost: %s\r\nContent-Type: application/json\r\nContent-Length: %zu\r\n\r\n%s\r\n",
		path, hostname, strlen(content), content);

	LOG_I(request);

	// Invia la richiesta
	n = write(sockfd, request, strlen(request));
	free(request); // Libera subito la memoria allocata

	if (n < 0) {
		LOG_E((char*)"Errore scrittura socket");
		close(sockfd);
		return 1;
	}

	// Leggi la risposta (semplificata)
	char response[4096];
	n = read(sockfd, response, sizeof(response) - 1); // CORRETTO: Evita hardcoding di 4095
	if (n < 0) {
		LOG_E((char*)"Errore lettura socket");
		close(sockfd);
		return 1;
	}

	response[n] = '\0';
	LOG_I((char*)"Response:");
	LOG_I(response); // Stampa la risposta

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
		LOG_E((char*)"Errore apertura socket");
		return 1;
	}

	server = gethostbyname(host);
	if (server == NULL) {
		LOG_E((char*)"Errore, host sconosciuto");
		close(sockfd);
		return 1;
	}

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	memcpy((char*)server->h_addr, (char*)&serv_addr.sin_addr.s_addr, server->h_length);
	serv_addr.sin_port = htons(port);

	if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
		LOG_E((char*)"Errore connessione");
		close(sockfd);
		return 1;
	}

	sprintf(request, "GET /?%s HTTP/1.1\r\nHost: %s:%d\r\n\r\n", data, host, port);
	//sprintf(request, "PUT /data HTTP/1.1\r\nHost: %s:%d\r\nContent-Type: text/plain\r\nContent-Length: %ld\r\n\r\n%s", host, port, strlen(data), data);

	n = write(sockfd, request, strlen(request));
	if (n < 0) {
		LOG_E((char*)"Errore scrittura socket");
		close(sockfd);
		return 1;
	}

	char response[4096];
	n = read(sockfd, response, 4095);
	if (n < 0) {
		LOG_E((char*)"Errore lettura socket");
		close(sockfd);
		return 1;
	}

	response[n] = '\0';
	LOG_I((char*)"Response:");
	LOG_I(response);

	close(sockfd);
	return 0;
}
//--------------------------------------------------------------------------------------------------
