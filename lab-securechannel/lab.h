#ifndef LAB_H_
#include <stdbool.h>    // for bool
#include <stdlib.h>     // for size_t

/*
 * represents a (possibly encrypted, possibly signed) message being sent over the network
 */
struct message {
    char from;              // message sender
    char to;                // message recipient
    bool is_encrypted;      // if true, 'data' is encrypted using to's public key
    bool is_signed;         // if true, 'data' is signed using from's private key
    unsigned char data[1024];        
    size_t data_size;
    unsigned char signature[64];     // only used if is_signed is true
};

void send_message(struct message *message);

struct message *new_message(
    char from,
    char to,
    const char *text,
    bool is_encrypted,
    bool is_signed
);

void decrypt_message_for_M(
    const struct message *message,
    char *buffer,
    size_t buffer_size
);

void display_message(struct message *message);

typedef void (*forward_func_type)(struct message*);

void run(int scenario, forward_func_type);

#endif  // LAB_H_
