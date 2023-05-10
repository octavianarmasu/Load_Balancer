/* Copyright 2023 <> */
#include "load_balancer.h"

#include <stdlib.h>
#include <string.h>

#include "server.h"
#include "utils.h"

struct info {
  void *key;
  void *value;
};

struct server_memory {
  int replica_id;
  int server_id;
  unsigned int hash;
  hashtable_t *ht;
};

struct load_balancer {
  server_memory **servers;
  int nr_servers;
};

load_balancer *init_load_balancer() {
  load_balancer *main = malloc(sizeof(*main));
  DIE(!main, "malloc failed");
  main->servers = calloc(3, sizeof(server_memory *));
  DIE(!main->servers, "calloc failed");
  main->nr_servers = 3;
  return main;
}

unsigned int hash_function_key(void *a) {
  unsigned char *puchar_a = (unsigned char *)a;
  unsigned int hash = 5381;
  int c;

  while ((c = *puchar_a++)) hash = ((hash << 5u) + hash) + c;

  return hash;
}

unsigned int hash_function_servers(void *a) {
  unsigned int uint_a = *((unsigned int *)a);

  uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
  uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
  uint_a = (uint_a >> 16u) ^ uint_a;
  return uint_a;
}

void loader_add_server(load_balancer *main, int server_id) {
  int i;

  /**
   * i are aceasta valoare pentru ca mereu vom avea ultimele 3 servere sa
   * pointeze la NULL, deci i va trebui sa inceapa de la nr de servere - 3
   */

  i = main->nr_servers - 3;

  // adaugam serverul si cele 2 replicii ale sale

  main->servers[i] = init_server_memory();
  main->servers[i]->replica_id = 0;
  main->servers[i]->server_id = server_id;
  main->servers[i]->hash = hash_function_servers(&server_id);

  main->servers[i + 1] = init_server_memory();
  main->servers[i + 1]->replica_id = 1;
  main->servers[i + 1]->server_id = server_id;
  int sv_id = server_id + 100000;
  main->servers[i + 1]->hash = hash_function_servers(&sv_id);

  main->servers[i + 2] = init_server_memory();
  main->servers[i + 2]->replica_id = 2;
  main->servers[i + 2]->server_id = server_id;
  sv_id = server_id + 200000;
  main->servers[i + 2]->hash = hash_function_servers(&sv_id);

  // sortam serverele in functie de hash
  for (int j = 0; j < main->nr_servers - 1; j++) {
    for (int k = j + 1; k < main->nr_servers; k++)
      if (main->servers[j]->hash > main->servers[k]->hash) {
        server_memory *aux = main->servers[j];
        main->servers[j] = main->servers[k];
        main->servers[k] = aux;
      }
  }

  // redistribuim elementele din serverele vechi in cele noi
  if (main->nr_servers != 3) {
    for (int j = 0; j < main->nr_servers; ++j) {
      server_memory *sv = main->servers[j];
      if (sv->server_id == server_id) {
        int p = j + 1;
        if (p >= main->nr_servers) p = 0;
        server_memory *sv1 = main->servers[p];
        int size = get_size(sv1);
        if (size == 0) continue;
        info **inf = malloc(sizeof(info *) * size);
        DIE(!inf, "malloc failed");
        for (int k = 0; k < size; k++) {
          inf[k] = malloc(sizeof(info));
          DIE(!inf[k], "malloc failed");
        }
        for (int k = 0; k < size; k++) {
          char *key = get_key_server(sv1);
          char *value = get_value_server(sv1);

          inf[k]->key = malloc(strlen(key) + 1);
          DIE(!inf[k]->key, "malloc failed");

          inf[k]->value = malloc(strlen(value) + 1);
          DIE(!inf[k]->value, "malloc failed");

          memcpy(inf[k]->key, key, strlen(key) + 1);
          memcpy(inf[k]->value, value, strlen(value) + 1);

          server_remove(sv1, key);
        }
        for (int k = 0; k < size; k++) {
          char *key = inf[k]->key;
          char *value = inf[k]->value;
          rebalansare(main, key, value);
        }
        for (int k = 0; k < size; ++k) {
          free(inf[k]->key);
          free(inf[k]->value);
          free(inf[k]);
        }
        free(inf);
      }
    }
  }

  // adaugam 3 servere noi care vor pointa la NULL

  main->nr_servers += 3;

  main->servers =
      realloc(main->servers, main->nr_servers * sizeof(server_memory *));
  DIE(!main->servers, "realloc failed");
  for (int j = main->nr_servers - 3; j < main->nr_servers; j++)
    main->servers[j] = NULL;
}

void loader_remove_server(load_balancer *main, int server_id) {
  for (int i = 0; i < main->nr_servers - 3; i++) {
    server_memory *sv = main->servers[i];
    if (sv->server_id == server_id) {
      main->nr_servers--;
      for (int j = i; j < main->nr_servers; j++)
        main->servers[j] = main->servers[j + 1];
      server_memory *sv2 = main->servers[i];
      if (sv2 == NULL) {
        sv2 = main->servers[0];
      }
      // functia folosita pentru mutarea elementelor dintr-un server in altul
      move_keys(sv, sv2);
      free_server_memory(sv);
      i--;
    }
  }
}

void loader_store(load_balancer *main, char *key, char *value, int *server_id) {
  unsigned int hash = hash_function_key(key);
  int i;
  int ok = 0;
  for (i = 0; i < main->nr_servers - 3; ++i) {
    server_memory *sv = main->servers[i];
    if (sv->hash >= hash) {
      ok = i;
      break;
    }
  }
  /**
   * folosim ok pentru a afla indicele serverului pe care trebuie sa il
   * vom adauga key si value
   */
  i = ok;
  server_memory *sv = main->servers[i];
  *server_id = sv->server_id;
  server_store(main->servers[i], key, value);
}

void rebalansare(load_balancer *main, char *key, char *value) {
  unsigned int hash = hash_function_key(key);
  int i;
  int ok = 0;
  for (i = 0; i < main->nr_servers; ++i) {
    server_memory *sv = main->servers[i];
    if (sv->hash >= hash) {
      ok = i;
      break;
    }
  }
  i = ok;
  server_store(main->servers[i], key, value);
}

char *loader_retrieve(load_balancer *main, char *key, int *server_id) {
  unsigned int hash = hash_function_key(key);
  int i;
  int ok = 0;
  for (i = 0; i < main->nr_servers - 3; ++i) {
    server_memory *sv = main->servers[i];
    if (sv->hash >= hash) {
      ok = i;
      break;
    }
  }
  i = ok;
  server_memory *sv = main->servers[i];
  *server_id = sv->server_id;
  char *aux = server_retrieve(sv, key);
  return aux;
}

void free_load_balancer(load_balancer *main) {
  for (int i = 0; i < main->nr_servers; i++) {
    free_server_memory(main->servers[i]);
  }
  free(main->servers);
  free(main);
}
