
#include   "tx_api.h"
#include   "nx_api.h"
#ifdef __PRODUCT_NETXDUO__
#include   "nxd_dhcp_client.h"
#include   "nxd_dhcp_server.h"
#else
#include   "nx_dhcp.h"
#include   "nx_dhcp_server.h"
#endif
#include  "nx_ram_network_driver_test_1500.h"

extern void    test_control_return(UINT status);

#if !defined(NX_DISABLE_IPV4) && (NX_MAX_PHYSICAL_INTERFACES >= 2)

#define     DEMO_STACK_SIZE             4096
#define     NX_PACKET_SIZE              1536
#define     NX_PACKET_POOL_SIZE         NX_PACKET_SIZE * 8

#define     NX_DHCP_SERVER_IP_ADDRESS_0 IP_ADDRESS(1,0,0,1)
#define     NX_DHCP_SUBNET_MASK_0       IP_ADDRESS(255,255,255,0)

#define     NX_DHCP_SERVER_IP_ADDRESS_1 IP_ADDRESS(10,0,0,1)   
#define     START_IP_ADDRESS_LIST_1     IP_ADDRESS(10,0,0,10)
#define     END_IP_ADDRESS_LIST_1       IP_ADDRESS(10,0,0,19)

#define     NX_DHCP_SUBNET_MASK_1       IP_ADDRESS(255,255,255,0)
#define     NX_DHCP_DEFAULT_GATEWAY_1   IP_ADDRESS(10,0,0,1)
#define     NX_DHCP_DNS_SERVER_1        IP_ADDRESS(10,0,0,1)

#define     NX_DHCP_INTERFACE_INDEX     1


/* Define the ThreadX and NetX object control blocks...  */
static TX_THREAD               client_thread;

static TX_THREAD               server_thread;
static NX_PACKET_POOL          server_pool;
static NX_IP                   server_ip;
static NX_DHCP_SERVER          dhcp_server;

/* Define the counters used in the demo application...  */

static ULONG                   state_changes;
static ULONG                   error_counter;
static CHAR                    *pointer;
static CHAR                    offer_packet = NX_FALSE;
static CHAR                    ack_packet = NX_FALSE;
static CHAR                    test_done = NX_FALSE;

static UCHAR message[50] = "My Ping Request!" ;


/* Define thread prototypes.  */

static void    server_thread_entry(ULONG thread_input);
static void    client_thread_entry(ULONG thread_input);
extern UINT    (*advanced_packet_process_callback)(NX_IP *ip_ptr, NX_PACKET *packet_ptr, UINT *operation_ptr, UINT *delay_ptr);
static UINT    my_packet_process(NX_IP *ip_ptr, NX_PACKET *packet_ptr, UINT *operation_ptr, UINT *delay_ptr);

/******** Optionally substitute your Ethernet driver here. ***********/
extern void    _nx_ram_network_driver_1024(struct NX_IP_DRIVER_STRUCT *driver_req);
extern void    _nx_ram_network_driver_1500(struct NX_IP_DRIVER_STRUCT *driver_req);


/* Note that the MAC address of Client is 00:11:22:33:44:56.
   the MAC address of Server is 00:11:22:33:44:57.  */

/* Frame (342 bytes) */
static const unsigned char pkt1[342] = {
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x11, /* ........ */
0x22, 0x33, 0x44, 0x56, 0x08, 0x00, 0x45, 0x00, /* "3DV..E. */
0x01, 0x48, 0x00, 0x01, 0x40, 0x00, 0x80, 0x11, /* .H..@... */
0xf9, 0xa4, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, /* ........ */
0xff, 0xff, 0x00, 0x44, 0x00, 0x43, 0x01, 0x34, /* ...D.C.4 */
0x59, 0x2a, 0x01, 0x01, 0x06, 0x00, 0x22, 0x33, /* Y*...."3 */
0x44, 0x6e, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, /* Dn...... */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, /* ........ */
0x22, 0x33, 0x44, 0x56, 0x00, 0x00, 0x00, 0x00, /* "3DV.... */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x63, 0x82, /* ......c. */
0x53, 0x63, 0x35, 0x01, 0x01, 0x33, 0x04, 0xff, /* Sc5..3.. */
0xff, 0xff, 0xff, 0x0c, 0x0b, 0x64, 0x68, 0x63, /* .....dhc */
0x70, 0x5f, 0x63, 0x6c, 0x69, 0x65, 0x6e, 0x74, /* p_client */
0x37, 0x03, 0x01, 0x03, 0x06, 0xff, 0x00, 0x00, /* 7....... */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00              /* ...... */
};

