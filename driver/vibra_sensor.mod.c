#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/export-internal.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;
BUILD_LTO_INFO;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif


static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0xd486412c, "i2c_smbus_read_i2c_block_data" },
	{ 0x4dfa8d4b, "mutex_lock" },
	{ 0x3213f038, "mutex_unlock" },
	{ 0xe2964344, "__wake_up" },
	{ 0x8da6585d, "__stack_chk_fail" },
	{ 0x4a13b6b6, "i2c_smbus_write_byte_data" },
	{ 0x742ca250, "i2c_unregister_device" },
	{ 0xc1514a3b, "free_irq" },
	{ 0xfe990052, "gpio_free" },
	{ 0x3c12dfe, "cancel_work_sync" },
	{ 0x3f74d235, "device_destroy" },
	{ 0xa169b1d6, "class_unregister" },
	{ 0x3486581a, "class_destroy" },
	{ 0x6bc3fbc0, "__unregister_chrdev" },
	{ 0xfe487975, "init_wait_entry" },
	{ 0x1000e51, "schedule" },
	{ 0x8c26d495, "prepare_to_wait_event" },
	{ 0x92540fbf, "finish_wait" },
	{ 0x6cbbfc54, "__arch_copy_to_user" },
	{ 0x2d3385d3, "system_wq" },
	{ 0xc5b6f236, "queue_work_on" },
	{ 0xdcb764ad, "memset" },
	{ 0x41fabaf5, "__register_chrdev" },
	{ 0x853d16c8, "__class_create" },
	{ 0xa4a9ecb6, "device_create" },
	{ 0xfdf6508b, "i2c_get_adapter" },
	{ 0xab2cc6c2, "i2c_new_client_device" },
	{ 0x74291057, "i2c_put_adapter" },
	{ 0x47229b5c, "gpio_request" },
	{ 0xe307b4ed, "gpio_to_desc" },
	{ 0x604f36d6, "gpiod_direction_input" },
	{ 0x6ab02cb6, "gpiod_to_irq" },
	{ 0x92d5838e, "request_threaded_irq" },
	{ 0x92997ed8, "_printk" },
	{ 0xe28e8a80, "module_layout" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "702E451212EE53F2A1F8041");
