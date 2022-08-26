#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/time.h>
#include <errno.h>
#include "packet.h"

void Client_run(int input, int packet_num, struct data_packet data, int seg_num, int Client, struct sockaddr_in ServerAddr);
void DATA_Build(struct data_packet* data, char seg_num, char length, char payload_len, unsigned short end_id);
int pkt_receive(int Client, struct sockaddr_in ServerAddr, char* req_buffer, int pkt_len);

int main() {

    int Client;
    struct sockaddr_in ServerAddr;  //Server's address to transmit data
    struct data_packet data;
    int seg_num = 1;  //segment number

    //Create client
    Client = socket(AF_INET, SOCK_DGRAM, 0);
    printf("Client created.\n");

    //Configure server's address.
    memset( &ServerAddr, 0, sizeof(ServerAddr) );  //clean cache
    ServerAddr.sin_family = AF_INET;  //ipv4
    ServerAddr.sin_port = htons(7891);  //Port 7891
    ServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);  //Any Address

    //Set timer
    struct timeval timeout = {3, 0};
    if (setsockopt(Client, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        perror("Error");
    }

    //First operation
    int input;
    printf("Press 0 to send 5 correct packets\n");
    printf("Press 1 to send 1 correct packet and 4 wrong packets\n");
    printf("Your input is:\n");
    scanf("%d", &input);  //Receive user's input

    printf("Start data transmission....\n\n");

    int pkt_type = 1;  //Type of packet

    //Transmission
    while(1) {
        int flag;

        //One transmission round ends(5 packet has been transmitted to server).
        //Do further operation based on user's input, continue transmitting or terminate client.
        if(pkt_type == 6){
            pkt_type = 1;  //Rest packet number
            printf("End of one transmission round.\n");
            printf("Press 0 to continue transmission and send 5 correct packets\n");
            printf("Press 1 to continue transmission and send 1 correct packet and 4 wrong packets\n");
            printf("Press -1 to terminate client.\n");
            printf("Your input is: \n");
            int input;
            scanf("%d", &input);
            if(input == -1) {
                printf("\nClient terminated.\n");
                return 0;
            }
            else if(input == 0 || input == 1){
                flag = input;
                printf("\nContinue transmitting...\n\n");
            }
        }
        //printf("%d", seg_num);
        Client_run(flag, pkt_type, data, seg_num, Client, ServerAddr);
        pkt_type++;
        seg_num++;
    }

    return 0;
}

//Build data packet, send it to server and listen to response.
void Client_run(int input, int pkt_type, struct data_packet data, int seg_num, int Client, struct sockaddr_in ServerAddr) {

    char buffer[1024];
    //Build correct data packet regardless of pkt_type.
    if(input == 0) {
        DATA_Build(&data, seg_num, 7, 7, END);

    //Build correct or incorrect data packet based on pkt_type.
    } else if(input == 1) {

        //Correct packet
        if(pkt_type == 1) {
            DATA_Build(&data, seg_num, 7, 7, END);
        }

        //Packet with wrong length
        else if(pkt_type == 2) {
            DATA_Build(&data, seg_num, 8, 7, END);
        }

        //Packet with no end ID
        else if(pkt_type == 3) {
            DATA_Build(&data, seg_num, 7, 7, '\0');
        }

        //Duplicate packet
        else if(pkt_type == 4) {
            DATA_Build(&data, seg_num - 1, 7, 7, END);
        }

        //Packet with wrong sequence number
        else if(pkt_type == 5) {
            DATA_Build(&data, seg_num + 10, 7, 7, END);
        }
    }

    int packet_len = DATA_Buffer(data, buffer);

    //Send data packet to server.
    sendto(Client, buffer, packet_len, 0, (struct sockaddr*)&ServerAddr, sizeof ServerAddr);
    printf("Packet %d Sent to server.\n", seg_num);

    //Receive response from server.
    int flag = pkt_receive(Client, ServerAddr, buffer, packet_len);
    if (flag == 1) {
        flag = pkt_receive(Client, ServerAddr, buffer, packet_len);
    }
    printf("\n\n");
}

//Build data packet.
void DATA_Build(struct data_packet* data, char seg_num, char length, char payload_len, unsigned short end_id) {

    data->start_id = START;
    data->client = 24;
    data->type = DATA;
    data->segment = seg_num;
    data->length = length;
    memcpy(data->payload, "Correct", strlen("Correct") + 1);
    payload_len = strlen(data->payload);
    data->end_id = end_id;
}

//Receive ACK or REJECT packet from server
int pkt_receive(int Client, struct sockaddr_in ServerAddr, char* req_buffer, int pkt_len) {

    char ack_buffer[1024];
    struct sockaddr sender;  //Information sender(server).
    socklen_t sendsize = sizeof(sender);
    memset(&sender, 0, sizeof(sender));

    //Bind server's address to client.
    bind(Client, (struct sockaddr *) &sender, sendsize);

    int retry_counter = 0;

    int flag = 0;

    while (retry_counter <= 3) {
        int receive_len = recvfrom(Client, ack_buffer, sizeof(ack_buffer), 0, &sender, &sendsize);

        //Packet Received
        if(receive_len >= 0) {
            unsigned short type;
            memcpy(&type, ack_buffer + 3, 2);

            printf("Response from server:\n");

            //ACK packet
            if(type == ACK) {
                printf("ACK \n");

            //REJECT packet
            } else if(type == REJECT) {
                unsigned short code;
                memcpy(&code, ack_buffer + 5, 2);
                if(code == OUT_OF_SEQUENCE)
                    printf("Error: Packet out of sequence. REJECT\n");
                else if(code == LENGTH_MISMATCH)
                    printf("Error: Length mismatch. REJECT\n");
                else if(code == END_OF_PACKET_MISSING)
                    printf("Error: End of packet missing. REJECT\n");
                else if(code == DUPLICATE_PACKET)
                    printf("Error: Duplicate packet. REJECT\n");
            }
            break;

        //Not received.
        } else {
            //Time out. No response..
            if(retry_counter == 3) {
                printf("Server does not respond.\n");
                flag = 1;
                break;
            }
            //Retransmit.
            retry_counter++;
            printf("Retransmitting. Attempt no. %d\n", retry_counter);
            //sendto(Client, req_buffer, pkt_len, 0, (struct sockaddr*)&ServerAddr, sizeof ServerAddr);
        }
    }
    return flag;
}
