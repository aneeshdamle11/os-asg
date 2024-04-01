#include <stdio.h>

int main(int argc, char *argv[]) {
    int fd, count, ngroups, i;
    struct ext2_super_block sb;
    struct ext2_group_desc bgdesc[1024];

    fd = open(argv[1], O_RDONLY); // argv[1] = /dev/sdb1
    if (fd == -1) {
        perror("readsuper:");
        exit(errno);
    }
    // skip the bootblock
    lseek(fd, 1024, SEEK_SET);
    printf("size of super block = %lu\n", sizeof(struct ex2_super_block));

    // Read super block
    count = read(fd, &sb, sizeof(struct ext2_super_block));
    printf("Magic: %x\n", sb.s_magic);
    printf("Inodes count: %d\n", sb.s_inodes_count);
    printf("block size entry: %d\n", sb.s_log_block_size); // 1024 left shifted by actual block size

    // Read all block group descriptors
    ngroups = sb.s_blocks_count / sb.s_block_per_group;
    lseek(fd, 1024 + 1024 << sb.s_log_block_size, SEEK_SET); // skip bootblock + superblock of first group
    count = read(fd, bgdesc, sizeof(struct ext2_group_desc) * ngroups);
    for (i = 0; i < ngroups; ++i) {
        printf("%d): Indoe Table: %d\n", i, bgdesc[i].bg_inode_table);
    }

    return 0;
}


