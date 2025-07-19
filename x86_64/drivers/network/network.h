/*
 * HellOS Network Stack Header
 * Network system definitions and structures
 */

#ifndef NETWORK_H
#define NETWORK_H

#include <stdint.h>
#include <stdbool.h>

// Forward declarations
typedef struct socket_s socket_t;
typedef struct network_interface_s network_interface_t;
typedef struct network_stats_s network_stats_t;
typedef struct ip_address_s ip_address_t;

// Network initialization
int init_network_driver(void);
void shutdown_network_driver(void);

// Socket API
int socket_create(int domain, int type, int protocol);
int socket_bind(int socket_id, const char* ip_str, uint16_t port);
int socket_listen(int socket_id, int backlog);
int socket_accept(int socket_id, char* client_ip, uint16_t* client_port);
int socket_connect(int socket_id, const char* ip_str, uint16_t port);
int socket_send(int socket_id, const void* data, size_t length);
int socket_receive(int socket_id, void* buffer, size_t buffer_size);
int socket_close(int socket_id);

// Network utilities
socket_t* find_socket_by_id(int socket_id);
void parse_ip_address(const char* ip_str, ip_address_t* ip);
void process_network_packets(void);
int send_packet(const void* data, size_t length, const ip_address_t* dest_ip, uint16_t dest_port, uint8_t protocol);
uint16_t calculate_ip_checksum(const void* data, size_t length);

// Network configuration
int set_network_config(const char* ip_str, const char* subnet_str, const char* gateway_str);
int ping_host(const char* ip_str);

// Network information
network_stats_t* get_network_stats(void);
network_interface_t* get_network_interface(void);

#endif // NETWORK_H 