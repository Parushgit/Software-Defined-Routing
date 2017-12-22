#include<stdio.h>
#include<stdlib.h>
#include<iostream>
#include <sstream>
#include <vector>

#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <sys/queue.h>
#include <unistd.h>
#include <string.h>
#include <inttypes.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <time.h>
#include <ctime>
#include <sys/time.h>
#include <fstream>
#include <string.h>
#include <sys/stat.h>

typedef unsigned long long timestamp_t;

//https://stackoverflow.com/questions/1861294/how-to-calculate-execution-time-of-a-code-snippet-in-c
static timestamp_t
get_timestamp ()
{
	struct timeval now;
	gettimeofday (&now, NULL);
	return  now.tv_usec + (timestamp_t)now.tv_sec * 1000000;
}

#define AUTHOR_STATEMENT "I, parushga, have read and understood the course academic integrity policy."
#define CNTRL_CONTROL_CODE_OFFSET 0x04
#define CNTRL_PAYLOAD_LEN_OFFSET 0x06

#define CONTROL_HANDLER_LIB_H_

#define CNTRL_HEADER_SIZE 8
#define CNTRL_RESP_HEADER_SIZE 8
using namespace std;
//#define PACKET_USING_STRUCT 
//#ifdef PACKET_USING_STRUCT

struct __attribute__((__packed__)) CONTROL_HEADER
{
	uint32_t dest_ip_addr;
	uint8_t control_code;
	uint8_t response_time;
	uint16_t payload_len;
};

struct __attribute__((__packed__)) RECV_HEADER
{
	uint32_t dest_ip;
    uint8_t transfer_id;
    uint8_t ttl;
    uint16_t seq_no;
    uint16_t fin;
    uint16_t pad;
};

struct __attribute__((__packed__)) CONTROL_RESPONSE_HEADER
{
	uint32_t controller_ip_addr;
	uint8_t control_code;
	uint8_t response_code;
	uint16_t payload_len;
};

struct __attribute__((__packed__)) router_details
{
	uint16_t router_id;
	uint16_t router_port;
	uint16_t data_port;
	uint16_t link_cost;
	uint32_t router_ip;
};

struct __attribute__((__packed__)) packet_structure
{
	uint16_t num_routers;
	uint16_t interval;
	router_details router_array[5];
};

struct __attribute__((__packed__)) update_router_details
{
	uint32_t r_ip;
	uint16_t r_port;
	uint16_t pad;
	uint16_t r_id;
	uint16_t cost;
};

struct __attribute__((__packed__)) routing_update_structure
{
	uint16_t num_updates;
	uint16_t source_port;
	uint32_t source_ip;
	struct update_router_details router_update_array[5];
};

struct __attribute__((__packed__)) routing_table
{
	uint16_t r_id;
	uint16_t pad;
	uint16_t next_hop;
	uint16_t cost;
};

struct __attribute__((__packed__)) sendFilePacketStructure
{
    uint32_t dest_ip;
    uint8_t transfer_id;
    uint8_t ttl;
    uint16_t seq_no;
	uint16_t fin;
    uint16_t pad;
    char *payload;
};

struct __attribute__((__packed__)) sendFileControlPayload
{
	uint32_t dest_ip;
	uint8_t ttl;
	uint8_t transfer_id;
	uint16_t seq_no;
};

struct __attribute__((__packed__)) statistics
{
    uint8_t transfer;
    uint8_t timetolive;
    uint16_t seqnum;
};
 
struct __attribute__((__packed__)) fillStats
{
    uint8_t transfer;
    uint8_t timetolive;
    uint16_t padding;
    uint16_t *seqnum;
};
 
vector<statistics> mystats;

struct node
{
	unsigned dist[20];
	unsigned from[20];

}rt[10];

static struct timeval tout;
struct timeval test_time;

struct router_neighbour
{
  uint16_t router_port;
  uint32_t router_ip;
  long int cur_time;
  int remainingCount;
  bool updateReceived;
};
int lastVal;
int numberofintervals = -1;
int recvcounter = 0;
int stats_payload_len;
int indexDestination;
int send_socket;
int indexNextHop;
uint16_t dataPortNextHop;
uint32_t destIPNextHop;
char *sendbuffer;
const char* fr_name;
char *payload;
int size;


vector <router_neighbour> router_time_vector;

struct packet_structure* snap_pkt;
struct sendFileControlPayload* send_pkt;

vector<router_neighbour> myNeighbours;
vector<int> myNeighbourHopList;
uint32_t cur_router_ip;
bool firstPacket_recv = true;
bool firstPacket_send = true;
char f_name[5];
const char *finame;

bool firstTime = true;
int countActiveRouters = 0;
string f_append;

int vectorMatrix[5][5];
int copy_vectorMatrix[5][5];
int *cost_vector;
char INF1;
char INF2;
int counter = 0;
char *last_packet_router;
char *penultimate_last_packet_router;
//#endif

/**
 * @parushga_assignment3
 * @author  PARUSH GARG <parushga@buffalo.edu>
 * @version 1.0
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * http://www.gnu.org/copyleft/gpl.html
 *
 * @section DESCRIPTION
 *
 * This contains the main function. Add further description here....
 */
 //Variables used in the program
 fd_set master_list, watch_list;
 int sock, control_socket, head_fd, router_socket, data_socket, num_routers, periodic_interval, data_port, connect_socket;
 uint16_t control_port, router_port, cur_router_index = 65534;


  /* Linked List for active data connections */
struct DataConn
{
    int sockfd;
    LIST_ENTRY(DataConn) next;
}*connection, *conn_temp;

LIST_HEAD(DataConnsHead, DataConn) data_conn_list;

int new_data_conn(int sock_index)
{
     //cout<<"New data connection"<<endl;
     int fdaccept;
     socklen_t caddr_len;
     struct sockaddr_in remote_controller_addr;
 
     caddr_len = sizeof(remote_controller_addr);
     fdaccept = accept(sock_index, (struct sockaddr *)&remote_controller_addr, &caddr_len);
     if(fdaccept < 0)
		 perror("accept() failed");
		 
	 //Insert into list of active control connections 
	 connection = (struct DataConn*)malloc(sizeof(struct DataConn));
	 connection->sockfd = fdaccept;
	 LIST_INSERT_HEAD(&data_conn_list, connection, next);
 
     return fdaccept;
 }


 int new_control_conn(int sock_index)
 {
	// //cout<<"New control connection"<<endl;
	 int fdaccept;
	 socklen_t caddr_len;
	 struct sockaddr_in remote_controller_addr;
 
	 caddr_len = sizeof(remote_controller_addr);
	 fdaccept = accept(sock_index, (struct sockaddr *)&remote_controller_addr, &caddr_len);
	 if(fdaccept < 0)
		 perror("accept() failed");
 
	 return fdaccept;
 }

 bool isData(int sock_index)
 {
	//cout<<"Is Data"<<endl;
	 LIST_FOREACH(connection, &data_conn_list, next)
		 if(connection->sockfd == sock_index) return true;
	 return false;
 }

 int getMaxValueInList()
 {
	 int max = 0;
	 for(int i = 0; i<=head_fd; i++)
	 {
		 if(FD_ISSET(i, &master_list))
		 {
			 if(i >= max)
				 max = i;
		 }
	 }
	 return max;
 }

 bool isAnyChangeInRow(int index)
 {
	 for(int i = 0 ; i<num_routers; i ++)
	 {
		 if(copy_vectorMatrix[index][i] != vectorMatrix[index][i])
		 {
			 //cout<<"Change for index: "<<index<<endl;
			 return true;
		 }
	 }
	 //cout<<"No Change for index: "<<index<<endl;
	 return false;
 }

void initialiseMatrix(uint16_t routers)
{
	// //cout<<"######################################"<<endl;
	// //cout<<"Initialising vector matrix"<<endl;
	for(int i = 0; i < num_routers; i++)
	{
		vectorMatrix[i][i]= 0;
			for(int j = 0; j < num_routers; j++)
			{
					if( i != j)	
					{
						vectorMatrix[i][j] = 65535;
					}
			}
		//}
	}

	//cout<<"Prnt matrix: "<<endl;

	for(int i = 0; i < num_routers; i++)
	{
		vectorMatrix[i][i]= 0;
			for(int j = 0; j < num_routers; j++)
			{
					if( i != j)	
					{
						vectorMatrix[i][j] = 65535;
					}
			//cout<<vectorMatrix[i][j]<<" ";
			}
			//cout<<endl;
		//}
	}

	firstTime = false;
	// //cout<<"######################################"<<endl;
}

/*
Implementing Bellman Ford Algorithm for distance vector routing:-
*/
void bellmanFordAlgorithm()
{
	//cout<<"######################################"<<endl;
	//cout<<"Applying Bellman Ford Algorithm"<<endl;

		//initialiseMatrix(num_routers);
		int i,j;
		
		for(i = 0; i<num_routers; i++)
		{
			for(j = 0; j<num_routers; j++)
			{
				if(vectorMatrix[cur_router_index][i] > vectorMatrix[cur_router_index][j] + vectorMatrix[j][i])
				{
					vectorMatrix[cur_router_index][i] = vectorMatrix[cur_router_index][j] + vectorMatrix[j][i];
					myNeighbourHopList.at(i) = j;
				}
			}
		}

			//printf("For router %d\n",cur_router_index+1);
			for(i=0;i<num_routers;i++)
			{
					//cout<<snap_pkt->router_array[myNeighbourHopList.at(myNeighbourHopList.at(i))].router_id<<endl;;
			}
		//printf("\n\n");
		//cout<<"Ending Bellman Ford Algorithm"<<endl;
		//cout<<"######################################"<<endl;
}

