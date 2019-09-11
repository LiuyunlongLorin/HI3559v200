#ifndef __WLAN_CONFIG_H__
#define __WLAN_CONFIG_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/*
 * Wireless Configuration commands are sent to the driver as driver direct
 * commands. As per NetX Reference Manual the command can be any number after
 * NX_LINK_USER_COMMAND
 * - Change this number if your system uses the same number with WLAN_COMMAND
 */
#if defined(THREADX)
#define NX_LINK_WLAN_COMMAND        (NX_LINK_USER_COMMAND + 10)
#elif defined(NUCLEOUS)
#define DEV_WLAN_CONFIG             (100)
#endif

extern int wpa_mode;

#define WC_MAC_ADDR_LEN             (6)
#define WC_MAX_RATES                (16)    /* Max no. of rates supported */

#define WCAPP_CMD_LINE_INTF_ENABLE  1
#define START_WPA_DEMO              1



#define WC_STATUS_SUCCESS           0
#define WC_STATUS_ERROR             -1

struct wc_param {
    void *buf;
    void *cb;               /* WPA Call Back -SD8786 Specific */
    unsigned int buf_len;
    unsigned int flags;
    unsigned int index;     /* Newly added for encode oper */
    unsigned int value;     /* Newly added for AuthMode oper */
};

struct wc_command {
    unsigned int cmd;       /* iwconfig cmd no. */
    struct wc_param iwcp;   /* param for iwconfig cmd */
};

/* iwconf cmd flags */
#define WC_FLAG_SET_CHANNEL         0x01    /* Set Using Channel number */
#define WC_FLAG_SET_FREQ            0x02    /* Set Using Frequency(MHz) */

#define WC_FLAG_TXPOW_AUTO          0x01    /* Tx Power Auto */
#define WC_FLAG_TXPOW_FIXED         0x02    /* Tx Power Fixed */
#define WC_FLAG_RADIO_OFF           0x04    /* Radio Off */
#define WC_FLAG_RADIO_ON            0x08    /* Radio On */

#define WC_FLAG_PS_DISABLE          0x01    /* Disable PS mode */
#define WC_FLAG_PS_ENABLE           0x02    /* Enable PS mode */

#define WC_FLAG_ENCODE_ENABLED      0x00
#define WC_FLAG_ENCODE_NOKEY        0x01
#define WC_FLAG_ENCODE_DISABLED     0x02
#define WC_FLAG_ENCODE_OPEN         0x04
#define WC_FLAG_ENCODE_RESTRICTED   0x08
#define WC_FLAG_ENCODE_TXKEY        0x10
#define WC_FLAG_ENCODE_WPA          0x20
#define WC_FLAG_ENCODE_RSN          0x40

#define WC_ESSID_MAX_SIZE           32      /* Max size of essid */
#define WC_MAX_ESSID_LEN            (WC_ESSID_MAX_SIZE + 1)  /* (+1) for '\0' termination */

#define WC_WPA_ESSID_LEN_MASK       (0x000000FF)
#define WC_WPA_ESSID_SHIFT          (0)
#define WC_WPA_PSK_LEN_MASK         (0x0000FF00)
#define WC_WPA_PSK_SHIFT            (8)
#define WC_WPA_PASSPHRASE_LEN_MASK  (0x00FF0000)
#define WC_WPA_PASSPHRASE_SHIFT     (16)

#define WC_WPA_ACTION_MASK          (0x0F)
#define WC_WPA_ESSID_PRESENT        (1 << 0)
#define WC_WPA_BSSID_PRESENT        (1 << 1)
#define WC_WPA_PSK_PRESENT          (1 << 2)
#define WC_WPA_PASSPHRASE_PRESENT   (1 << 3)
#define WC_WPA_CIPHER_TKIP          (1 << 4)
#define WC_WPA_CIPHER_AES           (1 << 5)
#define WC_WPA_MCIPHER_TKIP         (1 << 6)
#define WC_WPA_MCIPHER_AES          (1 << 7)
#define WC_WPA_VERSION_WPA          (1 << 8)
#define WC_WPA_VERSION_WPA2         (1 << 9)


