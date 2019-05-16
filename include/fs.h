#pragma once
#include "types.h"

//From James Molloy Tutorial

#define FS_FILE 0x01
#define FS_DIRECTORY 0x02
#define FS_CHARDEVICE 0x03
#define FS_BLOCKDEVICE 0x04
#define FS_PIPE 0x05
#define FS_SYMLINK 0x06
#define FS_MOUNTPOINT 0x08

struct fs_node;

typedef uint32_t ino_t;
//The directory stores child files like this (directory entry):

#define FS_NAME_MAX_LEN 64
#define BLOCK_SIZE 512

struct file;
struct dentry;
struct inode;

typedef int (*read_file_t)(struct file *, uint8_t *buf, uint32_t size,
			   uint32_t *offset);
typedef int (*write_file_t)(struct file *, uint32_t, uint32_t, uint8_t *);
typedef void (*open_type_t)(struct file *);
typedef void (*close_type_t)(struct file *);

typedef struct {
	read_file_t read;
} file_op_t;

//mount = mount_bdev/mount_nodev/mount_mtd

typedef struct super_operations {
	/*struct inode *(*alloc_inode)(struct super_block *sb);
	void (*destroy_inode)(struct inode *);

   	void (*dirty_inode) (struct inode *, int flags);
	int (*write_inode) (struct inode *, struct writeback_control *wbc);
	int (*drop_inode) (struct inode *);
	void (*evict_inode) (struct inode *);
	void (*put_super) (struct super_block *);
	int (*sync_fs)(struct super_block *sb, int wait);
	int (*freeze_super) (struct super_block *);
	int (*freeze_fs) (struct super_block *);
	int (*thaw_super) (struct super_block *);
	int (*unfreeze_fs) (struct super_block *);
	int (*statfs) (struct dentry *, struct kstatfs *);
	int (*remount_fs) (struct super_block *, int *, char *);
	void (*umount_begin) (struct super_block *);

	int (*show_options)(struct seq_file *, struct dentry *);
	int (*show_devname)(struct seq_file *, struct dentry *);
	int (*show_path)(struct seq_file *, struct dentry *);
	int (*show_stats)(struct seq_file *, struct dentry *);
#ifdef CONFIG_QUOTA
	ssize_t (*quota_read)(struct super_block *, int, char *, size_t, loff_t);
	ssize_t (*quota_write)(struct super_block *, int, const char *, size_t, loff_t);
	struct dquot **(*get_dquots)(struct inode *);
#endif
	int (*bdev_try_to_free_page)(struct super_block*, struct page*, gfp_t);
	long (*nr_cached_objects)(struct super_block *,
				  struct shrink_control *);
	long (*free_cached_objects)(struct super_block *,
				    struct shrink_control *);*/
} super_operations_t;

typedef struct inode_operations {
	//struct dentry *(*lookup)(struct inode *, struct dentry *, unsigned int);
	struct inode *(*find_child)(struct inode *parent, char *name);
	struct inode *(*get_child)(struct inode *parent, uint32_t index);

	int (*open)(struct inode, uint32_t);
	int (*close)(struct inode);
	int (*read)(struct inode *, void *buf, uint32_t offset, uint32_t size);
	int (*write)(struct inode *, void *buf, uint32_t offset, uint32_t size);

	/*const char * (*get_link) (struct dentry *, struct inode *, struct delayed_call *);
	int (*permission) (struct inode *, int);
	struct posix_acl * (*get_acl)(struct inode *, int);

	int (*readlink) (struct dentry *, char __user *,int);

	int (*create) (struct inode *,struct dentry *, umode_t, bool);
	int (*link) (struct dentry *,struct inode *,struct dentry *);
	int (*unlink) (struct inode *,struct dentry *);
	int (*symlink) (struct inode *,struct dentry *,const char *);
	int (*mkdir) (struct inode *,struct dentry *,umode_t);
	int (*rmdir) (struct inode *,struct dentry *);
	int (*mknod) (struct inode *,struct dentry *,umode_t,dev_t);
	int (*rename) (struct inode *, struct dentry *,
			struct inode *, struct dentry *, unsigned int);
	int (*setattr) (struct dentry *, struct iattr *);
	int (*getattr) (const struct path *, struct kstat *, uint32_t, unsigned int);
	ssize_t (*listxattr) (struct dentry *, char *, size_t);
	int (*fiemap)(struct inode *, struct fiemap_extent_info *, u64 start,
		      u64 len);
	int (*update_time)(struct inode *, struct timespec64 *, int);
	int (*atomic_open)(struct inode *, struct dentry *,
			   struct file *, unsigned open_flag,
			   umode_t create_mode);
	int (*tmpfile) (struct inode *, struct dentry *, umode_t);
	int (*set_acl)(struct inode *, struct posix_acl *, int);*/
} inode_operations_t;

typedef struct {
	super_operations_t *s_op;
} superblock_t;

typedef struct inode {
	ino_t ino;

	uint8_t type;

	size_t size;

	inode_operations_t *i_op;

	struct inode *mount;
} inode_t;

typedef struct {
	struct dentry *mnt_root;
	superblock_t *mnt_sb;
} vfs_mount_t;

typedef struct {
	uint32_t ino;
	char name[FS_NAME_MAX_LEN];
} __attribute__((packed)) ramfs_dirent_t;

typedef struct {
	uint32_t numDir;
	ramfs_dirent_t dirents[6];
} __attribute__((packed)) ramfs_dir_t;

typedef struct {
	vfs_mount_t *mnt;
	//vfs_node_t *dentry;
} path_t;

typedef struct file {
	//path_t *path;
	inode_t *f_inode;
} file_t;

int vfs_read(file_t *file, void *buf, size_t count, size_t offset);