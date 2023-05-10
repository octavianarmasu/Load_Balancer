/* Copyright 2023 <> */
#include "server.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"
#define HMAX 60

typedef struct ll_node_t {
  void *data;
  struct ll_node_t *next;
} ll_node_t;

typedef struct linked_list_t {
  ll_node_t *head;
  unsigned int data_size;
  unsigned int size;
} linked_list_t;

linked_list_t *ll_create(unsigned int data_size) {
  linked_list_t *ll;

  ll = malloc(sizeof(*ll));

  ll->head = NULL;
  ll->data_size = data_size;
  ll->size = 0;

  return ll;
}

void ll_add_nth_node(linked_list_t *list, unsigned int n,
                     const void *new_data) {
  ll_node_t *prev, *curr;
  ll_node_t *new_node;

  if (!list) {
    return;
  }

  if (n > list->size) {
    n = list->size;
  }

  curr = list->head;
  prev = NULL;
  while (n > 0) {
    prev = curr;
    curr = curr->next;
    --n;
  }

  new_node = malloc(sizeof(*new_node));
  new_node->data = malloc(list->data_size);
  memcpy(new_node->data, new_data, list->data_size);

  new_node->next = curr;
  if (prev == NULL) {
    list->head = new_node;
  } else {
    prev->next = new_node;
  }

  list->size++;
}

ll_node_t *ll_remove_nth_node(linked_list_t *list, unsigned int n) {
  ll_node_t *prev, *curr;

  if (!list || !list->head) {
    return NULL;
  }

  if (n > list->size - 1) {
    n = list->size - 1;
  }

  curr = list->head;
  prev = NULL;
  while (n > 0) {
    prev = curr;
    curr = curr->next;
    --n;
  }

  if (prev == NULL) {
    list->head = curr->next;
  } else {
    prev->next = curr->next;
  }

  list->size--;

  return curr;
}

unsigned int ll_get_size(linked_list_t *list) {
  if (!list) {
    return -1;
  }

  return list->size;
}

void ll_free(linked_list_t **pp_list) {
  ll_node_t *currNode;

  if (!pp_list || !*pp_list) {
    return;
  }

  while (ll_get_size(*pp_list) > 0) {
    currNode = ll_remove_nth_node(*pp_list, 0);
    free(currNode->data);
    currNode->data = NULL;
    free(currNode);
    currNode = NULL;
  }

  free(*pp_list);
  *pp_list = NULL;
}

typedef struct info info;
struct info {
  void *key;
  void *value;
};

typedef struct hashtable_t hashtable_t;
struct hashtable_t {
  linked_list_t **buckets;
  unsigned int size;
  unsigned int hmax;
  unsigned int (*hash_function)(void *);
  int (*compare_function)(void *, void *);
  void (*key_val_free_function)(void *);
};

int compare_function_strings(void *a, void *b) {
  if (a == NULL || b == NULL) return 1;
  char *str_a = (char *)a;
  char *str_b = (char *)b;

  return strcmp(str_a, str_b);
}

void key_val_free_function(void *data) {
  info *data_info = (info *)data;
  free(data_info->key);
  free(data_info->value);
}

hashtable_t *ht_create(unsigned int hmax, unsigned int (*hash_function)(void *),
                       int (*compare_function)(void *, void *),
                       void (*key_val_free_function)(void *)) {
  if (!hash_function || !compare_function) {
    return NULL;
  }

  hashtable_t *map = malloc(sizeof(hashtable_t));

  map->size = 0;
  map->hmax = hmax;
  map->hash_function = hash_function;
  map->compare_function = compare_function;
  map->key_val_free_function = key_val_free_function;

  map->buckets = malloc(map->hmax * sizeof(linked_list_t *));
  for (unsigned int i = 0; i < map->hmax; ++i) {
    map->buckets[i] = ll_create(sizeof(info));
  }

  return map;
}

int ht_has_key(hashtable_t *ht, void *key) {
  if (!ht || !key) {
    return -1;
  }

  int hash_index = ht->hash_function(key) % ht->hmax;
  ll_node_t *node = ht->buckets[hash_index]->head;

  while (node != NULL) {
    info *data_info = (info *)node->data;
    if (!ht->compare_function(data_info->key, key)) {
      return 1;
    }
    node = node->next;
  }

  return 0;
}

void *ht_get(hashtable_t *ht, void *key) {
  if (!ht || !key || ht_has_key(ht, key) != 1) {
    return NULL;
  }

  int hash_index = ht->hash_function(key) % ht->hmax;
  ll_node_t *node = ht->buckets[hash_index]->head;

  while (node != NULL) {
    info *data_info = (info *)node->data;
    if (!ht->compare_function(data_info->key, key)) {
      return data_info->value;
    }
    node = node->next;
  }

  return NULL;
}