#define WC_HTCAP_MASK               (0x7F)
#define WC_FLAG_HTCAP_ENABLE        (1 << 0)
#define WC_FLAG_HTCAP_40M           (1 << 1)
#define WC_FLAG_HTCAP_SHORTGI_20M   (1 << 2)
#define WC_FLAG_HTCAP_SHORTGI_40M   (1 << 3)
#define WC_FLAG_HTCAP_INTOL_40M     (1 << 4)
#define WC_FLAG_HTCAP_GREEN_FIELD   (1 << 5)
#define WC_FLAG_HTCAP_RX_STBC       (1 << 6)

/*
 * Bit 1: 20/40 Mhz Enable/Disable
 * Bit 4: Green Filed Enable/Disable
 * Bit 5: Short GI in 20 Mhz Enable/Disable
 * Bit 6: Short GI in 40 Mhz Enable/Disable
 *
 * Defualt Value is set to 0x20
 */

#define WC_FLAG_HTTXCFG_20_40M      (1 << 1)
#define WC_FLAG_HTTXCFG_GREEN_FIELD (1 << 4)
#define WC_FLAG_HTTXCFG_SHORTGI_20M (1 << 5)
#define WC_FLAG_HTTXCFG_SHORTGI_40M (1 << 6)

/*
 * Bit 8 : Enable/Disable 40Mhz Intolarent Bit
 * Bit 17: 20/40Mhz Enable/Disable
 * Bit 23: Short GI in 20 Mhz Enable/Disable
 * Bit 24: Short GI in 40 Mhz Enable/Disable
 * Bit 26: Rx STBC Support Enable/Disable
 * Bit 29: Green Field Enable/Disable
 *
 * Default Value is set to 0x4800000
 */
#define WC_FLAG_HTCAP_20_40M        (1 << 17)


/*Multicast Mode */
#define WC_PROMISC_MODE     1  /*Receive all packets */
#define WC_MULTICAST_MODE   2  /*Receive multicast packets in multicast list*/
#define WC_ALL_MULTI_MODE   4  /*Receive all multicast packets */

enum wep_index {
    WC_FLAG_ENCODE_INDEX_1=1,
    WC_FLAG_ENCODE_INDEX_2,
    WC_FLAG_ENCODE_INDEX_3,
    WC_FLAG_ENCODE_INDEX_4
};

typedef enum _wlan_mode {
    WC_FLAG_MODE_INFRA= 0,
    WC_FLAG_MODE_ADHOC,
    WC_FLAG_MODE_MASTER,
    WC_FLAG_MODE_AUTO
} wlan_mode;

typedef enum _wc_wlan_band {
    WC_FLAG_BAND_ANY = 0,
    WC_FLAG_BAND_11A_ONLY,
    WC_FLAG_BAND_11AG,
    WC_FLAG_BAND_11BG
} wc_wlan_band;

typedef enum _wc_wpa_config {
    WC_FLAG_GET_WPA =0,
    WC_FLAG_SET_WPA,
    WC_FLAG_CLEAR_WPA,
} wc_wpa_config;

typedef enum _wc_wps_mode {
    WC_FLAG_WPS_PIN = 0,
    WC_FLAG_WPS_PBC,
    WC_FLAG_WPS_AUTO
} wc_wps_mode;

/*
 * Config Methods
 * 0x0001 : USBA  : 0x0002 : Ethernet
 * 0x0004 : Label : 0x0008 : Display
 * 0x0010 : Ethernet NFC Token
 * 0x0020 : Integrated NFC Token
 * 0x0040 : NFC Interface
 * 0x0080 : Push Button
 * 0x0100 : Keypad
 */
#define WC_WPS_CONFIG_USBA              (1 << 0)
#define WC_WPS_CONFIG_ETHERNET          (1 << 1)
#define WC_WPS_CONFIG_LABEL             (1 << 2)
#define WC_WPS_CONFIG_DISPLAY           (1 << 3)
#define WC_WPS_CONFIG_ETHERNET_NFC      (1 << 4)
#define WC_WPS_CONFIG_INTEG_NFC         (1 << 5)
#define WC_WPS_CONFIG_NFC               (1 << 6)
#define WC_WPS_CONFIG_PBC               (1 << 7)
#define WC_WPS_CONFIG_KEYPAD            (1 << 8)

struct _CONFIG_PARAMS
{
    void *dev;
    char* (*InFunction)(char *buf);
    int (*OutFunction)(const char *buf,...);
};

