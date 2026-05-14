#include "lab.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void forward_normal(struct message *message) {
    send_message(message);
}

extern void forward_attack_0(struct message*);
extern void forward_attack_1(struct message*);
extern void forward_attack_2(struct message*);
extern void forward_attack_3(struct message*);
extern void forward_attack_4(struct message*);
extern void forward_attack_5(struct message*);

static forward_func_type attack_functions[] = {
    forward_attack_0,
    forward_attack_1,
    forward_attack_2,
    forward_attack_3,
    forward_attack_4,
    forward_attack_5,
};

static void usage(const char *argv0) {
    fprintf(stderr, 
        "usage: %s SCENARIO-ID normal\n"
        "   or: %s SCENARIO-ID attack\n",
        argv0, argv0);
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
    if (argc <= 2) {
        usage(argv[0]);
    }
    int scenario_id = atoi(argv[1]);
    if (scenario_id < 0 || scenario_id > 5) {
        fprintf(stderr, "ERROR: invalid scenario id %d\n", scenario_id);
        return EXIT_FAILURE;
    }
    forward_func_type forward;
    if (strcmp("normal", argv[2]) == 0) {
        forward = forward_normal;
    } else if (strcmp("attack", argv[2]) == 0) {
        forward = attack_functions[scenario_id];
    } else {
        usage(argv[0]);
    }
    run(scenario_id, forward);
}
