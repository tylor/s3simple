#include <linux/fs.h> // definitions for file ops attached to /dev/hello
#include <linux/init.h>
#include <linux/miscdevice.h> // support for registering misc devices
#include <linux/module.h> // we're writing a module...
//#include <curl/curl.h>
//#include <linux/kernel.h> // is this needed?
#include <linux/sched.h> // needed for sleeping
#include <asm/uaccess.h> 
#include <linux/pagemap.h>
#include <linux/string.h>
#include <asm/uaccess.h> // needed to copy to/from user

#include "hellofs.h"

//static struct super_operations hellofs_ops;
static struct inode_operations hellofs_dir_inode_operations;
static struct file_operations hellofs_directory_operations;
static struct address_space_operations hellofs_aops;
static struct dentry * hellofs_lookup(struct inode *dir, struct dentry *dentry, struct nameidata *nd);

int waiting = 0;
//int flag_wait = 0;

/*struct MemoryStruct {
	char *memory = NULL;
	size_t size = 0;
};*/

#define BUF_LEN 80

static char Message[BUF_LEN];

//struct MemoryStruct message;

int the_flag = 0;

DECLARE_WAIT_QUEUE_HEAD(flag_wait);


struct hellofs_inode {

};


/*
 * Get mount stats
 */
/*static int hellofs_statfs(struct super_block *sb, struct statfs *buf)
{
//	printk(KERN_DEBUG "hellofs_statfs\n");

	buf->f_type = 0x31412171; // random number
	buf->f_bsize = PAGE_CACHE_SIZE;
	buf->f_blocks = 2;
	buf->f_bfree = 0;
	buf->f_bavail = 0;
	buf->f_files = 1;
	buf->f_ffree = 0;
	buf->f_namelen = 20;
	return 0;
} */

/*
 * Read a hellofs directory entry.
 */
static int hellofs_readdir(struct file *filp, void *dirent, filldir_t filldir)
{
	int error;

	unsigned int offset = filp->f_pos;

        const char* name = "hello.txt";
        const ino_t ino = 1;
        const int namelen = strlen(name);

        struct hellofs_inode *de;

        if(offset > 0) return 0;


	error = filldir(dirent, name, namelen, offset, ino, 0x1FF);

        offset += namelen + sizeof(*de);

	filp->f_pos = offset;
        
	return 0;
}

/*
 * Reading from /dev/hello1
 */
static char * hello_read()
{
	//char *hello_str = "Hello, world!\n";
	//int cool = 0;
	
	//printk("Requested path: %s", file->f_path);
	
	waiting = 1;
	the_flag = 0;

	wait_event(flag_wait, (the_flag == 1)); // unconditionally wait
	// char *hello_str = "Hello, worldsssss!\n";
	printk(KERN_INFO "Message working: %s.\n", Message);
	
	return Message;
	/*
	char *hello_str = Message;
	//buf = Message;
	
	int len = strlen(hello_str);
	
	printk(KERN_INFO "Len here %d.\n");
	printk(KERN_INFO "Count here %d.\n");
	
	// read whole string
	if (count < len)
		return len;
		//return -EINVAL;

		printk(KERN_INFO "Doesn't get here 6.\n");

	if (*ppos != 0)
		return 0;

	// Shouldn't need to copy to user
	//if (copy_to_user(buf, hello_str, len))
		//return -EINVAL;

	*ppos = len;

	printk(KERN_INFO "Returning a read: %i\n", len);

	return len;*/
}

/**
 * Read page from file
 */