typedef struct _CONFIG_PARAMS CONFIG_PARAMS, *PCONFIG_PARAMS;
int telnet_gets(char *buf);

/* Atheros Get Target Stats - Add any if needed */
struct target_stats {
    unsigned long long tx_packets;
    unsigned long long tx_errors;
    unsigned long long tx_failed_cnt;
    unsigned long long tx_retry_cnt;
    unsigned long long rx_packets;
    unsigned long long rx_errors;
    unsigned long long rx_crcerr;
    unsigned long long cs_bmiss_cnt;
    unsigned long long cs_aveBeacon_snr;
    unsigned long long cs_aveBeacon_rssi;
    unsigned long long tkip_format_errors;
    unsigned long long ccmp_format_errors;
    unsigned long long rx_decrypt_err;
};

/**************************************************************************/
/* WirelessLAN Configuration Commands */
enum wc_commands {
    WC_SET_FREQ=0,          /* Set Freq */
    WC_GET_FREQ,            /* Get Freq */
    WC_SET_MODE,            /* Set Mode */
    WC_GET_MODE,            /* Get Mode */
    WC_SET_AUTH,            /* Set authentication mode */
    WC_GET_AUTH,            /* Get authentication mode */
    WC_SET_BSSID,           /* Set AP MAC addr */
    WC_GET_BSSID,           /* Get Curr AP MAC addr */
    WC_SET_ESSID,           /* Set ESSID */
    WC_GET_ESSID,           /* Get Curr ESSID */
    WC_SET_RATE,            /* Set Data Rate 10 */
    WC_GET_RATE,            /* Get Curr Data Rate */
    WC_SET_RTS_THRES,       /* Set RTS Thres */
    WC_GET_RTS_THRES,       /* Get Curr RTS Thres */
    WC_SET_FRAG_THRES,      /* Set RTS Thres */
    WC_GET_FRAG_THRES,      /* Get Curr RTS Thres */
    WC_SET_TXPOW,           /* Set Tx Power */
    WC_GET_TXPOW,           /* Get Curr Tx Power */
    WC_SET_PS_MODE,         /* Set PS Mode */
    WC_GET_PS_MODE,         /* Get PS Mode  */
    WC_SET_SCAN,            /* Start wireless scan 20 */
    WC_GET_SCAN,            /* Get wireless scan info */
    WC_SET_ENCODE,          /* Set WEP keyinfo */
    WC_GET_ENCODE,          /* Get WEP keyinfo */
    WC_SET_COMMIT,          /* Set Commit      */
    WC_SET_RETRY_LIMIT,     /* Set Retry Limit */
    WC_GET_RETRY_LIMIT,     /* Get Retry Limit */
    WC_GET_SET_HTCAP_INFO,  /* Set/GET HTCAP INFO */ //11n Marvell Specific
    WC_GET_SET_HTTX_CFG,    /* Set/Get HTTX CFG   */ //11n Marvell Specific
    WC_GET_SET_WPA,         /* Set/Get WPA Passphrase */
    WC_GET_SUPP_MODE,       /* Get eSupp Mode - 30 */
    WC_GET_WIRELESS_STATS,  /* Get Wireless Statistics */
    WC_GET_OWN_MAC,         /* Get Cards Mac Address - WPS Support */
    WC_GET_EXT_SCAN,        /* Get and Store Scan Results */
    WC_SET_DEAUTH,
    WC_SET_WPS_SESSION,     /* Set WPS Session Enable/Disable */
    WC_SET_WPS_INIT,        /* Init/Deinit WPS Module */
    WC_SET_WPS_INFO,        /* Set vendor specific info for WPS */
    WC_SET_WPS_MODE,        /* Select WPS Mode */
    WC_SET_WPS_START,       /* Start/Stop the WPS module */
    WC_GET_WPS_PIN,         /* Get PIN number from WPS module -40 */
    WC_SET_RAWHANDLER,      /* Set Raw packet receive handler */
    WC_SET_MULTICAST_MODE,  /* Set Multicase Mode-All(4),Promi(1),M_LIST(2)*/
    WC_SET_ENCODE_EXT,      /* Install the WPA/RSN Keys*/
    WC_SET_AUTH_TYPE,       /* Set Auth Type */
    WC_GET_AUTH_TYPE,       /* Get Auth Type */
    WC_SET_MLME,            /* Set MLME */
    WC_SET_GENIE,           /* Set Generate IE */
    WC_SET_TCMD,            /* Set Test Command */
    WC_AR6003_EXT,          /* Set extended Command */
    WC_GET_TARGET_STATS,     /* Get Target Stats - 50 */
    WC_SET_COUNTRY_CODE,
    WC_GET_CHANNEL_LIST,
    WC_GET_SET_BAND,
    WC_DISCONNECT
}; /* End wc_commands */

