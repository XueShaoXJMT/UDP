#include <string.h>
#include <stdio.h>

#define START 0xFFFF
#define END 0xFFFF
#define ACC_PER 0xFFF8
#define UNPAID 0xFFF9
#define NOT_EXIST 0xFFFA
#define ACCESS_OK 0xFFFB

//ID packet format
struct id_packet {
	unsigned short start_id;  //2 bits
	char client;  //client id
	unsigned short status;  //= ACC_PER | UNPAID | NOT_EXIST | ACCESS_OK
	char segment;  //Segment number
	char length;
	char tech;  //Technology
	unsigned int sub;  //Source subscriber number
	unsigned short end_id;
};

//Verification Database Format
struct db_detail {
    unsigned int sub;  //Subscriber number
    char tech;  //Technology
    char paid;
};

//Build buffer to send data of ID packet, return length of buffer.
int ID_Buffer(struct id_packet id, char* buffer) {

    int len = 0;

    memcpy(&buffer[len], &id.start_id, 2);
    len += 2;

    buffer[len] = id.client;
    len += 1;

    memcpy(&buffer[len], &id.status, 2);
    len += 2;

    buffer[len] = id.segment;
    len += 1;

    buffer[len] = id.length;
    len += 1;

    buffer[len] = id.tech;
    len += 1;

    memcpy(&buffer[len], &id.sub, 4);
    len += 4;

    memcpy(&buffer[len], &id.end_id, 2);
    len += 2;

    buffer[len + 1] = '\0';

    return len;
}

//Extract data from buffer to ID packet
int ID_Extract(struct id_packet* id, char* buffer) {

    int len = 0;

    memcpy(&(id->start_id), buffer + len, 2);
    len += 2;

    id->client = buffer[len];
    len += 1;

    memcpy(&(id->status), buffer + len, 2);
    len += 2;

    id->segment = buffer[len];
    len += 1;

    id->length = buffer[len];
    len += 1;

    id->tech = buffer[len];
    len += 1;

    memcpy(&(id->sub), buffer + len, 4);
    len += 4;

    memcpy(&(id->end_id), buffer + len, 2);

    return 0;
}