static int hellofs_readpage(struct file *file, struct page * page)
{
	void *pgdata;
	pgdata = kmap(page);

//	printk(KERN_DEBUG "hellofs_readpage\n");

	//char * buf = "Cool\n";
	//file * file;
	
	//ssize_t len = 5;
	//char * buf = hello_read();
	waiting = 1;
	the_flag = 0;

	wait_event(flag_wait, (the_flag == 1)); // unconditionally wait
	
	char * buf = Message;
	int len = strlen(buf);
	
	printk(KERN_INFO "Message: %s\n", buf);
	printk(KERN_INFO "Length: %d\n", len);

	memcpy(pgdata, buf, len);
				printk(KERN_INFO "Inside here 1.\n");
  memset(pgdata + len, 'x', PAGE_CACHE_SIZE - len);
  			printk(KERN_INFO "Inside here 2.\n");
	
	//flush_dcache_page(page);

	SetPageUptodate(page);
	kunmap(page);
	unlock_page(page);
	//UnlockPage(page);

	return 0;
}

static struct address_space_operations hellofs_aops = {
	readpage: hellofs_readpage
};

static struct file_operations hellofs_directory_operations = {
	read:		generic_read_dir,
	readdir:	hellofs_readdir,
};

static struct inode *get_hellofs_inode(struct super_block *sb, int is_dir)
{
	struct inode * inode = new_inode(sb);
        //        printk(KERN_INFO "\n");
	//printk(KERN_DEBUG "get_hellofs_inode\n");

	if (inode) {
		inode->i_mode = is_dir ? 040444 : 0100444;
		inode->i_uid = 0;
		inode->i_size = 29;
		inode->i_blocks = 1;
		// inode->i_blksize = PAGE_CACHE_SIZE;
		inode->i_gid = 0;
		inode->i_ino = is_dir ? 0 : 1;
		/* inode->i_nlink is left 1 - arguably wrong for directories,
		   but it's the best we can do without reading the directory
	           contents.  1 yields the right result in GNU find, even
		   without -noleaf option. */
		insert_inode_hash(inode);
		if (S_ISREG(inode->i_mode)) {
			//printk(KERN_DEBUG "dir\n");
			inode->i_fop = &generic_ro_fops;
			inode->i_data.a_ops = &hellofs_aops;
		} else if (S_ISDIR(inode->i_mode)) {
			inode->i_op = &hellofs_dir_inode_operations;
			inode->i_fop = &hellofs_directory_operations;
		}
	}
	return inode;
}

/*
 * Lookup and fill in the inode data..
 */
static struct dentry * hellofs_lookup(struct inode *dir, struct dentry *dentry, struct nameidata *nd)
{
	int retval;

//	printk(KERN_DEBUG "hellofs_lookup\n");

	retval = memcmp(dentry->d_name.name, "hello.txt", 9);
	if(!retval) {
		d_add(dentry, get_hellofs_inode(dir->i_sb, 0));      
	}
	else {
		d_add(dentry, NULL);
	}
	return NULL;
}

static struct inode_operations hellofs_dir_inode_operations = {
	lookup:		hellofs_lookup,
};

struct super_operations hellofs_super_ops = {
	.statfs         = simple_statfs,
//	.drop_inode     = generic_delete_inode, /* Not needed, is the default */
	//.put_super      = samplefs_put_super,
};
/**
 * Fill the super block with the fs info
 */
static int hellofs_fill_super(struct super_block * sb, void * data, int silent)
{
//	struct hellofs_super super;
	//struct super_block * retval = NULL;
	//struct inode * inode;
	//struct samplefs_sb_info * sfs_sb;

	// sb->s_maxbytes = MAX_LFS_FILESIZE; /* NB: may be too large for mem */
	sb->s_blocksize = PAGE_CACHE_SIZE;
	sb->s_blocksize_bits = PAGE_CACHE_SHIFT;
	
	
	// sb->s_magic = SAMPLEFS_MAGIC;
	sb->s_op = &hellofs_super_ops;
	// sb->s_time_gran = 1; /* 1 nanosecond time granularity */


//	sb->s_fs_info = kzalloc(sizeof(struct samplefs_sb_info),GFP_KERNEL);
	//sfs_sb = SFS_SB(sb);
	//if(!sfs_sb) {
	//	return -ENOMEM;
	//}

	//inode = samplefs_get_inode(sb, S_IFDIR | 0755, 0);
	//if(!inode) {
		//kfree(sfs_sb);
		//return -ENOMEM;
//	}

	sb->s_root = d_alloc_root(get_hellofs_inode(sb, 1));
	//sb->s_root = d_alloc_root(inode);
	//if (!sb->s_root) {
	//	iput(inode);
	//	kfree(sfs_sb);
	//	return -ENOMEM;
	//}
	
	/* below not needed for many fs - but an example of per fs sb data */
	// sfs_sb->local_nls = load_nls_default();

	// samplefs_parse_mount_options(data, sfs_sb);
	
	/* FS-FILLIN your filesystem specific mount logic/checks here */

	//retval = sb;
	//return sb;
	return 0;
}

