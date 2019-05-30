#include "connections.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

pthread_mutex_t client_list_mutex;

void establish_client_connections (struct sockaddr_in *server_addr, struct sockaddr_in *addr) {
	int error;

	sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (sock_fd == -1){
		perror("socket: ");
		exit(-1);
	}

	memset(server_addr, '0', sizeof(*server_addr));
	addr->sin_family = AF_INET;
	server_addr->sin_port = htons(PORT);

	if(inet_pton(AF_INET, "127.0.0.1", &server_addr->sin_addr)<=0)  
    { 
        printf("\nInvalid address/ Address not supported \n"); 
        exit (-1); 
    } 

	server_addr->sin_family = AF_INET;
    server_addr->sin_port = htons(PORT); 
 	error = connect(sock_fd, (struct sockaddr *)server_addr, sizeof(*server_addr));
	if(error == -1) {
		perror("connect");
		exit(-1);
	}
}

void establish_server_connections ( struct sockaddr_in *address, int *server_fd) {
	*server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (*server_fd == -1){
		perror("socket: ");
		exit(-1);
	}

	address->sin_family = AF_INET;
	address->sin_addr.s_addr = INADDR_ANY; 
    address->sin_port = htons( PORT );
    if (bind(*server_fd, (struct sockaddr *)address,  
                                 sizeof(*address))<0) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    }
    if (listen(*server_fd, 3) < 0)	// segundo argumento de listen é o limite da fila de conexoes 
    { 
        perror("listen"); 
        exit(EXIT_FAILURE); 
    }
    // Server is now ready to accept clients, so it initializes the client list mutex
    pthread_mutex_init(&client_list_mutex, NULL);
}

int server_accept_client (struct sockaddr_in *address, int *server_fd) {
    int new_socket;
    int addrlen = sizeof(*address);

    if ((new_socket = accept(*server_fd, (struct sockaddr *)&address,  
                       (socklen_t*)&addrlen))<0) { 
        perror("accept"); 
        exit(EXIT_FAILURE); 
    }

    return new_socket;
}

/*
    Receives a single card and sends it to all the clients contained in the Client_list
*/
void send_all_clients (card_info card) {
    Node* aux = Client_list;

    char* str = malloc(sizeof(card_info));
    memcpy(str, &card, sizeof(card_info));
    while(aux != NULL){
        pthread_mutex_lock(&client_list_mutex);
        write(aux->client.client_socket,str, sizeof(card_info));
        aux = aux->next;
        pthread_mutex_unlock(&client_list_mutex);
    }
}

Node * Add_Client (int new_client, int *number_of_clients){
    Node* new_node = NULL;

    new_node = malloc (sizeof(Node));
    if (new_node == NULL){
        printf("Erro de alocação\n");
        exit (EXIT_FAILURE);
    }

    new_node->client.client_socket = new_client;
    new_node->client.score = 0;
    new_node->next = NULL;

    pthread_mutex_lock(&client_list_mutex);
    if (Client_list == NULL)
        Client_list = new_node;
    else{
        new_node->next = Client_list;
        Client_list = new_node;
    } 
    pthread_mutex_unlock(&client_list_mutex);
    (*number_of_clients)++;
    return Client_list;
}

Node * Remove_Client (int client, int *number_of_clients){
    Node* aux = Client_list;
    Node* aux2 = Client_list;

    pthread_mutex_lock(&client_list_mutex);    
    if (Client_list == NULL)
        return Client_list; //ver
    aux = Client_list;
    aux2 = Client_list->next;

    while (aux2 != NULL && aux2->client.client_socket != client){
        aux = aux2;
        aux2 = aux2->next;
    }
    if (aux2 != NULL){
        aux->next = aux2->next;
        free (aux2);
    }
    pthread_mutex_unlock(&client_list_mutex);
    (*number_of_clients)--;
    if (*number_of_clients < 0) {
        printf("ERROR: NEGATIVE NUMBER OF CLIENTS\n");
        exit(EXIT_FAILURE);
    }
    return Client_list;
}

