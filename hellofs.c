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

struct HelloCommand {
	int waiting;
	int the_flag;
	int interesting;
//	char * command;
	char command[50];
	char * data;
} currentHelloCommand;

typedef struct {
	char * name;
  int size;
} s3files;

s3files files[100];

struct HelloCommand * currentCommand;

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

static void hellofs_parse_files(char * response) {
	int i = 0;
	char * next;
  char * size_end;
  
  next = strstr(response, "<Contents>");
  while(next != NULL) {
  	files[i].name = (char *) kmalloc((strstr(next, "</Key>") - strstr(next, "<Key>"))-5, GFP_KERNEL);
    strncpy(files[i].name, strstr(next, "<Key>") + 5, (strstr(next, "</Key>") - strstr(next, "<Key>"))-5);
    files[i].name[(strstr(next, "</Key>") - strstr(next, "<Key>"))-5] = '\0'; // might not need.. ?

    size_end = strstr(next, "</Size>");
    files[i].size = simple_strtol(strstr(next, "<Size>") + 6, &size_end, 10);
    
    // Advance to closing tag.
    next = strstr(next, "</Contents>");
    // Check for a new tag, if NULL the while loop will terminate.
    next = strstr(next, "<Contents>");
    i++;
  }
}

/*
 * Read a hellofs directory entry.
 */
static int hellofs_readdir(struct file *filp, void *dirent, filldir_t filldir)
{
	currentCommand->waiting = 1;
	currentCommand->the_flag = 0;
	strcpy(currentCommand->command, "dir");

	wait_event(flag_wait, (currentCommand->the_flag == 1)); // unconditionally wait
	
	char * buf = currentCommand->data;
	int len = strlen(buf);
	
	hellofs_parse_files(buf);
	
	//printk("Let's see: %s", files[0].name);
	

	int error;

	unsigned int offset = filp->f_pos;

        //const char* name = "README_public.txt";
        //const char * name;
        //strcpy(name, files[0].name);
        //printk("Hokay: %s\n", name);
        //const ino_t ino = 1;
        //const int namelen = strlen(name);

        struct hellofs_inode *de;

        if(offset > 0) return 0;


	//error = filldir(dirent, name, namelen, offset, ino, 0x1FF);

        //offset += namelen + sizeof(*de);
        
        // ****
        int i = 0;
        while (files[i].name != NULL) {
		      //const char* name2 = "hello.txt";
		      const ino_t ino2 = i + 1;
//		      const int namelen2 = strlen(name2);
		      
		      error = filldir(dirent, files[i].name, strlen(files[i].name), offset, ino2, 0x1FF);

		      offset += strlen(files[i].name) + sizeof(*de);
		      i++;
		    }
        // ***
        

	filp->f_pos = offset;
        
	return 0;
}

/**
 * Read page from file
 */
