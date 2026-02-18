#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <arpa/inet.h>
#include <sys/epoll.h>

#define MAX_PACKET_SIZE 65536
#define MAX_EVENTS 16
#define MY_PORT 8080
#define TARGET_PORT 8080

#define COL_RESET "\033[0m"
#define COL_GREEN "\033[32m" 
#define COL_CYAN  "\033[36m" 
#define CLEAR_LINE "\033[2K\r" 

unsigned short checksum(unsigned short *buf, int nwords) {
    unsigned long sum;
    for (sum = 0; nwords > 0; nwords--)
        sum += *buf++;
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    return ~sum;
}

void send_raw_packet(int sock, char *msg, char *src_ip_str, char *dst_ip_str) {
    char packet[MAX_PACKET_SIZE];
    memset(packet, 0, MAX_PACKET_SIZE);

    struct ip *ipHeader = (struct ip *)packet;
    struct udphdr *udpHeader = (struct udphdr *)(packet + sizeof(struct ip));
    char *payload = (char *)(packet + sizeof(struct ip) + sizeof(struct udphdr));

    strncpy(payload, msg, MAX_PACKET_SIZE - sizeof(struct ip) - sizeof(struct udphdr) - 1);
    int payload_len = strlen(payload);

    ipHeader->ip_hl = 5;
    ipHeader->ip_v = 4;
    ipHeader->ip_tos = 0;
    ipHeader->ip_len = htons(sizeof(struct ip) + sizeof(struct udphdr) + payload_len);
    ipHeader->ip_id = htons(54321);
    ipHeader->ip_off = 0;
    ipHeader->ip_ttl = 64;
    ipHeader->ip_p = IPPROTO_UDP;
    ipHeader->ip_src.s_addr = inet_addr(src_ip_str);
    ipHeader->ip_dst.s_addr = inet_addr(dst_ip_str);
    ipHeader->ip_sum = checksum((unsigned short *)ipHeader, sizeof(struct ip) / 2);
    udpHeader->source = htons(MY_PORT);
    udpHeader->dest = htons(TARGET_PORT);
    udpHeader->len = htons(sizeof(struct udphdr) + payload_len);
    udpHeader->check = 0;

    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_port = udpHeader->dest;
    sin.sin_addr.s_addr = ipHeader->ip_dst.s_addr;

    if (sendto(sock, packet, ntohs(ipHeader->ip_len), 0, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
        perror("sendto");
    }
}

void recv_and_print(int sock) {
    char buffer[MAX_PACKET_SIZE];
    struct sockaddr_in sender_addr;
    socklen_t addr_len = sizeof(sender_addr);

    int len = recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr *)&sender_addr, &addr_len);
    if (len < 0) return;

    struct ip *ip = (struct ip *)buffer;
    int ip_header_len = ip->ip_hl * 4;

    if (ip->ip_p != IPPROTO_UDP) return;

    struct udphdr *udp = (struct udphdr *)(buffer + ip_header_len);
    
    if (ntohs(udp->dest) != MY_PORT) return;

    char *msg = (char *)(buffer + ip_header_len + sizeof(struct udphdr));
    int msg_len = ntohs(udp->len) - sizeof(struct udphdr);
    
    if (msg_len > 0) {
        msg[msg_len] = '\0';
        printf(CLEAR_LINE);
        printf("%s[%s]: %s%s\n", COL_GREEN, inet_ntoa(ip->ip_src), msg, COL_RESET);
        printf("Input > ");
        fflush(stdout);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: sudo %s <Source IP> <Dest IP>\n", argv[0]);
        return 1;
    }

    int sock = socket(PF_INET, SOCK_RAW, IPPROTO_UDP);
    if (sock < 0) {
        perror("socket");
        return 1;
    }

    int one = 1;
    if (setsockopt(sock, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one)) < 0) {
        perror("setsockopt IP_HDRINCL");
        return 1;
    }

    int epfd = epoll_create(MAX_EVENTS);
    struct epoll_event ev, events[MAX_EVENTS];

    ev.events = EPOLLIN;
    ev.data.fd = STDIN_FILENO;
    epoll_ctl(epfd, EPOLL_CTL_ADD, STDIN_FILENO, &ev);

    ev.events = EPOLLIN;
    ev.data.fd = sock;
    epoll_ctl(epfd, EPOLL_CTL_ADD, sock, &ev);

    printf(CLEAR_LINE);
    printf("%s=== P2P Chat Started (UDP Port %d) ===%s\n", COL_CYAN, MY_PORT, COL_RESET);
    printf("Input > ");
    fflush(stdout);

    while (1) {
        int nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);
        for (int i = 0; i < nfds; i++) {
            if (events[i].data.fd == STDIN_FILENO) {
                char msg[1024];
                if (fgets(msg, sizeof(msg), stdin) != NULL) {
                    msg[strcspn(msg, "\n")] = 0;
                    if (strlen(msg) > 0) {
                        send_raw_packet(sock, msg, argv[1], argv[2]);
                    }
                    printf("Input > ");
                    fflush(stdout);
                }
            } else if (events[i].data.fd == sock) {
                recv_and_print(sock);
            }
        }
    }

    close(sock);
    return 0;
}