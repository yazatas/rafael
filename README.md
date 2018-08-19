# rafael

I've started to work on rafael again and the development has been going quite good so far. I have very rudimentary understanding of FS concepts so I think the first version of rafael (the one that's integrated into micael) is going to be crappy but I'll keep honing it until it's perfect.

Right now I've finished the mkfs/mount (and related disk I/O) routines. Next I'm going to start working on the inode CRUD support so that this file system has actually something useful to offer.

# Logging

rafael shall log extensively to make the future debugging less painful. The code base will be full of debug message of different levels and each function should log something.

rafael has 4 logging levels: debug, info, warning and emergency. The level of debugging can be set during compile time. In the future it may be possible to control it during run time.

* LOG_DEBUG
   * Debug messages should be used often but they should only carry information that would be helpful if you're debugging the system (f.ex. message that would flood logs should be debug level)
* LOG_INFO
   * This should the most often used logging level. All non-error related information which the user might benefit from knowing should be logged as info
* LOG_WARN
   * This is very rarely used log level but still useful in same cases. For example, mounted file system having dirty flag set would be a reason to use this log level.
* LOG_ERROR
   * All non-fatal error conditions are logged as errors. This includes failure to open a file or allocating an inode number
* LOG_EMERG
   * Emergency log messages are reserved, you guessed it, for emergencies. For example, failure to open disk or mounting the file system are considered emergencies.

# Return values

If the return value is a non-NULL pointer or an integer greater or equal to zero (depending on the function, see below) the function has succeeded.
If a return value is negative or NULL an error has occurred.

Examples:
   * rfs_alloc_inode() will return a pointer to inode on success and NULL on error
   * rfs_read_buf() will return the number of bytes read on success and negative errno on error
   * rfs_dir_insert() will return 0 on success and negated errno on error

# Disk layout

rafael has the following disk layout:

* Booting stuff (1 block)
   * Basic bootloader (512 bytes)
   * Second stage boot loader (3584 bytes [max])
* Superblock (1 block)
   * This is a little overkill as superblock is below 256 bytes but whatevs
* Inode bitmap (N blocks)
   * Size depends on the disk size
* Data block bitmap (N blocks)
   * Size depends on the disk size
* Inode map (N blocks)
   * Size depends on the disk size
* Data blocks (rest of the disk)

# TODO

Below is a list of stuff that should be done ASAP (roughly in that order too):
   * Inode CRUD support
   * Standardize return values (object/error code/bytes read or written?)
   * Some kind of partial caching for bitmaps
   * Block allocator
   * File and directory creation support
   * Block cache
   * Add unit testing
   * Journaling
      * Metadata journaling
      * Block journaling
   * Data compression
   * Data encryption

# Copying
rafael is free software. It's licensed under the MIT license.
