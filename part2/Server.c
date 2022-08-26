#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/time.h>
#include "packet.h"

void Database_load(struct db_detail* info);

int main() {
    int Server;
    struct id_packet id;
    char buffer[1024];
    struct sockaddr_in ServerAddr;
    memset(buffer, '\0', sizeof(buffer));

    Server = socket(AF_INET, SOCK_DGRAM, 0);

    //Configure server's address.
    memset(&ServerAddr, 0, sizeof(ServerAddr));
    ServerAddr.sin_family = AF_INET;
    ServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    ServerAddr.sin_port = htons(7891);
    printf("Server created.\n");

    //Bind server's address to its socket.
    bind(Server, (struct sockaddr*)&ServerAddr, sizeof(ServerAddr));

    struct sockaddr sender;  //Information sender(client).
    socklen_t sendersize = sizeof(sender);
    memset(&sender, 0, sizeof(sender));

    //set database information struct
    struct db_detail info[3];
    //load all the data from file
    Database_load(info);
    printf("Database loaded.\n");

    printf("Start data transmission....\n");
    //Transmission
    int counter = 1;
    while (1) {
        recvfrom(Server, buffer, sizeof(buffer), 0, &sender, &sendersize);
        printf("Packet %d received from client.\n", counter);

        //Generate and print information based on received packet.
        ID_Extract(&id, buffer); //Extract recived packet from buffer
        int i = 0;
        int found = 0;
        while (i < 3) {
            if (id.sub == info[i].sub && id.tech == info[i].tech) {
                //No error. Access permitted
                if (info[i].paid == 1){
                    id.status = ACCESS_OK;
                    found = 1;
                //Not paid. REJECT
                } else {
                    id.status = UNPAID;
                    found = 1;
                }
                break;
            }
            //Not exist - technology not match
            else if (id.sub == info[i].sub && id.tech != info[i].tech){
                id.status = NOT_EXIST;
            	found = 1;
            	break;
            }
            i++;
        }
        //Not exist - subscriber not found
        if(found == 0) {
            id.status = NOT_EXIST;
        }
        if(id.status == UNPAID) {
            printf("Not paid. REJECT\n");
        } else if(id.status == NOT_EXIST) {
            printf("Subscriber not exist. REJECT\n");
        } else if(id.status == ACCESS_OK) {
            printf("No error. Access permitted\n");
        }
        counter++;
        int len = ID_Buffer(id, buffer);

        //Send ACK or REJECT packet to client.
        sendto(Server, buffer, len, 0, (struct sockaddr*)&sender, sendersize);
        printf("\n\n");

        //One transmission round ends(5 packet has been transmitted to server).
        //Do further operation based on user's input, continue transmitting or terminate server.
        if(counter % 4 == 1) {
            printf("End of one transmission round.\n");
            printf("Press 0 to continue transmitting.\n");
            printf("Press -1 to terminate server.\n");
            printf("Your input is: \n");
            int input;
            scanf("%d", &input);
            if(input == -1) {
                printf("Server terminated.\n");
                return 0;
            }
            else if(input == 0){
                printf("Continue transmitting...\n\n");
                continue;
            }
        }
    }
    return 0;
}

//Load information of ID packets from database file
void Database_load(struct db_detail* info) {

    char sub[100];
    char tech[10];
    int paid;

    FILE* fp =NULL;
    fp = fopen("./Verification_Database.txt", "r");


    char buffer[100];
    fgets(buffer, 100, fp);  //Skip the first line as it is title.

    //Read data and load it to corresponding variable.
    int i = 0;
    while (i < 3) {
        char tmp[11];
        fscanf(fp, "%s %s %d\n", sub, tech, &paid);

        strncpy(tmp, sub, 3);
        strncpy(tmp + 3, sub + 4, 3);
        strncpy(tmp + 6, sub + 8, 4);

        info[i].sub = atoi(tmp);
        info[i].tech = atoi(tech);
        info[i].paid = paid;
        i++;
    }
    fclose(fp);
}
