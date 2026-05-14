#include <stdio.h>
#include "netsim.h"

struct AppState {
	char target_msg_num;
	int expected_seq;
	int total_seq;
	int timeout_id;
};

struct AppState state = { 
	.target_msg_num = 0,
	.expected_seq = 1,
	.total_seq = -1,
	.timeout_id = -1
};

void timeout_handler(void *arg);

char calc_checksum(char *data, size_t len) {
	char sum = 0;
	for (size_t i = 1; i < len; i++) {
		sum ^= data[i];
	}
	return sum;
}
// Helper: Construct and send the GET message
void send_get() {
    char data[5];
    data[1] = 'G'; 
    data[2] = 'E'; 
    data[3] = 'T'; 
    data[4] = state.target_msg_num; // Accessing struct
    data[0] = calc_checksum(data, 5); 
    
    send(5, data);
    
    if (state.timeout_id != -1) clearTimeout(state.timeout_id);
    state.timeout_id = setTimeout(timeout_handler, 1000, NULL);
}

// Helper: Construct and send an ACK
void send_ack(char seq) {
    char data[5];
    data[1] = 'A'; 
    data[2] = 'C'; 
    data[3] = 'K'; 
    data[4] = seq;
    data[0] = calc_checksum(data, 5);
    
    send(5, data);
    
    if (state.timeout_id != -1) {
        clearTimeout(state.timeout_id);
        state.timeout_id = -1;
    }
    
    // Only schedule next timeout if we aren't done yet
    if (state.total_seq == -1 || state.expected_seq <= state.total_seq) {
        state.timeout_id = setTimeout(timeout_handler, 1000, NULL);
    }
}

void timeout_handler(void *arg) {
	state.timeout_id = -1;

	if (state.expected_seq == 1) {
		send_get();
	} else {
		send_ack(state.expected_seq - 1);
	}
}

void recvd(size_t len, void* _data) {
    // FIX ME -- add proper handling of messages
    char *data = _data;
    
	// fail checksum
	char sent_checksum = data[0];	
	char real_checksum = calc_checksum(data, len); 
	if (sent_checksum != real_checksum) {
		return;
	}

	
	char seq_num = data[1];
    char packet_total = data[2];	
	if (seq_num == state.expected_seq) { 
		state.total_seq = packet_total;
		
		fwrite(data + 3, 1, len - 3, stdout);
		fflush(stdout);
		
		state.expected_seq++;
		send_ack(seq_num);
	} else if (seq_num < state.expected_seq) {
		send_ack(state.expected_seq - 1);
	}

}

int main(int argc, char *argv[]) {
    // this code should work without modification
    if (argc != 2) {
        fprintf(stderr, "USAGE: %s n\n    where n is a number between 0 and 3\n", argv[0]);
        return 1;
    }
    
	state.target_msg_num = argv[1][0];
	
	send_get();
    waitForAllTimeoutsAndMessagesThenExit();
}
