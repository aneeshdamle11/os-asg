// #include <ext2fs/ext2_fs.h>

int main(int argc, char *argv[])
{
    int fd;
    int count;
    int ino, i;
    int bgno, iindex, inode_size, block_size;
    unsigned long inode_offset;
    struct ext2_super_block sb;
    struct ext2_inode inode;
    struct ext2_group_desc bgdesc;

    ino = atoi(argv[2]);
    fd = open(argv[1], O_RDONLY); // argv[1] = /dev/sdb
    if (fd == -1) {
        perror("readsuper:");
        exit(errno);
    }

    lseek64(fd, 1024, SEEK_CUR);
    printf("size of super block = %lu\n", sizeof(struct ext2_super_block));
    count = read(fd, &sb, sizeof(struct ext2_super_block));
    printf("Magic: %x\n", sb.s_magic);
    printf("Inodes count: %d\n", sb.s_inodes_count);
    printf("Size of BG DESC = %lu\n", sizeof(struct ext2_group_desc));
    inode_size = sb.s_inode_size;
    block_size = 1024 << sb.s_log_block_size;

    bgno = (ino - 1) / sb.s_inodes_per_group;
    iindex = (ino - 1) % sb.s_inodes_per_group;
    lseek64(fd, 1024 + block_size * sizeof(bgdesc), SEEK_SET);
    count = read(fd, &bgdesc, sizeof(struct ext2_group_desc));
    printf("Inode Table: %d\n", bgdesc.bg_inode_table);

    inode_offset = bgdesc.bg_inode_table * block_size + iindex * inode_size;
    lseek64(fd, inode_offset, SEEK_SET);
    read(fd, &inode, sizeof(inode));
    printf("size of file: %d\n", inode.i_size);
    printf("number of blocks: %d\nblocks: ", inode.i_blocks);
    for (i = 0; i < inode.i_blocks; ++i) {
        printf("%d, ", inode.i_block[i]);
    }
    printf("\n");
    close(fd);

    return 0;
}
