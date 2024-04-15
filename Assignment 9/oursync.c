#include <dirent.h>
#define _GNU_SOURCE
#define _FILE_OFFSET_BITS 64
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <utime.h>
#include <string.h>

int isDirectoryExists(const char *path)
{
    struct stat stats;
    stat(path, &stats);

    // Check for directory existence
    if (S_ISDIR(stats.st_mode))
        return 1;

    return 0;
}

int isFileExists(const char *fname)
{
    FILE *file;
    if ((file = fopen(fname, "r")))
    {
        fclose(file);
        return 1;
    }
    return 0;
}

void delete_file(char *path)
{
    // delete file at path
    int n = remove(path);
    printf("[-] %s\n", path);
}

void delete_dir(const char *path)
{
    struct dirent *de;
    char fname[300];
    DIR *dr = opendir(path);
    if (dr == NULL)
    {
        printf("No file or directory found\n");
        return;
    }
    while ((de = readdir(dr)) != NULL)
    {
        if (!strcmp(de->d_name, ".") || !strcmp(de->d_name, ".."))
            continue;
        sprintf(fname, "%s/%s", path, de->d_name);
        struct stat statbuf;
        if (!stat(fname, &statbuf))
        {
            if (S_ISDIR(statbuf.st_mode))
            {
                delete_dir(fname);
                rmdir(fname);
            }
            else
            {
                unlink(fname);
                printf("[-] %s\n", fname);
            }
        }
    }
    closedir(dr);
    rmdir(path);
    printf("[-] %s\n", path);
}

void f(char *path, char *path2)
{

    DIR *d;
    struct dirent *dir;
    d = opendir(path);
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0)
                continue;
            if (dir->d_type == DT_REG)
            {
                // it is a file
                // check if this file is there in "dest/{path}" or not
                char file_path[500];
                sprintf(file_path, "%s/%s", path2, dir->d_name);

                // check if this file exists
                int exists_flag = isFileExists(file_path);

                // if this file is not there in "dest/{path}" then copy it
                if (!exists_flag)
                {
                    // file is not there in the directory , so make a copy of it
                    char src_file_path[500];
                    sprintf(src_file_path, "%s/%s", path, dir->d_name);
                    int fd_in = open(src_file_path, O_RDONLY);
                    int fd_out = open(file_path, O_WRONLY | O_CREAT | O_TRUNC, 0666);
                    struct stat stat;
                    fstat(fd_in, &stat);
                    int len = stat.st_size;

                    int ret;
                    do
                    {
                        ret = copy_file_range(fd_in, NULL, fd_out, NULL, len, 0);
                        len -= ret;
                    } while (len > 0 && ret > 0);

                    printf("[+] %s\n", file_path);

                    struct utimbuf newtime;
                    newtime.actime = stat.st_atime;
                    newtime.modtime = stat.st_mtime;

                    utime(file_path, &newtime);
                    printf("[t] %s\n", file_path);

                    // set the permissions of the file to be same as the source file
                    chmod(file_path, stat.st_mode);
                    printf("[p] %s\n", file_path);

                    // close the file descriptors
                    close(fd_in);
                    close(fd_out);
                }

                // if it is there , then check if the size of file and their timestamp is same or not
                else
                {
                    // file is there in the directory , so check if the size of file and their timestamp is same or not
                    struct stat stat1, stat2;
                    char src_file_path[500];
                    sprintf(src_file_path, "%s/%s", path, dir->d_name);
                    stat(src_file_path, &stat1);
                    stat(file_path, &stat2);
                    if (stat1.st_size != stat2.st_size || stat1.st_mtime != stat2.st_mtime || stat1.st_mode != stat2.st_mode)
                    {
                        // if not same then copy it
                        char src_file_path[500];
                        sprintf(src_file_path, "%s/%s", path, dir->d_name);
                        int fd_in = open(src_file_path, O_RDONLY);
                        int fd_out = open(file_path, O_WRONLY | O_CREAT | O_TRUNC, 0666);
                        struct stat stat;
                        fstat(fd_in, &stat);
                        off_t len = stat.st_size;

                        off_t ret = 1;
                        do
                        {
                            ret = copy_file_range(fd_in, NULL, fd_out, NULL, len, 0);
                            len -= ret;
                        } while (len > 0 && ret > 0);

                        printf("[o] %s\n", file_path);

                        struct utimbuf newtime;
                        newtime.actime = stat.st_atime;
                        newtime.modtime = stat.st_mtime;

                        utime(file_path, &newtime);
                        printf("[t] %s\n", file_path);

                        // check if the permissions of the file are same or not
                        if (stat1.st_mode != stat2.st_mode)
                        {
                            // if not same then set the permissions of the file to be same as the source file
                            chmod(file_path, stat1.st_mode);
                            printf("[p] %s\n", file_path);
                        }
                    }
                }
            }
            else
            {
                // it is a directory
                // check if this directory is there in "dest/{path}" or not
                char dir_path[500];
                sprintf(dir_path, "%s/%s", path2, dir->d_name);
                int exists_flag = isDirectoryExists(dir_path);
                // printf("Checking %d\n", exists_flag);

                // if this directory is not there in "dest/{path}" then create it and recursively copy all files and directories inside it
                if (!exists_flag)
                {
                    // directory is not there in the directory , so make a copy of it
                    char src_dir_path[500];
                    sprintf(src_dir_path, "%s/%s", path, dir->d_name);
                    sprintf(dir_path, "%s/%s", path2, dir->d_name);
                    struct stat statbuf;
                    stat(src_dir_path, &statbuf);
                    mkdir(dir_path, statbuf.st_mode);
                    printf("[+] %s\n", dir_path);
                    printf("[p] %s\n", dir_path);

                    // update the timestamp of the directory
                    struct utimbuf newtime;
                    newtime.actime = statbuf.st_atime;
                    newtime.modtime = statbuf.st_mtime;
                    utime(dir_path, &newtime);
                    printf("[t] %s\n", dir_path);

                    f(src_dir_path, dir_path);
                }

                // if it is there, then recursively call this function with "path/{dir->d_name}"
                else
                {
                    // directory is there in the directory , so recursively call this function with "path/{dir->d_name}"
                    char src_dir_path[500];
                    sprintf(src_dir_path, "%s/%s", path, dir->d_name);
                    sprintf(dir_path, "%s/%s", path2, dir->d_name);

                    // update the timestamp of the directory
                    struct stat statbuf;
                    stat(src_dir_path, &statbuf);
                    struct utimbuf newtime;
                    newtime.actime = statbuf.st_atime;
                    newtime.modtime = statbuf.st_mtime;
                    utime(dir_path, &newtime);
                    printf("[t] %s\n", dir_path);
                    f(src_dir_path, dir_path);
                }
            }
        }
    }
}