/**************************************************************************/
/* RR: newly added - this struct will be used to contain
 * scan info of each BSS , & user get_scan callback will
 * be called with this argument
 */
typedef struct _wc_bss_info {
    unsigned int mode;              /* 0=ADHOC;1=INFRA(Default) */
    unsigned long channel;
    unsigned int privacy;
    unsigned int phy_mode;          /* 0x00=Auto(Default);0x01=11a; */
    unsigned int nrates;
    int rate_set[WC_MAX_RATES];     /* (in bps) */
    unsigned char bssid[WC_MAC_ADDR_LEN];   /* MAC Addr */
    unsigned char ssid[WC_MAX_ESSID_LEN];   /* SSID */
    unsigned int ssid_len;                  /* SSID LEN */
    unsigned int wpa_ie_len;                /* WPA IE Length */
//    unsigned char p2p_ie[256];
    unsigned char wps_ie[256];              /* WPS IE */
    unsigned char wpa_ie[256],rsn_ie[256];  /*WPA IE and RSN IE */
    int wps_ie_len;                         /* WPS IE length */
//    int p2p_ie_len;
    unsigned int rsn_ie_len;                /* RSN IE Length */
    unsigned short beacon_period;           /* Beacon Period */
    int level;                              /* RSSI - Signal Level */
    int noise;                              /* Noise Level */
    unsigned int qual;                      /* LQ */
    unsigned char ap_num;                   /* AP List-Count*/
    unsigned char txpower;                  /* Tx Power */
    unsigned int rts;                       /* RTS Threshold */
    unsigned int frag;                      /* Frag Threshold */
    unsigned char pwr_mgmt;                 /* Power Management */
    int retry_limit;                        /* Retry Limit */
} wc_bss_info;



enum wowfiltertype
{
SRC_IP=0x1,
DST_IP,
SRC_MAC,
DST_MAC,
KNOWN_PATTERN
};

#define WM_WLAN_MAX_NETWORK_DESC_NUM  64

#define SCAN_AP_LIMIT          WM_WLAN_MAX_NETWORK_DESC_NUM

//#define SCAN_AP_LIMIT          (32)
struct ar6003_driver_data;

typedef struct _wc_wps_info {
    unsigned char uuid[16];
    char device_name[33];
    char manufacture[65];
    char model_name[33];
    char model_number[32];
    char serial_number[33];
    char device_type[8];
    unsigned int os_version;
    unsigned short config_methods;
} wc_wps_info;

/** Return type: failure */
#define WC_WPS_SUCCESS      (0)
#define WC_WPS_FAIL         (-1)
#define WC_WPS_OVERLAP      (-2)
#define WC_WPS_TIMEOUT      (-3)
#define WC_WPS_ABORT        (-4)
#define WC_WPS_AUTHFAIL     (-5)

typedef void (*wc_wps_cb)(int status);
typedef void (*wc_scan_cb)(wc_bss_info *);
typedef void (*wc_set_scan_cb)(void);
typedef void (*wc_print_wpa_cb)(char *,int);

int user_gets(char *buf);

wc_bss_info *wlan_get_bss_info_buffer(void);
void sd_CmdLineConfigThread(unsigned long pContext);

int wc_set_country_code(void *devptr, char *code);
int wc_set_country_code_hex(void *devptr, int code);
int wc_get_channel_list(void *devptr, int numChan, unsigned short *chanList);

int wc_set_essid(void *devptr, char *essid, int ssidlen);
int wc_get_essid(void *devptr, char *essid, int *ssidlen);

int wc_get_freq(void *devptr, unsigned int *val);
int wc_set_freq(void *devptr, unsigned int f_f_or_c, unsigned int val);

int wc_get_mode(void *devptr, unsigned int *mode);
int wc_set_mode(void *devptr, unsigned int mode);

