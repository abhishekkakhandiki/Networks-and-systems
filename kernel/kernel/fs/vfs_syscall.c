/******************************************************************************/
/* Important Fall 2015 CSCI 402 usage information:                            */
/*                                                                            */
/* This fils is part of CSCI 402 kernel programming assignments at USC.       */
/* Please understand that you are NOT permitted to distribute or publically   */
/*         display a copy of this file (or ANY PART of it) for any reason.    */
/* If anyone (including your prospective employer) asks you to post the code, */
/*         you must inform them that you do NOT have permissions to do so.    */
/* You are also NOT permitted to remove or alter this comment block.          */
/* If this comment block is removed or altered in a submitted file, 20 points */
/*         will be deducted.                                                  */
/******************************************************************************/

/*
 *  FILE: vfs_syscall.c
 *  AUTH: mcc | jal
 *  DESC:
 *  DATE: Wed Apr  8 02:46:19 1998
 *  $Id: vfs_syscall.c,v 1.10 2014/12/22 16:15:17 william Exp $
 */

#include "kernel.h"
#include "errno.h"
#include "globals.h"
#include "fs/vfs.h"
#include "fs/file.h"
#include "fs/vnode.h"
#include "fs/vfs_syscall.h"
#include "fs/open.h"
#include "fs/fcntl.h"
#include "fs/lseek.h"
#include "mm/kmalloc.h"
#include "util/string.h"
#include "util/printf.h"
#include "fs/stat.h"
#include "util/debug.h"

/* To read a file:
 *      o fget(fd)
 *      o call its virtual read fs_op
 *      o update f_pos
 *      o fput() it
 *      o return the number of bytes read, or an error
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EBADF
 *        fd is not a valid file descriptor or is not open for reading.
 *      o EISDIR
 *        fd refers to a directory.
 *
 * In all cases, be sure you do not leak file refcounts by returning before
 * you fput() a file that you fget()'ed.
 */
int do_read(int fd, void *buf, size_t nbytes) {

	dbg(DBG_PRINT, "GRADING2B in do_read\n");

	if (fd < 0 || fd >= NFILES || curproc->p_files[fd] == NULL) {
		dbg(DBG_PRINT, "GRADING2B in do_read\n");
		return -EBADF;
	}
	file_t *file = fget(fd);

	if (S_ISDIR(file->f_vnode->vn_mode)) {
		fput(file);
		dbg(DBG_PRINT, "GRADING2B in do_read\n");
		return -EISDIR;
	}

	int bytesread;

	bytesread = file->f_vnode->vn_ops->read(file->f_vnode, file->f_pos, buf,
			nbytes);
	if (bytesread == 0) {
		dbg(DBG_PRINT, "GRADING2B in do_read\n");

		if (file->f_mode != FMODE_READ
				&& file->f_mode != (FMODE_READ | FMODE_WRITE)) {
			dbg(DBG_PRINT, "GRADING2B in do_read\n");
			fput(file);
			return -EBADF;
		}
		fput(file);

		return 0;
	}

	file->f_pos = file->f_pos + bytesread;
	fput(file);
	return bytesread;

}

/* Very similar to do_read.  Check f_mode to be sure the file is writable.  If
 * f_mode & FMODE_APPEND, do_lseek() to the end of the file, call the write
 * fs_op, and fput the file.  As always, be mindful of refcount leaks.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EBADF
 *        fd is not a valid file descriptor or is not open for writing.
 */