void remove_control_conn(int sock_index)
{
 
	 close(sock_index);
}

void remove_data_conn(int sockindex)
{
	//cout<<"Remove data connection"<<endl;
	 LIST_FOREACH(connection, &data_conn_list, next) {
		 if(connection->sockfd == sockindex) LIST_REMOVE(connection, next); // this may be unsafe?
		 free(connection);
	 }
}

ssize_t recvALL(int sock_index, unsigned char *buffer, ssize_t nbytes)
{
	// //cout<<"Inside recvALL"<<endl;
	 ssize_t bytes = 0;
	 bytes = recv(sock_index, buffer, nbytes, 0);
	//  //cout<<"NBytes to rcv "<<nbytes<<endl;
	 if(bytes == 0) return -1;
	 while(bytes != nbytes)
		 bytes += recv(sock_index, buffer+bytes, nbytes-bytes, 0);
	 return bytes;
}

ssize_t recvALL_C(int sock_index, char *buffer, ssize_t nbytes)
{
    // //cout<<"Inside recvALL"<<endl;
     ssize_t bytes = 0;
     bytes = recv(sock_index, buffer, nbytes, 0);
    //  //cout<<"NBytes to rcv "<<nbytes<<endl;
     if(bytes == 0) return -1;
     while(bytes != nbytes)
         bytes += recv(sock_index, buffer+bytes, nbytes-bytes, 0);
     return bytes;
}
 
ssize_t sendALL(int sock_index, char *buffer, ssize_t nbytes)
{
	//  //cout<<"Inside sendALL"<<endl;
	 ssize_t bytes = 0;
	 bytes = send(sock_index, buffer, nbytes, 0);
 
	 if(bytes == 0) return -1;
	 while(bytes != nbytes)
		 bytes += send(sock_index, buffer+bytes, nbytes-bytes, 0);
	//  //cout<<"Bytes sent: "<<bytes<<endl;
	 return bytes;
}

struct routing_update_structure buildUpdatePacketStructure()
{
	//cout<<"######################################"<<endl;
	//cout<<"Create update packet"<<endl;

	//struct routing_update_structure* tempObj = (struct routing_update_structure*) malloc(sizeof(struct routing_update_structure));
	struct routing_update_structure tempObj;
	tempObj.num_updates = htons(snap_pkt->num_routers);
	tempObj.source_port = htons(router_port);
	tempObj.source_ip = htonl(cur_router_ip);