int wc_get_scan(void *devptr, wc_scan_cb callback);
int wc_set_scan(void *devptr);
int wc_set_scan_ssid(void *devptr, char *ssid, int length);
int wc_site_survey(void *devptr, wc_bss_info *results, int max_bss);
int wc_get_scan_results(void *devptr, void *results, int max_bss, int filter);

int wc_set_rate(void *devptr, int rate);
int wc_get_rate(void *devptr, int *rate);

int wc_set_rts(void *devptr, int rts_t);
int wc_get_rts(void *devptr, int *rts_t);

int wc_set_txpow(void *devptr, int pow_dbm, unsigned int pow_flags);
int wc_get_txpow(void *devptr, int *pow_dbm);

int wc_get_encode(void *devptr, char *key, int *keylen, int *index,
        unsigned int *flags);
int wc_set_encode(void *devptr, char *key, int keylen, int index,
        unsigned int flags);

int wc_get_htcap_info(void *devptr, int *data);
int wc_set_htcap_info(void *devptr, int data);

int wc_set_httxcfg(void *devptr, int data);
int wc_get_httxcfg(void *devptr, int *data);

int wc_set_psmode(void *devptr, int pwr_mgmt);
int wc_get_psmode(void *devptr, int *pwr_mgmt);

int wc_set_commit(void *devptr);

int wc_set_retry_limit(void *devptr, int retry_limit);
int wc_get_retry_limit(void *devptr, int *retry_limit);

int wc_set_auth(void *devptr,unsigned int authmode, int value);
int wc_get_auth(void *devptr,unsigned int *authmode);

int wc_set_frag(void *devptr, int rts_t);
int wc_get_frag(void *devptr, int *rts_t);

int wc_set_bssid(void *devptr, char *bssid);
int wc_get_bssid(void *devptr, char *bssid);

int wc_set_deauth(void *devptr);
int wc_wps_enable_session(void *devptr, int enable);
int wc_get_own_mac(void *devptr, char *own_mac);
int wc_get_supp_mode(void *devptr, int *mode, int *pw, int *gw);
int wc_get_wireless_stats(void *devptr, int *quality,
        int *rssi, int *noise);
int wc_set_arp_offload(void *devptr,unsigned long ipAddr);
int wc_config_wpa (void *devptr, char *essid, char *bssid, char *passphrase,
        char *psk);
int wc_ar6003_ext(void *devptr, unsigned int cmd, void *buf);
int wc_wps_init(void *devptr, unsigned int enable);
int wc_wps_connect(void *devptr, char *essid, int essidlen, wc_wps_cb wps_cb);
int wc_wps_disconnect(void *devptr);
int wc_wps_setmode(void *devptr, wc_wps_mode mode);
int wc_wps_setinfo(void *devptr, wc_wps_info *info);
int wc_wps_getpin(void *devptr, unsigned char *pin);
int wc_set_rawhandler(void *devptr, void *callback, void *callback_data);
int wc_set_multicast_mode(void *devptr, int flgas);
int wc_set_encodeext(void *devptr, void *ext, int index, int flags);
int wc_set_mlme(void *devptr,int cmd, int rcode, char *addr);
int wc_set_genie(void *devptr,unsigned char *ie,int length);
int wc_set_host_sleep_enable(void *devptr,int enable);

int wc_set_wow_mode_enable(void *devptr,int enable);

int wc_get_wow_list(void *devptr,int wow_id);


int wc_wow_add_pattern(void *devptr,int index, int offset, unsigned char *argPtr, int argLength);

int wc_wow_delete_pattern(void *devptr,int id);

int wc_get_station_count(void *devptr,char count);
int wc_test_cmd(void *devptr, char *buf, int len);
int wc_ar6002_ext(void *devptr, unsigned int cmd, void *buf);
int wc_get_target_stats(void *devptr, struct target_stats *stats);

int wc_set_band(void *devptr, unsigned int band);
int wc_get_band(void *devptr, unsigned int *band);

int wc_disconnect(void *devptr);

int wireless_init_event(void);

int wireless_send_event(int cmd,char *buf,unsigned int length);
extern int wc_enable_multicast_mode(void *devptr,char enable);
extern int wc_wow_add_multicast(void *devptr,int filter,int *mac);