static int hellofs_readpage(struct file *file, struct page * page)
{
	void *pgdata;
	pgdata = kmap(page);
	
	//file->f_dentry->d_name.name
	//file->f_path.dentry->d_name.name

	currentCommand->waiting = 1;
	currentCommand->the_flag = 0;
	strcpy(currentCommand->command, file->f_path.dentry->d_name.name);

	wait_event(flag_wait, (currentCommand->the_flag == 1)); // unconditionally wait
	
	char * buf = currentCommand->data;
	int len = strlen(buf);
	
	printk(KERN_INFO "Message: %s\n", buf);
	printk(KERN_INFO "Length: %d\n", len);

	memcpy(pgdata, buf, len);
	// Set what we don't have to Xs, just in case.
  memset(pgdata + len, 'x', PAGE_CACHE_SIZE - len);
	
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

static struct inode *get_hellofs_inode(struct super_block *sb, int is_dir, int size)
{
	struct inode * inode = new_inode(sb);

	if (inode) {
		inode->i_mode = is_dir ? 040444 : 0100444; // Give us readonly for each user.
		inode->i_uid = 0;
		inode->i_size = size;
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
 * Lookup and fill in the inode data.
 */
static struct dentry * hellofs_lookup(struct inode *dir, struct dentry *dentry, struct nameidata *nd)
{
	int retval;

 	int i = 0;
	while(files[i].name != NULL) {
		//retval = strncmp(dentry->d_name.name, files[i].name, strlen(files[i].name));
		if(strcmp(dentry->d_name.name, files[i].name) == 0) {
			d_add(dentry, get_hellofs_inode(dir->i_sb, 0, files[i].size));
			return NULL;
		}
		i++;
	}
	d_add(dentry, NULL);
	return NULL;
	/*
	retval = memcmp(dentry->d_name.name, "README_public.txt", 17); // push this comparison to get_hellofs_inode()
	if(!retval) {
		d_add(dentry, get_hellofs_inode(dir->i_sb, 0));     
	}
	else {
		retval = memcmp(dentry->d_name.name, "hello.txt", 9);
		if(!retval) {
			d_add(dentry, get_hellofs_inode(dir->i_sb, 0));     
		}
		else {
			d_add(dentry, NULL);
		}
	}
	return NULL;*/
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

	sb->s_root = d_alloc_root(get_hellofs_inode(sb, 1, 0));
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
 * Reading from /dev/hello
 */
static ssize_t hello_comm_read(struct file * file, char * buf, size_t count, loff_t *ppos)
{
	//printk("Just check text: %s\n", currentCommand->command);
	//char new_command[100] = "cool stuff";
	// memcpy(currentCommand->command, new_command, 100);
	//memcpy(currentCommand->command, new_command, sizeof(new_command));
//	flag_wait = 0;
	//char *hello_str = "Hello, world!\n";

	//char *hello_str = "0";
	//if(currentCommand->waiting == 1) {
 	//	hello_str = "1";
 	//}
	
	//the_flag = 1;
	//wake_up_all(&flag_wait);
	
	//int len = strlen(hello_str);
	size_t len = sizeof(currentCommand->command);
	
	// read whole string
	if (count < len)
		return -EINVAL;

	if (*ppos != 0)
		return 0;

	if (copy_to_user(buf, currentCommand->command, len))
		return -EINVAL;

	*ppos = len;

	return len;
}


// echo "hello there" > /dev/hello
static ssize_t hello_comm_write(struct file *filep, const char *buff, size_t len, loff_t *off) {

//	message.memory = 
//	message.size = len;

	//printk("write(%p, %s, %d)", filep, buff, len);
	
	// kfree(currentCommand->data);
	currentCommand->data = (char *) kmalloc(len, GFP_KERNEL);
 	int i;
	//for(i=0; i<len && i<BUF_LEN; i++) {
	for(i=0; i<len; i++) {
		get_user(currentCommand->data[i], buff+i);
 	}
 	
  strcpy(currentCommand->command, "0");
	currentCommand->waiting = 0;
	currentCommand->the_flag = 1;
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

static int __init init_hellofs(void)
{
	int ret;
	
	currentCommand = &currentHelloCommand;
	currentCommand->waiting = 0;
	currentCommand->the_flag = 0;
  strcpy(currentCommand->command, "0");

	//char new_command[50] = "dir";
	// memcpy(currentCommand->command, new_command, 100);
	//memcpy(currentCommand->command, new_command, sizeof(new_command));
	//currentCommand->command = "dir";
	//printk("Command location: %p", &(currentCommand->command));
	//strcpy(currentCommand->command, "dir");

	ret = misc_register(&hello_comm_dev);
	if (ret)
		printk(KERN_ERR "Unable to register s3simple.\n");

	printk(KERN_ERR "Registered s3simple.\n");
	//return ret;

	return register_filesystem(&hellofs_fs_type);
}

static void __exit exit_hellofs(void)
{
	printk(KERN_INFO "Unloading s3simple.\n");

	unregister_filesystem(&hellofs_fs_type);
	
	misc_deregister(&hello_comm_dev);
}


module_init(init_hellofs)
module_exit(exit_hellofs)

MODULE_LICENSE("GPL");
