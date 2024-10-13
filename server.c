/*
 * Copyright (c) 2024, Postelnicu Cristina-Marina 313CA
 */

#include <stdio.h>
#include "server.h"
#include "lru_cache.h"

#include "utils.h"

// functia dll_create are rolul de a crea o lista dublu inlantuita
doubly_linked_list_t *dll_create(unsigned int data_size)
{
	doubly_linked_list_t *list = malloc(1 * sizeof(doubly_linked_list_t));
	list->head = NULL;
	list->tail = NULL;
	list->size = 0;
	list->data_size = data_size;
	return list;
}

/* functia dll_add_nth_node adauga un nod in lista pe pozitia n, iar daca n este
mai mare decat list->size adauga elementul pe ultima pozitie */
void dll_add_nth_node(doubly_linked_list_t *list,
					  unsigned int n, const void* new_data)
{
	unsigned int i;
	dll_node_t *new_node = malloc(sizeof(dll_node_t));
	new_node->data = malloc(list -> data_size);
	memcpy(new_node->data, new_data, list->data_size);
	if (!list->head) {
		// adauga daca lista este goala
		list->head = new_node;
		list->tail = new_node;
		new_node->prev = NULL;
		new_node->next = NULL;
		list->size = list->size + 1;
	} else if (n == 0) {
		// adauga la inceput
		new_node->next = list->head;
		new_node->prev = NULL;
		list->head->prev = new_node;
		list->head = new_node;
		list->size = list->size + 1;
	} else if (n >= list->size) {
		// adauga la final
		dll_node_t *current_node = list->tail;
		current_node->next = new_node;
		new_node->prev = current_node;
		new_node->next = NULL;
		list->tail = new_node;
		list->size = list->size + 1;
	} else {
		// adauga pe orice alta pozitie
		dll_node_t *current_node = list->head;
		i = 0;
		while (i < n - 1) {
			current_node = current_node->next;
			i++;
		}
		new_node->next = current_node->next;
		current_node->next->prev = new_node;
		current_node->next = new_node;
		new_node->prev = current_node;
		list->size = list->size + 1;
	}
}

/* functia dll_remove_nth_node sterge un element de pe pozitia n
din lista si returneaza nodul pe care l a sters */
dll_node_t *dll_remove_nth_node(doubly_linked_list_t *list, unsigned int n)
{
	if (!list->head) {
		// daca lista este goala
		return NULL;
	} else if (n == 0) {
		// sterge primul eleemnt din lista
		dll_node_t *to_remove = list->head;
		list->head = to_remove->next;
		if (list->head) {
			list->head->prev = NULL;
		} else {
			list->tail = NULL;
		}
		list->size = list->size - 1;
		return to_remove;
	} else if (n >= list->size) {
		// sterge de la final
		dll_node_t *to_remove = list->tail;
		list->tail = to_remove->prev;
		if (list->tail) {
			list->tail->next = NULL;
		} else {
			list->head = NULL;
		}
		list->size = list->size - 1;
		return to_remove;
	}
	// sterge de pe orice alta pozitie
	unsigned int i = 0;
	dll_node_t *current_node = list->head;
	for (i = 0; i < n - 1; i++)
		current_node = current_node->next;
	dll_node_t *to_remove = current_node->next;
	to_remove->prev->next = to_remove->next;
	if (to_remove->next)
		to_remove->next->prev = current_node;
	list->size = list->size - 1;
	return to_remove;
}

// elibereaza memoria unei structuri de tipul document
void document_t_free_function(void *data) {
	if (((document_t *)data)->name)
		free(((document_t *)data)->name);
	if (((document_t *)data)->content)
		free(((document_t *)data)->content);
}

// functia q_create are rolul de a crea o coada cu elemente de tip request
queue_t * q_create(unsigned int data_size, unsigned int max_size)
{
	queue_t *q;
	unsigned i = 0;
	q = malloc(1 * sizeof(queue_t));
	q->buff = calloc (max_size , sizeof(void*));
	for (i = 0; i < max_size; i++) {
		q->buff[i] = malloc(data_size);
		((request *)q->buff[i])->doc_name = NULL;
		((request *)q->buff[i])->doc_content = NULL;
	}
	q->read_idx = 0;
	q->write_idx = 0;
	q->data_size = data_size;
	q->max_size = max_size;
	q->size = 0;
	return q;
}

