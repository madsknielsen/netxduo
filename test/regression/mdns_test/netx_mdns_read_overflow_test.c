#include   "tx_api.h"
#include   "nx_api.h"
#include   "nx_ram_network_driver_test_1500.h"
             
extern void    test_control_return(UINT status);

#if defined __PRODUCT_NETXDUO__ && !defined NX_MDNS_DISABLE_CLIENT && !defined NX_DISABLE_IPV4
#include   "nxd_mdns.h"

#define     DEMO_STACK_SIZE    2048
#define     BUFFER_SIZE        10240
#define     LOCAL_FULL_SERVICE_COUNT    16
#define     PEER_FULL_SERVICE_COUNT     16
#define     PEER_PARTIAL_SERVICE_COUNT  32

/* Define the ThreadX and NetX object control blocks...  */

static TX_THREAD               ntest_0;

static NX_PACKET_POOL          pool_0;
static NX_IP                   ip_0;

/* Define the NetX MDNS object control blocks.  */

static NX_MDNS                 mdns_0;
static UCHAR                   type[256];
static UCHAR                   buffer[BUFFER_SIZE];
static ULONG                   current_buffer_size;

static CHAR invalid_data_1[] = {
0x01, 0x00, 0x5e, 0x00, 0x00, 0xfb, 0x00, 0x1e, /* ..^..... */
0x8f, 0xb1, 0x7a, 0xd4, 0x08, 0x00, 0x45, 0x00, /* ..z...E. */
0x00, 0x8b, 0x76, 0xbf, 0x00, 0x00, 0xff, 0x11, /* ..v..... */
0xa2, 0xfa, 0xc0, 0xa8, 0x00, 0x04, 0xe0, 0x00, /* ........ */
0x00, 0xfb, 0x14, 0xe9, 0x14, 0xe9, 0x00, 0x77, /* .......w */
0xe2, 0xa6, 0x00, 0x00, 0x84, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x01, 0x00, 0x00, 0x00, 0x03, 0x05, 0x5f, /* ......._ */
0x68, 0x74, 0x74, 0x70, 0x04, 0x5f, 0x74, 0x63, /* http._tc */
0x70, 0x05, 0x6c, 0x6f, 0x63, 0x61, 0x6c, 0x00, /* p.local. */
0x00, 0x0c, 0x00, 0x01, 0x00, 0x00, 0x11, 0x94, /* ........ */
0x00, 0x0f, 0x0c, 0x43, 0x61, 0x6e, 0x6f, 0x6e, /* ...Canon */
0x4d, 0x46, 0x34, 0x35, 0x30, 0x30, 0x77, 0xc0, /* MF4500w. */
0x0c, 0x06, 0x72, 0x6f, 0x75, 0x74, 0x65, 0x72, /* ..router */
0xc0, 0x17, 0x00, 0x01, 0x80, 0x01, 0x00, 0x00, /* ........ */
0x00, 0x78, 0x00, 0x04, 0xc0, 0xa8, 0x00, 0x04, /* .x...... */
0xc0, 0x28, 0x00, 0x21, 0x80, 0x01, 0x00, 0x00, /* .(.!.... */
0x00, 0x78, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, /* .x...... */
0x00, 0x50, 0xc0, 0x37, 0xc0, 0x6a, 0x00, 0x10, /* .P.7.(.. */
0x80, 0x01, 0x00, 0x00, 0x11, 0x94, 0x00, 0x01, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static CHAR invalid_data_2[] = {
0x01, 0x00, 0x5e, 0x00, 0x00, 0xfb, 0x00, 0x1e, /* ..^..... */
0x8f, 0xb1, 0x7a, 0xd4, 0x08, 0x00, 0x45, 0x00, /* ..z...E. */
0x00, 0x8b, 0x76, 0xbf, 0x00, 0x00, 0xff, 0x11, /* ..v..... */
0xa2, 0xfa, 0xc0, 0xa8, 0x00, 0x04, 0xe0, 0x00, /* ........ */
0x00, 0xfb, 0x14, 0xe9, 0x14, 0xe9, 0x00, 0x77, /* .......w */
0xe2, 0xa6, 0x00, 0x00, 0x84, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x01, 0x00, 0x00, 0x00, 0x03, 0x05, 0x5f, /* ......._ */
0x68, 0x74, 0x74, 0x70, 0x04, 0x5f, 0x74, 0x63, /* http._tc */
0x70, 0x05, 0x6c, 0x6f, 0x63, 0x61, 0x6c, 0x00, /* p.local. */
0x00, 0x0c, 0x00, 0x01, 0x00, 0x00, 0x11, 0x94, /* ........ */
0x00, 0x0f, 0x0c, 0x43, 0x61, 0x6e, 0x6f, 0x6e, /* ...Canon */
0x4d, 0x46, 0x34, 0x35, 0x30, 0x30, 0x77, 0xc0, /* MF4500w. */
0x0c, 0x06, 0x72, 0x6f, 0x75, 0x74, 0x65, 0x72, /* ..router */
0xc0, 0x17, 0x00, 0x01, 0x80, 0x01, 0x00, 0x00, /* ........ */
0x00, 0x78, 0x00, 0x04, 0xc0, 0xa8, 0x00, 0x04, /* .x...... */
0xc0, 0x28, 0x00, 0x21, 0x80, 0x01, 0x00, 0x00, /* .(.!.... */
0x00, 0x78, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, /* .x...... */
0x00, 0x50, 0xc0, 0x37, 0xc0, 0x6d, 0x00, 0x10, /* .P.7.(.. */
0x80, 0x01, 0x00, 0x00, 0x11, 0x94, 0x00, 0x01, /* ........ */
0x01, 0xc0, 0x28, 0x00                          /* . */
};

static CHAR invalid_data_3[] = {
0x01, 0x00, 0x5e, 0x00, 0x00, 0xfb, 0x00, 0x1e, /* ..^..... */
0x8f, 0xb1, 0x7a, 0xd4, 0x08, 0x00, 0x45, 0x00, /* ..z...E. */
0x00, 0x8b, 0x76, 0xbf, 0x00, 0x00, 0xff, 0x11, /* ..v..... */
0xa2, 0xfa, 0xc0, 0xa8, 0x00, 0x04, 0xe0, 0x00, /* ........ */
0x00, 0xfb, 0x14, 0xe9, 0x14, 0xe9, 0x00, 0x77, /* .......w */
0xe2, 0xa6, 0x00, 0x00, 0x84, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x01, 0x00, 0x00, 0x00, 0x03, 0x05, 0x5f, /* ......._ */
0x68, 0x74, 0x74, 0x70, 0x04, 0x5f, 0x74, 0x63, /* http._tc */
0x70, 0x05, 0x6c, 0x6f, 0x63, 0x61, 0x6c, 0x00, /* p.local. */
0x00, 0x0c, 0x00, 0x01, 0x00, 0x00, 0x11, 0x94, /* ........ */
0x00, 0x0f, 0x0c, 0x43, 0x61, 0x6e, 0x6f, 0x6e, /* ...Canon */
0x4d, 0x46, 0x34, 0x35, 0x30, 0x30, 0x77, 0xc0, /* MF4500w. */
0x0C, 0x06, 0x72, 0x6f, 0x75, 0x74, 0x65, 0x72, /* ..router */
0xc0, 0x17, 0x00, 0x01, 0x80, 0x01, 0x00, 0x00, /* ........ */
0x00, 0x78, 0x00, 0x04, 0xc0, 0xa8, 0x00, 0x04, /* .x...... */
0xc0, 0x28, 0x00, 0x21, 0x80, 0x01, 0x00, 0x00, /* .(.!.... */
0x00, 0x78, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, /* .x...... */
0x00, 0x50, 0xc0, 0x37, 0xc0, 0x6e, 0x00, 0x10, /* .P.7.(.. */
0x80, 0x01, 0x00, 0x00, 0x11, 0x94, 0x00, 0x01, /* ........ */
0xc0, 0x28, 0x00,                               /* . */
};

static unsigned char invalid_data_4[] = {
0x01, 0x00, 0x5e, 0x00, 0x00, 0xfb, 0x00, 0x1e, /* ..^..... */
0x8f, 0xb1, 0x7a, 0xd4, 0x08, 0x00, 0x45, 0x00, /* ..z...E. */
0x00, 0x8b, 0x76, 0xbf, 0x00, 0x00, 0xff, 0x11, /* ..v..... */
0xa2, 0xfa, 0xc0, 0xa8, 0x00, 0x04, 0xe0, 0x00, /* ........ */
0x00, 0xfb, 0x14, 0xe9, 0x14, 0xe9, 0x00, 0x77, /* .......w */
0xe2, 0xa6, 0x00, 0x00, 0x84, 0x00, 0x00, 0x00, /* FG...... */
0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x0b, 0x41, /* .......A */
0x52, 0x4d, 0x4d, 0x44, 0x4e, 0x53, 0x54, 0x65, /* RMMDNSTe */
0x73, 0x74, 0x05, 0x6c, 0x6f, 0x63, 0x61, 0x6c, /* st.local */
0x00, 0x00, 0x01, 0x80, 0x01, 0x00, 0x00, 0x00, /* ........ */
0x78, 0x00, 0x04, 0x0a, 0x00, 0x00, 0x42, 0x0b, /* x.....B. */
0x41, 0x52, 0x4d, 0x4d, 0x44, 0x4e, 0x53, 0x54, /* ARMMDNST */
0x65, 0x73, 0x74, 0x05, 0x6c, 0x6f, 0x63, 0x61, /* est.loca */
0x6c, 0x00, 0x00, 0x2f, 0x80, 0x01, 0x00, 0x00, /* l../.... */
0x00, 0x78, 0x00, 0x16, 0x0b, 0x41, 0x52, 0x4d, /* .x...ARM */
0x4d, 0x44, 0x4e, 0x53, 0x54, 0x65, 0x73, 0x74, /* MDNSTest */
0x05, 0x6c, 0x6f, 0x63, 0x61, 0x6c, 0x00, 0x00, /* .local.. */
0x01, 0x40, 0x0b, 0x41, 0x52, 0x4d, 0x4d, 0x44, /* .@.ARMMD */
0x4e, 0x53, 0x54, 0x65, 0x73, 0x74, 0x05, 0x5f, /* NSTest._ */
0x68, 0x74, 0x74, 0x70, 0x04, 0x5f, 0x74, 0x63, /* http._tc */
0x70, 0x05, 0x6c, 0x6f, 0x63, 0x61, 0x6c, 0x00, /* p.local. */
0x00, 0x21, 0x80, 0x01, 0x00, 0x00, 0x00, 0x64, /* .!.....d */
0x00, 0x19, 0x00, 0x00, 0x00, 0x00, 0x00, 0x50, /* .......P */
0x0b, 0x41, 0x52, 0x4d, 0x4d, 0x44, 0x4e, 0x53, /* .ARMMDNS */
0x54, 0x65, 0x73, 0x74, 0x05, 0x6c, 0x6f, 0x63, /* Test.loc */
0x61, 0x6c, 0x00, 0x0b, 0x41, 0x52, 0x4d, 0x4d, /* al..ARMM */
0x44, 0x4e, 0x53, 0x54, 0x65, 0x73, 0x74, 0x05, /* DNSTest. */
0x5f, 0x68, 0x74, 0x74, 0x70, 0x04, 0x5f, 0x74, /* _http._t */
0x63, 0x70, 0x05, 0x6c, 0x6f, 0x63, 0x61, 0x6c, /* cp.local */
0x00, 0x00, 0x10, 0x80, 0x01, 0x00, 0x00, 0x00, /* ........ */
0x64, 0x00, 0x1e, 0x08, 0x70, 0x61, 0x70, 0x65, /* d...pape */
0x72, 0x3d, 0x41, 0x34, 0x0a, 0x76, 0x65, 0x72, /* r=A4.ver */
0x73, 0x69, 0x6f, 0x6e, 0x3d, 0x30, 0x31, 0x0a, 0x76, 0x65, 0x72, /* r=A4.ver */
0x73, 0x69, 0x6f, 0x6e, 0x3d, 0x30, 0x31
};

static unsigned char invalid_data_5[] = {
0x01, 0x00, 0x5e, 0x00, 0x00, 0xfb, 0x00, 0x1e, /* ..^..... */
0x8f, 0xb1, 0x7a, 0xd4, 0x08, 0x00, 0x45, 0x00, /* ..z...E. */
0x00, 0x8b, 0x76, 0xbf, 0x00, 0x00, 0xff, 0x11, /* ..v..... */
0xa2, 0xfa, 0xc0, 0xa8, 0x00, 0x04, 0xe0, 0x00, /* ........ */
0x00, 0xfb, 0x14, 0xe9, 0x14, 0xe9, 0x00, 0x35, /* .......w */
0xe2, 0xa6, 0x00, 0x00, 0x84, 0x00, 0x00, 0x00, /* .x...... */
0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x0d, 0x41, /* .......A */
0x52, 0x4d, 0x4d, 0x44, 0x4e, 0x53, 0x54, 0x65, /* RMMDNSTe */
0x73, 0x74, 0x5f, 0x30, 0x05, 0x6c, 0x6f, 0x63, /* st_0.loc */
0x61, 0x6c, 0x00, 0x00, 0x01, 0x80, 0x01, 0x00, /* al...... */
0x00, 0x00, 0x78, 0x00, 0x04, 0x0a, 0x00, 0x00, /* ..x..... */
0x42                                            /* B */
};

static unsigned char invalid_data_6[] = {
0x01, 0x00, 0x5e, 0x00, 0x00, 0xfb, 0x00, 0x1e, /* ..^..... */
0x8f, 0xb1, 0x7a, 0xd4, 0x08, 0x00, 0x45, 0x00, /* ..z...E. */
0x00, 0x8b, 0x76, 0xbf, 0x00, 0x00, 0xff, 0x11, /* ..v..... */
0xa2, 0xfa, 0xc0, 0xa8, 0x00, 0x04, 0xe0, 0x00, /* ........ */
0x00, 0xfb, 0x14, 0xe9, 0x14, 0xe9, 0x00, 0x41, /* .......w */
0xe2, 0xa6, 0x00, 0x00, 0x84, 0x00, 0x00, 0x00, /* .x...... */
0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x0d, 0x41, /* .......A */
0x52, 0x4d, 0x4d, 0x44, 0x4e, 0x53, 0x54, 0x65, /* RMMDNSTe */
0x73, 0x74, 0x5f, 0x30, 0x05, 0x6c, 0x6f, 0x63, /* st_0.loc */
0x61, 0x6c, 0x00, 0x00, 0x1c, 0x80, 0x01, 0x00, 0x00, 0x00, 0x78, /* .......x */
0x00, 0x10, 0xfe, 0x80, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x02, 0x11, 0x22, 0xff, 0xfe, 0x33, /* ...."..3 */
0x44, 0x57, 
};

static CHAR mdns_data[] = {
0x01, 0x00, 0x5e, 0x00, 0x00, 0xfb, 0x00, 0x1e, /* ..^..... */
0x8f, 0xb1, 0x7a, 0xd4, 0x08, 0x00, 0x45, 0x00, /* ..z...E. */
0x00, 0x8b, 0x76, 0xbf, 0x00, 0x00, 0xff, 0x11, /* ..v..... */
0xa2, 0xfa, 0xc0, 0xa8, 0x00, 0x04, 0xe0, 0x00, /* ........ */
0x00, 0xfb, 0x14, 0xe9, 0x14, 0xe9, 0x00, 0x77, /* .......w */
0xe2, 0xa6, 0x00, 0x00, 0x84, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x01, 0x00, 0x00, 0x00, 0x03, 0x05, 0x5f, /* ......._ */
0x68, 0x74, 0x74, 0x70, 0x04, 0x5f, 0x74, 0x63, /* http._tc */
0x70, 0x05, 0x6c, 0x6f, 0x63, 0x61, 0x6c, 0x00, /* p.local. */
0x00, 0x0c, 0x00, 0x01, 0x00, 0x00, 0x11, 0x94, /* ........ */
0x00, 0x0f, 0x0c, 0x43, 0x61, 0x6e, 0x6f, 0x6e, /* ...Canon */
0x4d, 0x46, 0x34, 0x35, 0x30, 0x30, 0x77, 0xc0, /* MF4500w. */
0x0c, 0x06, 0x72, 0x6f, 0x75, 0x74, 0x65, 0x72, /* ..router */
0xc0, 0x17, 0x00, 0x01, 0x80, 0x01, 0x00, 0x00, /* ........ */
0x00, 0x78, 0x00, 0x04, 0xc0, 0xa8, 0x00, 0x04, /* .x...... */
0xc0, 0x28, 0x00, 0x21, 0x80, 0x01, 0x00, 0x00, /* .(.!.... */
0x00, 0x78, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, /* .x...... */
0x00, 0x50, 0xc0, 0x37, 0xc0, 0x28, 0x00, 0x10, /* .P.7.(.. */
0x80, 0x01, 0x00, 0x00, 0x11, 0x94, 0x00, 0x01, /* ........ */
0x00                                            /* . */
};


/* Define the counters used in the test application...  */

static ULONG                   error_counter;
static CHAR                   *pointer;

/* Define thread prototypes.  */

static void    ntest_0_entry(ULONG thread_input);
extern VOID    _nx_ram_network_driver_1500(NX_IP_DRIVER *driver_req_ptr);

/* Define what the initial system looks like.  */

#ifdef CTEST
VOID test_application_define(void *first_unused_memory)
#else
void           netx_mdns_read_overflow_test(void *first_unused_memory)
#endif
{

UINT       status;

    /* Setup the working pointer.  */
    pointer = (CHAR *) first_unused_memory;
    error_counter = 0;

    /* Initialize the NetX system.  */
    nx_system_initialize();

    /* Create a packet pool.  */
    status = nx_packet_pool_create(&pool_0, "NetX Main Packet Pool", 512, pointer, 8192);
    pointer = pointer + 8192;

    if(status)
        error_counter++;

    /* Create an IP instance.  */
    status = nx_ip_create(&ip_0, "NetX IP Instance 0", IP_ADDRESS(192,168,0,31), 0xFFFFFF00UL, &pool_0, 
                          _nx_ram_network_driver_1500, pointer, 2048, 1);
    pointer = pointer + 2048;

    /* Check for IP create errors.  */
    if (status)
        error_counter++;

    /* Enable ARP and supply ARP cache memory for IP Instance 0.  */
    status = nx_arp_enable(&ip_0, (void *) pointer, 1024);
    pointer = pointer + 1024;
    if(status)
        error_counter++;

    /* Enable UDP processing for both IP instances.  */
    status = nx_udp_enable(&ip_0);

    /* Check UDP enable status.  */
    if(status)
        error_counter++;

    /* Create the test thread.  */
    tx_thread_create(&ntest_0, "thread 0", ntest_0_entry, NX_NULL,  
                     pointer, DEMO_STACK_SIZE, 
                     3, 3, TX_NO_TIME_SLICE, TX_AUTO_START);
    pointer = pointer + DEMO_STACK_SIZE;
}

/* Define the test threads.  */

#define        MDNS_START_OFFSET (34)

static void    ntest_0_entry(ULONG thread_input)
{
UINT       status;
ULONG      actual_status;
NX_PACKET   *response_packet;
UCHAR       *invalid_responses[] = 
{
    invalid_data_1,
    invalid_data_2,
    invalid_data_3,
    invalid_data_4,
    invalid_data_5,
    invalid_data_6,
};
UINT       invalid_responses_len[] = 
{
    sizeof(invalid_data_1),
    sizeof(invalid_data_2),
    sizeof(invalid_data_3),
    sizeof(invalid_data_4),
    sizeof(invalid_data_5),
    sizeof(invalid_data_6),
};
UCHAR       *response_ptr;
UINT        response_len;
UINT        i;
UINT        test_num = sizeof(invalid_responses_len) / sizeof(UINT);
NX_IPV4_HEADER *ip_header_ptr;
UCHAR test_tail_len[] =
{
    15, 3, 2, 11, 2, 2,
};

    printf("NetX Test:   MDNS Read Overflow Test...................................");

    /* Ensure the IP instance has been initialized.  */
    status = nx_ip_status_check(&ip_0, NX_IP_INITIALIZE_DONE, &actual_status, 100);

    /* Check status. */
    if(status != NX_SUCCESS)
    {
        printf("ERROR!\n");
        test_control_return(1);
    }

    /* Create mDNS. */
    current_buffer_size = (BUFFER_SIZE >> 1);
    status = nx_mdns_create(&mdns_0, &ip_0, &pool_0, 2, pointer, DEMO_STACK_SIZE, "NETX-MDNS",  
                            buffer, current_buffer_size, buffer + current_buffer_size, current_buffer_size, NX_NULL);
    pointer = pointer + DEMO_STACK_SIZE;

    /* Check status. */
    if(status != NX_SUCCESS)
    {
        printf("ERROR!\n");
        test_control_return(1);
    }

    /* Enable mDNS.  */
    status = nx_mdns_enable(&mdns_0, 0);

    /* Check status. */
    if(status != NX_SUCCESS)
    {
        printf("ERROR!\n");
        test_control_return(1);
    }

    nx_udp_socket_checksum_disable(&(mdns_0.nx_mdns_socket));

    /* Send test packet.  */
    for (i = 0; i < test_num; i ++)
    {

        nx_mdns_peer_cache_clear(&mdns_0);

        /* Allocate a response packet.  */
        status =  nx_packet_allocate(&pool_0, &response_packet, NX_UDP_PACKET, TX_WAIT_FOREVER);
     
        /* Check status.  */
        if (status)
        {
            error_counter++;
            return;
        }

        response_ptr = invalid_responses[i];
        response_len = invalid_responses_len[i];

        /* Write the DNS response messages into the packet payload!  */
        memcpy(response_packet -> nx_packet_prepend_ptr, response_ptr, response_len);
        response_len -= test_tail_len[i];

        ip_header_ptr = (NX_IPV4_HEADER *)(response_packet -> nx_packet_prepend_ptr + 14);
        NX_CHANGE_ULONG_ENDIAN(ip_header_ptr -> nx_ip_header_word_0);
        NX_CHANGE_ULONG_ENDIAN(ip_header_ptr -> nx_ip_header_word_1);
        NX_CHANGE_ULONG_ENDIAN(ip_header_ptr -> nx_ip_header_word_2);
        NX_CHANGE_ULONG_ENDIAN(ip_header_ptr -> nx_ip_header_source_ip);
        NX_CHANGE_ULONG_ENDIAN(ip_header_ptr -> nx_ip_header_destination_ip);
        response_packet -> nx_packet_ip_header = response_packet -> nx_packet_prepend_ptr + 14;

        /* Adjust the write pointer.  */
        response_packet -> nx_packet_prepend_ptr += MDNS_START_OFFSET;
        response_packet -> nx_packet_length =  response_len - MDNS_START_OFFSET;
        response_packet -> nx_packet_append_ptr =  response_packet -> nx_packet_prepend_ptr + response_packet -> nx_packet_length;

        response_packet -> nx_packet_ip_interface = &ip_0.nx_ip_interface[0];

        /* Send the UDP packet with the correct port.  */
        _nx_udp_packet_receive(&ip_0, response_packet);

        tx_thread_sleep(NX_IP_PERIODIC_RATE);

        if ((i < 3 && mdns_0.nx_mdns_peer_rr_count == 0x04) ||
            (i == 3 && mdns_0.nx_mdns_peer_rr_count == 0x03) ||
            (i == 4 && mdns_0.nx_mdns_peer_rr_count == 0x01) ||
            (i == 5 && mdns_0.nx_mdns_peer_rr_count == 0x01))
        {
            error_counter++;
        }
    }

    /* Determine if the test was successful.  */
    if(error_counter)
    {
        printf("ERROR!\n");
        test_control_return(1);
    }
    else
    {
        printf("SUCCESS!\n");
        test_control_return(0);
    }

}
#else

#ifdef CTEST
VOID test_application_define(void *first_unused_memory)
#else
void           netx_mdns_read_overflow_test(void *first_unused_memory)
#endif
{
    printf("NetX Test:   MDNS Read Overflow Test...................................N/A\n"); 
    test_control_return(3);
}
#endif /* NX_MDNS_DISABLE_CLIENT  */ 