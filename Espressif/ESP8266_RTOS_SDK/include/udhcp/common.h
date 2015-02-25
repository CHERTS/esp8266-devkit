/* vi: set sw=4 ts=4: */
/*
 * Russ Dill <Russ.Dill@asu.edu> September 2001
 * Rewritten by Vladimir Oleynik <dzo@simtreas.ru> (C) 2003
 *
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */
#ifndef UDHCP_COMMON_H
#define UDHCP_COMMON_H 1

#include "esp_common.h"

#include "lwip/sockets.h"

//PUSH_AND_SET_FUNCTION_VISIBILITY_TO_HIDDEN

extern const uint8_t MAC_BCAST_ADDR[6]; /* six all-ones */

#ifndef BUFSIZ
# define BUFSIZ 1024
#endif

typedef int smallint;
typedef unsigned smalluint;

/* Providing hard guarantee on minimum size (think of BUFSIZ == 128) */
enum { COMMON_BUFSIZE = (BUFSIZ >= 256*sizeof(void*) ? BUFSIZ+1 : 256*sizeof(void*)) };
extern char bb_common_bufsiz1[COMMON_BUFSIZE];

/* In this form code with pipes is much more readable */
struct fd_pair { int rd; int wr; };

/* Having next pointer as a first member allows easy creation
 * of "llist-compatible" structs, and using llist_FOO functions
 * on them.
 */
typedef struct llist_t {
	struct llist_t *link;
	char *data;
} llist_t;

void llist_add_to(llist_t **old_head, void *data);
void llist_add_to_end(llist_t **list_head, void *data);
void *llist_pop(llist_t **elm);
void llist_unlink(llist_t **head, llist_t *elm);
void llist_free(llist_t *elm, void (*freeit)(void *data));
llist_t *llist_rev(llist_t *list);
llist_t *llist_find_str(llist_t *first, const char *str);

//unsigned long long monotonic_ns(void);
//unsigned long long monotonic_us(void);
//unsigned long long monotonic_ms(void);
unsigned monotonic_sec(void);

/* performs reasonably well (gcc usually inlines memcpy here) */
# define move_from_unaligned_int(v, intp) (memcpy(&(v), (intp), sizeof(int)))
# define move_from_unaligned_long(v, longp) (memcpy(&(v), (longp), sizeof(long)))
# define move_from_unaligned16(v, u16p) (memcpy(&(v), (u16p), 2))
# define move_from_unaligned32(v, u32p) (memcpy(&(v), (u32p), 4))
# define move_to_unaligned16(u16p, v) do { \
	uint16_t __t = (v); \
	memcpy((u16p), &__t, 2); \
} while (0)
# define move_to_unaligned32(u32p, v) do { \
	uint32_t __t = (v); \
	memcpy((u32p), &__t, 4); \
} while (0)

enum { wrote_pidfile = 0 };
#define write_pidfile(path)  ((void)0)
#define remove_pidfile(path) ((void)0)

#ifndef ENABLE_FEATURE_UDHCP_8021Q
#define ENABLE_FEATURE_UDHCP_8021Q 0
#endif

#ifndef ENABLE_FEATURE_UDHCP_RFC3397
#define ENABLE_FEATURE_UDHCP_RFC3397 0
#endif

#ifndef ENABLE_LONG_OPTS
#define ENABLE_LONG_OPTS 0
#endif

#ifndef ENABLE_FEATURE_UDHCPC_ARPING
#define ENABLE_FEATURE_UDHCPC_ARPING 0
#endif

#ifndef ENABLE_FEATURE_UDHCP_PORT
#define ENABLE_FEATURE_UDHCP_PORT 0
#endif

#ifndef ENABLE_FEATURE_UDHCPD_BASE_IP_ON_MAC
#define ENABLE_FEATURE_UDHCPD_BASE_IP_ON_MAC 0
#endif

#ifndef ENABLE_NOMMU
#define ENABLE_NOMMU 0
#endif

#ifndef ENABLE_FEATURE_IPV6
#define ENABLE_FEATURE_IPV6 0
#endif