// functia q_is_empty verifica daca coada este goala
unsigned int q_is_empty(queue_t *q)
{
	if (q->size == 0)
		 return 1;
	return 0;
}

// functia returneaza primul element din coada
void *q_front(queue_t *q)
{
	return q->buff[q->read_idx];
}

// functia q_dequeue elimina primul element din coada
int q_dequeue(queue_t *q)
{
	if (q->size != 0) {
		if (q->buff[q->read_idx]) {
			free(q->buff[q->read_idx]);
			q->buff[q->read_idx] = NULL;
			q->read_idx = (q->read_idx + 1) % q->max_size;
			q->size--;
			return 1;
		}
	}
	return 0;
}

// functia q_enqueue adauga un element in coada
int q_enqueue(queue_t *q, void *new_data)
{
	if (q->size <= q->max_size) {
		if (q->buff[q->write_idx] == NULL) {
			q->buff[q->write_idx] = malloc(q->data_size);
		}
		memcpy(q->buff[q->write_idx], new_data, q->data_size);
		q->write_idx = (q->write_idx + 1) % q->max_size;
		q->size++;
		return 1;
	}
	return 0;
}

// functia q_clear elibereaza memoria pentru toate elementele din coada
void q_clear(queue_t *q)
{
	unsigned i;
	for (i = 0; i < q->max_size; i++) {
		if (q->buff[i]) {
			 if (((request *)q->buff[i])->doc_name &&
			((request *)q->buff[i])->doc_content) {
				free(((request *)q->buff[i])->doc_name);
				free(((request *)q->buff[i])->doc_content);
			}
		}
		free(q->buff[i]);
	}
	q->read_idx = 0;
	q->write_idx = 0;
	q->size = 0;
}

/* functia q_free elibereaza memoria pentru toate elementele din coada
si pentru coada */
void q_free(queue_t *q)
{
	q_clear(q);
	free(q->buff);
	free(q);
}

// functia server_edit_document se ocupa de request urile de tip EDIT
static response
*server_edit_document(server *s, char *doc_name, char *doc_content) {
	unsigned int ok = 0, i;
	response *response1 = malloc(1 * sizeof(response));
	if (response1 == NULL) {
		printf("Allocation failed\n");
		free(response1);
	}
	response1->server_log = malloc(1 * MAX_LOG_LENGTH);
	if (response1->server_log == NULL) {
		printf("Allocation failed\n");
		free(response1->server_log);
	}
	response1->server_response = malloc(1 * MAX_RESPONSE_LENGTH);
	if (response1->server_response == NULL) {
		printf("Allocation failed\n");
		free(response1->server_response);
	}
	response1->server_id = s->id;
	// verificam daca cheia se afla in hashtable
	ok = ht_has_key(s->cache->hashtable, doc_name);
	dll_node_t *content;
	// obtinem valorea asociata lui doc_name
	content = lru_cache_get(s->cache, doc_name);
	if (ok == 1) {
		/* daca documentul se gaseste in cache se modifica continutul si
		log ul si response ul*/
		sprintf(response1->server_log, LOG_HIT, doc_name);
		sprintf(response1->server_response, MSG_B, doc_name);
		strcpy(((info*)content->data)->value, doc_content);
		// se cauta si in database si se modifica continutul
		dll_node_t *current_node = s->database->head;
		for (i = 0; i < s->database->size; i++) {
			if (!strcmp((char *)((document_t *)current_node->data)->name, doc_name)) {
				strcpy((char *)((document_t *)current_node->data)->content, doc_content);
				break;
			}
			current_node = current_node->next;
		}
	} else {
		/* daca documentul se afla in cache se actualizeaza log ul si
		response ul este continutul acestuia */
		unsigned int verif = 0;
		dll_node_t *current_node = s->database->head;
		for (i = 0; i < s->database->size; i++) {
			if (!strcmp((char *)((document_t *)current_node->data)->name, doc_name)) {
				verif = 1;
				break;
			}
			current_node = current_node->next;
		}
		if (verif == 1) {
			/* daca documentul se afla in database se modifica continutul in
			database si se aduga in cache */
			void *evicted_key = NULL;
			// se actualizeaza response ul
			sprintf(response1->server_response, MSG_B, doc_name);
			strcpy((char *)((document_t *)current_node->data)->content, doc_content);
			if (lru_cache_is_full(s->cache) != 1) {
				lru_cache_put(s->cache, doc_name, doc_content, &evicted_key);
				// se actualizeaza log ul
				sprintf(response1->server_log, LOG_MISS, doc_name);
			} else {
				lru_cache_put(s->cache, doc_name, doc_content, &evicted_key);
				// se actualizeaza log ul
				sprintf(response1->server_log, LOG_EVICT, doc_name, (char *)evicted_key);
			}
			free(evicted_key);
		} else {
			/* daca documentul nu se afla in database se adauga si in database
			si in cache */
			void *evicted_key = NULL;
			// se actualizeaza response ul
			sprintf(response1->server_response, MSG_C, doc_name);
			if (lru_cache_is_full(s->cache) != 1) {
				// se actualizeaza log ul
				sprintf(response1->server_log, LOG_MISS, doc_name);
				lru_cache_put(s->cache, doc_name, doc_content, &evicted_key);
				document_t *new_node;
				new_node = malloc(sizeof(document_t));
				if (new_node == NULL) {
					printf("Allocation failed\n");
					free(new_node);
				}
				new_node->name = malloc(DOC_NAME_LENGTH * sizeof(char));
				snprintf(new_node->name, DOC_NAME_LENGTH, "%s", doc_name);
				new_node->content = malloc(DOC_CONTENT_LENGTH * sizeof(char));
				snprintf(new_node->content, DOC_CONTENT_LENGTH, "%s",
					 doc_content);
				// adaugam documentul in database
				dll_add_nth_node(s->database, s->database->size, new_node);
				free(new_node);
			} else {
				lru_cache_put(s->cache, doc_name, doc_content, &evicted_key);
				sprintf(response1->server_log, LOG_EVICT, doc_name, (char *)evicted_key);
				document_t *new_node;
				new_node = malloc(sizeof(document_t));
				if (new_node == NULL) {
					printf("Allocation failed\n");
					free(new_node);
				}
				new_node->name = malloc(DOC_NAME_LENGTH * sizeof(char));
				snprintf(new_node->name, DOC_NAME_LENGTH, "%s", doc_name);
				new_node->content = malloc(DOC_CONTENT_LENGTH * sizeof(char));
				snprintf(new_node->content, DOC_CONTENT_LENGTH, "%s",
					 doc_content);
				// adaugam documentul in database
				dll_add_nth_node(s->database, s->database->size, new_node);
				free(new_node);
			}
			free(evicted_key);
		}
	}
	return response1;
}

