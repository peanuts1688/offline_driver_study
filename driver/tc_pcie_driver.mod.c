#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x35ec255d, "module_layout" },
	{ 0x487d9343, "param_ops_ushort" },
	{ 0x44a8e76b, "pci_unregister_driver" },
	{ 0xe7472ec, "__pci_register_driver" },
	{ 0x6143a69d, "dev_warn" },
	{ 0x2f287f0d, "copy_to_user" },
	{ 0x362ef408, "_copy_from_user" },
	{ 0xf20dabd8, "free_irq" },
	{ 0x69d6e079, "pci_iounmap" },
	{ 0x411ae61c, "pci_disable_msi" },
	{ 0x2f35ab80, "cdev_del" },
	{ 0xddffa24, "dev_get_drvdata" },
	{ 0x9a756412, "pci_enable_device" },
	{ 0x37a0cba, "kfree" },
	{ 0x7485e15e, "unregister_chrdev_region" },
	{ 0xdeae679e, "pci_disable_device" },
	{ 0x5e9c3b78, "pci_release_regions" },
	{ 0x42c8de35, "ioremap_nocache" },
	{ 0x2072ee9b, "request_threaded_irq" },
	{ 0x7087c350, "dma_alloc_from_coherent" },
	{ 0x91095cab, "x86_dma_fallback_dev" },
	{ 0x103fba37, "dma_supported" },
	{ 0x24d52bf2, "dma_set_mask" },
	{ 0x1d400e71, "pci_bus_read_config_byte" },
	{ 0xe2735d4f, "pci_enable_msi_block" },
	{ 0x435f447e, "pci_set_master" },
	{ 0x1d70b1d9, "pci_request_regions" },
	{ 0x35af9caf, "dev_err" },
	{ 0xebd5feb1, "cdev_add" },
	{ 0xb2c831b6, "cdev_init" },
	{ 0x50eedeb8, "printk" },
	{ 0x29537c9e, "alloc_chrdev_region" },
	{ 0x93d085b2, "dev_set_drvdata" },
	{ 0x18882e2f, "_dev_info" },
	{ 0x41ad0272, "kmem_cache_alloc_trace" },
	{ 0xcea0a119, "kmalloc_caches" },
	{ 0xf1243fad, "dma_release_from_coherent" },
	{ 0x31944a28, "dma_ops" },
	{ 0xb4390f9a, "mcount" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";

MODULE_ALIAS("pci:v*d*sv*sd*bc*sc*i*");

MODULE_INFO(srcversion, "CE41931D3484EE50314B2D2");
