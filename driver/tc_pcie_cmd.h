#ifndef _TC_PCIE_CMD_H
#define _TC_PCIE_CMD_H

#define TC_PCIE_DRIVER_NAME    "TC_PCIe_driver"
#define TC_PCIE_DEVFILE        "tc_pcie_driver"

#define TC_PCIE_DRIVER_VERSION "1.0"

#define TC_KMEM_COUNT 4
#define TC_KMEM_LENGTH_BYTES  4096
#define TC_KMEM_LENGTH_WORDS  (TC_KMEM_LENGTH_BYTES/sizeof(unsigned int))
#define TC_IOC_MAGIC 'T'
#define TC_IOC_DRIVER_INFO  _IOWR(TC_IOC_MAGIC, 1, unsigned int)  // inputs: none, outputs: driver version
#define TC_IOC_KMEM_INFO    _IOWR(TC_IOC_MAGIC, 2, unsigned int)  // inputs: none, outputs: TC_KMEM_COUNT of physical address
#define TC_IOC_KMEM_WR      _IOWR(TC_IOC_MAGIC, 3, unsigned int)  // inputs: memIndex, offset, length, data
#define TC_IOC_KMEM_RD      _IOWR(TC_IOC_MAGIC, 4, unsigned int)  // inputs: memIndex, offset, length; outputs: returns data
#define TC_IOC_BAR0_WR      _IOWR(TC_IOC_MAGIC, 5, unsigned int)  // inputs: barIndex, offset, data
#define TC_IOC_BAR0_RD      _IOWR(TC_IOC_MAGIC, 6, unsigned int)  // inputs: barIndex, offset, outputs: returns data
#define TC_IOC_INTCNT_RD    _IOWR(TC_IOC_MAGIC, 7, unsigned int)  // no input; outputs: returns interrupt counter
#define TC_IOC_MAXNR                            8

struct tc_ioc_data_struct {
  unsigned int index;
  unsigned int offset;
  unsigned int length;
  unsigned int data[0];
};

// register offsets
#define BAR0_VERSION        0
#define BAR0_CONTROL_REG    1
#define BAR0_STATUS_REG     4
#define BAR0_DMA0_LENSTAT   5
#define BAR0_DMA0_ADDR_HI   6
#define BAR0_DMA0_ADDR_LO   7
#define BAR0_DMA1_LENSTAT   8
#define BAR0_DMA1_ADDR_HI   9
#define BAR0_DMA1_ADDR_LO  10
#define NUM_BAR0_REGS       11
// bit positions
#define BITPOS_DMA_START    0
#define BITPOS_DMA_64       1  // ADDR reg bits
#define BITPOS_DMA0_DONE    0
#define BITPOS_DMA1_DONE    1
#define BITPOS_RX_READY     2
#define BITPOS_DMA0_RESET   30
#define BITPOS_DMA1_RESET   31

#endif /* _TC_PCIE_CMD_H */
