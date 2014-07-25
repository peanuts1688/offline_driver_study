#include <linux/time.h>
#include <linux/init.h>       // needed for the macros
#include <linux/fs.h>
#include <linux/module.h>     // needed by all modules
#include <linux/device.h>
#include <linux/pci.h>
#include <linux/cdev.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/kernel.h>     // needed for KERN_ALERT
                              // KERN_EMERG             : emergency, precede a crash
                              // KERN_ALERT             : requiring immediate action
                              // KERN_CRIT              : critical condition, related to serious hardware or software failures
                              // KERN_ERR               : error condition, device driver usually use to report hardware difficulties
                              // KERN_WARNING (dafault) : situation that do no create serious problems with the system
                              // KERN_NOTICE            : normal situation that still worthy of notice, some security related issue 
                              // KERN_INFO              : informational messages, many driver use to inform about the hardware found at startup
                              // KERN_DEBUG             : debugging messages

#include "tc_pcie_cmd.h"
#include "tc_pcie_driver.h"

//#define DEBUG_NO_HW
//#define DEBUG_VIRTUALPC
//#define DEBUG_LABPC

#ifdef DEBUG_NO_HW
#if defined (DEBUG_VIRTUALPC)
static unsigned short vid = 0x80ee;
static unsigned short did = 0xbeef;
#elif defined (DEBUG_LABPC)
static unsigned short vid = 0x8086;
static unsigned short did = 0x10CE;
#endif  // virtualpc

#else
static unsigned short vid = 0x1556; //0x1172;
static unsigned short did = 0x1100; //0xE003;
#endif // no_hw

module_param(vid, ushort, S_IRUGO);
module_param(did, ushort, S_IRUGO);


////////////////////////////////////////////////////////////////////////////////
// PCI ISR
#ifndef DEBUG_NO_HW
static irqreturn_t tc_isr(int irq, void *dev_id)
{
  return IRQ_HANDLED;
}
#endif
////////////////////////////////////////////////////////////////////////////////
// Read

ssize_t tc_fops_read(struct file *file, char __user *buf, size_t count, loff_t *pos) {
//     if (tc_pcie_rw(file, buf, count, pos, 1) < 0)
//         return -1;
//     return count;
  return -1;
}

////////////////////////////////////////////////////////////////////////////////
// Write