#ifndef ENABLE_FEATURE_UDHCPD_WRITE_LEASES_EARLY
#define ENABLE_FEATURE_UDHCPD_WRITE_LEASES_EARLY 0
#endif

/*
 * For 0.9.29 and svn, __ARCH_USE_MMU__ indicates no-mmu reliably.
 * For earlier versions there is no reliable way to check if we are building
 * for a mmu-less system.
 */
#if ENABLE_NOMMU
# define BB_MMU 0
# define USE_FOR_NOMMU(...) __VA_ARGS__
# define USE_FOR_MMU(...)
#else
# define BB_MMU 1
# define USE_FOR_NOMMU(...)
# define USE_FOR_MMU(...) __VA_ARGS__
#endif

#define ARRAY_SIZE(x) ((unsigned)(sizeof(x) / sizeof((x)[0])))

/*** DHCP packet ***/

/* DHCP protocol. See RFC 2131 */
#define DHCP_MAGIC              0x63825363
#define DHCP_OPTIONS_BUFSIZE    308
#define BOOTREQUEST             1
#define BOOTREPLY               2

//TODO: rename ciaddr/yiaddr/chaddr
struct dhcp_packet {
	uint8_t op;      /* BOOTREQUEST or BOOTREPLY */
	uint8_t htype;   /* hardware address type. 1 = 10mb ethernet */
	uint8_t hlen;    /* hardware address length */
	uint8_t hops;    /* used by relay agents only */
	uint32_t xid;    /* unique id */
	uint16_t secs;   /* elapsed since client began acquisition/renewal */
	uint16_t flags;  /* only one flag so far: */
#define BROADCAST_FLAG 0x8000 /* "I need broadcast replies" */
	uint32_t ciaddr; /* client IP (if client is in BOUND, RENEW or REBINDING state) */
	uint32_t yiaddr; /* 'your' (client) IP address */
	/* IP address of next server to use in bootstrap, returned in DHCPOFFER, DHCPACK by server */
	uint32_t siaddr_nip;
	uint32_t gateway_nip; /* relay agent IP address */
	uint8_t chaddr[16];   /* link-layer client hardware address (MAC) */
	uint8_t sname[64];    /* server host name (ASCIZ) */
	uint8_t file[128];    /* boot file name (ASCIZ) */
	uint32_t cookie;      /* fixed first four option bytes (99,130,83,99 dec) */
	uint8_t options[DHCP_OPTIONS_BUFSIZE];// + CONFIG_UDHCPC_SLACK_FOR_BUGGY_SERVERS];
};

#define DHCP_PKT_SNAME_LEN      64
#define DHCP_PKT_FILE_LEN      128
#define DHCP_PKT_SNAME_LEN_STR "64"
#define DHCP_PKT_FILE_LEN_STR "128"

//struct ip_udp_dhcp_packet {
//	struct ip_hdr ip;
//	struct udp_hdr udp;
//	struct dhcp_packet data;
//};
//
//struct udp_dhcp_packet {
//	struct udp_hdr udp;
//	struct dhcp_packet data;
//};

enum {
//	IP_UDP_DHCP_SIZE = sizeof(struct ip_udp_dhcp_packet),// - CONFIG_UDHCPC_SLACK_FOR_BUGGY_SERVERS,
//	UDP_DHCP_SIZE    = sizeof(struct udp_dhcp_packet),// - CONFIG_UDHCPC_SLACK_FOR_BUGGY_SERVERS,
	DHCP_SIZE        = sizeof(struct dhcp_packet),// - CONFIG_UDHCPC_SLACK_FOR_BUGGY_SERVERS,
};

/* Let's see whether compiler understood us right */
//struct BUG_bad_sizeof_struct_ip_udp_dhcp_packet {
//	char c[IP_UDP_DHCP_SIZE == 576 ? 1 : -1];
//};


/*** Options ***/

