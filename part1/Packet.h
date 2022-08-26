#include <string.h>
#include<stdio.h>

#define START 0xFFFF
#define END 0xFFFF
#define DATA 0xFFF1
#define ACK 0xFFF2
#define REJECT 0xFFF3
#define OUT_OF_SEQUENCE 0xFFF4
#define LENGTH_MISMATCH 0xFFF5
#define END_OF_PACKET_MISSING 0xFFF6
#define DUPLICATE_PACKET  0xFFF7

//DATA packet format
struct data_packet {
    unsigned short start_id;  //2 bits
    char client;  //Client ID, i bit
    unsigned short type;  //==DATA
    char segment;  //Segment number
    char length;
    char payload[255];
    unsigned short end_id;
};

//ACK packet format
struct ack_packet {
    unsigned short start_id;
    char client;
    unsigned short type;  //==ACK
    char segment;
    unsigned short end_id;
};

//REJECT packet format
struct reject_packet {
    unsigned short start_id;
    char client;
    unsigned short type;  //==REJECT
    unsigned short sub_code;
    char segment;
    unsigned short end_id;
};

//Build buffer to send data of DATA packet, return length of buffer.
int DATA_Buffer(struct data_packet data, char* buffer) {

    //Buffer's current length. Also used as index when increasing storage to store data.
    int length = 0;

    //Copy bytes to store corresponding data of the packet.
    memcpy(&buffer[length], &data.start_id, 2);
    length += 2;

    buffer[length] = data.client;
    length += 1;

    memcpy(&buffer[length], &data.type, 2);
    length += 2;

    buffer[length] = data.segment;
    length += 1;

    buffer[length] = data.length;
    length += 1;

    memcpy(&buffer[length], &data.payload, strlen(data.payload));
    length += strlen(data.payload);

    memcpy(&buffer[length], &data.end_id, 2);
    length += 2;

    buffer[length + 1] = '\0';

    return length;
}

//Build buffer to send data of ACK packet.
int ACK_Buffer(struct ack_packet ack, char* buffer) {

    int length = 0;

    memcpy(&buffer[length], &ack.start_id, 2);
    length += 2;

    buffer[length] = ack.client;
    length += 1;

    memcpy(&buffer[length], &ack.type, 2);
    length += 2;

    buffer[length] = ack.segment;
    length += 1;

    memcpy(&buffer[length], &ack.end_id, 2);
    length += 2;

    buffer[length + 1] = '\0';

    return length;
}

//Build buffer to send data of REJECT packet.
int REJECT_Buffer(struct reject_packet reject, char* buffer) {

    int length = 0;

    memcpy(&buffer[length], &reject.start_id, 2);
    length += 2;

    buffer[length] = reject.client;
    length += 1;

    memcpy(&buffer[length], &reject.type, 2);
    length += 2;

    memcpy(&buffer[length], &reject.sub_code, 2);
    length += 2;

    buffer[length] = reject.segment;
    length += 1;

    memcpy(&buffer[length], &reject.end_id, 2);
    length += 2;

    buffer[length + 1] = '\0';

    return length;
}

//Extract DATA packet from buffer to DATA packet
int DATA_Extract(struct data_packet* data, char* buffer) {

    int length = 0;

    memcpy(&(data->start_id), buffer + length, 2);
    length += 2;

    data->client = buffer[length];
    length += 1;

    memcpy(&(data->type), buffer + length, 2);
    length += 2;

    data->segment = buffer[length];
    length += 1;

    data->length = buffer[length];
    length += 1;

    memcpy(&(data->payload), buffer + length, 7);
    length += 7;

    unsigned short end_packet;
    memcpy(&end_packet, buffer + length, 2);

    //Valid end of packet ID.
    if(end_packet == END) {
        memcpy(&(data->end_id), &end_packet, 2);
        return 0;

    //Invalid
    } else {
        return END_OF_PACKET_MISSING;
    }

    return 0;
}