void ht_put(hashtable_t *ht, void *key, unsigned int key_size, void *value,
            unsigned int value_size) {
  if (!ht || !key || !value) {
    return;
  }

  int hash_index = ht->hash_function(key) % ht->hmax;

  if (ht_has_key(ht, key) == 1) {
    ll_node_t *node = ht->buckets[hash_index]->head;
    while (node != NULL) {
      info *data_info = node->data;

      if (!ht->compare_function(data_info->key, key)) {
        free(data_info->value);

        data_info->value = malloc(value_size);

        memcpy(data_info->value, value, value_size);
        return;
      }

      node = node->next;
    }
  }

  info *data_info = malloc(sizeof(info));

  data_info->key = malloc(key_size);
  data_info->value = malloc(value_size);

  memcpy(data_info->key, key, key_size);
  memcpy(data_info->value, value, value_size);

  ll_add_nth_node(ht->buckets[hash_index], 0, data_info);
  ht->size++;

  free(data_info);
}

void ht_remove_entry(hashtable_t *ht, void *key) {
  if (!ht || !key || ht_has_key(ht, key) != 1) {
    return;
  }

  int hash_index = ht->hash_function(key) % ht->hmax;
  ll_node_t *node = ht->buckets[hash_index]->head;

  unsigned int node_nr = 0;

  while (node != NULL) {
    info *data_info = (info *)node->data;

    if (!ht->compare_function(data_info->key, key)) {
      ht->key_val_free_function(data_info);
      free(data_info);

      ll_node_t *deleted_node =
          ll_remove_nth_node(ht->buckets[hash_index], node_nr);
      free(deleted_node);

      ht->size--;
      return;
    }

    node = node->next;
    node_nr++;
  }
}

void ht_free(hashtable_t *ht) {
  if (!ht) {
    return;
  }

  for (unsigned int i = 0; i < ht->hmax; ++i) {
    ll_node_t *node = ht->buckets[i]->head;

    while (node != NULL) {
      ht->key_val_free_function(node->data);
      node = node->next;
    }

    ll_free(&ht->buckets[i]);
  }

  free(ht->buckets);
  free(ht);
}
unsigned int ht_get_size(hashtable_t *ht) {
  if (ht == NULL) return 0;

  return ht->size;
}

unsigned int ht_get_hmax(hashtable_t *ht) {
  if (ht == NULL) return 0;

  return ht->hmax;
}
unsigned int hash_function(void *a) {
  unsigned char *puchar_a = (unsigned char *)a;
  unsigned int hash = 5381;
  int c;

  while ((c = *puchar_a++)) hash = ((hash << 5u) + hash) + c;

  return hash;
}

struct server_memory {
  int replica_id;
  int server_id;
  unsigned int hash;
  hashtable_t *ht;
};

server_memory *init_server_memory() {
  server_memory *server = malloc(sizeof(server_memory));
  DIE(server == NULL, "server malloc failed");
  server->ht = ht_create(HMAX, hash_function, compare_function_strings,
                         key_val_free_function);
  return server;
}

void server_store(server_memory *server, char *key, char *value) {
  ht_put(server->ht, key, strlen(key) + 1, value, strlen(value) + 1);
}

char *server_retrieve(server_memory *server, char *key) {
  return ht_get(server->ht, key);
}

void server_remove(server_memory *server, char *key) {
  ht_remove_entry(server->ht, key);
}

void free_server_memory(server_memory *server) {
  if (server == NULL) return;
  ht_free(server->ht);
  free(server);
}

int get_size(server_memory *server) { return ht_get_size(server->ht); }

void move_keys(server_memory *server, server_memory *new_server) {
  hashtable_t *ht1 = server->ht;
  hashtable_t *ht2 = new_server->ht;
  int size = ht1->hmax;
  for (int i = 0; i < size; i++) {
    if (ht1->buckets[i]->size != 0) {
      ll_node_t *node = ht1->buckets[i]->head;
      while (node != NULL) {
        info *data_info = (info *)node->data;
        ht_put(ht2, data_info->key, strlen(data_info->key) + 1,
               data_info->value, strlen(data_info->value) + 1);
        node = node->next;
      }
    }
  }
  for (int i = 0; i < size; i++) {
    while (ht1->buckets[i]->size != 0) {
      ll_node_t *node = ht1->buckets[i]->head;
      info *data_info = (info *)node->data;
      ht_remove_entry(ht1, data_info->key);
    }
  }
}

char *get_key_server(server_memory *server) {
  int size = server->ht->hmax;
  for (int i = 0; i < size; ++i) {
    if (server->ht->buckets[i]->size != 0) {
      ll_node_t *node = server->ht->buckets[i]->head;
      info *data_info = (info *)node->data;
      return data_info->key;
    }
  }
  return NULL;
}

char *get_value_server(server_memory *server) {
  int size = server->ht->hmax;
  for (int i = 0; i < size; ++i) {
    if (server->ht->buckets[i]->size != 0) {
      ll_node_t *node = server->ht->buckets[i]->head;
      info *data_info = (info *)node->data;
      return data_info->value;
    }
  }
  return NULL;
}