#include "connections.h"

pthread_mutex_t CLIENT_LIST_MUTEX;  // mutex to protect changes to the client list

/*  function establish_client_connections
    Connects a client to the server 
*/
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

/*  function establish_server_connections
    Prepares the server to accept connections with clients
*/
void establish_server_connections ( struct sockaddr_in *address, int *server_fd) {
	*server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (*server_fd == -1){
		perror("socket: ");
		exit(-1);
	}

    // Fixes the bind error address
    if (setsockopt(*server_fd, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)) < 0) {
        printf("setsockopt(SO_REUSEADDR) failed");
    }

	address->sin_family = AF_INET;
	address->sin_addr.s_addr = INADDR_ANY; 
    address->sin_port = htons(PORT);
    if (bind(*server_fd, (struct sockaddr *)address, sizeof(*address))<0) { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    }
    if (listen(*server_fd, 10) < 0) {	// segundo argumento de listen é o limite da fila de conexoes  
        perror("listen"); 
        exit(EXIT_FAILURE); 
    }
    // Server is now ready to accept clients, so it initializes the client list mutex
    if (pthread_mutex_init(&CLIENT_LIST_MUTEX, NULL) != 0) {
        printf("Mutex init has failed!\n");
        exit(EXIT_FAILURE);
    }
}

/*  function server_accept_client
    accepts a client connection and returns the new socket
*/
int server_accept_client (struct sockaddr_in *address, int *server_fd, int number_of_clients) {
    int new_socket;
    int addrlen = sizeof(*address);

    if (number_of_clients == MAX_CLIENTS){
        return -1;
    }

    if ((new_socket = accept(*server_fd, (struct sockaddr *)&address,  
                       (socklen_t*)&addrlen))<0) { 
        perror("accept"); 
        exit(EXIT_FAILURE); 
    }

    return new_socket;
}

/*  function send_all_clients
     Sends a single card to all the clients contained in the Client_list
*/
void send_all_clients (card_info card) {
    Node* aux;
    char* str = malloc(sizeof(card_info));
    if (str == NULL){
      printf("Allocation error\n");
      exit(EXIT_FAILURE);
    }
    memcpy(str, &card, sizeof(card_info));
    
    pthread_mutex_lock(&CLIENT_LIST_MUTEX);
    aux = Client_list;
    // sends the card to all of the clients
    while(aux != NULL){
        write(aux->client.client_socket,str, sizeof(card_info));
        aux = aux->next;
    }
    pthread_mutex_unlock(&CLIENT_LIST_MUTEX);
    free(str);
}

/*  function Add_Client
    adds a new_client to the client list
    increments the variable number_of_clients
*/
int Add_Client (int new_client, int *number_of_clients){
    Node* new_node = NULL;

    if (new_client == -1)
        return -1;

    new_node = malloc (sizeof(Node));
    if (new_node == NULL){
        printf("Allocation error\n");
        exit (EXIT_FAILURE);
    }

    new_node->client.client_socket = new_client;
    new_node->client.score = 0;
    new_node->next = NULL;

    pthread_mutex_lock(&CLIENT_LIST_MUTEX);
    // Insert the client on the list
    if (Client_list == NULL)
        Client_list = new_node;
    else{
        new_node->next = Client_list;
        Client_list = new_node;
    } 
    pthread_mutex_unlock(&CLIENT_LIST_MUTEX);
    (*number_of_clients)++;
    return 0;
}

/*  function Remove_Client
    removes a client from the client list
    updates de Client list accordingly
*/
void Remove_Client (int client, int *number_of_clients){
    Node* temp = Client_list;
    Node* aux = NULL;

    pthread_mutex_lock(&CLIENT_LIST_MUTEX);
    
    // if the head of the list is the node to be removed
    if (temp != NULL && temp->client.client_socket == client) {
        aux = temp->next;
        free(temp);
        Client_list = aux;
        pthread_mutex_unlock(&CLIENT_LIST_MUTEX);
        (*number_of_clients)--;
        if (*number_of_clients < 0) {
            printf("ERROR: NEGATIVE NUMBER OF CLIENTS\n");
            exit(EXIT_FAILURE);
        }
        return;
    }

    // if it is not the head, search the list
    while(temp != NULL && temp->client.client_socket != client) {
        aux = temp;
        temp = temp->next;
    }

    if (temp == NULL) {
        printf("ERROR: client was not found in the client list\n");
        exit(EXIT_FAILURE);
    }
    // node to be removed was found
    aux->next = temp->next;
    free(temp);
    // Head was not changed
    pthread_mutex_unlock(&CLIENT_LIST_MUTEX);
    
    (*number_of_clients)--;
    if (*number_of_clients < 0) {
        printf("ERROR: NEGATIVE NUMBER OF CLIENTS\n");
        exit(EXIT_FAILURE);
    }
    return;
}