ssize_t tc_fops_write(struct file *file, char __user *buf, size_t count, loff_t *pos) 
{
//     if (tc_pcie_rw(file, buf, count, pos, 0) < 0)
//         return -1;
//     return count;
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
// IOCTL

long tc_fops_ioctl (struct file *filep, unsigned int cmd, unsigned long arg)
{
  int err = 0;
  unsigned int data, cnt;
  struct tc_pcie_bookkeep *bk_ptr = filep->private_data;
  struct tc_ioc_data_struct param_i, *param = &param_i;
  struct pci_dev *dev = bk_ptr->pci_dev;

  // error checking
  if (_IOC_TYPE(cmd) != TC_IOC_MAGIC) return -ENOTTY;
  if (_IOC_NR(cmd) > TC_IOC_MAXNR) return -ENOTTY;
  if (_IOC_DIR(cmd) & _IOC_READ)
    err = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
  else if (_IOC_DIR(cmd) & _IOC_WRITE)
    err = !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));
  if (err) return -EFAULT;

  // special cases; app reads only
  switch(cmd)
  {
    case TC_IOC_DRIVER_INFO:
      //dev_info(&dev->dev, "TC_IOC_DRIVER_INFO");
      // return = driver name [0] driver version [0]: dangerous since buffer can overflow
      copy_to_user((void __user *)arg, (void *)(TC_PCIE_DRIVER_NAME), sizeof(TC_PCIE_DRIVER_NAME));
      arg += sizeof(TC_PCIE_DRIVER_NAME);
      copy_to_user((void __user *)arg, (void *)(TC_PCIE_DRIVER_VERSION), sizeof(TC_PCIE_DRIVER_VERSION));
      arg += sizeof(TC_PCIE_DRIVER_VERSION);
      return 0;
    case TC_IOC_KMEM_INFO:
      //dev_info(&dev->dev, "TC_IOC_KMEM_INFO");
      for(cnt=0; cnt<TC_KMEM_COUNT; cnt++)
      {
        copy_to_user((void __user *)arg, (void *)(&bk_ptr->buffer_bus[cnt]), sizeof(void *));
        arg += sizeof(void *);
      }
      return 0;
    default: break;
  }

  // get args from user
  copy_from_user((void *)param, (void __user *)arg, sizeof(struct tc_ioc_data_struct));
  arg += sizeof(struct tc_ioc_data_struct);

  switch (cmd)
  {
    case TC_IOC_KMEM_WR    :
      //dev_info(&dev->dev, "TC_IOC_KMEM_WR, index=%d, offset=0x%x, length=%d", param->index, param->offset, param->length);
      if (param->index >= TC_KMEM_COUNT) {
        dev_warn(&dev->dev, "TC_IOC_KMEM_WR: param->index >= TC_KMEM_COUNT");
        return -EFAULT;
      }
      if (param->offset+param->length > TC_KMEM_LENGTH_WORDS) {
        dev_warn(&dev->dev, "TC_IOC_KMEM_WR: param->offset+param->length > TC_KMEM_LENGTH_WORDS");
        return -EFAULT;
      }
      copy_from_user((void *)&bk_ptr->buffer_virt[param->index][param->offset]
                   , (void __user *)arg
                   , param->length * 4);
      break;

    case TC_IOC_KMEM_RD    :
      //dev_info(&dev->dev, "TC_IOC_KMEM_RD, index=%d, offset=0x%x, length=%d", param->index, param->offset, param->length);
      if (param->index >= TC_KMEM_COUNT) {
        dev_warn(&dev->dev, "TC_IOC_KMEM_RD: param->index >= TC_KMEM_COUNT");
        return -EFAULT;
      }
      if (param->offset+param->length > TC_KMEM_LENGTH_WORDS) {
        dev_warn(&dev->dev, "TC_IOC_KMEM_RD: param->offset+param->length > TC_KMEM_LENGTH_WORDS");
        return -EFAULT;
      }
      copy_to_user((void __user *)arg
                 , (void *)&bk_ptr->buffer_virt[param->index][param->offset]
                 , param->length * 4);
      break;

    case TC_IOC_BAR0_RD    : dev_warn(&dev->dev, "TC_IOC_BAR0_RD");
      if (param->index != 0) {
        dev_warn(&dev->dev, "TC_IOC_BAR0_RD: param->index != 0");
        return -EFAULT;
      }
      if (param->offset > 16) {
        dev_warn(&dev->dev, "TC_IOC_BAR0_RD: param->offset > 16");
        return -EFAULT;
      }
      if (param->length > 16) {
        dev_warn(&dev->dev, "TC_IOC_BAR0_RD: param->length > 16");
        return -EFAULT;
      }
      
      for(cnt=0; cnt<param->length; cnt++)
      {
#ifndef DEBUG_NO_HW
        data = be32_to_cpu(readl((u32 *)(bk_ptr->bar[0]+param->offset*sizeof(int))));
#else
        data = 0x12345678;
#endif
        copy_to_user((void __user *)arg, (void *)&data, sizeof(int));
        param->offset ++;
        arg += sizeof(int);
      }
      break;

    case TC_IOC_BAR0_WR    : dev_warn(&dev->dev, "TC_IOC_BAR0_WR");
      if (param->index != 0) {
        dev_warn(&dev->dev, "TC_IOC_BAR0_WR: param->index != 0");
        return -EFAULT;
      }
      if (param->offset > 16) {
        dev_warn(&dev->dev, "TC_IOC_BAR0_WR: param->offset > 16");
        return -EFAULT;
      }
      if (param->length > 16) {
        dev_warn(&dev->dev, "TC_IOC_BAR0_WR: param->length > 16");
        return -EFAULT;
      }
      for(cnt=0; cnt<param->length; cnt++)
      {
        copy_from_user((void *)&data, (void __user *)arg, sizeof(int));
#ifndef DEBUG_NO_HW
        writel (cpu_to_be32(data), (u32 *)(bk_ptr->bar[0]+param->offset*sizeof(int)));
        wmb();
#endif
        param->offset ++;
        arg += sizeof(int);
      }
      break;

#ifndef DEBUG_NO_HW
    case TC_IOC_INTCNT_RD  :
      break;
#endif
      
    default: return -ENOTTY;
  }

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
// fops open/close
int tc_fops_open(struct inode *inode, struct file *file) {
  struct tc_pcie_bookkeep *bk_ptr = 0;
  //int cnt;

  bk_ptr = container_of(inode->i_cdev, struct tc_pcie_bookkeep, cdev);
  file->private_data = bk_ptr;

//   bk_ptr->user_pid = current->pid;
  return 0;
}

