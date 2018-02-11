# rafael

main.c is the operating system and hdd_file is an HDD

I decided to write file system separately and add it to micael after it works (maybe never). It's much easier to debug when the only thing I have to worry about rafael and not micael and all its peculiarities.
When rafael is in working condition, I can just transfer all files to micael and only rewrite driver.c.

rafael assumes the first 512 bytes of a device is reserved for booting and the superblock is written right after that.

`make; ./main`

# Copying
rafael is free software. It's licensed under the MIT license.