	for(int i = 0; i< num_routers; i++)
	{
		//tempObj.router_update_array[i] = malloc(sizeof(router_update_array));
		tempObj.router_update_array[i].r_ip = htonl(snap_pkt->router_array[i].router_ip);
		tempObj.router_update_array[i].r_port = htons(snap_pkt->router_array[i].router_port);
		tempObj.router_update_array[i].pad = htons(0);
		tempObj.router_update_array[i].r_id = htons(snap_pkt->router_array[i].router_id);
		tempObj.router_update_array[i].cost = htons(vectorMatrix[cur_router_index][i]);

	}
	return tempObj;
}

 char* create_response_header(int sock_index, uint8_t control_code, uint8_t response_code, uint16_t payload_len)
 {
	//  //cout<<"Creating response header"<<endl;
	 char *buffer;
 
	 struct CONTROL_RESPONSE_HEADER *cntrl_resp_header;
 
	 struct sockaddr_in addr;
	 socklen_t addr_size;
 
	 buffer = (char *) malloc(sizeof(char)*CNTRL_RESP_HEADER_SIZE);
		 cntrl_resp_header = (struct CONTROL_RESPONSE_HEADER *) buffer;
 
	 addr_size = sizeof(struct sockaddr_in);
	 getpeername(sock_index, (struct sockaddr *)&addr, &addr_size);
 
		 /* Controller IP Address */
		 memcpy(&(cntrl_resp_header->controller_ip_addr), &(addr.sin_addr), sizeof(struct in_addr));
		 /* Control Code */
		 cntrl_resp_header->control_code = control_code;
		 /* Response Code */
		 cntrl_resp_header->response_code = response_code;
		 /* Payload Length */
		 cntrl_resp_header->payload_len = htons(payload_len);
	 return buffer;
 }

 char* createRoutingTableVector(int payload_len)
 {
	//cout<<"######################################"<<endl;
	//cout<<"Create routing table"<<endl;
	int structlen = 0;
	uint16_t next_hop;
	char *buffer;
	buffer = (char *) malloc(sizeof(char)*payload_len);
	//cout<<"|||||||||||||||Printing routing table: "<<endl;
	for(int i = 0; i< num_routers; i++)
	{
		struct routing_table *r_table = (struct routing_table *) (buffer + structlen);//(cntrl_response_payload + structlen);
		r_table->r_id = htons(snap_pkt->router_array[i].router_id);
		r_table->pad = htons(0);
		if(vectorMatrix[cur_router_index][i] == 65535)
		{
			r_table->next_hop = htons(65535);
			r_table->cost = htons(65535);
			//cout<<snap_pkt->router_array[i].router_id<<"\t"<<"0"<<"\t"<<endl;
			//cout<<"65535"<<"\t"<<"65535"<<"\t"<<endl;
		}
		else
		{
			if(myNeighbourHopList.at(i) == -1)
            {
                r_table->next_hop = htons(65535);
                r_table->cost = htons(65535);
            }
            else 
            {
                r_table->next_hop = htons(snap_pkt->router_array[myNeighbourHopList.at(i)].router_id);
                r_table->cost = htons(vectorMatrix[cur_router_index][i]);
            }
			//cout<<snap_pkt->router_array[i].router_id<<"\t"<<"0"<<"\t"<<endl;
			//cout<<snap_pkt->router_array[myNeighbourHopList.at(i)].router_id<<"\t"<<vectorMatrix[cur_router_index][i]<<"\t"<<endl;
		}
		//memcpy(cntrl_response_payload, &r_table, sizeof(r_table));
		structlen = structlen + 8;
	}
	//cout<<"|||||||||||||||Ending printing routing table: "<<endl;

	return buffer;
 }

 void author_response(int sock_index)
 {
	 //cout<<"Author response"<<endl;
	 uint16_t payload_len, response_len;
	 char *cntrl_response_header, *cntrl_response_payload, *cntrl_response;
 
	 payload_len = sizeof(AUTHOR_STATEMENT)-1; // Discount the NULL chararcter
	 cntrl_response_payload = (char *) malloc(payload_len);
	 memcpy(cntrl_response_payload, AUTHOR_STATEMENT, payload_len);
 
	 cntrl_response_header = create_response_header(sock_index, 0, 0, payload_len);
 
	 response_len = CNTRL_RESP_HEADER_SIZE+payload_len;
	 cntrl_response = (char *) malloc(response_len);
	 /* Copy Header */
	 memcpy(cntrl_response, cntrl_response_header, CNTRL_RESP_HEADER_SIZE);
	 free(cntrl_response_header);
	 /* Copy Payload */
	 memcpy(cntrl_response+CNTRL_RESP_HEADER_SIZE, cntrl_response_payload, payload_len);
	 free(cntrl_response_payload);
 
	 sendALL(sock_index, cntrl_response, response_len);
 
	 free(cntrl_response);
 }

 void init_response(int sock_index)
 {
	 //cout<<"######################################"<<endl;
	 //cout<<"Init response"<<endl;
	 uint16_t payload_len, response_len;
	 char *cntrl_response_header, *cntrl_response_payload, *cntrl_response;
 
	 cntrl_response_payload = (char *) malloc(payload_len);
	 memcpy(cntrl_response_payload, NULL, payload_len);
 
	 cntrl_response_header = create_response_header(sock_index, 1, 0, payload_len);
 
	 response_len = CNTRL_RESP_HEADER_SIZE+payload_len;
	 cntrl_response = (char *) malloc(response_len);
	 /* Copy Header */
	 memcpy(cntrl_response, cntrl_response_header, CNTRL_RESP_HEADER_SIZE);
	 free(cntrl_response_header);
	 /* Copy Payload */
	 memcpy(cntrl_response+CNTRL_RESP_HEADER_SIZE, cntrl_response_payload, payload_len);
	 free(cntrl_response_payload);
 
	 sendALL(sock_index, cntrl_response, response_len);
 
	 free(cntrl_response);
	 //cout<<"######################################"<<endl;
 }

 void routingTable_response(int sock_index)
 {
	//cout<<"######################################"<<endl;
	//cout<<"Routing Table response"<<endl;
	uint16_t payload_len, response_len;
	payload_len = 8 * num_routers;
	char *cntrl_response_header, *cntrl_response, *cntrl_response_payload;

	////cout<<"Size of vector: "<<sizeof(cntrl_response_payload)<<endl;
	cntrl_response_header = create_response_header(sock_index, 2, 0, payload_len);

	response_len = CNTRL_RESP_HEADER_SIZE + payload_len;

	cntrl_response_payload = createRoutingTableVector(payload_len);

	cntrl_response = (char *) malloc(response_len);
	/* Copy Header */
	memcpy(cntrl_response, cntrl_response_header, CNTRL_RESP_HEADER_SIZE);
	free(cntrl_response_header);
	/* Copy Payload */
	memcpy(cntrl_response+CNTRL_RESP_HEADER_SIZE, cntrl_response_payload, payload_len);
	free(cntrl_response_payload);

	sendALL(sock_index, cntrl_response, response_len);

	free(cntrl_response);
	//cout<<"######################################"<<endl;
 }

 void update_response(int sock_index)
 {
	 //cout<<"######################################"<<endl;
	 //cout<<"Update response"<<endl;
	 uint16_t payload_len = 0, response_len;
	 char *cntrl_response_header, *cntrl_response_payload, *cntrl_response;
 
	 cntrl_response_payload = (char *) malloc(payload_len);
	 memcpy(cntrl_response_payload, NULL, payload_len);
 
	 cntrl_response_header = create_response_header(sock_index, 3, 0, payload_len);
 
	 response_len = CNTRL_RESP_HEADER_SIZE+payload_len;
	 cntrl_response = (char *) malloc(response_len);
	 /* Copy Header */
	 memcpy(cntrl_response, cntrl_response_header, CNTRL_RESP_HEADER_SIZE);
	 free(cntrl_response_header);
	 /* Copy Payload */
	 memcpy(cntrl_response+CNTRL_RESP_HEADER_SIZE, cntrl_response_payload, payload_len);
	 free(cntrl_response_payload);
 
	 sendALL(sock_index, cntrl_response, response_len);
 
	 free(cntrl_response);
	 //cout<<"######################################"<<endl;
 }

 void crash_response(int sock_index)
 {
	 //cout<<"######################################"<<endl;
	 //cout<<"Update response"<<endl;
	 uint16_t payload_len = 0, response_len;
	 char *cntrl_response_header, *cntrl_response_payload, *cntrl_response;
 
	 cntrl_response_payload = (char *) malloc(payload_len);
	 memcpy(cntrl_response_payload, NULL, payload_len);
 
	 cntrl_response_header = create_response_header(sock_index, 4, 0, payload_len);
 
	 response_len = CNTRL_RESP_HEADER_SIZE+payload_len;
	 cntrl_response = (char *) malloc(response_len);
	 /* Copy Header */
	 memcpy(cntrl_response, cntrl_response_header, CNTRL_RESP_HEADER_SIZE);
	 free(cntrl_response_header);
	 /* Copy Payload */
	 memcpy(cntrl_response+CNTRL_RESP_HEADER_SIZE, cntrl_response_payload, payload_len);
	 free(cntrl_response_payload);
 
	 sendALL(sock_index, cntrl_response, response_len);
 
	 free(cntrl_response);
	 //cout<<"######################################"<<endl;
 }

 void send_response(int sock_index)
 {
	 //cout<<"######################################"<<endl;
	 //cout<<"Send response"<<endl;
	 uint16_t payload_len = 0, response_len;
	 char *cntrl_response_header, *cntrl_response_payload, *cntrl_response;
 
	 cntrl_response_payload = (char *) malloc(payload_len);
	 memcpy(cntrl_response_payload, NULL, payload_len);
 
	 cntrl_response_header = create_response_header(sock_index, 5, 0, payload_len);
 
	 response_len = CNTRL_RESP_HEADER_SIZE+payload_len;
	 cntrl_response = (char *) malloc(response_len);
	 /* Copy Header */
	 memcpy(cntrl_response, cntrl_response_header, CNTRL_RESP_HEADER_SIZE);
	 free(cntrl_response_header);
	 /* Copy Payload */
	 memcpy(cntrl_response+CNTRL_RESP_HEADER_SIZE, cntrl_response_payload, payload_len);
	 free(cntrl_response_payload);
 
	 sendALL(sock_index, cntrl_response, response_len);
 
	 free(cntrl_response);
	 //cout<<"######################################"<<endl;
 }

 void stats_response(int sock_index, unsigned char* payload)
 {
     //cout<<"######################################"<<endl;
     //cout<<"Statistics response"<<endl;
	 //cout<<"Payload_len: "<<payload_len;
	 uint8_t transfer;	
	 memcpy(&transfer, payload, 1);

	 //uint8_t transfer =  (uint8_t)payload[0];
	 uint8_t ttl;
	 uint16_t pad = htons(0);
	 uint16_t varSeqNum;
	 int totalseqnumber = 0;
	 
	 for(int i = 0; i < mystats.size(); i++)
	 {
		 if(mystats[i].transfer == transfer)
		 {
			 totalseqnumber++;
			 ttl = mystats.at(i).timetolive;
		 }
	 }
  
	 char * stats_header;
  
	 stats_payload_len = 4 + (2 * totalseqnumber);
	 //stats_header = (unsigned char *)malloc(totalseqnumber);
	 stats_header = ( char *)malloc(stats_payload_len);
  
	 memcpy(stats_header, &transfer, 1);
	 memcpy(stats_header + 1, &ttl, 1);
	 memcpy(stats_header + 2, &pad, 2);
	 int count = 0;
  
	 for(int i = 0; i < mystats.size(); i++)
	 {
		 if(mystats[i].transfer == transfer)
		 {
			 varSeqNum = htons(mystats[i].seqnum);
			 memcpy(stats_header + 4 + (2 * count), &varSeqNum, 2);
			 count++;
		 }
	 }
 
     uint16_t response_len;
     char *cntrl_response_header, *cntrl_response;
	 
	 response_len = CNTRL_RESP_HEADER_SIZE + stats_payload_len;
 
     cntrl_response_header = create_response_header(sock_index, 6, 0, stats_payload_len);
 
     
     cntrl_response = (char *) malloc(response_len);
     /* Copy Header */
     memcpy(cntrl_response, cntrl_response_header, CNTRL_RESP_HEADER_SIZE);
     free(cntrl_response_header);
     /* Copy Payload */
     memcpy(cntrl_response + CNTRL_RESP_HEADER_SIZE, stats_header, stats_payload_len);
	 free(stats_header);

     sendALL(sock_index, cntrl_response, response_len);
 
	 free(cntrl_response);
	 
     //cout<<"######################################"<<endl;
 }

 void last_data_response(int sock_index)
 {
     //cout<<"######################################"<<endl;
     //cout<<"Last Data Packet response"<<endl;
     uint16_t payload_len = 1036, response_len;
     char *cntrl_response_header, *cntrl_response_payload, *cntrl_response;
 
     cntrl_response_payload = (char *) malloc(payload_len);
     memcpy(cntrl_response_payload, last_packet_router, 1036);
     //free(last_packet_router);
 
     cntrl_response_header = create_response_header(sock_index, 7, 0, payload_len);
 
     response_len = CNTRL_RESP_HEADER_SIZE+payload_len;
     cntrl_response = (char *) malloc(response_len);
     /* Copy Header */
     memcpy(cntrl_response, cntrl_response_header, CNTRL_RESP_HEADER_SIZE);
     free(cntrl_response_header);
     /* Copy Payload */
     memcpy(cntrl_response+CNTRL_RESP_HEADER_SIZE, cntrl_response_payload, 1036);
     free(cntrl_response_payload);
 
     sendALL(sock_index, cntrl_response, response_len);
 
     free(cntrl_response);
     //cout<<"######################################"<<endl;
 }

 void penultimate_data_response(int sock_index)
 {
     //cout<<"######################################"<<endl;
     //cout<<"Penultimate response"<<endl;
     uint16_t payload_len = 1036, response_len;
     char *cntrl_response_header, *cntrl_response_payload, *cntrl_response;
 
     cntrl_response_payload = (char *) malloc(payload_len);
     memcpy(cntrl_response_payload, penultimate_last_packet_router, payload_len);
     //free(penultimate_last_packet_router);
 
     cntrl_response_header = create_response_header(sock_index, 8, 0, payload_len);
 
     response_len = CNTRL_RESP_HEADER_SIZE+payload_len;
     cntrl_response = (char *) malloc(response_len);
     /* Copy Header */
     memcpy(cntrl_response, cntrl_response_header, CNTRL_RESP_HEADER_SIZE);
     free(cntrl_response_header);
     /* Copy Payload */
     memcpy(cntrl_response+CNTRL_RESP_HEADER_SIZE, cntrl_response_payload, 1036);
     free(cntrl_response_payload);
 
     sendALL(sock_index, cntrl_response, response_len);
 
     free(cntrl_response);
     //cout<<"######################################"<<endl;
 }

unsigned char *packet;
  
// https://stackoverflow.com/questions/1680365/integer-to-ip-address-c

 char* get_ip(uint32_t ip)
 {
	 unsigned char bytes[4];
	 bytes[0] = ip & 0xFF;
	 bytes[1] = (ip >> 8) & 0xFF;
	 bytes[2] = (ip >> 16) & 0xFF;
	 bytes[3] = (ip >> 24) & 0xFF; 
	 char *IP = NULL;  
	 asprintf(&IP, "%d.%d.%d.%d", bytes[3], bytes[2], bytes[1], bytes[0]);  
	 //cout<<"IP: "<<IP<<endl;
	 return IP;
	//  string IPRouter(IP);
	//  return IPRouter;
	 //IP = (char)bytes[3] + "." + (char)bytes[2] + "." + (char)bytes[1] + "." + (char)bytes[0];
	 //return bytes[3] + "." + bytes[2] + "." + bytes[1] + "." + bytes[0];    
 }


