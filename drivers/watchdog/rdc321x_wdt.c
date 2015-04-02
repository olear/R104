/*
 * Trivial RDC321x hardware watchdog driver, by Bifferos, bifferos@yahoo.co.uk
 * ~5 second timout.
 */

#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/watchdog.h>
#include <linux/io.h>

#define PFX "rdc321x watchdog: "

/* 
 * Experiment with other values lower than 7 for shorter delays 
 * 7 works out at approx 5 seconds
 */
#define WDT_TIMEOUT (1<<7)

/* Also doubles as 'enable' */
#define WDT_RESET (1<<23)|(1<<20)|(1<<19)|(1<<18)|WDT_TIMEOUT

/* Write this value to disable */
#define WDT_DISABLE (1<<20)|(1<<19)|(1<<18)|WDT_TIMEOUT

static spinlock_t g_lock;

static inline void wdt_write(u32 val)
{
	unsigned long flags;
	spin_lock_irqsave(&g_lock, flags);
	outl(0x80003844, 0xcf8);
	outl(val, 0xcfc);
	spin_unlock_irqrestore(&g_lock, flags);
}

static inline u32 wdt_read(void)
{
	unsigned long flags;
	u32 ret;
	spin_lock_irqsave(&g_lock, flags);
	outl(0x80003844, 0xcf8);
	ret = inl(0xcfc);
	spin_unlock_irqrestore(&g_lock, flags);
	return ret;
}


static int rdc321x_wdt_open(struct inode *inode, struct file *file)
{
	wdt_write(WDT_RESET);
	return nonseekable_open(inode, file);
}


static ssize_t rdc321x_wdt_write(struct file *file, const char __user *buf,
				size_t count, loff_t *ppos)
{
	if (!count)
		return -EIO;
	wdt_write(WDT_RESET);
	return count;
}

static int rdc321x_wdt_release(struct inode *inode, struct file *file)
{
#ifndef CONFIG_WATCHDOG_NOWAYOUT
	wdt_write(WDT_DISABLE);
#endif
	return 0;
}

static const struct file_operations rdc321x_wdt_fops = {
	.owner		= THIS_MODULE,
	.llseek		= no_llseek,
	.open		= rdc321x_wdt_open,
	.write		= rdc321x_wdt_write,
	.release	= rdc321x_wdt_release,
};

static struct miscdevice rdc321x_wdt_misc = {
	.minor	= WATCHDOG_MINOR,
	.name	= "watchdog",
	.fops	= &rdc321x_wdt_fops,
};

static int __init rdc321x_wdt_init(void)
{
	unsigned long flags;
	u32 tmp;
	int err = misc_register(&rdc321x_wdt_misc);
	if (err < 0) {
		printk(KERN_ERR PFX "misc_register failed\n");
		return err;
	}

	spin_lock_init(&g_lock);

	spin_lock_irqsave(&g_lock, flags);
	outl(0x80003840, 0xcf8);
	tmp = inl(0xcfc);
	/* link PCIRST_n to soft reset, so something actually happens 
	 * when the WDT fires!
	 */
	tmp |= 0x1000;  
	outl(tmp, 0xcfc);
	spin_unlock_irqrestore(&g_lock, flags);

	pr_info(PFX "Loaded\n");
	return 0;
}

static void __exit rdc321x_wdt_exit(void)
{
	misc_deregister(&rdc321x_wdt_misc);
	pr_info(PFX "Unloaded\n");
}

module_init(rdc321x_wdt_init);
module_exit(rdc321x_wdt_exit);

MODULE_AUTHOR("Bifferos <bifferos@yahoo.co.uk>");
MODULE_DESCRIPTION("RDC321x watchdog driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS_MISCDEV(WATCHDOG_MINOR);