enum SecurityType
{
    eOPEN=1,
    eWEP,
    eWPA,
    eWPA2,
};


#define IWEVCUSTOM          0x8C02      /* Driver specific ascii string */
#define IWEVASSOCREQIE      0x8C07      /* IEs used in (Re) Assoc Request */
#define IWEVASSOCRESPIE     0x8C08      /* IEs used in (Re) Assoc Response */
#define IWEVREGISTERED      0x8C03
#define IWEVEXPIRED         0x8C04


/*
 * Wireless Flags Defininations
 */
#define WC_CUSTOM_MAX        256    /* In bytes */

/* MLME requests (SIOCSIWMLME / struct iw_mlme) */
#define WC_MLME_DEAUTH        0
#define WC_MLME_DISASSOC    1
#define WC_MLME_AUTH        2
#define WC_MLME_ASSOC        3

/* SIOCSIWAUTH/SIOCGIWAUTH struct iw_param flags */
#define WC_AUTH_INDEX        0x0FFF
#define WC_AUTH_FLAGS        0xF000

/* SIOCSIWAUTH/SIOCGIWAUTH parameters (0 .. 4095)
 * (IW_AUTH_INDEX mask in struct iw_param flags; this is the index of the
 * parameter that is being set/get to; value will be read/written to
 * struct iw_param value field) */
#define WC_AUTH_WPA_VERSION                0
#define WC_AUTH_CIPHER_PAIRWISE            1
#define WC_AUTH_CIPHER_GROUP            2
#define WC_AUTH_KEY_MGMT                3
#define WC_AUTH_TKIP_COUNTERMEASURES    4
#define WC_AUTH_DROP_UNENCRYPTED        5
#define WC_AUTH_80211_AUTH_ALG            6
#define WC_AUTH_WPA_ENABLED                7
#define WC_AUTH_RX_UNENCRYPTED_EAPOL    8
#define WC_AUTH_ROAMING_CONTROL            9
#define WC_AUTH_PRIVACY_INVOKED            10
#define WC_AUTH_CIPHER_GROUP_MGMT        11
#define WC_AUTH_MFP                        12

/* IW_AUTH_WPA_VERSION values (bit field) */
#define WC_AUTH_WPA_VERSION_DISABLED    0x00000001
#define WC_AUTH_WPA_VERSION_WPA            0x00000002
#define WC_AUTH_WPA_VERSION_WPA2        0x00000004

/* IW_AUTH_PAIRWISE_CIPHER and IW_AUTH_GROUP_CIPHER values (bit field) */
#define WC_AUTH_CIPHER_NONE        0x00000001
#define WC_AUTH_CIPHER_WEP40    0x00000002
#define WC_AUTH_CIPHER_TKIP        0x00000004
#define WC_AUTH_CIPHER_CCMP        0x00000008
#define WC_AUTH_CIPHER_WEP104    0x00000010

/* IW_AUTH_KEY_MGMT values (bit field) */
#define WC_AUTH_KEY_MGMT_802_1X    1
#define WC_AUTH_KEY_MGMT_PSK    2

/* IW_AUTH_80211_AUTH_ALG values (bit field) */
#define WC_AUTH_ALG_OPEN_SYSTEM    0x00000001
#define WC_AUTH_ALG_SHARED_KEY    0x00000002
#define WC_AUTH_ALG_LEAP        0x00000004

/* IW_AUTH_ROAMING_CONTROL values */
#define WC_AUTH_ROAMING_ENABLE    0    /* driver/firmware based roaming */
#define WC_AUTH_ROAMING_DISABLE    1    /* user space program used for roaming
                     * control */
/* struct iw_encode_ext ->ext_flags */
#define WC_ENCODE_EXT_TX_SEQ_VALID    0x00000001
#define WC_ENCODE_EXT_RX_SEQ_VALID    0x00000002
#define WC_ENCODE_EXT_GROUP_KEY        0x00000004
#define WC_ENCODE_EXT_SET_TX_KEY    0x00000008

/* SIOCSIWENCODEEXT definitions */
#define WC_ENCODE_SEQ_MAX_SIZE    8
/* struct wc_encode_ext ->alg */
#define WC_ENCODE_ALG_NONE    0
#define WC_ENCODE_ALG_WEP    1
#define WC_ENCODE_ALG_TKIP    2
#define WC_ENCODE_ALG_CCMP    3
#define WC_ENCODE_ALG_PMK    4
#define WC_ENCODE_ALG_AES_CMAC    5