void CreateYourNeighbours(int index)
{
	router_neighbour tempobj;
	tempobj.cur_time = 0;
	tempobj.router_port = snap_pkt->router_array[index].router_port;
	tempobj.router_ip = snap_pkt->router_array[index].router_ip;
	tempobj.remainingCount = 3;
	myNeighbours.push_back(tempobj);
}

 void create_packet_structure(unsigned char *cntrl_payload)
 {
	//cout<<"######################################"<<endl;
	//cout<<"Building packet started"<<endl;
	snap_pkt = (struct packet_structure *) malloc(sizeof(struct packet_structure));
	snap_pkt->num_routers = ((unsigned int)cntrl_payload[0]<<8) | cntrl_payload[1];
	//defineMatrix();

	// vectorMatrix = new int*[num_routers];
	// for(int i = 0; i < num_routers; ++i)
	// vectorMatrix[i] = new int[num_routers];

	// copy_vectorMatrix = new int*[num_routers];
	// for(int i = 0; i < num_routers; ++i)
	// copy_vectorMatrix[i] = new int[num_routers];
	snap_pkt->interval = ((unsigned int)cntrl_payload[2]<<8) | cntrl_payload[3];
	num_routers = snap_pkt->num_routers;
	periodic_interval = snap_pkt->interval;
	initialiseMatrix(snap_pkt->num_routers);
	tout.tv_sec = periodic_interval;
	lastVal = periodic_interval - 1;

	//cout<<"New timeout set as: "<<tout.tv_sec<<"s"<<endl;

	int j = 4;
	for (int i = 0; i <num_routers; i++)
	{
		snap_pkt->router_array[i].router_id = ((unsigned int)cntrl_payload[(num_routers * i) + j]<<8) | cntrl_payload[(num_routers * i) + 1 + j];
		snap_pkt->router_array[i].router_port = ((unsigned int)cntrl_payload[(num_routers * i) + 2 + j]<<8) | cntrl_payload[(num_routers * i) + 3 + j];
		snap_pkt->router_array[i].data_port = ((unsigned int)cntrl_payload[(num_routers * i) + 4 + j]<<8) | cntrl_payload[(num_routers * i) + 5 + j];
		snap_pkt->router_array[i].link_cost = ((unsigned int)cntrl_payload[(num_routers * i) + 6 + j]<<8) | cntrl_payload[(num_routers * i) + 7 + j];
		snap_pkt->router_array[i].router_ip = (((unsigned int)cntrl_payload[(num_routers * i) + 8 + j]<<24) |
		(cntrl_payload[(num_routers * i) + 9 + j]<<16) | 
		(cntrl_payload[(num_routers * i) + 10 + j]<<8)  | 
		cntrl_payload[(num_routers * i) + 11 + j]);
		get_ip(snap_pkt->router_array[i].router_ip);
		j = j + 12 - num_routers;
	}
	//cout<<"Number of routers : "<<snap_pkt->num_routers<<endl;
	//cout<<"Update interval : "<<snap_pkt->interval<<endl;
	for (int i = 0; i <num_routers; i++)
	{
		//cout<<"Router ID: "<<snap_pkt->router_array[i].router_id<<" Router Port: "<<snap_pkt->router_array[i].router_port<<" Data Port: "<<snap_pkt->router_array[i].data_port<<" Link Cost: "<<snap_pkt->router_array[i].link_cost<<" IP: "<<snap_pkt->router_array[i].router_ip<<endl;
	}
	//cout<<"Add neighbours in the list"<<endl;
	for (int i = 0; i <num_routers; i++)
	{
		if(snap_pkt->router_array[i].link_cost !=0 && snap_pkt->router_array[i].link_cost !=65535)
		{
			CreateYourNeighbours(i);
		}
	}

	for (int i = 0; i <num_routers; i++)
	{
		if(snap_pkt->router_array[i].link_cost == 65535)
			myNeighbourHopList.push_back(-1);
		else
			myNeighbourHopList.push_back(i);
	}

	//cout<<"Building packet finished"<<endl;
	//cout<<"######################################"<<endl;
}

int create_router_socket()
{
	//cout<<"Creating router socket"<<endl;
	struct sockaddr_in control_addr;
    socklen_t addrlen = sizeof(control_addr);

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock < 0)
        perror("socket() failed");

    /* Make socket re-usable */
    // if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (int[]){1}, sizeof(int)) < 0)
	// 	perror("setsockopt() failed");

    bzero(&control_addr, sizeof(control_addr));
	//cout<<"Router port which needs to be listened for receiving: "<<router_port<<endl;
    control_addr.sin_family = AF_INET;
    control_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    control_addr.sin_port = htons(router_port);

    if(bind(sock, (struct sockaddr *)&control_addr, sizeof(control_addr)) < 0)
		perror("bind() failed");

	//LIST_INIT(&control_conn_list);
	return sock;
}

int create_send_socket()
{
	//cout<<"Creating sending socket"<<endl;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock < 0)
        perror("socket() failed");
	return sock;
}

int create_data_socket()
{
	//cout<<"Creating data socket"<<endl;
	struct sockaddr_in control_addr;
    socklen_t addrlen = sizeof(control_addr);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0)
        perror("socket() failed");

    bzero(&control_addr, sizeof(control_addr));

    control_addr.sin_family = AF_INET;
    control_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    control_addr.sin_port = htons(data_port);

    if(bind(sock, (struct sockaddr *)&control_addr, sizeof(control_addr)) < 0)
		perror("bind() failed");

    if(listen(sock, 5) < 0)
		perror("listen() failed");

	LIST_INIT(&data_conn_list);
	return sock;
}