enum {
	OPTION_IP = 1,
	OPTION_IP_PAIR,
	OPTION_STRING,
	/* Opts of STRING_HOST type will be sanitized before they are passed
	 * to udhcpc script's environment: */
	OPTION_STRING_HOST,
//	OPTION_BOOLEAN,
	OPTION_U8,
	OPTION_U16,
//	OPTION_S16,
	OPTION_U32,
	OPTION_S32,
	OPTION_BIN,
	OPTION_STATIC_ROUTES,
	OPTION_6RD,
#if ENABLE_FEATURE_UDHCP_RFC3397
	OPTION_DNS_STRING,  /* RFC1035 compressed domain name list */
	OPTION_SIP_SERVERS,
#endif

	OPTION_TYPE_MASK = 0x0f,
	/* Client requests this option by default */
	OPTION_REQ  = 0x10,
	/* There can be a list of 1 or more of these */
	OPTION_LIST = 0x20,
};

/* DHCP option codes (partial list). See RFC 2132 and
 * http://www.iana.org/assignments/bootp-dhcp-parameters/
 * Commented out options are handled by common option machinery,
 * uncommented ones have special cases (grep for them to see).
 */
#define DHCP_PADDING            0x00
#define DHCP_SUBNET             0x01
//#define DHCP_TIME_OFFSET      0x02 /* (localtime - UTC_time) in seconds. signed */
//#define DHCP_ROUTER           0x03
//#define DHCP_TIME_SERVER      0x04 /* RFC 868 time server (32-bit, 0 = 1.1.1900) */
//#define DHCP_NAME_SERVER      0x05 /* IEN 116 _really_ ancient kind of NS */
//#define DHCP_DNS_SERVER       0x06
//#define DHCP_LOG_SERVER       0x07 /* port 704 UDP log (not syslog)
//#define DHCP_COOKIE_SERVER    0x08 /* "quote of the day" server */
//#define DHCP_LPR_SERVER       0x09
#define DHCP_HOST_NAME          0x0c /* either client informs server or server gives name to client */
//#define DHCP_BOOT_SIZE        0x0d
//#define DHCP_DOMAIN_NAME      0x0f /* server gives domain suffix */
//#define DHCP_SWAP_SERVER      0x10
//#define DHCP_ROOT_PATH        0x11
//#define DHCP_IP_TTL           0x17
//#define DHCP_MTU              0x1a
//#define DHCP_BROADCAST        0x1c
//#define DHCP_ROUTES           0x21
//#define DHCP_NIS_DOMAIN       0x28
//#define DHCP_NIS_SERVER       0x29
//#define DHCP_NTP_SERVER       0x2a
//#define DHCP_WINS_SERVER      0x2c
#define DHCP_REQUESTED_IP       0x32 /* sent by client if specific IP is wanted */
#define DHCP_LEASE_TIME         0x33
#define DHCP_OPTION_OVERLOAD    0x34
#define DHCP_MESSAGE_TYPE       0x35
#define DHCP_SERVER_ID          0x36 /* by default server's IP */
#define DHCP_PARAM_REQ          0x37 /* list of options client wants */
//#define DHCP_ERR_MESSAGE      0x38 /* error message when sending NAK etc */
#define DHCP_MAX_SIZE           0x39
#define DHCP_VENDOR             0x3c /* client's vendor (a string) */
#define DHCP_CLIENT_ID          0x3d /* by default client's MAC addr, but may be arbitrarily long */
//#define DHCP_TFTP_SERVER_NAME 0x42 /* same as 'sname' field */
//#define DHCP_BOOT_FILE        0x43 /* same as 'file' field */
//#define DHCP_USER_CLASS       0x4d /* RFC 3004. set of LASCII strings. "I am a printer" etc */
#define DHCP_FQDN               0x51 /* client asks to update DNS to map its FQDN to its new IP */
//#define DHCP_DOMAIN_SEARCH    0x77 /* RFC 3397. set of ASCIZ string, DNS-style compressed */
//#define DHCP_SIP_SERVERS      0x78 /* RFC 3361. flag byte, then: 0: domain names, 1: IP addrs */
//#define DHCP_STATIC_ROUTES    0x79 /* RFC 3442. (mask,ip,router) tuples */
//#define DHCP_VLAN_ID          0x84 /* 802.1P VLAN ID */
//#define DHCP_VLAN_PRIORITY    0x85 /* 802.1Q VLAN priority */
//#define DHCP_PXE_CONF_FILE    0xd1 /* RFC 5071 Configuration File */
//#define DHCP_MS_STATIC_ROUTES 0xf9 /* Microsoft's pre-RFC 3442 code for 0x79? */
//#define DHCP_WPAD             0xfc /* MSIE's Web Proxy Autodiscovery Protocol */
#define DHCP_END                0xff

