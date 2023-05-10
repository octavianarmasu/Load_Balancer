/* Copyright 2023 <> */
#ifndef SERVER_H_
#define SERVER_H_

struct server_memory;
typedef struct server_memory server_memory;
typedef struct hashtable_t hashtable_t;
typedef struct info info;

/** init_server_memory() -  Initializes the memory for a new server struct.
 * 							Make sure to check what
 * is returned by malloc using DIE. Use the linked list implementation from the
 * lab.
 *
 * Return: pointer to the allocated server_memory struct.
 */
server_memory *init_server_memory();

/** free_server_memory() - Free the memory used by the server.
 * 						   Make sure to also free the
 * pointer to the server struct. You can use the server_remove() function for
 * this.
 *
 * @arg1: Server to free
 */
void free_server_memory(server_memory *server);

/**
 * server_store() - Stores a key-value pair to the server.
 *
 * @arg1: Server which performs the task.
 * @arg2: Key represented as a string.
 * @arg3: Value represented as a string.
 */
void server_store(server_memory *server, char *key, char *value);

/**
 * server_remove() - Removes a key-pair value from the server.
 *					 Make sure to free the memory of
 *everything that is related to the entry removed.
 *
 * @arg1: Server which performs the task.
 * @arg2: Key represented as a string.
 */
void server_remove(server_memory *server, char *key);

/**
 * server_retrieve() - Gets the value associated with the key.
 * @arg1: Server which performs the task.
 * @arg2: Key represented as a string.
 *
 * Return: String value associated with the key
 *         or NULL (in case the key does not exist).
 */
char *server_retrieve(server_memory *server, char *key);

/**
 * move_keys() - Copiaza cheiile de pe primul server pe al doilea
 */

void move_keys(server_memory *server, server_memory *new_server);

/**
 * returneaza size-ul hashtable-ului(cate elemente sunt in hashtable)
 */

int get_size(server_memory *server);

/**
 * returneaza valoarea asociata cheii
 */

char *get_value_server(server_memory *server);

/**
 * returneaza prima cheie gasita in server
 */

char *get_key_server(server_memory *server);

#endif /* SERVER_H_ */