void sendUDP(int port, uint32_t ip, int sockfd, struct routing_update_structure vector)
{
	//cout<<"######################################"<<endl;
	//cout<<"Inside sendUDP"<<endl;

	// for(int j = 0; j <num_routers; j++)
	// 	copy_cost_vector[j] = vectorMatrix[cur_router_index][j];

	struct hostent *hp;     /* host information */
	struct sockaddr_in servaddr;    /* server address */

    /* fill in the server's address and data */
    memset((char*)&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
	inet_pton(AF_INET, get_ip(ip), &(servaddr.sin_addr));

    /* put the host's address into the server address structure */
	// memcpy((void *)&servaddr.sin_addr, (const void *)&ip, 32);
	// int test = sendto(sockfd, vector, sizeof(vector), 0, (struct sockaddr *)&servaddr, sizeof(servaddr));
	//cout<<"Sending to IP: "<<get_ip(ip)<<endl;
    if(sendto(sockfd, &vector, sizeof(vector), 0, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
      perror("sendUDP--sendto failure");
      return ;
	}
	else
	{}
		//cout<<"Send success to port "<<port<<endl;
	//cout<<"SendUDP finished"<<endl;
	//cout<<"######################################"<<endl;
}

void updatevectors()
{
	cost_vector = new int [num_routers];
	//copy_cost_vector = new int [num_routers];
	//cout<<"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"<<endl;
	//cout<<"Updating Vectors started"<<endl;

	//Get the router socket and cost of your row
	for (int i = 0; i <num_routers; i++)
	{
		if(snap_pkt->router_array[i].link_cost == 0)
		{
			router_port = snap_pkt->router_array[i].router_port;
			cur_router_ip = snap_pkt->router_array[i].router_ip;
			data_port = snap_pkt->router_array[i].data_port;
			router_socket = create_router_socket();
			data_socket = create_data_socket();

			/* Register the router socket */
			
			FD_SET(router_socket, &master_list);
			if(router_socket > head_fd) head_fd = router_socket;

			FD_SET(data_socket, &master_list);
			if(data_socket > head_fd) head_fd = data_socket;

			// int max = getMaxValueInList();
			// if(max >= router_socket)
			// 	head_fd = max;
			// else 
				//head_fd = router_socket;

			// max = getMaxValueInList();
			// if(max >= data_socket)
			// 	head_fd = max;
			// else 
			// 	head_fd = data_socket;
			
			cur_router_index = i;
		}
		cost_vector[i] = snap_pkt->router_array[i].link_cost;
	}

	//Updating in the matrix
	for(int j = 0; j< num_routers; j++)
	{
		vectorMatrix[cur_router_index][j] = cost_vector[j];
	}
	
	//BellmanFord Algorithm
	//bellmanFordAlgorithm(vectorMatrix);

	//cout<<"Printing matrix recvd"<<endl;
    for(int i = 0; i < num_routers; i++)
    {
        for(int j = 0; j < num_routers; j++)
        {
            //cout<<vectorMatrix[i][j]<<"\t";
 
        }
        //cout<<endl;
    }

	for(int i = 0; i < num_routers; i++)
	{
		for(int j = 0; j < num_routers; j++)
		{
			copy_vectorMatrix[i][j] = vectorMatrix[i][j];
		}
	}

	//strcpy(vector_update, vector);
	//cout<<"Updating vectors finished"<<endl;
	//cout<<"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"<<endl;
}

int knowIndexOfRouter(uint16_t router_id)
{
	for (int i = 0; i <num_routers; i++)
		if(snap_pkt->router_array[i].router_id == router_id)	
			return i;
	return -1;
}

int knowIndexOfRouterByRouterPort(uint16_t router_port)
{
	for (int i = 0; i <num_routers; i++)
		if(snap_pkt->router_array[i].router_id == ntohs(router_port))	
			return i;
	return -1;
}

bool checkIfNeighbourIsAlreadyThere(int searchingIndex)
{
	for(int i = 0; i <myNeighbours.size(); i++)
		if(i == searchingIndex)
			return true;
	return false;
}

void updateRouterCost(unsigned char *cntrl_payload)
{
	//cout<<"######################################"<<endl;
	//cout<<"Updating vector after update"<<endl;
	uint16_t r_id =  ((unsigned int)cntrl_payload[0]<<8) | cntrl_payload[1];
	uint16_t r_cost = ((unsigned int)cntrl_payload[2]<<8) | cntrl_payload[3];
	//cout<<"R_ID to be updated: "<<r_id<<" Cost:"<<r_cost<<endl;
	int r_index = knowIndexOfRouter(r_id);
	vectorMatrix[cur_router_index][r_index] = r_cost;
	bellmanFordAlgorithm();

	//cout<<"Matrix after UPDATE COMMAND:"<<endl;
	// for(int i = 0; i < num_routers; i++)
	// {
	// 	for(int j = 0; j < num_routers; j++)
	// 	{
	// 		//cout<<vectorMatrix[i][j]<<"\t";
	// 	}
	// 	//cout<<endl;
	// }

	// if(r_cost == 65535)
	// 	myNeighbourHopList.at(cur_router_index) = -1;
	// else 
	
	myNeighbourHopList.at(cur_router_index) = r_index;
	
	// if(!checkIfNeighbourIsAlreadyThere(r_index))
	// {
	// 	router_neighbour tempobj;
	// 	tempobj.cur_time = 0;
	// 	tempobj.router_port = snap_pkt->router_array[r_index].router_port;
	// 	tempobj.router_ip = snap_pkt->router_array[r_index].router_ip;
	// 	tempobj.remainingCount = 3;
	// 	myNeighbours.push_back(tempobj);
	// }

	//bellmanFordAlgorithm();

	//cout<<"Matrix after BELLMANFORD UPDATE COMMAND:"<<endl;
	// for(int i = 0; i < num_routers; i++)
	// {
	// 	for(int j = 0; j < num_routers; j++)
	// 	{
	// 		//cout<<vectorMatrix[i][j]<<"\t";
	// 	}
	// 	//cout<<endl;
	// }

	//cout<<endl;
	//cout<<"Finishing vector after update"<<endl;
	//cout<<"######################################"<<endl;
}

void crashRouter(int sock_index)
{

	close(router_socket);
	FD_CLR(sock_index, &master_list);
}

int connect_to_host(uint32_t nextHop_ip, uint16_t nextHop_port)
{
	int fdsocket = socket(AF_INET, SOCK_STREAM, 0);
	//cout<<"Next hop port: "<<nextHop_port<<endl;
	//cout<<"Next hop IP: "<<nextHop_ip<<endl;
    if(fdsocket < 0)
    {
    //cse4589_print_and_log("[LOGIN:ERROR]\n");
    //cse4589_print_and_log("[LOGIN:END]\n");
    }
 
    struct sockaddr_in remote_server_addr;
 
    bzero(&remote_server_addr, sizeof(remote_server_addr));
    remote_server_addr.sin_family = AF_INET;
    inet_pton(AF_INET, get_ip(nextHop_ip), &remote_server_addr.sin_addr);
	remote_server_addr.sin_port = htons(nextHop_port);
	int test = connect(fdsocket, (struct sockaddr*)&remote_server_addr, sizeof(remote_server_addr)) ;
    if(test < 0)
    {
        //cse4589_print_and_log("[LOGIN:ERROR]\n");
        //cse4589_print_and_log("[LOGIN:END]\n");
	}
	//cout<<"Value for connect: "<<test<<endl;
        //perror("Connect failed");
 
    return fdsocket;
}

void fillTheNextHopForSendingFile()
{
    for(int i = 0; i < num_routers; i++)
    {
        if(snap_pkt->router_array[i].router_ip == send_pkt->dest_ip)
        {
            indexDestination = i;
            break;
        }
    }
    indexNextHop = myNeighbourHopList.at(indexDestination);
	dataPortNextHop = snap_pkt->router_array[indexNextHop].data_port;
	destIPNextHop = snap_pkt->router_array[indexNextHop].router_ip;
	//cout<<"Next hop port is: "<<dataPortNextHop<<endl;
	//cout<<"Next hop IP is: "<<destIPNextHop<<endl;
}

int packetsSent = 0;
uint16_t seqnumber;
 
char* createSendPacket(unsigned char buff[1024], int packetsSent, int totalPacketsToBeSent, char* send_packet, bool firstPacket_send)
{
	//cout<<"Inside creation of send packet"<<endl;
	uint16_t payload_len = 1024;
	uint16_t header_len = 12;
	uint16_t send_pkt_len;
	send_pkt_len = payload_len + header_len;

	char *send_header;

	send_header = (char *) malloc((sizeof(char) * header_len));

	struct sendFilePacketStructure *temp = (struct sendFilePacketStructure *) (send_header);
	temp->dest_ip = htonl(send_pkt->dest_ip);
	temp->transfer_id = send_pkt->transfer_id;
	temp->ttl = send_pkt->ttl;

	if(packetsSent == 0)
    {
        //cout<<"Packets sent: "<<packetsSent<<endl;
        //cout<<"Total packets sent: "<<totalPacketsToBeSent<<endl;
        temp->seq_no = htons(send_pkt->seq_no);
    }
    else 
    {
        //cout<<"Packets sent: "<<packetsSent<<endl;
        //cout<<"Total packets sent: "<<totalPacketsToBeSent<<endl;
        temp->seq_no = htons(send_pkt->seq_no + packetsSent);
    }
 
    uint16_t f;

	if(packetsSent == totalPacketsToBeSent - 1)
    {
        //cout<<"Fin bit set to 1\n";
        //cout<<"Packets sent: "<<packetsSent<<endl;
        //cout<<"Total packets sent: "<<totalPacketsToBeSent<<endl;
        packetsSent = 0;
 
        temp->fin = htons(0x8000);
        f = htons(0x8000);
        //cout<<"For the last packet"<<endl;
        firstPacket_send = true;
        //printf("1 FIN: %04x", ntohs(temp->fin));
    }
    else 
    {
        temp->fin = htons(0);
        f = htons(0);
        //cout<<"Fin bit set to 0\n";
    }
	temp->pad = htons(0);

	char *last_packet;
	last_packet = (char *)malloc((sizeof(char) * 1036));

	char *last_pkt_header;
	last_pkt_header = ( char *) malloc((sizeof( char) * 12));

	bzero(last_packet, 1036);
	
	uint32_t dip = htonl(send_pkt->dest_ip);
    uint8_t tid = send_pkt->transfer_id;
    uint8_t tt = send_pkt->ttl;
	uint16_t sno = temp->seq_no;
	uint16_t p = htons(0);

	//cout<<"First packet: "<<endl;
	//Do nothing. No penultimate

	memcpy(last_pkt_header, &dip, 4);
	memcpy(last_pkt_header + 4, &tid, 1);
	memcpy(last_pkt_header + 5, &tt, 1);
	memcpy(last_pkt_header + 6, &sno, 2);
	memcpy(last_pkt_header + 8, &f, 2);
	memcpy(last_pkt_header + 10, &p, 2);

	memcpy(last_packet, last_pkt_header, 12);
	free(last_pkt_header);

	//bzero(last_packet_router, 1036);

	memcpy(last_packet + 12, buff, 1024);

	////cout<<"Seq number: "<<send_pkt->seq_no + packetsSent<<endl;
	
	// memcpy(send_packet, send_header, 12);
	// free(send_header);
	//cout<<"PacketsSent: "<<packetsSent<<endl;
	//cout<<"Totalpackets: "<<totalPacketsToBeSent<<endl;

	//memcpy(send_packet + 12, buff, 1024);
	//cout<<"Ending creation of send packet"<<endl;
    return last_packet;
}

void readingFileandSendingIt(string filename, bool firstPacket_send)
{
	/*Find the best next hop here and get it's IP and DATA port*/
	int packetsSent = 0;
 
	fillTheNextHopForSendingFile();

	int totalPacketsToBeSent;
 
	int sockfd = connect_to_host(destIPNextHop, dataPortNextHop);
	
	//https://techoverflow.net/2013/08/21/how-to-get-filesize-using-stat-in-cc/
	struct stat st;
    stat(filename.c_str(), &st);
    size = st.st_size;
 
    //cout<<"Size of file: "<<size<<endl;
	totalPacketsToBeSent = size / 1024;

	//totalPacketsToBeSent = 2;

	//cout<<"Total packets: "<<totalPacketsToBeSent<<endl;
 
    const char* fs_name = filename.c_str();
    unsigned char sdbuf[1024]; 
 
    FILE *fs = fopen(fs_name, "r");
    if(fs == NULL)
    {
        //printf("ERROR: File %s not found.\n", fs_name);
        exit(1);
    }
    bzero(sdbuf, 1024); 
	int fs_block_sz; 
	//unsigned char *buf;
	struct sendFilePacketStructure tempObj;
	statistics mystatsobj;
	for(int i = 0; i <totalPacketsToBeSent; i++)
	{
		if((fs_block_sz = fread(sdbuf, sizeof(unsigned char), 1024, fs)) > 0)
        {
            mystatsobj.transfer = send_pkt->transfer_id;
            mystatsobj.timetolive = send_pkt->ttl;
			mystatsobj.seqnum = send_pkt->seq_no + packetsSent;
			//cout<<"SEQUENCENUMBER AFTER RECEIVING: "<<mystatsobj.seqnum<<endl;
 
            mystats.push_back(mystatsobj);
 
            char *buf = ( char *)malloc((sizeof( char) * 1036));
            char *send_packet = ( char *)malloc((sizeof( char) * 1036));
			buf = createSendPacket(sdbuf, packetsSent, totalPacketsToBeSent, send_packet, firstPacket_send);
			
            memcpy(penultimate_last_packet_router,last_packet_router,1036);
			memcpy(last_packet_router,buf,1036);
			
            if(sendALL(sockfd, buf, 1036) < 0)
            {
                //fprintf(stderr, "ERROR: Failed to send file %s. ()\n", fs_name);
                free(buf);
                break;
            }
            //free(buf);
            //free(send_packet);
            //my_stats.seqnum[i] = send_pkt->seq_no + packetsSent;
        }
        packetsSent++;
	}
	fclose(fs);

}

void sendFile(int sock_index, unsigned char* cntrl_payload, uint16_t payloadLen)
{
	firstPacket_send = true;
    string filename;
    //cout<<"######################################"<<endl;
    //cout<<"Building send file packet started"<<endl;
    send_pkt = (struct sendFileControlPayload *) malloc(sizeof(struct sendFileControlPayload));
    send_pkt->dest_ip = (((unsigned int)cntrl_payload[0]<<24) |
    (cntrl_payload[1]<<16) | 
    (cntrl_payload[2]<<8)  | 
	cntrl_payload[3]);
	//cout<<"Destination IP:"<<send_pkt->dest_ip<<endl;
 
    send_pkt->ttl = cntrl_payload[4];
    send_pkt->transfer_id = cntrl_payload[5];
 
    send_pkt->seq_no = ((unsigned int)cntrl_payload[6]<<8) | cntrl_payload[7];
	//filename = string((cntrl_payload + 8),(unsigned long)(payloadLen - 8));
	//(cntrl_payload + 8), (payloadLen - 8)

	filename = string(reinterpret_cast<char*>(cntrl_payload + 8), (unsigned long)(payloadLen - 8));

	seqnumber = ntohs(send_pkt->seq_no);
	//cout<<"Filename: "<<filename<<endl;
	readingFileandSendingIt(filename, firstPacket_send);
	//cout<<"End of send file packet started"<<endl;
	//cout<<"######################################"<<endl;
	//free(send_pkt);
}

int knowIndexOfRouterByRouterIP(uint32_t router_ip)
{
	//cout<<"Inside index finding Router IP"<<router_ip<<endl;
	for (int i = 0; i <num_routers; i++)
		if(snap_pkt->router_array[i].router_ip == router_ip)	
		{
			//cout<<" "<<snap_pkt->router_array[i].router_ip<<" "<<router_ip<<endl;
			return i;
		}
	return -1;
}


bool samePacket = true;
bool data_recv_hook(int sock_index)
{

	//cout<<"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$"<<endl;
	//cout<<"Data receive hook"<<endl;
   
	char *recv_payload;
	char *recv_header;
	char *last_packet_recv;
	last_packet_recv = (char *)malloc((sizeof(char) * 1036));

	char *last_pkt_header;
	last_pkt_header = (char *) malloc((sizeof(char) * 12));
	
	bzero(last_packet_recv, 1036);
	//unsigned char *last_pkt;
	
	uint32_t dest_ip;
    uint8_t transfer_id;
    uint8_t ttl;
    uint16_t seq_no;
	uint16_t fin;
	uint16_t pad;
	
	int payload_len = 1036;

	recv_header = (char *) malloc(sizeof(char)*12);

	bzero(recv_header, 12);

	if(recvALL_C(sock_index, recv_header, 12) < 0){
        remove_data_conn(sock_index);
        free(recv_header);
        return false;
    }

	/* Get data from the header */
	struct RECV_HEADER *header = (struct RECV_HEADER *) recv_header;

		 //printf("Before TTL: %u\n", header->ttl);
		 dest_ip = ntohl(header->dest_ip);
		 transfer_id = header->transfer_id;
		 ttl = header->ttl;
		 seq_no = ntohs(header->seq_no);
		 fin = ntohs(header->fin);
		 pad = ntohs(header->pad);

	int destIndex = knowIndexOfRouterByRouterIP(dest_ip);

		 ////cout<<"Destination IP: "<<dest_ip<<endl;
		 ////cout<<"Destination IP: "<<htonl(dest_ip)<<endl;

	/* Get data payload */
    if(payload_len != 0){
        recv_payload = ( char *) malloc(sizeof( char)*1024);
        bzero(recv_payload, 1024);
        //cout<<"Getting data payload"<<endl;
        if(recvALL_C(sock_index, recv_payload, 1024) < 0){
            remove_control_conn(sock_index);
            free(recv_payload);
            return false;
        }
	}
	
	ttl--;

	if(ttl == 0)
		return true;
	
	statistics mystats_recv; 
	mystats_recv.transfer = transfer_id;
	mystats_recv.timetolive = ttl;
	mystats_recv.seqnum = seq_no;
	mystats.push_back(mystats_recv);

	uint32_t dest_ip_n;
    uint8_t transfer_id_n;
    uint8_t ttl_n;
    uint16_t seq_no_n;
	uint16_t fin_n;
	uint16_t pad_n;

	dest_ip_n = htonl(dest_ip);
	seq_no_n = htons(seq_no);
	fin_n = htons(fin);
	pad_n = htons(pad);
	ttl_n = ttl;
	transfer_id_n = transfer_id;

	memcpy(last_pkt_header, &dest_ip_n, 4);
	memcpy(last_pkt_header + 4, &transfer_id_n, 1);
	memcpy(last_pkt_header + 5, &ttl_n, 1);
	memcpy(last_pkt_header + 6, &seq_no_n, 2);
	memcpy(last_pkt_header + 8, &fin_n, 2);
	memcpy(last_pkt_header + 10, &pad_n, 2);

	memcpy(last_packet_recv, last_pkt_header, 12);
	free(last_pkt_header);


	memcpy(last_packet_recv + 12, recv_payload, 1024);

	memcpy(penultimate_last_packet_router,last_packet_router,1036);
    memcpy(last_packet_router,last_packet_recv,1036);
	
	//printf("Transfer ID: %u\n", transfer_id);
	//printf("After TTL: %u\n", ttl);
	//cout<<"Seq Number: "<<seq_no<<endl;
	//printf("FIN: %04x\n", fin);
	//cout<<"PAD: 0"<<endl;


	if(destIndex == cur_router_index)
	{
		recvcounter++;
		//cout<<"I am the destination!"<<endl;
		
		//https://stackoverflow.com/questions/26161072/inserting-int-into-file-name-using-stringstream
		if(firstPacket_recv)
		{
			int tr_id = transfer_id;
			std::stringstream file;
			file << "file-" << tr_id;
			f_append = file.str();
			finame = f_append.c_str();
			firstPacket_recv = false;
		}

		//const char* fr_name = "output";
		FILE *fr = fopen(finame, "a");
		if(fr == NULL)
		{
			//printf("File %s cannot be opened.\n", finame);
		}
		else
		{

		//https://www.linuxquestions.org/questions/programming-9/tcp-file-transfer-in-c-with-socket-server-client-on-linux-help-with-code-4175413995/
		////cout<<"Not null"<<endl;
		int block_sz = 1024;
		int write_size;
		if((write_size = fwrite(recv_payload, sizeof(unsigned char), block_sz, fr)) > 0)
		{
			////cout <<"Value of write size"<<write_size<<endl;
			if(write_size < block_sz)
			{
				perror("File write failed.\n");
			}
			free(recv_payload);
		}
		fclose(fr);
		}

		if(fin == 0x8000)
		{
			//cout<<"Inside FIN 1"<<endl;
			firstPacket_recv = true;
		}
		
		//free(fr_name);
	}
	else 
	{
		//Find next hop
		//cout<<"Destination Index: "<<destIndex<<endl;

		for(int i = 0; i < myNeighbourHopList.size(); i++)
		{
			//cout<<"Next Hop: "<<myNeighbourHopList.at(i)<<endl;
		}
		int hopIndex = myNeighbourHopList.at(destIndex);
		//cout<<"Hop Index: "<<hopIndex<<endl;
		uint16_t nextdataPort = snap_pkt->router_array[hopIndex].data_port;
		//cout<<"Next data port: "<<nextdataPort<<endl;
		uint32_t nextdataIP = snap_pkt->router_array[hopIndex].router_ip;

		uint16_t payload_len = 1024;
		uint16_t header_len = 12;
		uint16_t send_pkt_len;

		// unsigned char *send_r_header, *send_r_packet;
		// send_r_packet = (unsigned char *)malloc((sizeof(unsigned char) * 1036));
		// send_r_header = (unsigned char *) malloc((sizeof(unsigned char) * header_len));


		// struct sendFilePacketStructure *temp = (struct sendFilePacketStructure *) (send_r_header + 0);
		
		// temp->dest_ip = htonl(send_pkt->dest_ip);
		// temp->transfer_id = send_pkt->transfer_id;
		// temp->ttl = ttl;
		// temp->seq_no = htons(temp_header->seq_no);
		// temp->fin = temp_header->fin;
		// temp->pad = 0;

		// memcpy(send_r_packet, send_r_header, 12);
		// free(send_r_header);


		// send_pkt_len = payload_len + header_len;

		// memcpy(send_r_packet + 12, recv_payload, 1024);
		
		if(firstPacket_recv && fin != 0x8000)
		{
			send_socket = connect_to_host(nextdataIP, nextdataPort);
		}

		if(sendALL(send_socket, last_packet_recv, 1036) < 0)
		{
			perror("Sending failed:");
			return false;
		}



		//free(send_r_packet);
		//free(send_r_header);
		free(last_packet_recv);
		//free(temp);
	

	}

	 if(fin == 0x8000)
	 {
		 firstPacket_recv = true;
	 }
	 else 
		 firstPacket_recv = false;

	 //free(temp_header);
	 //free(recv_header);
	 //free(recv_payload);

	return true;
}

// char* fillStatistics(unsigned char *cntrl_payload)
// {
//     // cout<<"######################################"<<endl;
//     // cout<<"Starting Statistics"<<endl;
 
//     uint8_t transfer =  (unsigned int)cntrl_payload[0];
//     uint8_t ttl;
//     uint16_t pad = htons(0);
//     uint16_t varSeqNum;
//     int totalseqnumber = 0;
//     for(int i = 0; i < mystats.size(); i++)
//     {
//         if(mystats.at(i).transfer == transfer)
//         {
//             // printf("Transfer ID: %u \t", mystats.at(i).transfer);
//             // printf("Time to live: %u \t", mystats.at(i).timetolive);
//             // cout<<"Sequence number: "<<mystats.at(i).seqnum<<endl;
//         }
//     }
 
// 	//cout<<"Printing my vector for statistics:"<<endl;
	
// 	for(int i = 0; i < mystats.size(); i++)
//     {
//         // printf("Transfer ID: %u \t", mystats.at(i).transfer);
// 		// printf("Time to live: %u \t", mystats.at(i).timetolive);
// 		// cout<<"Sequence number: "<<mystats.at(i).seqnum<<endl;
//     }
    
//     for(int i = 0; i < mystats.size(); i++)
//     {
//         if(mystats.at(i).transfer == transfer)
//         {
//             totalseqnumber++;
//             ttl = mystats.at(i).timetolive;
//         }
//     }
 
//     char * stats_header;
 
//     stats_payload_len = 4 + (2 * totalseqnumber);
//     //stats_header = (unsigned char *)malloc(totalseqnumber);
//     stats_header = ( char *)malloc(stats_payload_len);
 
//     memcpy(stats_header, &transfer, 1);
//     memcpy(stats_header + 1, &ttl, 1);
// 	memcpy(stats_header + 2, &pad, 2);
// 	int count = 0;
 
//     for(int i = 0; i < mystats.size(); i++)
//     {
//         if(mystats.at(i).transfer == transfer)
//         {
//             varSeqNum = htons(mystats.at(i).seqnum);
// 			memcpy(stats_header + 4 + (2 * count), &varSeqNum, 2);
// 			count++;
//         }
//     }
//     //memcpy(stats_header, &sendobj, stats_payload_len);
//     // cout<<"Finishing statistics"<<endl;
//     // cout<<"######################################"<<endl;
//     return stats_header;
// }

 bool control_recv_hook(int sock_index)
 {
	 ////cout<<"Control receive hook"<<endl;
	 unsigned char *cntrl_header;
	 unsigned char *cntrl_payload;
	 uint8_t control_code;
	 uint16_t payload_len;
 
	 /* Get control header */
	 cntrl_header = (unsigned char *) malloc(sizeof(unsigned char)*CNTRL_HEADER_SIZE);
	 bzero(cntrl_header, CNTRL_HEADER_SIZE);
	 //cout<<"Getting control header"<<endl;
	 if(recvALL(sock_index, cntrl_header, CNTRL_HEADER_SIZE) < 0){
		 remove_control_conn(sock_index);
		 free(cntrl_header);
		 return false;
	 }
 
	 /* Get control code and payload length from the header */
	struct CONTROL_HEADER *header = (struct CONTROL_HEADER *) cntrl_header;
	control_code = header->control_code;
	payload_len = ntohs(header->payload_len);
	//cout<<"Payload length: "<<payload_len<<endl;
	//cout<<"Control code: "<<control_code<<endl;
	 //#endif
 
	 free(cntrl_header);
 
	 /* Get control payload */
	 if(payload_len != 0){
		 cntrl_payload = (unsigned char *) malloc(sizeof(unsigned char)*payload_len);
		 bzero(cntrl_payload, payload_len);
		 //cout<<"Getting control payload"<<endl;
		 if(recvALL(sock_index, cntrl_payload, payload_len) < 0){
			 remove_control_conn(sock_index);
			 free(cntrl_payload);
			 return false;
		 }
	 }
 
	 /* Triage on control_code */
	 switch(control_code){
		 case 0: author_response(sock_index);
				 break;
 
		 case 1: //cout<<"&&&&Control code 1"<<endl;
		 		 init_response(sock_index);
				 create_packet_structure(cntrl_payload); 
				 updatevectors();
				 break;

		 case 2: //cout<<"&&&&Control code 2"<<endl;
				 routingTable_response(sock_index);
				 break;

		 case 3: //cout<<"&&&&Control code 3"<<endl;
				 updateRouterCost(cntrl_payload);
				 update_response(sock_index);
				 break;

		 case 4: //cout<<"&&&&Control code 4"<<endl;
				 crashRouter(sock_index);
				 crash_response(sock_index);
				 break;

		 case 5: //cout<<"&&&&Control code 5"<<endl;
				 sendFile(sock_index, cntrl_payload, payload_len);
				 send_response(sock_index);
				 break;

		 case 6: //cout<<"&&&&Control code 6"<<endl;
		 		 //payload = fillStatistics(cntrl_payload);
		 		 stats_response(sock_index, cntrl_payload);
				 break;

		 case 7: //cout<<"&&&&Control code 7"<<endl;
				 last_data_response(sock_index);
				 break;

		 case 8: //cout<<"&&&&Control code 7"<<endl;
				 penultimate_data_response(sock_index);
				 break;
			
	 }
 
	 if(payload_len != 0) free(cntrl_payload);
	 return true;
 }

bool isAllowedForBellmanAlgorithm(int vectorMatrix[5][5])
{
	for(int i = 0; i < num_routers; i++)
		for(int j = 0; j < num_routers; j++)
			if(copy_vectorMatrix[i][j] != vectorMatrix[i][j])
				return true;
	return false;;
}

void updateMatrix(int *recvdVector)
{
	// //cout<<"######################################"<<endl;
	// //cout<<"Updating vector after receive"<<endl;
	int indexToBeUpdated;
	for(int i = 0; i < num_routers; i++)
	{
		if(recvdVector[i] == 0)
			indexToBeUpdated = i;
	}
	for(int j = 0; j < num_routers; j++)
	{
		vectorMatrix[indexToBeUpdated][j] = recvdVector[j];
	}

	//cout<<"Printing matrix after Update received"<<endl;
	for(int i = 0; i < num_routers; i++)
	{
		for(int j = 0; j < num_routers; j++)
		{
			//cout<<vectorMatrix[i][j]<<"\t";
		}
		//cout<<endl;
	}

	//bool permit = isAllowedForBellmanAlgorithm(vectorMatrix);
	////cout<<"Permit value: "<<permit<<endl;
	//if(permit)
		bellmanFordAlgorithm();
	//else 
	//{
		//Nothing
	//}
		
	//cout<<"Copying Matrix after Bellman Ford algorithm is: "<<endl;
	for(int i = 0; i < num_routers; i++)
	{
		
		for(int j = 0; j < num_routers; j++)
		{
			//cout<<vectorMatrix[i][j]<<"\t";
		}
		//cout<<endl;
	}

	for(int i = 0; i < num_routers; i++)
	{
		for(int j = 0; j < num_routers; j++)
		{
			copy_vectorMatrix[i][j] = vectorMatrix[i][j];
		}
	}

	//cout<<"Printing matrix after Bellman Ford"<<endl;
	for(int i = 0; i < num_routers; i++)
	{
		for(int j = 0; j < num_routers; j++)
		{
			//cout<<vectorMatrix[i][j]<<"\t";
		}
		//cout<<endl;
	}
	//cout<<"Updating vector finished"<<endl;
	//cout<<"######################################"<<endl;
}

void recvUDP(int sock)
{
	countActiveRouters++;
	//cout<<"----------------------------------------------"<<endl;
	//cout<<"Inside recvfrom"<<endl;
	int *recvdVector;
	recvdVector = new (nothrow) int[num_routers];
	//struct routing_update_structure* rcvdVector = (struct routing_update_structure*) malloc(sizeof(struct routing_update_structure));
	// struct sockaddr_storage sender;
	struct routing_update_structure rcvdVector;
	struct sockaddr_in sender;
	socklen_t sendsize = sizeof(sender);
	bzero(&sender, sizeof(sender));
	int testBytes = recvfrom(sock, &rcvdVector, sizeof(rcvdVector), 0, (struct sockaddr *)&sender, &sendsize);
	if( testBytes < 0)
	{
		perror("recvUDP--recvfrom failure");
		return;
	}
	//cout<<"Bytes rcvd: "<<testBytes<<endl;
	uint16_t sender_port = ntohs(sender.sin_port);
	
	char* ipString = inet_ntoa(sender.sin_addr);

	//cout<<"Data received from IP is: "<<ipString<<endl;
	
	//Printing the neighbours
	for(int i = 0; i<myNeighbours.size(); i++)
	{
		//cout<<"$$$$$$$$$$$$$$$$$$$Neighbour Port - "<<myNeighbours.at(i).router_port<<endl;
	}

	//cout<<"Bytes received: "<<sizeof(rcvdVector)<<endl;
	//cout<<"Vector rcvd";
	
	//cout<<"###############Start - Priniting data received################"<<endl;
	////cout<<"Total updates : "<<ntohs(rcvdVector.num_updates)<<endl;
	////cout<<"Source port:"<<ntohs(rcvdVector.source_port)<<endl;
	////cout<<"Source IP:"<<ntohl(rcvdVector.source_ip)<<endl;
	for(int i = 0; i< num_routers; i++)
	{
		////cout<<"IP:"<<ntohl(rcvdVector.router_update_array[i].r_ip)<<"\t";
		////cout<<"Port:"<<ntohs(rcvdVector.router_update_array[i].r_port)<<"\t";
		////cout<<"Pad:"<<ntohs(rcvdVector.router_update_array[i].pad)<<"\t";
		////cout<<"ID:"<<ntohs(rcvdVector.router_update_array[i].r_id)<<"\t";
		recvdVector[i]=(int)ntohs(rcvdVector.router_update_array[i].cost);
		//cout<<recvdVector[i]<<" ";
		////cout<<endl;
	}
	//cout<<endl;
	//cout<<"###############Finish - Priniting data received################"<<endl;

	if(counter < periodic_interval)
	for(int i = 0; i < myNeighbours.size(); i++)
    {
        if(sender_port == myNeighbours.at(i).router_port)
        {
            myNeighbours.at(i).updateReceived = true;
        }
    }
 
    //     for(int i = 0; i < myNeighbours.size(); i++)
    //         if(myNeighbours.at(i).router_port != ntohs(rcvdVector.source_port))
    //             myNeighbours.at(i).remainingCount --;
 
    // //Setting cost to INF for three periodic interval non updates
    // for(int i = 0; i < myNeighbours.size(); i++)
    // {
    //     if(myNeighbours.at(i).remainingCount == 0)
    //     {
    //         int j = knowIndexOfRouterByRouterPort(myNeighbours.at(i).router_port);
    //         vectorMatrix[cur_router_index][j] = 65535;
    //         cost_vector[j] = 65535;
    //         myNeighbours.erase(myNeighbours.begin() + i);
    //     }
    // }
 
    // for(int i = 0; i < myNeighbours.size(); i++)
    // {
    //     //cout<<"Neighbour:"<<i+1<<"\t"<<" Router port:"<<myNeighbours.at(i).router_port<<"\t"<<"Time:"<<myNeighbours.at(i).cur_time<<endl;
    // }

	for(int i = 0; i < myNeighbours.size(); i++)
	{
		//cout<<"Neighbour:"<<i+1<<"\t"<<" Router port:"<<myNeighbours.at(i).router_port<<"\t"<<"Time:"<<myNeighbours.at(i).cur_time<<endl;
	}
	/*uint16_t test1 = ((unsigned int)rcvdVector[18]<<8) | rcvdVector[19];
	uint16_t test2 = ((unsigned int)rcvdVector[26]<<8) | rcvdVector[27];
	uint16_t test3 = ((unsigned int)rcvdVector[34]<<8) | rcvdVector[35];
	//cout<<"Vector rcvd";
	//cout<<test1<<" "<<test2<<" "<<test3<<endl;*/
	updateMatrix(recvdVector);
	//cout<<"RecvUDP finished"<<endl;
	//cout<<"----------------------------------------------"<<endl;
}

void settout(double diff)
{
	//cout<<"**********************************************"<<endl;
	//cout<<"Inside timeout setting"<<endl;
	long int first, second;
	int firstindex, secondindex;
	first = second = 0;
	firstindex = secondindex = 0;
	for (int i = 0; i < myNeighbours.size() ; i ++)
	{
		/* If current element is smaller than first 
			then update both first and second */
		if (myNeighbours.at(i).cur_time < first)
		{
			second = first;
			first = myNeighbours.at(i).cur_time;
			firstindex = i;
		}
		/* If arr[i] is in between first and second 
			then update second  */
		else if (myNeighbours.at(i).cur_time < second && myNeighbours.at(i).cur_time != first)
		{
			second = myNeighbours.at(i).cur_time;
			secondindex = i;
		}
	}

	if(first == second)
		tout.tv_sec = periodic_interval;
	else 
		tout.tv_sec = periodic_interval - diff;
	myNeighbours.at(firstindex).cur_time = myNeighbours.at(firstindex).cur_time + (long int)periodic_interval;
	//cout<<"**********************************************"<<endl;
}

void main_loop()
{
	//cout<<"######################################"<<endl;
	//cout<<"Main Loop"<<endl;
	 int selret, sock_index, fdaccept;
	 tout.tv_sec = 100;
	 tout.tv_usec = 0;

	 test_time.tv_sec = 100;
	 test_time.tv_usec = 0;
	 while(true)
	 {
			watch_list = master_list;
			//cout<<"Before Select"<<endl;
			//cout<<"File descriptors present in watchlist"<<endl;
			for(sock_index = 0; sock_index<=head_fd; sock_index++)
			{
				if(FD_ISSET(sock_index, &watch_list))
				{
					//cout<<"FD: "<<sock_index<<endl;
				}
			}

			//time_t begin, end;
			//time (&begin);
			//begin = time(NULL);
			//clock_t begin = clock();
			timestamp_t t0 = get_timestamp();

			selret = select(head_fd+1, &watch_list, NULL, NULL, &tout);
			numberofintervals++;
            counter++;
			timestamp_t t1 = get_timestamp();

			//clock_t end = clock();
			//time (&end);
			//end = time(NULL);
			//float difference = difftime(end,begin);
			//double time_spent = (double)(end - begin)/CLOCKS_PER_SEC;
			double secs = (t1 - t0) / 1000000.0L;
			//float difference = end - begin;
			////cout<<"Time select took:"<<secs<<"s"<<endl;

			if(selret < 0)
				perror("select failed.");
			
			else if(selret == 0)
			{	
				if(counter % periodic_interval == 0)
					counter = 0;
				//tout occured
				//cout<<"Inside timeout"<<endl;
				//test_time.tv_sec = 20;
				tout.tv_sec = periodic_interval;
				struct routing_update_structure sendUpdatePacket;
				sendUpdatePacket = buildUpdatePacketStructure();
				////cout<<"Size of packet building"<<sizeof(sendUpdatePacket)<<endl;
				int sockfd = create_send_socket();
		
				for(int i = 0; i < myNeighbourHopList.size(); i ++)
				{
					if(myNeighbourHopList.at(i) != -1)
					{
					sendUDP(snap_pkt->router_array[myNeighbourHopList.at(i)].router_port, snap_pkt->router_array[myNeighbourHopList.at(i)].router_ip, sockfd, sendUpdatePacket);
					}
				}
				//close(sockfd);
				//updatevectors();
				//settout(secs);
			}
			else 
			{
			/* Loop through file descriptors to check which ones are ready */
			for(sock_index=0; sock_index<=head_fd; sock_index+=1)
				{
					////cout<<"Main Loop: Inside for loop"<<endl;
					if(FD_ISSET(sock_index, &watch_list)){
		
						/* control_socket */
						if(sock_index == control_socket){
							fdaccept = new_control_conn(sock_index);
							//cout<<"Inside FDAccept"<<endl;
							/* Add to watched socket list */
							FD_SET(fdaccept, &master_list);
							// int max = getMaxValueInList();
							// if(max >= fdaccept)
							// 	head_fd = max;
							// else 
							//head_fd = fdaccept;
							  if(fdaccept > head_fd) head_fd = fdaccept;
						}
		
						/* router_socket */
						else if(sock_index == router_socket){
							//cout<<"Inside router socket"<<endl;
							recvUDP(router_socket);
							//call handler that will call recvfrom() .....
						}
		
						/* data_socket */
						else if(sock_index == data_socket){
							//cout<<"Inside data socket"<<endl;
							fdaccept = new_data_conn(sock_index);
							//cout<<"Inside FDAccept"<<endl;
							
                            /* Add to watched socket list */
                            FD_SET(fdaccept, &master_list);
                              if(fdaccept > head_fd) head_fd = fdaccept;
						}
		
						/* Existing connection */
						else
						{
							//cout<<"Existing connection"<<endl;
							//cout<<"SockIndex Test:"<<sock_index<<endl;
							//  if(isControl(sock_index)){
							// 	 if(!control_recv_hook(sock_index)) FD_CLR(sock_index, &master_list);
							//  }
							//  else
							//  {

							//  }

							 if(isData(sock_index)){
								 if(!data_recv_hook(sock_index)) FD_CLR(sock_index, &master_list);
							 }
							 else 
							 {
								if(!control_recv_hook(sock_index)) FD_CLR(sock_index, &master_list);
							 }
							//else if isData(sock_index);
							//else perror("Unknown socket index");
						}
					}
				}
			}
	}
}

int create_control_socket()
{
	//cout<<"Creating control socket"<<endl;
	struct sockaddr_in control_addr;
    socklen_t addrlen = sizeof(control_addr);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0)
        perror("socket() failed");

    /* Make socket re-usable */
    // if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (int[]){1}, sizeof(int)) < 0)
	// 	perror("setsockopt() failed");

    bzero(&control_addr, sizeof(control_addr));

    control_addr.sin_family = AF_INET;
    control_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    control_addr.sin_port = htons(control_port);

    if(bind(sock, (struct sockaddr *)&control_addr, sizeof(control_addr)) < 0)
		perror("bind() failed");

    if(listen(sock, 5) < 0)
		perror("listen() failed");

	//LIST_INIT(&control_conn_list);
	return sock;
}

 void initialisation()
 {
	//cout<<"######################################"<<endl;
	//cout<<"Initialisation"<<endl;
	control_socket = create_control_socket();
	//cout<<"Control socket created\n";
	FD_ZERO(&master_list);
	FD_ZERO(&watch_list);
	/* Register the control socket */
    FD_SET(control_socket, &master_list);
    head_fd = control_socket;
    main_loop();
 }

/**
 * main function
 *
 * @param  argc Number of arguments
 * @param  argv The argument list
 * @return 0 EXIT_SUCCESS
 */
int main(int argc, char **argv)
{
	/*Start Here*/
	//cout<<"Here"<<endl;
	//freopen("logs.txt", "a", stdout);
	penultimate_last_packet_router = ( char*)(malloc(1036));
	last_packet_router = ( char*)(malloc(1036));
	bzero(penultimate_last_packet_router, 1036);
	bzero(last_packet_router, 1036);
	control_port = atoi(argv[1]);
	initialisation();	
	return 0;
}

//./grader_controller -c grader.cfg -d /home/csgrad/parushga/grader_dir/ -s ../parushga_pa3.tar -ctrl ../controller/controller -t author




//./grader_controller -c grader.cfg -s ../parushga_pa3.tar -d /home/csgrad/parushga/grader_dir/ -ctrl ../controller/controller -t init -nu -nb

//./grader_controller -c grader.cfg -d /local/Fall_2017/parushga/cse489589_assignment3/ -s ../parushga_pa3.tar -ctrl ../controller/controller -t init

///home/csgrad/parushga/cse489589_assignment3/parushga
//cd local/Fall_2017/parushga/cse489589_assignment3

//-f 1 2 10 1234 100 testfile