int tc_fops_release(struct inode *inode, struct file *file) {
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
// PCI support functions

/* ***************************************************************************** 
 *
 * file_operation structure is defined in linux/fs.h, and holds pointers to functions 
 * defined by the driver that perform various operations on the device. Each field
 * of the structure corresponds to the address of some function (function pointer)
 * defined by the driver to handle a requested operation.
 *
 * struct file_operations {
       struct module *owner;
       loff_t (*llseek) (struct file *, loff_t, int);
       ssize_t (*read) (struct file *, char *, size_t, loff_t *);
       ssize_t (*write) (struct file *, const char *, size_t, loff_t *);
       int (*readdir) (struct file *, void *, filldir_t);
       unsigned int (*poll) (struct file *, struct poll_table_struct *);
       int (*ioctl) (struct inode *, struct file *, unsigned int, unsigned long);
       int (*mmap) (struct file *, struct vm_area_struct *);
       int (*open) (struct inode *, struct file *);
       int (*flush) (struct file *);
       int (*release) (struct inode *, struct file *);
       int (*fsync) (struct file *, struct dentry *, int datasync);
       int (*fasync) (int, struct file *, int);
       int (*lock) (struct file *, int, struct file_lock *);
    	 ssize_t (*readv) (struct file *, const struct iovec *, unsigned long,
          loff_t *);
    	 ssize_t (*writev) (struct file *, const struct iovec *, unsigned long,
          loff_t *);
    };
 *
 * http://www.tldp.org/LDP/lkmpg/2.4/html/c577.htm
 *
 ***************************************************************************** */
// every device is represented in the kernel by a file structure, tc_pcie device is now represented as file_operations: tc_pcie_fops , later, this file structure can be "registered" as a block/char device
struct file_operations tc_pcie_fops = {
  .owner =   THIS_MODULE,
  .read =    tc_fops_read,
  .write =   (void *)tc_fops_write,
  .unlocked_ioctl =   tc_fops_ioctl,
  .open =    tc_fops_open,
  .release = tc_fops_release,
};
#ifndef DEBUG_NO_HW
static int scan_bars(struct tc_pcie_bookkeep *bk_ptr, struct pci_dev *dev)
{
  int i;
  for (i = 0; i < TC_PCIE_NUM_BARS; i++) {
    unsigned long bar_start = pci_resource_start(dev, i);
    unsigned long bar_end = pci_resource_end(dev, i);
    unsigned long bar_flags = pci_resource_flags(dev, i);
    bk_ptr->bar_length[i] = pci_resource_len(dev, i);
    dev_info(&dev->dev, "BAR[%d] 0x%08lx-0x%08lx flags 0x%08lx, length %d", i, bar_start, bar_end, bar_flags, (int)bk_ptr->bar_length[i]);
  }
  return 0;
}

static int __init map_bars(struct tc_pcie_bookkeep *bk_ptr, struct pci_dev *dev)
{
  int i;
  for (i = 0; i < TC_PCIE_NUM_BARS; i++) {
    unsigned long bar_start = pci_resource_start(dev, i);
    //unsigned long bar_end = pci_resource_end(dev, i);
    //unsigned long bar_flags = pci_resource_flags(dev, i);
    bk_ptr->bar_length[i] = pci_resource_len(dev, i);
    if (!bk_ptr->bar_length[i]) {
        bk_ptr->bar[i] = NULL;
        continue;
    }
    bk_ptr->bar[i] = ioremap(bar_start, bk_ptr->bar_length[i]);
    if (!bk_ptr->bar[i]) {
      dev_err(&dev->dev, "could not map BAR[%d]", i);
      return -1;
    } else
    dev_info(&dev->dev, "BAR[%d] mapped to 0x%p, length %lu", i, bk_ptr->bar[i], (long unsigned int)bk_ptr->bar_length[i]);
  }
  return 0;
}

static void unmap_bars(struct tc_pcie_bookkeep *bk_ptr, struct pci_dev *dev)
{
    int i;
    for (i = 0; i < TC_PCIE_NUM_BARS; i++) {
        if (bk_ptr->bar[i]) {
            pci_iounmap(dev, bk_ptr->bar[i]);
            bk_ptr->bar[i] = NULL;
        }
    }
}
#endif
static int __init init_chrdev (struct tc_pcie_bookkeep *bk_ptr) {
    int dev_minor = 0;
    int dev_major = 0;
    int devno = -1;

    int result = alloc_chrdev_region(&bk_ptr->cdevno, dev_minor, 1, TC_PCIE_DEVFILE);

    dev_major = MAJOR(bk_ptr->cdevno);
    if (result < 0) {
        printk(KERN_DEBUG "cannot get major ID %d", dev_major);
    }
    printk(KERN_DEBUG "major ID = %d", dev_major);

    devno = MKDEV(dev_major, dev_minor);

    cdev_init(&bk_ptr->cdev, &tc_pcie_fops);
    bk_ptr->cdev.owner = THIS_MODULE;
    bk_ptr->cdev.ops = &tc_pcie_fops;
    result = cdev_add(&bk_ptr->cdev, devno, 1);

    if (result)
        return -1;
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
// PCI probe/remove

// priorities:
// GFP_KERNEL
// GFP_AUTOMIC
// GFP_DMA
// ...
//
static int __init tc_pcie_probe(struct pci_dev *dev, const struct pci_device_id *id)
{
  int rc = 0, cnt;
  struct tc_pcie_bookkeep *bk_ptr = NULL;
//  void **debug;
  bk_ptr = kzalloc(sizeof(struct tc_pcie_bookkeep), GFP_KERNEL);     
  if(!bk_ptr)
    goto err_bk_alloc;

  dev_info(&dev->dev, "tc_pcie_probe");

//   //////////////////////////////////////////////////////////////////////////////
//   // debug code begin
//   //dev_info(&dev->dev, "VID = 0x%04x, Device = 0x%04x, Class = 0x%x, bus:slot.func = %02x:%02x.%02x",
//   //dev->vendor, dev->device, dev->class, dev->bus->number, PCI_SLOT(dev->devfn), PCI_FUNC(dev->devfn));
//   debug = kmalloc(100*sizeof(void*), GFP_KERNEL);
//   for(cnt=0; cnt<100; cnt++)
//   {
//     debug[cnt] = kzalloc(TC_KMEM_LENGTH_BYTES, GFP_KERNEL);
//     if (!debug[cnt]) break;
//     dev_info(&dev->dev, "tc_pcie_MIK TEST: cnt=%d, sizeof(void*)=%d, addr=%lx"
//         , cnt, (int)sizeof(void*), (long)debug[cnt]);
//   }
//   for(cnt=0; cnt<100; cnt++)
//   {
//     if (!debug[cnt]) break;
//     kfree(debug[cnt]);
//   }
//   kfree(debug);
//   // debug code end
//   //////////////////////////////////////////////////////////////////////////////

//    dev_info(&dev->dev, "VID = 0x%04x, Device = 0x%04x, Class = 0x%x, bus:slot.func = %02x:%02x.%02x",
//    dev->vendor, dev->device, dev->class, dev->bus->number, PCI_SLOT(dev->devfn), PCI_FUNC(dev->devfn));
//    dev_info(&dev->dev, "vid = 0x%04x, did = 0x%04x", vid, did);

  bk_ptr->pci_dev = dev;
  pci_set_drvdata(dev, bk_ptr);
  if(dev->vendor == vid && dev->device == did) {
    dev_info(&dev->dev, "VID = 0x%04x, Device = 0x%04x, Class = 0x%x, bus:slot.func = %02x:%02x.%02x",
    dev->vendor, dev->device, dev->class, dev->bus->number, PCI_SLOT(dev->devfn), PCI_FUNC(dev->devfn));

    rc = init_chrdev(bk_ptr);
    if (rc) {
      dev_err(&dev->dev, "init_chrdev() failed\n");
      goto err_initchrdev;
    }
#ifndef DEBUG_NO_HW
    rc = pci_enable_device(dev);
    if (rc) {
      dev_err(&dev->dev, "pci_enable_device() failed\n");
      goto err_enable;
    } else {
      dev_info(&dev->dev, "pci_enable_device() successful on device [vid 0x%04X, did 0x%04X]\n", vid, did);
    }
    rc = pci_request_regions(dev, TC_PCIE_DRIVER_NAME);
    if (rc) {
      dev_err(&dev->dev, "pci_request_regions() failed\n");
      goto err_regions;
    }
    pci_set_master(dev);
    rc = pci_enable_msi(dev);
    if (rc) {
      dev_info(&dev->dev, "pci_enable_msi() failed\n");
      bk_ptr->msi_enabled = 0;
    } else {
      dev_info(&dev->dev, "pci_enable_msi() successful\n");
      bk_ptr->msi_enabled = 1;
    }
    pci_read_config_byte(dev, PCI_REVISION_ID, &bk_ptr->revision);
    pci_read_config_byte(dev, PCI_INTERRUPT_PIN, &bk_ptr->irq_pin);
    pci_read_config_byte(dev, PCI_INTERRUPT_LINE, &bk_ptr->irq_line);

    // dma mask
    if (!pci_set_dma_mask(dev, DMA_BIT_MASK(64))) {
        // If DMA can directly access "consistent memory" in System RAM above 4G physical address, register this
        pci_set_consistent_dma_mask(dev, DMA_BIT_MASK(64));
        dev_info(&dev->dev, "using a 64-bit irq mask\n");
    } else {
        dev_info(&dev->dev, "unable to use 64-bit irq mask\n");
        goto err_dma_mask;
    }

    for(cnt=0; cnt<TC_KMEM_COUNT; cnt++)
    {
      bk_ptr->buffer_virt[cnt] = (void *)pci_alloc_consistent(dev, TC_KMEM_LENGTH_BYTES, &bk_ptr->buffer_bus[cnt]);
      if (!bk_ptr->buffer_virt[cnt])
      {
        dev_info(&dev->dev, "unable pci_alloc_consistent beyond %d buffers\n", cnt);
        goto err_kmem_alloc;
      }
      dev_info(&dev->dev, "bk_ptr->buffer_virt[cnt]=%x, bk_ptr->buffer_bus[cnt]=%x\n", 
        (unsigned int)bk_ptr->buffer_virt[cnt], (unsigned int)bk_ptr->buffer_bus[cnt]);
    }

    dev_info(&dev->dev, "irq pin: %d\n", bk_ptr->irq_pin);
    dev_info(&dev->dev, "irq line: %d\n", bk_ptr->irq_line);
    dev_info(&dev->dev, "irq: %d\n", dev->irq);

    rc = request_irq(bk_ptr->irq_line, tc_isr, IRQF_SHARED, TC_PCIE_DRIVER_NAME, (void *)bk_ptr);
    if (rc) {
        dev_info(&dev->dev, "Could not request IRQ #%d", bk_ptr->irq_line);
        bk_ptr->irq_line = -1;
        goto err_irq;
    } else {
        dev_info(&dev->dev, "request irq: %d", bk_ptr->irq_line);
    }

    scan_bars(bk_ptr, dev);
    map_bars(bk_ptr, dev);

    // set default settings to run
//     bk_ptr->tc_pcie_num_dwords = TC_PCIE_NUM_DWORDS;
//     bk_ptr->tc_pcie_descriptor_num = TC_PCIE_DESCRIPTOR_NUM;

//     bk_ptr->run_write = 1;
//     bk_ptr->run_read = 1;
//     bk_ptr->run_simul = 1;
//     bk_ptr->offset = 0;
#endif
    return 0;
  } else
    return -ENODEV; // device not found

// Error clean up
#ifndef DEBUG_NO_HW
err_irq:
    dev_err(&dev->dev, "goto err_irq");
err_kmem_alloc:
    dev_err(&dev->dev, "goto err_kmem_alloc");
    for(cnt=0; cnt<TC_KMEM_COUNT; cnt++)
    {
      if (!bk_ptr->buffer_virt[cnt]) break;
      pci_free_consistent(dev, TC_KMEM_LENGTH_BYTES, bk_ptr->buffer_virt[cnt], bk_ptr->buffer_bus[cnt]);
    }
err_dma_mask:
    dev_err(&dev->dev, "goto err_dma_mask");
    pci_release_regions(dev);
err_regions:
    dev_err(&dev->dev, "goto err_regions");
    pci_disable_device(dev);
err_enable:
    dev_err(&dev->dev, "goto err_enable");
    unregister_chrdev_region (bk_ptr->cdevno, 1);
#endif
err_initchrdev:
    dev_err(&dev->dev, "goto err_initchrdev");
    kfree(bk_ptr);
err_bk_alloc:
    dev_err(&dev->dev, "goto err_bk_alloc");
    return -ENODEV;
}

static void __exit tc_pcie_remove(struct pci_dev *dev)
{
  struct tc_pcie_bookkeep *bk_ptr = NULL;
  int cnt;

  dev_info(&dev->dev, "tc_pcie_remove");
  bk_ptr = pci_get_drvdata(dev);

  cdev_del(&bk_ptr->cdev);
  unregister_chrdev_region(bk_ptr->cdevno, 1);

  if(bk_ptr) {
    dev_info(&dev->dev, "tc_pcie_remove-buffer_virt free");
    for(cnt=0; cnt<TC_KMEM_COUNT; cnt++)
    {
      if (!bk_ptr->buffer_virt[cnt]) break;
      pci_free_consistent(dev, TC_KMEM_LENGTH_BYTES, bk_ptr->buffer_virt[cnt], bk_ptr->buffer_bus[cnt]);
    }
    if(bk_ptr->msi_enabled) {
      pci_disable_msi(dev);
      bk_ptr->msi_enabled = 0;
    }
  }
#ifndef DEBUG_NO_HW
  unmap_bars(bk_ptr, dev);
  pci_disable_device(dev);
  pci_release_regions(dev);
  if (bk_ptr->irq_line >= 0) {
    printk(KERN_DEBUG "Freeing IRQ #%d", bk_ptr->irq_line);
    free_irq(bk_ptr->irq_line, (void *)bk_ptr);
  }
#endif
  kfree(bk_ptr);
  printk(KERN_DEBUG TC_PCIE_DRIVER_NAME ": " "tc_pcie_remove()," " " __DATE__ " " __TIME__ " " "\n");
}

static struct pci_device_id pci_ids[] = {
  { PCI_DEVICE(PCI_ANY_ID, PCI_ANY_ID) },
  { 0 }
};

static struct pci_driver tc_pcie_driver_ops = {
  .name = TC_PCIE_DRIVER_NAME,
  .id_table = pci_ids,
  .probe = tc_pcie_probe,
  .remove = tc_pcie_remove,
};

////////////////////////////////////////////////////////////////////////////////
// Driver init / exit

static int __init tc_pcie_init(void)
{
  int rc = 0;

  printk(KERN_DEBUG TC_PCIE_DRIVER_NAME ": " "tc_pcie_init()," " " __DATE__ " " __TIME__ " " "\n");
  rc = pci_register_driver(&tc_pcie_driver_ops);
  if (rc) {
    printk(KERN_ERR TC_PCIE_DRIVER_NAME ": PCI driver registration failed\n");
    goto exit;
  }

exit:
  return rc;
}

static void __exit tc_pcie_exit(void)
{
  pci_unregister_driver(&tc_pcie_driver_ops);
}


module_init(tc_pcie_init);        // macro to the original init_module(), defined in linux/init.h
module_exit(tc_pcie_exit);        // macro to the original cleanup_module(), defined in linux/init.h

MODULE_AUTHOR("Mik Kim <mkim@tensorcom.com>");
MODULE_DESCRIPTION("TC PCIe device driver");
MODULE_VERSION(TC_PCIE_DRIVER_VERSION);
MODULE_LICENSE("Dual BSD/GPL");         // defined in linux/module.h

MODULE_DEVICE_TABLE(pci, pci_ids);