// functia server_get_document se ocupa de requesturile de tip GET
static response
*server_get_document(server *s, char *doc_name) {
	unsigned int ok = 0, i;
	dll_node_t *content;
	// obtinem valorea asociata lui doc_name
	content = lru_cache_get(s->cache, doc_name);
	// alocam memorie pentru response
	response *response1;
	response1 = malloc(sizeof(response));
	if (response1 == NULL) {
		printf("Allocation failed\n");
		free(response1);
	}
	response1->server_log = malloc(MAX_LOG_LENGTH * sizeof(char));
	if (response1->server_log == NULL) {
		printf("Allocation failed\n");
		free(response1->server_log);
	}
	response1->server_id = s->id;
	// verificam daca cheia se afla in hashtable
	ok = ht_has_key(s->cache->hashtable, doc_name);
	if (ok == 1) {
		/* daca documentul se afla in cache se actualizeaza log ul si
		response ul este continutul acestuia */
		response1->server_response = malloc(MAX_RESPONSE_LENGTH * sizeof(char));
		strcpy(response1->server_response, (char *)((info*)content->data)->value);
		sprintf(response1->server_log, LOG_HIT, doc_name);
	} else {
		// daca documentul nu se afla in cache se cauta in database
		unsigned int verif = 0;
		dll_node_t *current_node = s->database->head;
		for (i = 0; i < s->database->size; i++) {
			if (strcmp((char *)((document_t *)current_node->data)->name,
				doc_name) == 0) {
				verif = 1;
				break;
			}
			current_node = current_node->next;
		}
		if (verif == 1) {
			/* daca se gaseste documentul in database se aduga in cache si 
			response ul este continutul acestuia*/
			void *evicted_key = NULL;
			response1->server_response = malloc(MAX_RESPONSE_LENGTH * sizeof(char));
			strcpy(response1->server_response,
				   (char *)((document_t*)current_node->data)->content);
			// verificam daca cache ul este plin inainte sa adaugam documentul
			if (lru_cache_is_full(s->cache) != 1) {
				lru_cache_put(s->cache, doc_name, response1->server_response, &evicted_key);
				sprintf(response1->server_log, LOG_MISS, doc_name);
			} else {
				lru_cache_put(s->cache, doc_name, response1->server_response, &evicted_key);
				sprintf(response1->server_log, LOG_EVICT, doc_name, (char *)evicted_key);
			}
			free(evicted_key);
		} else {
			/* daca documentul nu este in database se actualizeaza log ul si
			response ul este NULL */
			response1->server_response = NULL;
			sprintf(response1->server_log, LOG_FAULT, doc_name);
		}
	}
	return response1;
}

