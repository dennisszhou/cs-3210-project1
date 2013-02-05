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
	unsigned long long rand;
	struct list_head list;
};

ssize_t lfprng_write(struct file *filp, const char *buffer, unsigned long count, void *data);
int lfprng_read(char *page, char **start, off_t offset, int count, int *eof, void *data);
void free_thread_list(void);
struct thread_node* find_thread_node(int pid);

static struct proc_dir_entry *proc_entry;
static struct thread_node *head_node;
pid_t tgid;
int thread_count = 0;

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
	remove_proc_entry("lfprng", NULL);
	printk(KERN_INFO "lfprng: Module unloaded.\n");
}


module_init(init_lfprng_module);
module_exit(cleanup_lfprng_module);

ssize_t lfprng_write(struct file *filp, const char *buffer, unsigned long count, void *data) {
	
	int length;
	char temp[1000];
	unsigned long long seed;
	struct thread_node *temp_node;
	
	length = count > 999 ? 999 : count;

	if(copy_from_user(temp, buffer, length))
		return -EFAULT;

	((char*)temp)[length] = '\0';

	sscanf((const char*)temp, "%llu", &seed);

	if(tgid != current->tgid) {
		tgid = current->tgid;
		//memory clean up of the list
		if(tgid != 0)
			free_thread_list();

		//create new list
		head_node = (struct thread_node*) vmalloc(sizeof(struct thread_node));
		head_node->pid = current->pid;
		head_node->rand = seed;

		INIT_LIST_HEAD(&head_node->list);
		thread_count++;
	}
	else if(find_thread_node(current->pid) == NULL){
		temp_node = (struct thread_node*) vmalloc(sizeof(struct thread_node));
		temp_node->pid = current->pid;
		temp_node->rand = seed;

		list_add(&(temp_node->list), &(head_node->list));
		thread_count++;
	}

	printk(KERN_INFO "My current tgid is %d\n", current->tgid);
	printk(KERN_INFO "My current pid is %d\n", current->pid);

	return length;
}


int lfprng_read(char *page, char **start, off_t offset, int count, int *eof, void *data) {
	struct thread_node *temp;
	int length = 0;

	temp = find_thread_node(current->pid);

	if(temp == NULL) {
		printk(KERN_INFO "THREAD NOT FOUND");
		return 0;
	}

	length = sprintf(page, "%llu", temp->rand);

	return length + 1;
}

void free_thread_list(void) {
	struct thread_node *temp;
	struct list_head *pos, *q;

	list_for_each_safe(pos, q, &(head_node->list)) {
		temp = list_entry(pos, struct thread_node, list);
		list_del(pos);
		vfree(temp);
	}
}

struct thread_node* find_thread_node(int pid) {
	struct thread_node *temp;
	struct list_head *pos;

	list_for_each(pos, &(head_node->list)) {
		temp = list_entry(pos, struct thread_node, list);

		if(temp->pid == pid)
			return temp;
	}

	return NULL;
}
