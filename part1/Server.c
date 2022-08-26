#include <stdio.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include "packet.h"

void REJECT_Build(char segment, struct reject_packet* reject, int sub_code);

int main() {
    int Server;
    char buffer[1024];
    struct sockaddr_in ServerAddr;  //Server's address to transmit data
    struct data_packet data;
    struct ack_packet ack;
    struct reject_packet reject;

    Server = socket(AF_INET, SOCK_DGRAM, 0);

    //Configure server's address.
    memset( &ServerAddr, 0, sizeof(ServerAddr) );
    ServerAddr.sin_family = AF_INET;
    ServerAddr.sin_port = htons(7891);
    ServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    printf("Server created.\n");

    //Bind server's address to its socket.
    bind(Server, (struct sockaddr *) &ServerAddr, sizeof(ServerAddr));

    struct sockaddr sender;  //Information sender(client).
    socklen_t sendsize = sizeof(sender);
    memset(&sender, 0, sizeof(sender));

    printf("Start data transmission....\n\n");

    //Transmission
    int counter = 0;  //Number of transmitted packets.
    int last = 0;  //last packet's segment number, is used to check duplicate packets.
    while (1) {
        int length = 0;
        recvfrom(Server, buffer, sizeof(buffer), 0, &sender, &sendsize);
        counter++;


        //Generate and print information based on received packet.
        int response = DATA_Extract(&data, buffer);  //Extract recived packet from buffer

        printf("Packet %d received from client.\n", counter);

        if(response == 0) {
            //Error: Packet out of sequence. REJECT
            if(data.segment > counter) {
                REJECT_Build(data.segment, &reject, OUT_OF_SEQUENCE);
                length = REJECT_Buffer(reject, buffer);
                printf("Error: Packet out of sequence. REJECT\n");
            }
            //Error: Length Mismatch. REJECT
            else if(data.length != strlen(data.payload)) {
                REJECT_Build(data.segment, &reject, LENGTH_MISMATCH);
                length = REJECT_Buffer(reject, buffer);
                printf("Error: Length mismatch. REJECT\n");
            }
            //Error: Duplicate Packet. REJECT
            else if(data.segment == last) {
                REJECT_Build(data.segment, &reject, DUPLICATE_PACKET);
                length = REJECT_Buffer(reject, buffer);
                printf("Error: Duplicate packet. REJECT\n");
            }
            //No error. ACK
            else {
                ack.start_id = START;
                ack.client = 24;
                ack.type = ACK;
                ack.segment = data.segment;
                ack.end_id = END;

                length = ACK_Buffer(ack, buffer);
                printf("No error. ACK\n");
            }
        //Error: End of packet missing. REJECT
        } else if(response == END_OF_PACKET_MISSING) {
            REJECT_Build(data.segment, &reject, END_OF_PACKET_MISSING);
            length = REJECT_Buffer(reject, buffer);
            printf("Error: End of packet missing. REJECT\n");
        }

        last = data.segment;

        //Send ACK or REJECT packet to client.
        sendto(Server, buffer, length, 0, (struct sockaddr*)&sender, sendsize);

        printf("\n\n");

        //One transmission round ends(5 packet has been transmitted to server).
        //Do further operation based on user's input, continue transmitting or terminate server.
        if(counter % 5 == 0) {
            printf("End of one transmission round.\n");
            printf("Press 0 to continue transmitting.\n");
            printf("Press -1 to terminate server.\n");
            printf("Your input is: \n");
            int input;
            scanf("%d", &input);
            if(input == -1) {
                printf("\nServer terminated.\n");
                return 0;
            }
            else if(input == 0){
                printf("\nContinue transmitting...\n\n");
                continue;
            }
        }

    }
    return 0;
}

//Build reject packet based on received data packet segment number.
void REJECT_Build(char segment, struct reject_packet* reject, int sub_code) {

    reject->start_id = START;
    reject->client = 24;
    reject->type = REJECT;
    reject->sub_code = END_OF_PACKET_MISSING;
    reject->segment = segment;
    reject->end_id = END;
    reject->sub_code = sub_code;

    printf("REJECT CODE - %x\n", reject->sub_code);
}
