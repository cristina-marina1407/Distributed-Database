/*
 * Copyright (c) 2024, <>
 */

#include "load_balancer.h"
#include "server.h"

load_balancer *init_load_balancer(bool enable_vnodes) {
	load_balancer *main = malloc(sizeof(load_balancer));
    main->hash_function_servers = hash_uint;
    main->hash_function_docs = hash_string;
    main->enable_vnodes = enable_vnodes;
    return main;
}

void loader_add_server(load_balancer* main, int server_id, int cache_size) {
    main->test_server = init_server(cache_size);
    main->test_server->id = server_id;
}

response *loader_forward_request(load_balancer* main, request *req) {
    response *resp = server_handle_request(main->test_server, req);
    return resp;
}

void free_load_balancer(load_balancer** main) {
    free_server(&(*main)->test_server);
    free(*main);

    *main = NULL;
}