/* Frame (342 bytes) */
static const unsigned char pkt2[342] = {
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x11, /* ........ */
0x22, 0x33, 0x44, 0x57, 0x08, 0x00, 0x45, 0x00, /* "3DW..E. */
0x01, 0x48, 0x00, 0x01, 0x40, 0x00, 0x80, 0x11, /* .H..@... */
0xef, 0xa3, 0x0a, 0x00, 0x00, 0x01, 0xff, 0xff, /* ........ */
0xff, 0xff, 0x00, 0x43, 0x00, 0x44, 0x01, 0x34, /* ...C.D.4 */
0xbd, 0x6f, 0x02, 0x01, 0x06, 0x00, 0x22, 0x33, /* .o...."3 */
0x44, 0x6e, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, /* Dn...... */
0x00, 0x00, 0x0a, 0x00, 0x00, 0x0a, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, /* ........ */
0x22, 0x33, 0x44, 0x56, 0x00, 0x00, 0x00, 0x00, /* "3DV.... */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x63, 0x82, /* ......c. */
0x53, 0x63, 0x35, 0x01, 0x02, 0x36, 0x04, 0x0a, /* Sc5..6.. */
0x00, 0x00, 0x01, 0x01, 0x04, 0xff, 0xff, 0xff, /* ........ */
0x00, 0x03, 0x04, 0x0a, 0x00, 0x00, 0x01, 0x06, /* ........ */
0x04, 0x0a, 0x00, 0x00, 0x01, 0x33, 0x04, 0x00, /* .....3.. */
0x00, 0x27, 0x10, 0x3a, 0x04, 0x00, 0x00, 0x13, /* .'.:.... */
0x88, 0x3b, 0x04, 0x00, 0x00, 0x22, 0x2e, 0xff, /* .;...".. */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00              /* ...... */
};

/* Frame (342 bytes) */
static const unsigned char pkt3[342] = {
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x11, /* ........ */
0x22, 0x33, 0x44, 0x56, 0x08, 0x00, 0x45, 0x00, /* "3DV..E. */
0x01, 0x48, 0x00, 0x02, 0x40, 0x00, 0x80, 0x11, /* .H..@... */
0xf9, 0xa3, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, /* ........ */
0xff, 0xff, 0x00, 0x44, 0x00, 0x43, 0x01, 0x34, /* ...D.C.4 */
0xdf, 0x49, 0x01, 0x01, 0x06, 0x00, 0x22, 0x33, /* .I...."3 */
0x44, 0x6e, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, /* Dn...... */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, /* ........ */
0x22, 0x33, 0x44, 0x56, 0x00, 0x00, 0x00, 0x00, /* "3DV.... */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x63, 0x82, /* ......c. */
0x53, 0x63, 0x35, 0x01, 0x03, 0x0c, 0x0b, 0x64, /* Sc5....d */
0x68, 0x63, 0x70, 0x5f, 0x63, 0x6c, 0x69, 0x65, /* hcp_clie */
0x6e, 0x74, 0x32, 0x04, 0x0a, 0x00, 0x00, 0x0a, /* nt2..... */
0x36, 0x04, 0x0a, 0x00, 0x00, 0x01, 0x37, 0x03, /* 6.....7. */
0x01, 0x03, 0x06, 0xff, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00              /* ...... */
};

/* Frame (342 bytes) */
static const unsigned char pkt4[342] = {
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x11, /* ........ */
0x22, 0x33, 0x44, 0x57, 0x08, 0x00, 0x45, 0x00, /* "3DW..E. */
0x01, 0x48, 0x00, 0x02, 0x40, 0x00, 0x80, 0x11, /* .H..@... */
0xef, 0xa2, 0x0a, 0x00, 0x00, 0x01, 0xff, 0xff, /* ........ */
0xff, 0xff, 0x00, 0x43, 0x00, 0x44, 0x01, 0x34, /* ...C.D.4 */
0xba, 0x6f, 0x02, 0x01, 0x06, 0x00, 0x22, 0x33, /* .o...."3 */
0x44, 0x6e, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, /* Dn...... */
0x00, 0x00, 0x0a, 0x00, 0x00, 0x0a, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, /* ........ */
0x22, 0x33, 0x44, 0x56, 0x00, 0x00, 0x00, 0x00, /* "3DV.... */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x63, 0x82, /* ......c. */
0x53, 0x63, 0x35, 0x01, 0x05, 0x36, 0x04, 0x0a, /* Sc5..6.. */
0x00, 0x00, 0x01, 0x01, 0x04, 0xff, 0xff, 0xff, /* ........ */
0x00, 0x03, 0x04, 0x0a, 0x00, 0x00, 0x01, 0x06, /* ........ */
0x04, 0x0a, 0x00, 0x00, 0x01, 0x33, 0x04, 0x00, /* .....3.. */
0x00, 0x27, 0x10, 0x3a, 0x04, 0x00, 0x00, 0x13, /* .'.:.... */
0x88, 0x3b, 0x04, 0x00, 0x00, 0x22, 0x2e, 0xff, /* .;...".. */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00              /* ...... */
};