/**
 * Basically return the hellofs_fill_super function
 */
int hellofs_get_sb(struct file_system_type *fs_type,
        int flags, const char *dev_name, void *data, struct vfsmount *mnt)
{
	return get_sb_nodev(fs_type, flags, data, hellofs_fill_super, mnt);
}

static struct file_system_type hellofs_fs_type = {
	.owner = THIS_MODULE,
	.name = "hellofs",
	.get_sb = hellofs_get_sb,
	.kill_sb = kill_anon_super, // For virtual devices: 
	/*.kill_sb = kill_litter_super,*/
	/*  .fs_flags */
};

/*
 * Simple function to get a hello world from Amazon S3.
 */
static char * get_from_s3(void)
{
	char *new_string = "Hello, blah!\n";
	return new_string;
}

/*
 * Reading from /dev/hello
 */
static ssize_t hello_comm_read(struct file * file, char * buf, size_t count, loff_t *ppos)
{
//	flag_wait = 0;
	//char *hello_str = "Hello, world!\n";

	char *hello_str = "0";
	if(waiting == 1) {
		hello_str = "1";
	}
	
	//the_flag = 1;
	//wake_up_all(&flag_wait);
	
	int len = strlen(hello_str);
	
	// read whole string
	if (count < len)
		return -EINVAL;

	if (*ppos != 0)
		return 0;

	if (copy_to_user(buf, hello_str, len))
		return -EINVAL;

	*ppos = len;

	return len;
}


// echo "hello there" > /dev/hello
static ssize_t hello_comm_write(struct file *filep, const char *buff, size_t len, loff_t *off) {

//	message.memory = 
//	message.size = len;

	//printk("write(%p, %s, %d)", filep, buff, len);
	
	int i;
	for(i=0; i<len && i<BUF_LEN; i++) {
		get_user(Message[i], buff+i);
	}
	
	waiting = 0;
	the_flag = 1;
	wake_up_all(&flag_wait);
	
	//printk("<1>Tylor sez: not supported yet.\n");
	return i;
}

/* Communication channel */
static const struct file_operations hello_comm_fops = {
	.owner	= THIS_MODULE,
	.read	= hello_comm_read,
	.write = hello_comm_write
};

static struct miscdevice hello_comm_dev = {
	MISC_DYNAMIC_MINOR,
	"hello",
	&hello_comm_fops,
};

/* Device */
static const struct file_operations hello_fops = {
	.owner	= THIS_MODULE,
	.read	= hello_read,
	//.write = hello_write
	// presumably can do .write
};

static struct miscdevice hello_dev = {
	MISC_DYNAMIC_MINOR,
	"hello1",
	&hello_fops,
};

static int __init init_hellofs(void)
{
	int ret;

	ret = misc_register(&hello_comm_dev);
	ret = misc_register(&hello_dev);
	if (ret)
		printk(KERN_ERR "Unable to register \"Hello, world!\" misc device\n");

	printk(KERN_ERR "Tylor sez: Registered my device.\n");
	//return ret;

	return register_filesystem(&hellofs_fs_type);
}

static void __exit exit_hellofs(void)
{
	printk(KERN_INFO "unloading hellofs\n");

	unregister_filesystem(&hellofs_fs_type);
	
	misc_deregister(&hello_comm_dev);
	misc_deregister(&hello_dev);
}


module_init(init_hellofs)
module_exit(exit_hellofs)

MODULE_LICENSE("GPL");
