#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/vmalloc.h>
#include <asm/uaccess.h>

MODULE_LICENSE("GPL");
MOUDLE_DESCRIPTION("Leap Frog Pseudo Random Number Generator");

static struct proc_dir_entry *proc_entry;
int tgid;

struct thread_node {
	int pid;
	unsigned int rand;
	struct list_head list;
}

ssize_t fortune_write(struct file *filp, const char _user *buffer, unsugned long len, void *data);

int init_lfprng_module(void) {

	int ret = 0;

	proc_entry = create_proc_entry("lfprng", 0644, NULL);

	if(proc_entry == NULL) {
		ret = -ENOMEM;
		printk(KERN_INFO "lfprng: Couldn't create proc entry\n");
	}
	else {
		proc_entry->read_proc = lfprng_read;
		proc_entry->write_proc = lfprng_write;
		printk(KERN_INFO "lfprng: Module loaded.\n");
	}

	return ret;
}

void cleanup_lfprng_module(void) {
	remove_proc_entry("lfprng, NULL);
	printk(KERN_INFO "lfprng: Module unloaded.\n");
}


module_init(init_lfprng_module);
module_exit(cleanup_lfprng_module);