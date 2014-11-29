#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <linux/delay.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Al Poole");
MODULE_DESCRIPTION("Random Number Generator");

ssize_t random_read(struct file *f, char *buf, size_t count, loff_t *ofp)
{
	int i;
	unsigned int eax_val;

	unsigned long tmp;
		
	// need to find a better register!
	for (i = 0; i < count; i++) {
//		msleep(1000);
		asm("mov %%eax, %0": "=r"(eax_val));
		printk(KERN_INFO "it is %u\n", eax_val);
		tmp = eax_val;
		put_user(tmp, buf + i);
	}

	return count;
}

ssize_t random_write(struct file *f, const char buf, size_t count, loff_t *ofp)
{
	return -EINVAL;
}

struct file_operations random_fops = {
	.open = NULL,
	.read = random_read,
	.write = random_write,
	.release = NULL,
};


int major = 0;
const char *dev_name = "/dev/poole";

static int __init random_init(void)
{
	int result = 0;

	result = register_chrdev(0, dev_name, &random_fops);
	if (result < 0)
		return 1;

	major = result;

	printk(KERN_INFO "Device created with major: %d\n", major);

	return 0;
}

static void __exit random_exit(void)
{
	unregister_chrdev(major, dev_name);
	printk(KERN_INFO "Unloaded module\n");
}


module_init(random_init);
module_exit(random_exit);

