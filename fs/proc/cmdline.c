// SPDX-License-Identifier: GPL-2.0
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#ifdef CONFIG_MACH_LGE
#include <asm/setup.h>
#include <soc/qcom/lge/board_lge.h>
#include <linux/slab.h>

#ifdef CONFIG_INITRAMFS_IGNORE_SKIP_FLAG
#define INITRAMFS_STR_FIND "skip_initramf"
#define INITRAMFS_STR_REPLACE "want_initramf"
#define INITRAMFS_STR_LEN (sizeof(INITRAMFS_STR_FIND) - 1)
#endif // CONFIG_INITRAMFS_IGNORE_SKIP_FLAG

static char proc_command_line[COMMAND_LINE_SIZE];

static void proc_cmdline_set(char *name, char *value)
{
	char *flag_pos, *flag_after;
	char *flag_pos_str = kmalloc(sizeof(char), COMMAND_LINE_SIZE);
	scnprintf(flag_pos_str, COMMAND_LINE_SIZE, "%s=", name);
	flag_pos = strstr(proc_command_line, flag_pos_str);
	if (flag_pos) {
		flag_after = strchr(flag_pos, ' ');
		if (!flag_after)
			flag_after = "";
		scnprintf(proc_command_line, COMMAND_LINE_SIZE, "%.*s%s=%s%s",
				(int)(flag_pos - proc_command_line),
				proc_command_line, name, value, flag_after);
	} else {
		// flag was found, insert it
		scnprintf(proc_command_line, COMMAND_LINE_SIZE, "%s %s=%s", proc_command_line, name, value);
	}
}

static void proc_command_line_init(void) {

#ifdef CONFIG_INITRAMFS_IGNORE_SKIP_FLAG
	char *offset_addr;
	strcpy(proc_command_line, saved_command_line);
	offset_addr = strstr(proc_command_line, INITRAMFS_STR_FIND);
	if (!offset_addr)
		goto set_lge_bootmode;
	memcpy(offset_addr, INITRAMFS_STR_REPLACE, INITRAMFS_STR_LEN);

set_lge_bootmode:
#endif // CONFIG_INITRAMFS_IGNORE_SKIP_FLAG
	if (lge_get_boot_mode() == LGE_BOOT_MODE_CHARGERLOGO) {
		proc_cmdline_set("androidboot.mode", "charger");
	}
}

static int cmdline_proc_show(struct seq_file *m, void *v)
{
#ifdef CONFIG_MACH_LGE
	seq_printf(m, "%s\n", proc_command_line);
#else
	seq_printf(m, "%s\n", saved_command_line);
#endif
	return 0;
}

static int cmdline_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, cmdline_proc_show, NULL);
}

static const struct file_operations cmdline_proc_fops = {
	.open		= cmdline_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static int __init proc_cmdline_init(void)
{
#ifdef CONFIG_MACH_LGE
	proc_command_line_init();
#endif

	proc_create("cmdline", 0, NULL, &cmdline_proc_fops);
	return 0;
}
fs_initcall(proc_cmdline_init);
