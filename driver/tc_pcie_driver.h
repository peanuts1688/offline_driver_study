#ifndef _TC_PCIE_H
#define _TC_PCIE_H

#define TC_PCIE_NUM_BARS (1)
// #define TC_PCIE_DESCRIPTOR_NUM 127

// #define TC_PCIE_WR_DMA_CTRL          0x0000
// #define TC_PCIE_WR_DESC_STATUS       0x0004
// #define TC_PCIE_WR_RC_DESC_BASE_LOW  0x0008
// #define TC_PCIE_WR_RC_DESC_BASE_HIGH 0x000C
// #define TC_PCIE_WR_LAST_DESC_IDX     0x0010
// #define TC_PCIE_WR_EP_DESC_BASE_LOW  0x0014
// #define TC_PCIE_WR_EP_DESC_BASE_HIGH 0x0018
// #define TC_PCIE_WR_DMA_PERF          0x001C

// #define TC_PCIE_RD_DMA_CTRL          0x0100
// #define TC_PCIE_RD_DESC_STATUS       0x0104
// #define TC_PCIE_RD_RC_DESC_BASE_LOW  0x0108
// #define TC_PCIE_RD_RC_DESC_BASE_HIGH 0x010C
// #define TC_PCIE_RD_LAST_DESC_IDX     0x0110
// #define TC_PCIE_RD_EP_DESC_BASE_LOW  0x0114
// #define TC_PCIE_RD_EP_DESC_BASE_HIGH 0x0118
// #define TC_PCIE_RD_DMA_PERF          0x011C

// #define TC_PCIE_NUM_DWORDS           0x1000

// #define ONCHIP_MEM_BASE                 0x0000
// #define ONCHIP_MEM_DESC_MEM_BASE        0x8000

// #define DESC_CTRLLER_BASE               0x8000

// #define CTL_STS_BITS                    0x0100

// #define TIMEOUT_THRESH                  0xFFFF

// struct dma_descriptor {
//     u32 src_addr_ldw;
//     u32 src_addr_udw;
//     u32 dest_addr_ldw;
//     u32 dest_addr_udw;
//     u32 ctl_dma_len;
//     u32 reserved[3];
// } __attribute__ ((packed));

// struct dma_header {
//     u32 eplast;
//     u32 reserved[7];
// } __attribute__ ((packed));

// struct dma_desc_table {
//     struct dma_header header;
//     struct dma_descriptor descriptors[TC_PCIE_DESCRIPTOR_NUM];
// } __attribute__ ((packed));

struct tc_pcie_bookkeep {
    struct pci_dev *pci_dev;

    u8 revision;
    u8 irq_pin;
    char msi_enabled;
    u8 irq_line;
    char dma_capable;

    void * __iomem bar[TC_PCIE_NUM_BARS];
    size_t bar_length[TC_PCIE_NUM_BARS];

//     struct dma_desc_table *table_rd_cpu_virt_addr;
//     struct dma_desc_table *table_wr_cpu_virt_addr;
//     dma_addr_t table_rd_bus_addr;
//     dma_addr_t table_wr_bus_addr;

    dev_t cdevno;
    struct cdev cdev;

    unsigned int *buffer_virt[TC_KMEM_COUNT];
    dma_addr_t buffer_bus[TC_KMEM_COUNT];
};

//     int user_pid;
//     struct task_struct *user_task;

//     int tc_pcie_num_dwords;
//     int tc_pcie_descriptor_num;

//     u8 run_write;
//     u8 run_read;
//     u8 run_simul;

//     int length_transfer;
//     struct timeval write_time;
//     struct timeval read_time;
//     struct timeval simul_time;
//     char pass_read;
//     char pass_write;
//     char pass_simul;
//     char read_eplast_timeout;
//     char write_eplast_timeout;

//     int offset;
//

// static int scan_bars(struct tc_pcie_bookkeep *bk_ptr, struct pci_dev *dev) __init;
// static int map_bars(struct tc_pcie_bookkeep *bk_ptr, struct pci_dev *dev) __init;
// static int dma_test(struct tc_pcie_bookkeep *bk_ptr, struct pci_dev *dev);
// static irqreturn_t tc_isr(int irq, void *dev_id);

// static int tc_pcie_probe(struct pci_dev *dev, const struct pci_device_id *id) __init;
// static int set_table_header(struct dma_header *header, u32 eplast);
// static int print_table_header(struct dma_header *header);
// static int set_read_desc(struct dma_descriptor *rd_desc, dma_addr_t source, u64 dest, u32 ctl_dma_len, u32 id);
// //static int print_desc(struct dma_descriptor *desc);
// static int set_write_desc(struct dma_descriptor *wr_desc, u64 source, dma_addr_t dest, u32 ctl_dma_len, u32 id);
// static int scan_bars(struct tc_pcie_bookkeep *bk_ptr, struct pci_dev *dev);
// static int init_rp_mem(u8 *rp_buffer_virt_addr, u32 num_dwords, u32 init_value, u8 increment);
// static int rp_compare(u8 *virt_addr1, u8 *virt_addr2, u32 num_dwords);
// static int rp_ep_compare(u8 *virt_addr, struct tc_pcie_bookkeep *bk_ptr, u32 mem_byte_offset, u32 num_dwords);
// //static int print_ep_data(struct tc_pcie_bookkeep *bk_ptr, u32 mem_byte_offset, u32 num_dwords);
// //static int print_rp_data(u8 *virt_addr, u32 num_dwords);
// static int init_ep_mem(struct tc_pcie_bookkeep *bk_ptr, u32 mem_byte_offset, u32 num_dwords, u32 init_value, u8 increment);
// static void tc_pcie_remove(struct pci_dev *dev) __exit;
// static int eplast_busy_wait(struct tc_pcie_bookkeep *bk_ptr, u32 expected_eplast, u8 rw);
// static int diff_timeval(struct timeval *result, struct timeval *t2, struct timeval *t1);
// static int init_chrdev (struct tc_pcie_bookkeep *bk_ptr) __init;

// ssize_t tc_fops_read(struct file *file, char __user *buf, size_t count, loff_t *pos);
// ssize_t tc_fops_write(struct file *file, char __user *buf, size_t count, loff_t *pos);
// ssize_t tc_pcie_rw(struct file *file, char __user *buf, size_t count, loff_t *pos, int read);
// int tc_fops_open(struct inode *inode, struct file *file);
// int tc_fops_release(struct inode *inode, struct file *file);
// ssize_t tc_pcie_exec_cmd(struct dma_cmd *ucmd, struct tc_pcie_bookkeep *bk_ptr);

#endif /* _TC_PCIE_H */
