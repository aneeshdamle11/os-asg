#include <stdio.h>

int get_block_group(int fd);
int get_inode_table_location(int fd, int block_group);

int main(int argc, char *argv[])
{
    /* usage: arg check */
    if (argc != 3) {
        printf("Usage: ./a.out device-file-name inode-number\n");
        return 1;
    }

    int fd, block_group, inode_table_pos;
    fd = open(argv[1], O_RDONLY);

    block_group = get_block_group(fd);
    inode_table_pos = get_inode_table_location(fd, block_group);
    offset = get_inode_offset(fd, inode_table_pos, atoi(argv[2]));

    return 0;
}

int get_block_group(int fd) {
    return 0;
}

int get_inode_table_location(int fd, int block_group) {
    return 0;
}