int do_write(int fd, const void *buf, size_t nbytes) {

	dbg(DBG_PRINT, "GRADING2B in do_write\n");
	if (fd < 0 || fd > NFILES || curproc->p_files[fd] == NULL) {

		dbg(DBG_PRINT, "GRADING2B in do_write\n");
		return -EBADF;
	}

	file_t *file = fget(fd);

	int file_old_pos = file->f_pos;

	if (!(file->f_mode & FMODE_APPEND) && !(file->f_mode & FMODE_WRITE)) {

		dbg(DBG_PRINT, "GRADING2B in do_write\n");

		fput(file);
		return -EBADF;
	}
	if ((file->f_mode & FMODE_APPEND) || (file->f_mode & FMODE_WRITE)) {

		if (file->f_mode & FMODE_APPEND) {
			do_lseek(fd, 0, SEEK_END);
		}
		int byteswrote;
		byteswrote = file->f_vnode->vn_ops->write(file->f_vnode, file->f_pos,
				buf, nbytes);
		if (byteswrote > 0) {
			dbg(DBG_PRINT, "GRADING 2B in do_write\n");
			KASSERT(
					(S_ISCHR(file->f_vnode->vn_mode)) || (S_ISBLK(file->f_vnode->vn_mode)) || ((S_ISREG(file->f_vnode->vn_mode)) && (file->f_pos <= file->f_vnode->vn_len)));
			dbg(DBG_PRINT,
					"GRADING2A 3.a file position is less than or equal to file->f_vnode->vn_len \n");
			/*file_old_pos+=byteswrote;
			 file->f_pos =file_old_pos;*/

			file->f_pos += byteswrote;

			fput(file);
			return byteswrote;
		}

	}

	return -EBADF;
}

/*
 * Zero curproc->p_files[fd], and fput() the file. Return 0 on success
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EBADF
 *        fd isn't a valid open file descriptor.
 */
int do_close(int fd) {

	dbg(DBG_PRINT, "GRADING2B in do_close\n");
	if (!(fd < 0 || fd >= NFILES || curproc->p_files[fd] == NULL)) {
		dbg(DBG_PRINT, "GRADING2B in do_close\n");
		fput(curproc->p_files[fd]);
		curproc->p_files[fd] = NULL;
		return 0;

	}
	return -EBADF;
}

/* To dup a file:
 *      o fget(fd) to up fd's refcount
 *      o get_empty_fd()
 *      o point the new fd to the same file_t* as the given fd
 *      o return the new file descriptor
 *
 * Don't fput() the fd unless something goes wrong.  Since we are creating
 * another reference to the file_t*, we want to up the refcount.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EBADF
 *        fd isn't an open file descriptor.
 *      o EMFILE
 *        The process already has the maximum number of file descriptors open
 *        and tried to open a new one.
 */
int do_dup(int fd) {
	int fdnew = 0;
	dbg(DBG_PRINT, "GRADING2B in do_dup\n");
	if (fd >= 0 && fd < NFILES && curproc->p_files[fd]) {
		dbg(DBG_PRINT, "GRADING2B in do_dup\n");
		fdnew = get_empty_fd(curproc);
		file_t *f = fget(fd);

		if (fdnew >= 0 && fdnew < NFILES) {
			dbg(DBG_PRINT, "GRADING2B in do_dup\n");
			if (f != NULL) {
				dbg(DBG_PRINT, "GRADING2B in do_dup\n");
				curproc->p_files[fdnew] = f;
				return fdnew;
			}
			fput(f);
			return -EBADF;
		}

	} else {
		dbg(DBG_PRINT, "GRADING2B in do_dup\n");
		return -EBADF;
	}
	return fdnew;

}

/* Same as do_dup, but insted of using get_empty_fd() to get the new fd,
 * they give it to us in 'nfd'.  If nfd is in use (and not the same as ofd)
 * do_close() it first.  Then return the new file descriptor.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EBADF
 *        ofd isn't an open file descriptor, or nfd is out of the allowed
 *        range for file descriptors.
 */
int do_dup2(int ofd, int nfd) {
	dbg(DBG_PRINT, "GRADING2B in do_dup2\n");
	if (!(ofd >= 0 && ofd < NFILES && curproc->p_files[ofd] != NULL)) {
		dbg(DBG_PRINT, "GRADING2B in do_dup2\n");
		return -EBADF;
	}

	if (!(nfd >= 0 && nfd < NFILES)) {
		dbg(DBG_PRINT, "GRADING2B in do_dup2\n");
		return -EBADF;
	}

	file_t* f_old = fget(ofd);
	dbg(DBG_PRINT, "GRADING2B in do_dup2\n");
	file_t* f_new = fget(nfd);
	if (ofd != nfd && f_new != NULL) {
		dbg(DBG_PRINT, "GRADING2B in do_dup2\n");
		fput(f_new);
		int retval = 0;

		do_close(nfd);
	} else if (ofd == nfd) {
		dbg(DBG_PRINT, "GRADING2B in do_dup2\n");
		fput(f_old);
		fput(f_new);
	}

	curproc->p_files[nfd] = f_old;
	return nfd;

}