/* Define what the initial system looks like.  */

#ifdef CTEST
VOID test_application_define(void *first_unused_memory)
#else
void    netx_dhcp_server_second_interface_test_application_define(void *first_unused_memory)
#endif
{

UINT    status;


    /* Setup the working pointer.  */
    pointer =  (CHAR *) first_unused_memory;

    /* Create the client thread.  */
    tx_thread_create(&client_thread, "thread client", client_thread_entry, 0,  
            pointer, DEMO_STACK_SIZE, 
            4, 4, TX_NO_TIME_SLICE, TX_AUTO_START);
    pointer =  pointer + DEMO_STACK_SIZE;

    /* Create the server thread.  */
    tx_thread_create(&server_thread, "thread server", server_thread_entry, 0,  
            pointer, DEMO_STACK_SIZE, 
            3, 3, TX_NO_TIME_SLICE, TX_AUTO_START);
    pointer =  pointer + DEMO_STACK_SIZE;

    /* Initialize the NetX system.  */
    nx_system_initialize();

    /* Create the server packet pool.  */
    status =  nx_packet_pool_create(&server_pool, "NetX Main Packet Pool", 1024, pointer, NX_PACKET_POOL_SIZE);
    pointer = pointer + NX_PACKET_POOL_SIZE;

    /* Check for pool creation error.  */
    if (status)
        error_counter++;

    /* Create an IP instance for the DHCP Server.  */
    status = nx_ip_create(&server_ip, "DHCP Server", NX_DHCP_SERVER_IP_ADDRESS_0, NX_DHCP_SUBNET_MASK_0, &server_pool, _nx_ram_network_driver_1024, pointer, 2048, 1);

    pointer =  pointer + 2048;

    /* Check for IP create errors.  */
    if (status)
        error_counter++;

    /* Attach the second interface. */
    status = nx_ip_interface_attach(&server_ip, "DHCP Server Secondary Interface", NX_DHCP_SERVER_IP_ADDRESS_1, NX_DHCP_SUBNET_MASK_1, _nx_ram_network_driver_1500);

    if (status)
    {
        error_counter++;
    }
    
    /* Enable ARP and supply ARP cache memory for DHCP Server IP.  */
    status =  nx_arp_enable(&server_ip, (void *) pointer, 1024);
    pointer = pointer + 1024;

    /* Check for ARP enable errors.  */
    if (status)
        error_counter++;
    
    /* Enable UDP traffic.  */
    status =  nx_udp_enable(&server_ip);

    /* Check for UDP enable errors.  */
    if (status)
        error_counter++;

    /* Enable ICMP.  */
    status =  nx_icmp_enable(&server_ip);

    /* Check for errors.  */
    if (status)
        error_counter++;

    return;
}

/* Define the test threads.  */

void    server_thread_entry(ULONG thread_input)
{

UINT        status;
UINT        addresses_added;

    NX_PARAMETER_NOT_USED(thread_input);

    printf("NetX Test:   NetX DHCP Server Second Interface Test....................");

    advanced_packet_process_callback = my_packet_process;

#ifdef __PRODUCT_NETXDUO__
    /* Update the MAC address.  */
    status = nx_ip_interface_physical_address_set(&server_ip, 0, 0x00000011, 0x22334455, NX_TRUE);
    status += nx_ip_interface_physical_address_set(&server_ip, 1, 0x00000011, 0x22334457, NX_TRUE);

    /* Check for errors. */
    if (status)
    {
        printf("ERROR!\n");
        test_control_return(1);
    }
#else
    server_ip.nx_ip_interface[0].nx_interface_physical_address_msw = 0x00000011;
    server_ip.nx_ip_interface[0].nx_interface_physical_address_lsw = 0x22334455;

    server_ip.nx_ip_interface[1].nx_interface_physical_address_msw = 0x00000011;
    server_ip.nx_ip_interface[1].nx_interface_physical_address_lsw = 0x22334457;

#endif

    /* Create the DHCP Server.  */
    status =  nx_dhcp_server_create(&dhcp_server, &server_ip, pointer, DEMO_STACK_SIZE, 
                                   "DHCP Server", &server_pool);
    
    pointer = pointer + DEMO_STACK_SIZE;
    
    /* Check for errors creating the DHCP Server. */
    if (status)
    {
        printf("ERROR!\n");
        test_control_return(1);
    }

    /* Load the assignable DHCP IP addresses.  */
    status = nx_dhcp_create_server_ip_address_list(&dhcp_server, NX_DHCP_INTERFACE_INDEX, START_IP_ADDRESS_LIST_1, 
                                                   END_IP_ADDRESS_LIST_1, &addresses_added);

    /* Check for errors creating the list. */
    if (status)
    {
        printf("ERROR!\n");
        test_control_return(1);
    }

    /* Verify all the addresses were added to the list. */
    if (addresses_added != 10)
    {
        printf("ERROR!\n");
        test_control_return(1);
    }

    status = nx_dhcp_set_interface_network_parameters(&dhcp_server, NX_DHCP_INTERFACE_INDEX, NX_DHCP_SUBNET_MASK_1, 
                                                      NX_DHCP_DEFAULT_GATEWAY_1, NX_DHCP_DNS_SERVER_1);

    /* Check for errors setting network parameters. */
    if (status)
    {
        printf("ERROR!\n");
        test_control_return(1);
    }

    /* Start DHCP Server task.  */
    status = nx_dhcp_server_start(&dhcp_server);

    /* Check for errors starting up the DHCP server.  */
    if (status)
    {
        printf("ERROR!\n");
        test_control_return(1);
    }

    tx_thread_sleep(20 * NX_IP_PERIODIC_RATE);

    if ((error_counter != 0) || (offer_packet != NX_TRUE) || (ack_packet != NX_TRUE))
    {
        printf("ERROR!\n");
        test_control_return(1);
    }
    else
    {
        printf("SUCCESS!\n");
        test_control_return(0);
    }

    return;
}


