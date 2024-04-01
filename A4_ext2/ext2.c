/* OS ASSIGNMENT EXT2 - PART B
 * Name: Aneesh Damle
 * MIS: 112003032
 * TY COMPUTER DIV : 1 (ADS-T3)
 */

/* Given the complete path name, prints
 * a. inode of the file/directory specified, from an ext2 file-system
 * b. contents of the file or directory */
#include <stdio.h>
#include <string.h>
#include <ext2fs/ext2_fs.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#define BLOCK_SIZE (1024 << superblock.s_log_block_size)

// Global
struct ext2_super_block superblock;
struct ext2_group_desc bgdesc;
int isfile = 0;

// Function declarations
struct ext2_inode get_inode(int fd, int n);
void fetch_superblock(int fd);
void fetch_bgdesc(int fd, int n);
int get_file_inode_number(int fd, struct ext2_inode inode, char *filename);

// Main code
int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Usage: ./ext2 device-file-name path-on-partition inode/data\n");
        return 1;
    }

    int fd;
    unsigned int inodeno;
    char *filename = NULL, pathname[1024];
    struct ext2_inode inode; 

    // Open device file
    fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror("device opening");
        exit(errno);
    }

    // Get global superblock
    fetch_superblock(fd);

    // Search for each directory
    strncpy(pathname, argv[2], 1024);
    filename = strtok(pathname, "/");
    inodeno = 2;
    do {
        inode = get_inode(fd, inodeno);
        isfile = 0;
        if ((inodeno = get_file_inode_number(fd, inode, filename)) == -1) {
            printf("File not found:(\n");
            return 2;
        }
        if (isfile == 1) {
            break;
        }
        filename = strtok(NULL, "/");
    } while (filename);

    inode = get_inode(fd, inodeno);
    if (strcmp(argv[3], "inode") == 0) {
        printf(" === PRINTING INODE === \n");
        printf("mode: %d\n", inode.i_mode);
        printf("size: %d\n", inode.i_size);
        printf("blocks: %d\n", inode.i_blocks);
        unsigned int inode_blocks = inode.i_blocks / (BLOCK_SIZE / 512);
        inode_blocks = (inode_blocks > 12) ? 12 : inode_blocks;
        for (int i = 0; i < inode_blocks; i++)
            printf("%d ", inode.i_block[i]);
        printf("\n=== INODE PRINTED === \n");
    } else if (strcmp(argv[3], "data") == 0) {
        unsigned int inode_blocks = inode.i_blocks / (BLOCK_SIZE / 512);
        inode_blocks = (inode_blocks > 12) ? 12 : inode_blocks;
        printf(" === PRINTING DATA === \n");
        for (int i = 0; i < inode_blocks; i++) {
            lseek64(fd, BLOCK_SIZE * inode.i_block[i], SEEK_SET);
            char buf[1024];
            read(fd, buf, 1024);
            write(1, buf, 1024);
        }
        printf("\n=== DATA PRINTED === \n");
    }

    return 0;
}

int get_file_inode_number(int fd, struct ext2_inode inode, char *filename) {
    struct ext2_dir_entry_2 dir;
    unsigned int inode_blocks;
    // Read root directory block
    inode_blocks = inode.i_blocks / (BLOCK_SIZE / 512);
    // TODO: Could try for other indirects
    inode_blocks = (inode_blocks > 12) ? 12 : inode_blocks;
    for (int i = 0; i < inode_blocks; ++i) {
        lseek64(fd, BLOCK_SIZE * inode.i_block[i], SEEK_SET);
        int dir_offset = 0, j = 0;
        while (dir_offset < BLOCK_SIZE) {
            j++;
            read(fd, &dir, sizeof(dir));
            if (strcmp(dir.name, filename) == 0) {
                printf("Found: %s!\n", dir.name);
                if (dir.file_type == EXT2_FT_REG_FILE)
                    isfile = 1;
                return dir.inode;
            }
            dir_offset += dir.rec_len;
            lseek64(fd, BLOCK_SIZE * inode.i_block[i], SEEK_SET);
            lseek64(fd, dir_offset, SEEK_CUR);
        }
    }

    return -1;
}

void fetch_superblock(int fd) {
    // Read super block
    lseek64(fd, 1024, SEEK_SET);
    ssize_t bytes_read = read(fd, &superblock, sizeof(struct ext2_super_block));
    printf(" === Reading SUPERBLOCK\n");
    printf("Magic: %x\n", superblock.s_magic);
    printf("Total inodes: %d\n", superblock.s_inodes_count);
    printf("Inodes per group: %d\n", superblock.s_inodes_per_group);
    printf(" === [SUPERBLOCK] Bytes read: %ld\n", bytes_read);
    return;
}
   
void fetch_bgdesc(int fd, int inode_number) {
    ssize_t bytes_read;
    unsigned int blockgroup_number;
    // Read block group descriptor
    blockgroup_number = (inode_number - 1) / superblock.s_inodes_per_group;
    lseek64(fd, 1024 + BLOCK_SIZE, SEEK_SET);
    lseek64(fd, blockgroup_number * sizeof(struct ext2_group_desc), SEEK_CUR);
    bytes_read = read(fd, &bgdesc, sizeof(struct ext2_group_desc));
    return;
}

struct ext2_inode get_inode(int fd, int inode_number) {
    unsigned int inode_offset; 
    struct ext2_inode inode;
    
    fetch_bgdesc(fd, inode_number); // Get block group descriptor
    // Read inode from inode table
    inode_offset = (inode_number - 1) % superblock.s_inodes_per_group;
    lseek64(fd, BLOCK_SIZE * bgdesc.bg_inode_table, SEEK_SET);
    lseek64(fd, superblock.s_inode_size * inode_offset, SEEK_CUR);
    read(fd, &inode, sizeof(struct ext2_inode));
    return inode;
}



