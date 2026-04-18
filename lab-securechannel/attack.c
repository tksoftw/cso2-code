#include "lab.h"
#include <string.h>

/* forward function for "attack 0" case.
   change this code to implement your attack
 */
void forward_attack_0(struct message *message) {
    if (message->from == 'A' && message->to == 'B') {
        const char *payload = "PAY $10001438 TO M";
        strcpy((char*)(message->data), payload);
        message->data_size = strlen(payload);
    }   
    send_message(message);
}
     

/* forward function for "attack 1" case.
   change this code to implement your attack
 */
void forward_attack_1(struct message *message) {
     if (message->from == 'A' && message->to == 'B') {
        struct message *forged = new_message(
            'A',                  
            'B',                  
            "PAY $10001438 TO M", 
            true,               
            false               
        );
        send_message(forged);
    } else {
        send_message(message);
    }
}

/* forward function for "attack 2" case.
   change this code to implement your attack
 */
void forward_attack_2(struct message *message) {
    send_message(message);
    send_message(message);
}

void forward_attack_3(struct message *message) {
    static int s3_count = 0;
    static struct message saved_m2;

    if (message->from == 'A' && message->to == 'B') {
        s3_count++;
        
        if (s3_count == 1) {
            send_message(message); 
        } else if (s3_count == 2) {
            saved_m2 = *message; 
            send_message(message);
        } else if (s3_count == 3) {
            send_message(&saved_m2);
        }
    } else {
        send_message(message);
    }
}

/* forward function for "attack 4" case.
   change this code to implement your attack */
void forward_attack_4(struct message *message) {
    send_message(message);
}

/* forward function for "attack 5" case.
   I did not intend this one to be possible. */
void forward_attack_5(struct message *message) {
    send_message(message);
}
