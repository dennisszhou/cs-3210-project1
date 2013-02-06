#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/string.h>
#include <linux/list.h>
#include <linux/vmalloc.h>
#include <asm/uaccess.h>

MODULE_LICENSE("GPL");

struct thread_node {
	pid_t pid;
	unsigned long long num;
	struct thread_node *next;
};

ssize_t lfprng_write(struct file *filp, const char *buffer, unsigned long count, void *data);
int lfprng_read(char *page, char **start, off_t offset, int count, int *eof, void *data);
void add_thread_node(unsigned long long seed);
int check_process(void);
void remove_thread(struct thread_node *node);
struct thread_node* find_thread_node(struct thread_node *node);

static struct proc_dir_entry *proc_entry;

static struct thread_node *head_node;
pid_t tgid;
int thread_count;

int init_lfprng_module(void) {

	int ret = 0;

	proc_entry = create_proc_entry("lfprng", 0666, NULL);

	if(proc_entry == NULL) {
		ret = -ENOMEM;
		printk(KERN_INFO "lfprng: Couldn't create proc entry\n");
	}
	else {
		head_node = NULL;
		tgid = -1;

		proc_entry->read_proc = lfprng_read;
		proc_entry->write_proc = lfprng_write;
		printk(KERN_INFO "lfprng: Module loaded.\n");
	}

	return ret;
}

void cleanup_lfprng_module(void) {
	remove_proc_entry("lfprng", NULL);
	remove_thread(head_node);
	printk(KERN_INFO "lfprng: Module unloaded.\n");
}

module_init(init_lfprng_module);
module_exit(cleanup_lfprng_module);

ssize_t lfprng_write(struct file *filp, const char *buffer, unsigned long count, void *data) {
	int length;
	char temp[1000];
	unsigned long long seed;

	length = count > 999 ? 999 : count;

	if(copy_from_user(temp, buffer, length))
		return -EFAULT;

	((char*)temp)[length] = '\0';

	sscanf((const char*)temp, "%llu", &seed);

	if(tgid == current->tgid || !check_process()) {
		tgid = current->tgid;
		add_thread_node(seed);
	}

	printk(KERN_INFO "My current tgid is %d\n", current->tgid);
	printk(KERN_INFO "My current pid is %d\n", current->pid);
	printk(KERN_INFO "My given seed is %llu\n", seed);

	return length;
}
int lfprng_read(char *page, char **start, off_t offset, int count, int *eof, void *data) {
	int length;
	struct thread_node *node;
	unsigned long long seed = 10;

	printk(KERN_INFO "Find Thread Node Call\n");
	node = find_thread_node(head_node);

	if(node != NULL)
		length = sprintf(page, "%llu", node->num);
	else
		length = sprintf(page, "%llu", seed);

	return length + 1;
}

void add_thread_node(unsigned long long seed) {
	struct thread_node *node = vmalloc(sizeof(struct thread_node));
	node->pid = current->pid;
	node->num = seed;
	if(head_node != NULL)
		node->next = head_node;
	else
		node->next = NULL;
	head_node = node;
}

struct thread_node* find_thread_node(struct thread_node *node) {
	if(node == NULL)
		return NULL;

	if(node->pid == current->pid) {
		printk(KERN_INFO "Node Found: %d\n", node->pid);
		return node;
	}
	else if(node->next != NULL)
		return find_thread_node(node->next);
	else {
		printk(KERN_INFO "Invalid Thread Access\n");
		return NULL;
	}
}

int check_process(void) {
	int found = 0;

	struct task_struct *p;
	for_each_process(p) {
		if(p->tgid == tgid) {
			found = 1;
			break;
		}
	}

	if(!found) {
		printk(KERN_INFO "Deleting process: %d\n", tgid);
		remove_thread(head_node);
	}

	return found;
}

void remove_thread(struct thread_node *node) {
	if(node == NULL)
		return;

	if(node->next != NULL) {
		remove_thread(node->next);
	}

	vfree(node);
	head_node = NULL;
}