/* Offsets in option byte sequence */
#define OPT_CODE                0
#define OPT_LEN                 1
#define OPT_DATA                2
/* Bits in "overload" option */
#define OPTION_FIELD            0
#define FILE_FIELD              1
#define SNAME_FIELD             2

/* DHCP_MESSAGE_TYPE values */
#define DHCPDISCOVER            1 /* client -> server */
#define DHCPOFFER               2 /* client <- server */
#define DHCPREQUEST             3 /* client -> server */
#define DHCPDECLINE             4 /* client -> server */
#define DHCPACK                 5 /* client <- server */
#define DHCPNAK                 6 /* client <- server */
#define DHCPRELEASE             7 /* client -> server */
#define DHCPINFORM              8 /* client -> server */
#define DHCP_MINTYPE DHCPDISCOVER
#define DHCP_MAXTYPE DHCPINFORM

struct dhcp_optflag {
	uint8_t flags;
	uint8_t code;
};

struct option_set {
	uint8_t *data;
	struct option_set *next;
};

extern const struct dhcp_optflag dhcp_optflags[];
extern const char dhcp_option_strings[];
extern const uint8_t dhcp_option_lengths[];

unsigned udhcp_option_idx(const char *name);

uint8_t *udhcp_get_option(struct dhcp_packet *packet, int code);
int udhcp_end_option(uint8_t *optionptr);
void udhcp_add_binary_option(struct dhcp_packet *packet, uint8_t *addopt);
void udhcp_add_simple_option(struct dhcp_packet *packet, uint8_t code, uint32_t data);
#if ENABLE_FEATURE_UDHCP_RFC3397
char *dname_dec(const uint8_t *cstr, int clen, const char *pre);
uint8_t *dname_enc(const uint8_t *cstr, int clen, const char *src, int *retlen);
#endif
struct option_set *udhcp_find_option(struct option_set *opt_list, uint8_t code);


// RFC 2131  Table 5: Fields and options used by DHCP clients
//
// Fields 'hops', 'yiaddr', 'siaddr', 'giaddr' are always zero
//
// Field      DHCPDISCOVER          DHCPINFORM            DHCPREQUEST           DHCPDECLINE         DHCPRELEASE
// -----      ------------          ------------          -----------           -----------         -----------
// 'xid'      selected by client    selected by client    'xid' from server     selected by client  selected by client
//                                                        DHCPOFFER message
// 'secs'     0 or seconds since    0 or seconds since    0 or seconds since    0                   0
//            DHCP process started  DHCP process started  DHCP process started
// 'flags'    Set 'BROADCAST'       Set 'BROADCAST'       Set 'BROADCAST'       0                   0
//            flag if client        flag if client        flag if client
//            requires broadcast    requires broadcast    requires broadcast
//            reply                 reply                 reply
// 'ciaddr'   0                     client's IP           0 or client's IP      0                   client's IP
//                                                        (BOUND/RENEW/REBIND)
// 'chaddr'   client's MAC          client's MAC          client's MAC          client's MAC        client's MAC
// 'sname'    options or sname      options or sname      options or sname      (unused)            (unused)
// 'file'     options or file       options or file       options or file       (unused)            (unused)
// 'options'  options               options               options               message type opt    message type opt
//
// Option                     DHCPDISCOVER  DHCPINFORM  DHCPREQUEST      DHCPDECLINE  DHCPRELEASE
// ------                     ------------  ----------  -----------      -----------  -----------
// Requested IP address       MAY           MUST NOT    MUST (in         MUST         MUST NOT
//                                                      SELECTING or
//                                                      INIT-REBOOT)
//                                                      MUST NOT (in
//                                                      BOUND or
//                                                      RENEWING)
// IP address lease time      MAY           MUST NOT    MAY              MUST NOT     MUST NOT
// Use 'file'/'sname' fields  MAY           MAY         MAY              MAY          MAY
// Client identifier          MAY           MAY         MAY              MAY          MAY
// Vendor class identifier    MAY           MAY         MAY              MUST NOT     MUST NOT
// Server identifier          MUST NOT      MUST NOT    MUST (after      MUST         MUST
//                                                      SELECTING)
//                                                      MUST NOT (after
//                                                      INIT-REBOOT,
//                                                      BOUND, RENEWING
//                                                      or REBINDING)
// Parameter request list     MAY           MAY         MAY              MUST NOT     MUST NOT
// Maximum message size       MAY           MAY         MAY              MUST NOT     MUST NOT
// Message                    SHOULD NOT    SHOULD NOT  SHOULD NOT       SHOULD       SHOULD
// Site-specific              MAY           MAY         MAY              MUST NOT     MUST NOT
// All others                 MAY           MAY         MAY              MUST NOT     MUST NOT