/* Flags for encoding (along with the token) */
#define WC_ENCODE_INDEX        0x00FF    /* Token index (if needed) */
#define WC_ENCODE_FLAGS        0xFF00    /* Flags defined below */
#define WC_ENCODE_MODE        0xF000    /* Modes defined below */
#define WC_ENCODE_DISABLED    0x8000    /* Encoding disabled */
#define WC_ENCODE_ENABLED    0x0000    /* Encoding enabled */
#define WC_ENCODE_RESTRICTED    0x4000    /* Refuse non-encoded packets */
#define WC_ENCODE_OPEN        0x2000    /* Accept non-encoded packets */
#define WC_ENCODE_NOKEY        0x0800  /* Key is write only, so not present */
#define WC_ENCODE_TEMP        0x0400  /* Temporary key */

struct wc_encode_ext
{
    unsigned int ext_flags;
    unsigned char tx_seq[WC_ENCODE_SEQ_MAX_SIZE];
    unsigned char rx_seq[WC_ENCODE_SEQ_MAX_SIZE];
    unsigned char addr[6];
    unsigned short alg;
    unsigned short key_len;
    unsigned char key[64];
};

typedef struct _hostapd_conf {
    unsigned char  bssid[6];
    char ssid[WC_MAX_ESSID_LEN];
    unsigned char  channel_num;
    int wpa_key_mgmt;
    int wpa_pairwise;
    unsigned char key[100];
    unsigned char macaddr_acl;
    int auth_algs;
    unsigned char  wep_idx;
    int wpa;
    /*WPS Related */
} hostapd_conf;

int wpa_supplicant_start(void *devptr);
int wpa_supplicant_stop(void *devptr);
int wpa_cli_connect(unsigned int auth_mode, unsigned int encryption,
                    char *ssid, char *key);
int wpa_cli_disconnect(void);

int hostapd_start(void *devptr);
int hostapd_stop(void *devptr);
void configure_hostapd(hostapd_conf *hconf);

int ar6003_driver_init(int testMode);


#define WPA_CIPHER_NONE BIT(0)
#define WPA_CIPHER_WEP40 BIT(1)
#define WPA_CIPHER_WEP104 BIT(2)
#define WPA_CIPHER_TKIP BIT(3)
#define WPA_CIPHER_CCMP BIT(4)
#define WPA_CIPHER_AES_128_CMAC BIT(5)
#define WPA_CIPHER_GCMP BIT(6)
#define WPA_CIPHER_SMS4 BIT(7)
#define WPA_CIPHER_GCMP_256 BIT(8)
#define WPA_CIPHER_CCMP_256 BIT(9)
#define WPA_CIPHER_BIP_GMAC_128 BIT(11)
#define WPA_CIPHER_BIP_GMAC_256 BIT(12)
#define WPA_CIPHER_BIP_CMAC_256 BIT(13)
#define WPA_CIPHER_GTK_NOT_USED BIT(14)

#define WPA_KEY_MGMT_IEEE8021X BIT(0)
#define WPA_KEY_MGMT_PSK BIT(1)
#define WPA_KEY_MGMT_NONE BIT(2)
#define WPA_KEY_MGMT_IEEE8021X_NO_WPA BIT(3)
#define WPA_KEY_MGMT_WPA_NONE BIT(4)
#define WPA_KEY_MGMT_FT_IEEE8021X BIT(5)
#define WPA_KEY_MGMT_FT_PSK BIT(6)
#define WPA_KEY_MGMT_IEEE8021X_SHA256 BIT(7)
#define WPA_KEY_MGMT_PSK_SHA256 BIT(8)
#define WPA_KEY_MGMT_WPS BIT(9)
#define WPA_KEY_MGMT_SAE BIT(10)
#define WPA_KEY_MGMT_FT_SAE BIT(11)
#define WPA_KEY_MGMT_WAPI_PSK BIT(12)
#define WPA_KEY_MGMT_WAPI_CERT BIT(13)
#define WPA_KEY_MGMT_CCKM BIT(14)
#define WPA_KEY_MGMT_OSEN BIT(15)


void init_ar6k3_network(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif  /* __WLAN_CONFIG_H__ */