void oursync(char *path, char *path2)
{
    // check if there are files or directories in "dest" which are not there in "src"
    // if there are then delete them
    DIR *d;
    struct dirent *dir;
    d = opendir(path2);
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0)
                continue;
            char path3[500];
            sprintf(path3, "%s/%s", path, dir->d_name);
            if (dir->d_type == DT_REG)
            {
                // it is a file
                // check if this file is there in "src/{path}" or not
                char file_path[500];
                sprintf(file_path, "%s/%s", path, dir->d_name);

                // check if this file exists
                int exists_flag = isFileExists(file_path);

                // if this file is not there in "src/{path}" then delete it
                if (!exists_flag)
                {
                    // file is not there in the directory , so delete it in dest
                    char dest_file_path[500];
                    sprintf(dest_file_path, "%s/%s", path2, dir->d_name);
                    delete_file(dest_file_path);
                }
            }
            else
            {
                // it is a directory
                // check if this directory is there in "src/{path}" or not
                char dir_path[500];
                sprintf(dir_path, "%s/%s", path, dir->d_name);
                int exists_flag = isDirectoryExists(dir_path);
                // printf("Checking %d\n", exists_flag);

                // if this directory is not there in "src/{path}" then delete it
                if (!exists_flag)
                {
                    // directory is not there in the directory , so delete it
                    char dest_dir_path[500];
                    sprintf(dest_dir_path, "%s/%s", path2, dir->d_name);
                    delete_dir(dest_dir_path);
                }
            }
        }
    }

    f(path, path2);
}

int main(int argc, char *argv[])
{

    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <source> <destination>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    oursync(argv[1], argv[2]);

    return (0);
}