/* functia init_server creeaza un server ce contine un cache, coada de 
request uri si database ul */
server *init_server(unsigned int cache_size) {
	server *s;
	s = malloc(sizeof(server));
	s->cache = init_lru_cache(cache_size);
	s->q = q_create(sizeof(request), TASK_QUEUE_SIZE);
	s->database = dll_create(sizeof(document_t));
	return s;
}

/* functia server_handle_request gestioneaza request urile si returneaza un
response corespunzator */
response *server_handle_request(server *s, request *req) {
	response *response1;
	// obtinem tipul request ului
	char *type = get_request_type_str(req->type);
	if (strcmp(type, "EDIT") == 0) {
		// folosim o copie a request ului
		request *req_copy;
		req_copy = malloc(sizeof(request));
		if (req_copy == NULL) {
			printf("Allocation failed\n");
			free(req_copy);
		}
		req_copy->type = req->type;
		req_copy->doc_name = malloc(DOC_NAME_LENGTH * sizeof(char));
		if (req_copy->doc_name == NULL) {
			printf("Allocation failed\n");
			free(req_copy->doc_name);
		}
		strcpy(req_copy->doc_name, req->doc_name);
		req_copy->doc_content = malloc(DOC_CONTENT_LENGTH * sizeof(char));
		if (req_copy->doc_content == NULL) {
			printf("Allocation failed\n");
			free(req_copy->doc_content);
		}
		strcpy(req_copy->doc_content, req->doc_content);
		// adaugam request ul in coada
		q_enqueue(s->q, req_copy);
		// alocam memorie pentru response
		response1 = malloc(sizeof(response));
		if (response1 == NULL) {
			printf("Allocation failed\n");
			free(response1);
		}
		response1->server_log = malloc(MAX_LOG_LENGTH * sizeof(char));
		if (response1->server_log == NULL) {
			printf("Allocation failed\n");
			free(response1->server_log);
		}
		response1->server_response = malloc(MAX_RESPONSE_LENGTH * sizeof(char));
		if (response1->server_response == NULL) {
			printf("Allocation failed\n");
			free(response1->server_response);
		}
		response1->server_id = s->id;
		/* actualizam log ul si response ul ce contin informatii despre starea
		curenta a serverului*/
		sprintf(response1->server_log, LOG_LAZY_EXEC, s->q->size);
		sprintf(response1->server_response, MSG_A, type, req_copy->doc_name);
		// eliberam memoria alocata copiei
		free(req_copy);
	} else if (strcmp(type, "GET") == 0) {
		while (q_is_empty(s->q) != 1) {
			request *aux;
			response *response_aux;
			// obtinem primul request din coada
			aux = q_front(s->q);
			if (aux != NULL) {
				// se execută fiecare operație stocată în coada de task-uri
				response_aux = server_edit_document(s, aux->doc_name, aux->doc_content);
				// printam response ul corespunzator fiecarei operatii
				PRINT_RESPONSE(response_aux);
				free(aux->doc_name);
				if (aux->doc_content)
					free(aux->doc_content);
			}
			// scoatem request ul din coada
			q_dequeue(s->q);
		}
		// apelam functia GET
		response1 = server_get_document(s, req->doc_name);
	}
	return response1;
}

// functia free_server elibereaza memoria alocata pentru un server
void free_server(server **s) {
	// eliberam memoria cache ului
	free_lru_cache(&(*s)->cache);
	// eliberam coada
	q_free((*s)->q);
	// eliberam memoria pentru database ce contine elemente de tip document
	dll_node_t *current_node = ((*s)->database)->head;
	while (current_node != NULL) {
		dll_node_t *aux = current_node;
		current_node = current_node->next;
		document_t_free_function(aux->data);
		free(aux->data);
		free(aux);
	}
	free(((*s)->database));
	// eliberam serverul
	free(*s);
}
