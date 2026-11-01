/*
 * Experimental: Not a part of clib2 from Olaf Barthel
 */

#ifndef _UNISTD_HEADERS_H
#include "unistd_headers.h"
#endif /* _UNISTD_HEADERS_H */

#ifndef _STDIO_HEADERS_H
#include "stdio_headers.h"
#endif /* _STDIO_HEADERS_H */

#ifndef _STDLIB_MEMORY_H
#include "stdlib_memory.h"
#endif /* _STDLIB_MEMORY_H */

/****************************************************************************/

/* There is no fork() here, so both ends of a pipe stay within the program:
   it is a buffer in memory that grows as needed.  PIPE: is not an alternative,
   as it need not be mounted, and has no ends that can be closed separately
   with an end-of-file to show for it. */
struct pipe_data
{
	char *	pd_Buffer;
	size_t	pd_Size;
	size_t	pd_Start;
	size_t	pd_End;
	int		pd_NumEnds;			/* 2, then 1, and it goes away at 0 */
	BOOL	pd_WriterClosed;
	BOOL	pd_ReaderClosed;
};

/****************************************************************************/

static int
read_pipe(struct fd * fd,struct pipe_data * pd,struct file_action_message * fam)
{
	size_t available;

	while(pd->pd_Start == pd->pd_End)
	{
		/* An end of file, once the other end is gone. */
		if(pd->pd_WriterClosed)
			return(0);

		if(FLAG_IS_SET(fd->fd_Flags,FDF_NON_BLOCKING))
		{
			fam->fam_Error = EAGAIN;
			return(ERROR);
		}

		/* Another thread of this program may write to it. */
		__fd_unlock(fd);

		__check_abort();

		PROFILE_OFF();
		Delay(1);
		PROFILE_ON();

		__fd_lock(fd);
	}

	available = pd->pd_End - pd->pd_Start;
	if(available > (size_t)fam->fam_Size)
		available = (size_t)fam->fam_Size;

	memmove(fam->fam_Data,&pd->pd_Buffer[pd->pd_Start],available);
	pd->pd_Start += available;

	return((int)available);
}

/****************************************************************************/

static int
write_pipe(struct pipe_data * pd,struct file_action_message * fam)
{
	size_t size = (size_t)fam->fam_Size;

	if(pd->pd_ReaderClosed)
	{
		fam->fam_Error = EPIPE;
		return(ERROR);
	}

	/* Move what is left to the front, and grow if that isn't enough. */
	if(pd->pd_End + size > pd->pd_Size)
	{
		size_t used = pd->pd_End - pd->pd_Start;

		memmove(pd->pd_Buffer,&pd->pd_Buffer[pd->pd_Start],used);

		pd->pd_Start = 0;
		pd->pd_End = used;

		if(used + size > pd->pd_Size)
		{
			size_t new_size = 2 * (used + size);
			char * new_buffer = realloc(pd->pd_Buffer,new_size);

			if(new_buffer == NULL)
			{
				fam->fam_Error = ENOMEM;
				return(ERROR);
			}

			pd->pd_Buffer = new_buffer;
			pd->pd_Size = new_size;
		}
	}

	memmove(&pd->pd_Buffer[pd->pd_End],fam->fam_Data,size);
	pd->pd_End += size;

	return((int)size);
}

/****************************************************************************/

static int
pipe_hook_entry(struct fd * fd,struct file_action_message * fam)
{
	struct pipe_data * pd = fd->fd_UserData;
	struct FileInfoBlock * fib;
	BOOL is_aliased;
	int result = ERROR;

	/* Closing changes the file descriptor table, and that needs the stdio
	   lock before this descriptor's. */
	if(fam->fam_Action == file_action_close)
		__stdio_lock();

	__fd_lock(fd);

	switch(fam->fam_Action)
	{
		case file_action_read:

			result = read_pipe(fd,pd,fam);
			break;

		case file_action_write:

			result = write_pipe(pd,fam);
			break;

		case file_action_close:

			result = OK;

			is_aliased = __fd_is_aliased(fd);
			if(is_aliased)
			{
				__remove_fd_alias(fd);
			}
			else
			{
				if(FLAG_IS_SET(fd->fd_Flags,FDF_WRITE))
					pd->pd_WriterClosed = TRUE;
				else
					pd->pd_ReaderClosed = TRUE;

				if(--pd->pd_NumEnds == 0)
				{
					free(pd->pd_Buffer);
					free(pd);
				}
			}

			__fd_unlock(fd);

			#if defined(__THREAD_SAFE)
			{
				if(NOT is_aliased)
					__delete_semaphore(fd->fd_Lock);
			}
			#endif /* __THREAD_SAFE */

			memset(fd,0,sizeof(*fd));

			fd = NULL;

			break;

		case file_action_seek:

			fam->fam_Error = ESPIPE;
			break;

		case file_action_set_blocking:

			if(fam->fam_Arg != 0)
				CLEAR_FLAG(fd->fd_Flags,FDF_NON_BLOCKING);
			else
				SET_FLAG(fd->fd_Flags,FDF_NON_BLOCKING);

			result = OK;
			break;

		case file_action_examine:

			fib = fam->fam_FileInfo;

			memset(fib,0,sizeof(*fib));

			fib->fib_DirEntryType = ST_PIPEFILE;

			PROFILE_OFF();
			DateStamp(&fib->fib_Date);
			PROFILE_ON();

			result = OK;
			break;

		default:

			fam->fam_Error = EBADF;
			break;
	}

	__fd_unlock(fd);

	if(fam->fam_Action == file_action_close)
		__stdio_unlock();

	return(result);
}

/****************************************************************************/

/* A vacant slot in the descriptor table, or -1. */
static int
new_fd(struct pipe_data * pd,ULONG flags)
{
	struct SignalSemaphore * lock = NULL;
	int fd_slot_number;

	fd_slot_number = __find_vacant_fd_entry();
	if(fd_slot_number < 0)
	{
		if(__grow_fd_table(0) < 0)
			return(-1);

		fd_slot_number = __find_vacant_fd_entry();
	}

	#if defined(__THREAD_SAFE)
	{
		lock = __create_semaphore();
		if(lock == NULL)
		{
			__set_errno(ENOMEM);
			return(-1);
		}
	}
	#endif /* __THREAD_SAFE */

	__initialize_fd(__fd[fd_slot_number],pipe_hook_entry,ZERO,FDF_IN_USE | flags,lock);

	__fd[fd_slot_number]->fd_UserData = pd;

	return(fd_slot_number);
}

/****************************************************************************/

int
pipe(int fildes[2])
{
	struct pipe_data * pd;
	int result = ERROR;

	if(fildes == NULL)
	{
		__set_errno(EFAULT);
		return(ERROR);
	}

	pd = calloc(1,sizeof(*pd));
	if(pd == NULL)
	{
		__set_errno(ENOMEM);
		return(ERROR);
	}

	pd->pd_NumEnds = 2;

	__stdio_lock();

	fildes[0] = new_fd(pd,FDF_READ);
	if(fildes[0] >= 0)
	{
		fildes[1] = new_fd(pd,FDF_WRITE);

		if(fildes[1] >= 0)
			result = OK;
		else
			close(fildes[0]);
	}

	__stdio_unlock();

	if(result != OK && fildes[0] < 0)
		free(pd);

	return(result);
}
