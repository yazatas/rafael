# rafael

I've started to work on rafael again and the development has been going quite good so far. I have very rudimentary understanding of FS concepts so I think the first version of rafael (the one that's integrated into micael) but I'll keep honing it until it's perfect.

Right now I've finished the mkfs routine (and related disk I/O) routines. Next I'm going to start working on the inode CRUD support this file system has actually something useful to offer.

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
   * File and directory creation support
   * Block cache
   * Journaling

# Copying
rafael is free software. It's licensed under the MIT license.