/* Define the test threads.  */

void    client_thread_entry(ULONG thread_input)
{

UINT        status;
NX_PACKET   *discover_packet;

    NX_PARAMETER_NOT_USED(thread_input);

    /* Allocate a Discover packet.  */
    status =  nx_packet_allocate(&server_pool, &discover_packet, NX_UDP_PACKET, NX_NO_WAIT);
    
    /* Check status.  */
    if (status)
    {
        error_counter++;
    }

    /* Append the data.  */
    status = nx_packet_data_append(discover_packet, (void *)&pkt1[14], (sizeof(pkt1) - 14), &server_pool, NX_NO_WAIT);

    if (status)
    {
        error_counter++;
    }

    /* Set the interface.  */
    discover_packet -> nx_packet_ip_interface = &server_ip.nx_ip_interface[1];

    /* Call API to receive the packet.  */
    _nx_ip_packet_receive(&server_ip, discover_packet);
}

static UINT    my_packet_process(NX_IP *ip_ptr, NX_PACKET *packet_ptr, UINT *operation_ptr, UINT *delay_ptr)
{
    
UINT        status;
NX_PACKET   *request_packet;

    NX_PARAMETER_NOT_USED(ip_ptr);
    NX_PARAMETER_NOT_USED(operation_ptr);
    NX_PARAMETER_NOT_USED(delay_ptr);

    /* Check the packet interface.  */
    if (packet_ptr -> nx_packet_ip_interface == &server_ip.nx_ip_interface[1])
    {

        /* Check the offer_packet flag.  */
        if (offer_packet == NX_FALSE)
        {

            /* Compare the packet.  */
            if ((packet_ptr -> nx_packet_length == sizeof(pkt2) - 14) &&
                (memcmp(packet_ptr -> nx_packet_prepend_ptr, &pkt2[14], packet_ptr -> nx_packet_length) == 0))
            {
                offer_packet = NX_TRUE;

                /* Allocate a Request packet.  */
                status =  nx_packet_allocate(&server_pool, &request_packet, NX_UDP_PACKET, NX_NO_WAIT);
    
                /* Check status.  */
                if (status)
                {
                    error_counter++;
                }

                /* Append the data.  */
                status = nx_packet_data_append(request_packet, (void *)&pkt3[14], sizeof(pkt3) - 14, &server_pool, NX_NO_WAIT);

                if (status)
                {
                    error_counter++;
                }

                /* Set the interface.  */
                request_packet -> nx_packet_ip_interface = &server_ip.nx_ip_interface[1];

                /* Call API to receive the packet.  */
                _nx_ip_packet_receive(&server_ip, request_packet);
            }
        }
        else
        {
            
            /* Compare the packet.  */
            if ((packet_ptr -> nx_packet_length == sizeof(pkt4) - 14) &&
                (memcmp(packet_ptr -> nx_packet_prepend_ptr, &pkt4[14], packet_ptr -> nx_packet_length) == 0))
            {
                ack_packet = NX_TRUE;
            }
        }
    }

    nx_packet_release(packet_ptr);
    return NX_FALSE;
}

#else

#ifdef CTEST
VOID test_application_define(void *first_unused_memory)
#else
void    netx_dhcp_server_second_interface_test_application_define(void *first_unused_memory)
#endif
{

    /* Print out test information banner.  */
    printf("NetX Test:   NetX DHCP Server Second Interface Test....................N/A\n"); 

    test_control_return(3);  
}      
#endif