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
	{ 0xec5ba0f9, "module_layout" },
	{ 0xc2d48f35, "generic_read_dir" },
	{ 0xd84c8a78, "kill_anon_super" },
	{ 0x5b79b6f3, "simple_statfs" },
	{ 0xb876b71a, "d_rehash" },
	{ 0x3c65d1ec, "d_instantiate" },
	{ 0xb4045eb4, "d_alloc_root" },
	{ 0x697ef85, "generic_ro_fops" },
	{ 0xea16248b, "__insert_inode_hash" },
	{ 0xd91cc584, "new_inode" },
	{ 0xb198ffa, "unlock_page" },
	{ 0x1775cec7, "kunmap" },
	{ 0x8e4a0f04, "kmap" },
	{ 0x955ab75, "get_sb_nodev" },
	{ 0x58683da1, "register_filesystem" },
	{ 0x1ba23e64, "misc_register" },
	{ 0x2f287f0d, "copy_to_user" },
	{ 0xd0d8621b, "strlen" },
	{ 0x642e54ac, "__wake_up" },
	{ 0x167e7f9d, "__get_user_1" },
	{ 0xa1c76e0a, "_cond_resched" },
	{ 0x9ccb2622, "finish_wait" },
	{ 0x33d92f9a, "prepare_to_wait" },
	{ 0x4292364c, "schedule" },
	{ 0xc8b57c27, "autoremove_wake_function" },
	{ 0xb281b25f, "per_cpu__current_task" },
	{ 0x4cca6420, "misc_deregister" },
	{ 0xda40d708, "unregister_filesystem" },
	{ 0xb72397d5, "printk" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "28245296F42358BEC4BB3C3");