/*
 * This routine creates a special file of the type specified by 'mode' at
 * the location specified by 'path'. 'mode' should be one of S_IFCHR or
 * S_IFBLK (you might note that mknod(2) normally allows one to create
 * regular files as well-- for simplicity this is not the case in Weenix).
 * 'devid', as you might expect, is the device identifier of the device
 * that the new special file should represent.
 *
 * You might use a combination of dir_namev, lookup, and the fs-specific
 * mknod (that is, the containing directory's 'mknod' vnode operation).
 * Return the result of the fs-specific mknod, or an error.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EINVAL
 *        mode requested creation of something other than a device special
 *        file.
 *      o EEXIST
 *        path already exists.
 *      o ENOENT
 *        A directory component in path does not exist.
 *      o ENOTDIR
 *        A component used as a directory in path is not, in fact, a directory.
 *      o ENAMETOOLONG
 *        A component of path was too long.
 */
int do_mknod(const char *path, int mode, unsigned devid) {
	dbg(DBG_PRINT, "GRADING2B in do_mknod\n");
	int lookupret;
	const char* name;
	size_t pathlen = 0;
	vnode_t *res_vnode, *result;

	lookupret = dir_namev(path, &pathlen, &name, NULL, &res_vnode);

	vput(res_vnode);
	lookupret = lookup(res_vnode, name, pathlen, &result);

	if (lookupret == -ENOENT) {
		dbg(DBG_PRINT, "GRADING2B in do_mknod\n");
		switch (mode) {
		case S_IFCHR:
			dbg(DBG_PRINT, "GRADING2B in do_mknod\n");
			KASSERT(NULL != res_vnode->vn_ops->mknod);
			dbg(DBG_PRINT, "GRADING2A 3.b res_vnode is not null\n");
			lookupret = res_vnode->vn_ops->mknod(res_vnode, name, pathlen,
			S_IFCHR, devid);
			return lookupret;
		}
	}
	return lookupret;

}

/* Use dir_namev() to find the vnode of the dir we want to make the new
 * directory in.  Then use lookup() to make sure it doesn't already exist.
 * Finally call the dir's mkdir vn_ops. Return what it returns.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EEXIST
 *        path already exists.
 *      o ENOENT
 *        A directory component in path does not exist.
 *      o ENOTDIR
 *        A component used as a directory in path is not, in fact, a directory.
 *      o ENAMETOOLONG
 *        A component of path was too long.
 */
int do_mkdir(const char *path) {

	dbg(DBG_PRINT, "GRADING2B in do_mkdir\n");
	int retval = 0, lookupret = 0;
	size_t namelen = 0;
	const char *name;
	vnode_t *res_vnode;

	retval = dir_namev(path, &namelen, &name, NULL, &res_vnode);
	if (retval < 0) {
		dbg(DBG_PRINT, "GRADING2B in do_mkdir\n");
		return retval;
	}

	vput(res_vnode);
	lookupret = lookup(res_vnode, name, namelen, &res_vnode);

	if (lookupret == 0) {

		dbg(DBG_PRINT, "GRADING 2B in do_mkdir() \n");
		vput(res_vnode);
		return -EEXIST;
	} else if (lookupret == -ENOENT) {
		KASSERT(NULL != res_vnode->vn_ops->mkdir);
		dbg(DBG_PRINT, "GRADING2A 3.c res_vnode is not null\n");
		lookupret = res_vnode->vn_ops->mkdir(res_vnode, name, namelen);
		return lookupret;
	}

	return lookupret;

}