/*** Logging ***/

#if defined CONFIG_UDHCP_DEBUG && CONFIG_UDHCP_DEBUG >= 1
# define IF_UDHCP_VERBOSE(...) __VA_ARGS__
extern unsigned dhcp_verbose;
# define log1(...) do { if (dhcp_verbose >= 1) bb_info_msg(__VA_ARGS__); } while (0)
# if CONFIG_UDHCP_DEBUG >= 2
void udhcp_dump_packet(struct dhcp_packet *packet) FAST_FUNC;
#  define log2(...) do { if (dhcp_verbose >= 2) bb_info_msg(__VA_ARGS__); } while (0)
# else
#  define udhcp_dump_packet(...) ((void)0)
#  define log2(...) ((void)0)
# endif
# if CONFIG_UDHCP_DEBUG >= 3
#  define log3(...) do { if (dhcp_verbose >= 3) bb_info_msg(__VA_ARGS__); } while (0)
# else
#  define log3(...) ((void)0)
# endif
#else
# define IF_UDHCP_VERBOSE(...)
# define udhcp_dump_packet(...) ((void)0)
# define log1(...) ((void)0)
# define log2(...) ((void)0)
# define log3(...) ((void)0)
#endif

//#define UDHCP_DBG

#ifdef UDHCP_DBG
#define UDHCP_DEBUG os_printf
#else
#define UDHCP_DEBUG
#endif

/*** Other shared functions ***/

/* 2nd param is "uint32_t*" */
int udhcp_str2nip(const char *str, void *arg);
/* 2nd param is "struct option_set**" */
int udhcp_str2optset(const char *str, void *arg);

void udhcp_init_header(struct dhcp_packet *packet, char type);

int udhcp_recv_kernel_packet(struct dhcp_packet *packet, int fd);

//int udhcp_send_raw_packet(struct dhcp_packet *dhcp_pkt,
//		uint32_t source_nip, int source_port,
//		uint32_t dest_nip, int dest_port, const uint8_t *dest_arp,
//		int ifindex);

int udhcp_send_kernel_packet(struct dhcp_packet *dhcp_pkt,
		uint32_t source_nip, int source_port,
		uint32_t dest_nip, int dest_port);

void udhcp_sp_setup(void);
int udhcp_sp_fd_set(fd_set *rfds, int extra_fd);
int udhcp_sp_read(const fd_set *rfds);

int udhcp_read_interface(const char *interface, int *ifindex, uint32_t *nip, uint8_t *mac);

int udhcp_listen_socket(/*uint32_t ip,*/ int port, const char *inf);

/* Returns 1 if no reply received */
int arpping(uint32_t test_nip,
		const uint8_t *safe_mac,
		uint32_t from_ip,
		uint8_t *from_mac,
		const char *interface);

/* note: ip is a pointer to an IPv6 in network order, possibly misaliged */
int sprint_nip6(char *dest, /*const char *pre,*/ const uint8_t *ip);

//POP_SAVED_FUNCTION_VISIBILITY

#endif
