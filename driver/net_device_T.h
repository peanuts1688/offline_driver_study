struct net_device
{

        /*
         * This is the first field of the "visible" part of this structure
         * (i.e. as seen by users in the "Space.c" file).  It is the name
         * the interface.
         */
        char                    name[IFNAMSIZ];

        /*
         *      I/O specific fields
         *      FIXME: Merge these and struct ifmap into one
         */
        unsigned long           rmem_end;       /* shmem "recv" end     */
        unsigned long           rmem_start;     /* shmem "recv" start   */
        unsigned long           mem_end;        /* shared mem end       */
        unsigned long           mem_start;      /* shared mem start     */
        unsigned long           base_addr;      /* device I/O address   */
        unsigned int            irq;            /* device IRQ number    */

        /*
         *      Some hardware also needs these fields, but they are not
         *      part of the usual set specified in Space.c.
         */

        unsigned char           if_port;        /* Selectable AUI, TP,..*/
        unsigned char           dma;            /* DMA channel          */

        unsigned long           state;

        struct net_device       *next;
        
        /* The device initialization function. Called only once. */
        int                     (*init)(struct net_device *dev);

        /* ------- Fields preinitialized in Space.c finish here ------- */

        struct net_device       *next_sched;

        /* Interface index. Unique device identifier    */
        int                     ifindex;
        int                     iflink;


        struct net_device_stats* (*get_stats)(struct net_device *dev);
        struct iw_statistics*   (*get_wireless_stats)(struct net_device *dev);

        /* List of functions to handle Wireless Extensions (instead of ioctl).
         * See <net/iw_handler.h> for details. Jean II */
        struct iw_handler_def * wireless_handlers;

        struct ethtool_ops *ethtool_ops;

        /*
         * This marks the end of the "visible" part of the structure. All
         * fields hereafter are internal to the system, and may change at
         * will (read: may be cleaned up at will).
         */

        /* These may be needed for future network-power-down code. */
        unsigned long           trans_start;    /* Time (in jiffies) of last Tx */
        unsigned long           last_rx;        /* Time of last Rx      */

        unsigned short          flags;  /* interface flags (a la BSD)   */
        unsigned short          gflags;
        unsigned short          priv_flags; /* Like 'flags' but invisible to userspace. */
        unsigned short          unused_alignment_fixer; /* Because we need priv_flags,
                                                         * and we want to be 32-bit aligned.
                                                         */

        unsigned                mtu;    /* interface MTU value          */
        unsigned short          type;   /* interface hardware type      */
        unsigned short          hard_header_len;        /* hardware hdr length  */
        void                    *priv;  /* pointer to private data      */

        struct net_device       *master; /* Pointer to master device of a group,
                                          * which this device is member of.
                                          */

        /* Interface address info. */
        unsigned char           broadcast[MAX_ADDR_LEN];        /* hw bcast add */
        unsigned char           dev_addr[MAX_ADDR_LEN]; /* hw address   */
        unsigned char           addr_len;       /* hardware address length      */

        struct dev_mc_list      *mc_list;       /* Multicast mac addresses      */
        int                     mc_count;       /* Number of installed mcasts   */
        int                     promiscuity;
        int                     allmulti;

        int                     watchdog_timeo;
        struct timer_list       watchdog_timer;

        /* Protocol specific pointers */
        
        void                    *atalk_ptr;     /* AppleTalk link       */
        void                    *ip_ptr;        /* IPv4 specific data   */  
        void                    *dn_ptr;        /* DECnet specific data */
        void                    *ip6_ptr;       /* IPv6 specific data */
        void                    *ec_ptr;        /* Econet specific data */

        struct list_head        poll_list;      /* Link to poll list    */
        int                     quota;
        int                     weight;

        struct Qdisc            *qdisc;
        struct Qdisc            *qdisc_sleeping;
        struct Qdisc            *qdisc_list;
        struct Qdisc            *qdisc_ingress;
        unsigned long           tx_queue_len;   /* Max frames per queue allowed */

        /* hard_start_xmit synchronizer */
        spinlock_t              xmit_lock;
        /* cpu id of processor entered to hard_start_xmit or -1,
           if nobody entered there.
         */
        int                     xmit_lock_owner;
        /* device queue lock */
        spinlock_t              queue_lock;
        /* Number of references to this device */
        atomic_t                refcnt;
        /* The flag marking that device is unregistered, but held by an user */
        int                     deadbeaf;

        /* Net device features */
        int                     features;
#define NETIF_F_SG              1       /* Scatter/gather IO. */
#define NETIF_F_IP_CSUM         2       /* Can checksum only TCP/UDP over IPv4. */
#define NETIF_F_NO_CSUM         4       /* Does not require checksum. F.e. loopack. */
#define NETIF_F_HW_CSUM         8       /* Can checksum all the packets. */
#define NETIF_F_DYNALLOC        16      /* Self-dectructable device. */
#define NETIF_F_HIGHDMA         32      /* Can DMA to high memory. */
#define NETIF_F_FRAGLIST        64      /* Scatter/gather IO. */
#define NETIF_F_HW_VLAN_TX      128     /* Transmit VLAN hw acceleration */
#define NETIF_F_HW_VLAN_RX      256     /* Receive VLAN hw acceleration */
#define NETIF_F_HW_VLAN_FILTER  512     /* Receive filtering on VLAN */
#define NETIF_F_VLAN_CHALLENGED 1024    /* Device cannot handle VLAN packets */

        /* Called after device is detached from network. */
        void                    (*uninit)(struct net_device *dev);
        /* Called after last user reference disappears. */
        void                    (*destructor)(struct net_device *dev);

        /* Pointers to interface service routines.      */
        int                     (*open)(struct net_device *dev);
        int                     (*stop)(struct net_device *dev);
        int                     (*hard_start_xmit) (struct sk_buff *skb,
                                                    struct net_device *dev);
#define HAVE_NETDEV_POLL
        int                     (*poll) (struct net_device *dev, int *quota);
        int                     (*hard_header) (struct sk_buff *skb,
                                                struct net_device *dev,
                                                unsigned short type,
                                                void *daddr,
                                                void *saddr,
                                                unsigned len);
        int                     (*rebuild_header)(struct sk_buff *skb);
#define HAVE_MULTICAST                   
        void                    (*set_multicast_list)(struct net_device *dev);
#define HAVE_SET_MAC_ADDR                
        int                     (*set_mac_address)(struct net_device *dev,
                                                   void *addr);
#define HAVE_PRIVATE_IOCTL
        int                     (*do_ioctl)(struct net_device *dev,
                                            struct ifreq *ifr, int cmd);
#define HAVE_SET_CONFIG
        int                     (*set_config)(struct net_device *dev,
                                              struct ifmap *map);
#define HAVE_HEADER_CACHE
        int                     (*hard_header_cache)(struct neighbour *neigh,
                                                     struct hh_cache *hh);
        void                    (*header_cache_update)(struct hh_cache *hh,
                                                       struct net_device *dev,
                                                       unsigned char *  haddr);
#define HAVE_CHANGE_MTU
        int                     (*change_mtu)(struct net_device *dev, int new_mtu);

#define HAVE_TX_TIMEOUT
        void                    (*tx_timeout) (struct net_device *dev);

        void                    (*vlan_rx_register)(struct net_device *dev,
                                                    struct vlan_group *grp);
        void                    (*vlan_rx_add_vid)(struct net_device *dev,
                                                   unsigned short vid);
        void                    (*vlan_rx_kill_vid)(struct net_device *dev,
                                                    unsigned short vid);

        int                     (*hard_header_parse)(struct sk_buff *skb,
                                                     unsigned char *haddr);
        int                     (*neigh_setup)(struct net_device *dev, struct neigh_parms *);
        int                     (*accept_fastpath)(struct net_device *, struct dst_entry*);

        /* open/release and usage marking */
        struct module *owner;

        /* bridge stuff */
        struct net_bridge_port  *br_port;

#ifdef CONFIG_NET_FASTROUTE
#define NETDEV_FASTROUTE_HMASK 0xF
        /* Semi-private data. Keep it at the end of device struct. */
        rwlock_t                fastpath_lock;
        struct dst_entry        *fastpath[NETDEV_FASTROUTE_HMASK+1];
#endif
#ifdef CONFIG_NET_DIVERT
        /* this will get initialized at each interface type init routine */
        struct divert_blk       *divert;
#endif /* CONFIG_NET_DIVERT */
};