/* Use dir_namev() to find the vnode of the directory containing the dir to be
 * removed. Then call the containing dir's rmdir v_op.  The rmdir v_op will
 * return an error if the dir to be removed does not exist or is not empty, so
 * you don't need to worry about that here. Return the value of the v_op,
 * or an error.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EINVAL
 *        path has "." as its final component.
 *      o ENOTEMPTY
 *        path has ".." as its final component.
 *      o ENOENT
 *        A directory component in path does not exist.
 *      o ENOTDIR
 *        A component used as a directory in path is not, in fact, a directory.
 *      o ENAMETOOLONG
 *        A component of path was too long.
 */
int do_rmdir(const char *path) {

	dbg(DBG_PRINT, "GRADING2B in do_rmdir\n");
	const char * name;
	size_t namelen = 0;
	vnode_t *res_vnode, *result;
	int retval, lookupret;

	retval = dir_namev(path, &namelen, &name, NULL, &res_vnode);
	if (retval < 0) {

		dbg(DBG_PRINT, "GRADING2B in do_rmdir\n");
		return retval;
	}

	if (path[strlen(path) - 1] == '.') {

		dbg(DBG_PRINT, "GRADING2B in do_rmdir\n");
		if (path[strlen(path) - 2] == '.') {

			dbg(DBG_PRINT, "GRADING2B in do_rmdir\n");
			vput(res_vnode);
			return -ENOTEMPTY;
		} else {

			dbg(DBG_PRINT, "GRADING2B in do_rmdir\n");
			vput(res_vnode);
		}
		return -EINVAL;
	}

	dbg(DBG_PRINT, "GRADING2B in do_rmdir\n");
	KASSERT(NULL != res_vnode->vn_ops->rmdir);
	dbg(DBG_PRINT, "GRADING2A 3.d res_vnode is not null\n");

	retval = res_vnode->vn_ops->rmdir(res_vnode, name, namelen);
	vput(res_vnode);
	if (retval < 0) {

		dbg(DBG_PRINT, "GRADING 2B in do_rmdir \n");
		return retval;

	}
	return 0;
}

/*
 * Same as do_rmdir, but for files.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EISDIR
 *        path refers to a directory.
 *      o ENOENT
 *        A component in path does not exist.
 *      o ENOTDIR
 *        A component used as a directory in path is not, in fact, a directory.
 *      o ENAMETOOLONG
 *        A component of path was too long.
 */
int do_unlink(const char *path) {

	dbg(DBG_PRINT, "GRADING 2B in do_unlink \n");
	const char *name;
	size_t namelen = 0;
	vnode_t *res_vnode, *result;
	int catchlook;
	int catcherr = dir_namev(path, &namelen, &name, NULL, &res_vnode);

	catcherr = lookup(res_vnode, name, namelen, &result);

	if (catcherr < 0) {

		dbg(DBG_PRINT, "GRADING 2B in do_unlink \n");
		vput(res_vnode);
		return catcherr;
	}
	if (S_ISDIR(result->vn_mode) == 1) {
		dbg(DBG_PRINT, "GRADING 2B in do_unlink \n");
		vput(result);
		vput(res_vnode);

		return -EISDIR;
	}
	dbg(DBG_PRINT, "GRADING 2B in do_unlink \n");
	KASSERT(NULL != res_vnode->vn_ops->unlink);

	dbg(DBG_PRINT, "GRADING2A 3.b res_vnode is not null\n");

	catcherr = res_vnode->vn_ops->unlink(res_vnode, name, namelen);

	vput(res_vnode);
	vput(result);
	return catcherr;

}

/* To link:
 *      o open_namev(from)
 *      o dir_namev(to)
 *      o call the destination dir's (to) link vn_ops.
 *      o return the result of link, or an error
 *
 * Remember to vput the vnodes returned from open_namev and dir_namev.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EEXIST
 *        to already exists.
 *      o ENOENT
 *        A directory component in from or to does not exist.
 *      o ENOTDIR
 *        A component used as a directory in from or to is not, in fact, a
 *        directory.
 *      o ENAMETOOLONG
 *        A component of from or to was too long.
 *      o EISDIR
 *        from is a directory.
 */
