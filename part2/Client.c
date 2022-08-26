#include <stdio.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/time.h>
#include "packet.h"

void ID_Build(int Client, struct sockaddr_in ServerAddr, int tech, int sub, int segment);
int pkt_receive(int Client, struct sockaddr_in ServerAddr, char* buffer, int len);
void Client_run(int Client, struct sockaddr_in ServerAddr, int seg);


int main() {

    int Client;
    Client = socket(AF_INET, SOCK_DGRAM, 0);
    printf("Client created.\n");

    //Configure server's address.
    struct sockaddr_in ServerAddr;
    memset(&ServerAddr, 0, sizeof(ServerAddr));
    ServerAddr.sin_family = AF_INET;
    ServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    ServerAddr.sin_port = htons(7891);

    //Set timer.
    struct timeval count = { 3, 0 };
    if (setsockopt(Client, SOL_SOCKET, SO_RCVTIMEO, &count, sizeof(count)) < 0) {
        printf("ERROR!\n");
    }

    printf("Start data transmission....\n\n");

    //Transmission. First round need no guide from user because there is only 1 transmission mode of client.
    int seg = 1;
    Client_run(Client, ServerAddr, seg);
    while(1) {
        seg += 4;
        printf("End of one transmission round.\n");
        printf("Press 0 to continue transmitting.\n");
        printf("Press -1 to terminate client.\n");
        printf("Your input is: \n");
        int input;
        scanf("%d", &input);
        if(input == -1) {
            printf("Client terminated.\n");
            return 0;
        }
        else if(input == 0){
            printf("Continue transmitting...\n\n");
            Client_run(Client, ServerAddr, seg);
        }
    }

}

//Build 1 correct packet and 3 incorrect packet.
 void Client_run(int Client, struct sockaddr_in ServerAddr, int seg) {

    //Correct packet
    ID_Build(Client, ServerAddr, 2, 4086808821, seg);

    //Not paid
    ID_Build(Client, ServerAddr, 3, 4086668821, seg+1);

    //Not exist - subscriber not found
    ID_Build(Client, ServerAddr, 2, 1042349876, seg+2);

    //Not exist - technology not match
    ID_Build(Client, ServerAddr, 2, 4085546805, seg+3);
 }

 //Build ID packet, send it to server and listen to response.
void ID_Build(int Client, struct sockaddr_in ServerAddr, int tech, int sub, int segment) {
    struct id_packet id;
    char buffer[1024];
    memset(buffer, '\0', sizeof(buffer));                     //clear cache

    id.start_id = START;
    id.client = 24;
    id.status = ACC_PER;
    id.segment = segment;
    id.length = 5;
    id.tech = tech;
    id.sub = sub;
    id.end_id = END;

    int len = ID_Buffer(id, buffer);
    sendto(Client, buffer, len, 0, (struct sockaddr*)&ServerAddr, sizeof(ServerAddr));
    printf("Packet %d sent to server.\n", segment);
    int flag = pkt_receive(Client, ServerAddr, buffer, len);
    if(flag == 1) {
        flag = pkt_receive(Client, ServerAddr, buffer, len);
    }

    printf("\n\n");
}

//Receive ACK or REJECT packet from server
int pkt_receive(int Client, struct sockaddr_in ServerAddr, char* buffer, int len) {

    struct id_packet msg;
    char msg_buffer[1024];
    memset(msg_buffer, '\0', sizeof(msg_buffer));

    struct sockaddr remote;
    socklen_t remote_len = sizeof(remote);
    memset(&remote, 0, sizeof(remote));
    bind(Client, (struct sockaddr*)&remote, remote_len);

    int counter = 0;
    int flag = 0;
    while (counter <= 3) {
        int recv = recvfrom(Client, msg_buffer, sizeof(msg_buffer), 0, &remote, &remote_len);

        //Packet Received
        if (recv >= 0) {
            printf("Response from server:\n");
            ID_Extract(&msg, msg_buffer);
            if(msg.status == UNPAID) {
                printf("Not paid. REJECT\n");
            } else if(msg.status == NOT_EXIST) {
                printf("Subscriber not exist. REJECT\n");
            } else if(msg.status == ACCESS_OK) {
                printf("No error. Access permitted\n");
            }
            break;
        }

        //Not received.
        else {
            if (counter == 3) {
                printf("Server does not respond.\n");
                flag = 1;
                break;
            }
            counter++;
            printf("Retransmitting. Attempt: %d\n", counter);
        }            //sendto(Client, buffer, len, 0, (struct sockaddr*)&ServerAddr, sizeof(ServerAddr));

    }
    return flag;
}