int do_link(const char *from, const char *to) {
	NOT_YET_IMPLEMENTED("VFS: do_link");
	return -1;
}

/*      o link newname to oldname
 *      o unlink oldname
 *      o return the value of unlink, or an error
 *
 * Note that this does not provide the same behavior as the
 * Linux system call (if unlink fails then two links to the
 * file could exist).
 */
int do_rename(const char *oldname, const char *newname) {
	NOT_YET_IMPLEMENTED("VFS: do_rename");
	return -1;
}

/* Make the named directory the current process's cwd (current working
 * directory).  Don't forget to down the refcount to the old cwd (vput()) and
 * up the refcount to the new cwd (open_namev() or vget()). Return 0 on
 * success.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o ENOENT
 *        path does not exist.
 *      o ENAMETOOLONG
 *        A component of path was too long.
 *      o ENOTDIR
 *        A component of path is not a directory.
 */
int do_chdir(const char *path) {

	dbg(DBG_PRINT, "GRADING2B in do_chdir\n");

	vnode_t *res_vnode;

	int catchopenv;
	catchopenv = open_namev(path, O_RDONLY, &res_vnode, NULL);
	if (catchopenv < 0) {
		dbg(DBG_PRINT, "GRADING 2B in do_chdir \n");
		return catchopenv;
	}
	if (S_ISDIR(res_vnode->vn_mode) == 0) {
		dbg(DBG_PRINT, "GRADING 2B in do_chdir \n");
		vput(res_vnode);
		return -ENOTDIR;
	}
	vput(curproc->p_cwd);
	curproc->p_cwd = res_vnode;

	return 0;
}

/* Call the readdir fs_op on the given fd, filling in the given dirent_t*.
 * If the readdir fs_op is successful, it will return a positive value which
 * is the number of bytes copied to the dirent_t.  You need to increment the
 * file_t's f_pos by this amount.  As always, be aware of refcounts, check
 * the return value of the fget and the virtual function, and be sure the
 * virtual function exists (is not null) before calling it.
 *
 * Return either 0 or sizeof(dirent_t), or -errno.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EBADF
 *        Invalid file descriptor fd.
 *      o ENOTDIR
 *        File descriptor does not refer to a directory.
 */
int do_getdent(int fd, struct dirent *dirp) {

	dbg(DBG_PRINT, "GRADING2B in do_getdent\n");
	if (fd < 0 || fd > NFILES || curproc->p_files[fd] == NULL) {
		dbg(DBG_PRINT, "GRADING 2B in do_getdent\n");
		return -EBADF;
	}
	file_t *f;
	f = fget(fd);
	int catchreadd = 0;

	if (!S_ISDIR(f->f_vnode->vn_mode)) {
		dbg(DBG_PRINT, "GRADING 2B in do_getdent \n");
		fput(f);
		return -ENOTDIR;
	}
	if (f != NULL) {
		dbg(DBG_PRINT, "GRADING 2B in do_getdent \n");
		catchreadd = f->f_vnode->vn_ops->readdir(f->f_vnode, f->f_pos, dirp);
		f->f_pos += catchreadd;
		fput(f);
		if (catchreadd == 0) {
			dbg(DBG_PRINT, "GRADING 2B in do_getdent \n");
			return 0;
		}

		return sizeof(struct dirent);

	}
	return catchreadd;
}

/*
 * Modify f_pos according to offset and whence.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o EBADF
 *        fd is not an open file descriptor.
 *      o EINVAL
 *        whence is not one of SEEK_SET, SEEK_CUR, SEEK_END; or the resulting
 *        file offset would be negative.
 */
int do_lseek(int fd, int offset, int whence) {
	dbg(DBG_PRINT, "GRADING 2B in do_lseek \n");
	if (whence != SEEK_SET) {

		if (whence != SEEK_CUR) {
			if (whence != SEEK_END) {
				dbg(DBG_PRINT, "GRADING 2B in lseek \n");
				return -EINVAL;
			}
			dbg(DBG_PRINT, "GRADING 2B in lseek \n");
		}
		dbg(DBG_PRINT, "GRADING 2B in lseek \n");
	}

	if (fd < 0 || fd >= NFILES || curproc->p_files[fd] == NULL) {
		dbg(DBG_PRINT, "GRADING 2B in lseek \n");
		return -EBADF;
	}
	file_t *f;
	f = fget(fd);

	int retval;

	int file_old_pos;
	file_old_pos = f->f_pos;

	switch (whence) {
	case SEEK_SET:
		dbg(DBG_PRINT, "GRADING 2B in lseek \n");
		file_old_pos = offset;
		retval = file_old_pos;

		break;
	case SEEK_CUR:
		dbg(DBG_PRINT, "GRADING 2B in lseek \n");
		file_old_pos += offset;
		retval = file_old_pos;
		break;
	case SEEK_END:
		dbg(DBG_PRINT, "GRADING 2B in lseek \n");
		file_old_pos = f->f_vnode->vn_len + offset;
		retval = file_old_pos;
		break;

	}

	fput(f);
	if (retval < 0) {
		dbg(DBG_PRINT, "GRADING 2B in lseek \n");
		f->f_pos = 0;
		return -EINVAL;
	}
	f->f_pos = file_old_pos;
	return retval;

}
/*
 * Find the vnode associated with the path, and call the stat() vnode operation.
 *
 * Error cases you must handle for this function at the VFS level:
 *      o ENOENT
 *        A component of path does not exist.
 *      o ENOTDIR
 *        A component of the path prefix of path is not a directory.
 *      o ENAMETOOLONG
 *        A component of path was too long.
 */
int do_stat(const char *path, struct stat *buf) {
	dbg(DBG_PRINT, "GRADING 2B in do_stat\n");

	vnode_t *res_vnode;
	const char * name;
	size_t namelen;
	int retval = 0;

	retval = dir_namev(path, &namelen, &name, NULL, &res_vnode);
	if (retval < 0) {

		dbg(DBG_PRINT, "GRADING 2B in do_stat \n");
		return retval;
	}
	vput(res_vnode);

	retval = lookup(res_vnode, name, namelen, &res_vnode);
	if (retval < 0) {

		dbg(DBG_PRINT, "GRADING 2B in do_stat \n");
		return retval;
	}
	vput(res_vnode);
	dbg(DBG_PRINT, "GRADING 2B in do_stat\n");
	KASSERT(NULL!=res_vnode->vn_ops->stat);
	retval = res_vnode->vn_ops->stat(res_vnode, buf);

	return retval;

}

#ifdef __MOUNTING__
/*
 * Implementing this function is not required and strongly discouraged unless
 * you are absolutely sure your Weenix is perfect.
 *
 * This is the syscall entry point into vfs for mounting. You will need to
 * create the fs_t struct and populate its fs_dev and fs_type fields before
 * calling vfs's mountfunc(). mountfunc() will use the fields you populated
 * in order to determine which underlying filesystem's mount function should
 * be run, then it will finish setting up the fs_t struct. At this point you
 * have a fully functioning file system, however it is not mounted on the
 * virtual file system, you will need to call vfs_mount to do this.
 *
 * There are lots of things which can go wrong here. Make sure you have good
 * error handling. Remember the fs_dev and fs_type buffers have limited size
 * so you should not write arbitrary length strings to them.
 */
int
do_mount(const char *source, const char *target, const char *type)
{
	NOT_YET_IMPLEMENTED("MOUNTING: do_mount");
	return -EINVAL;
}

/*
 * Implementing this function is not required and strongly discouraged unless
 * you are absolutley sure your Weenix is perfect.
 *
 * This function delegates all of the real work to vfs_umount. You should not worry
 * about freeing the fs_t struct here, that is done in vfs_umount. All this function
 * does is figure out which file system to pass to vfs_umount and do good error
 * checking.
 */
int
do_umount(const char *target)
{
	NOT_YET_IMPLEMENTED("MOUNTING: do_umount");
	return -EINVAL;
}
#endif
