# Synchronizing two directory trees

In this assignment, we have created a program that synchronizes two directory trees. The program takes two directory paths as input and synchronizes the respective directory trees. The program copies files from the source directory to the destination directory if the files are not present in the destination directory.

Our program mimics the behavior of the `rsync` command in Unix-like systems. We have written our program in C language file `oursync.c`. A helper program is also provided in the file `rnd_dir_generator.py` to generate random directory trees for testing purposes. (It uses the `randomfiletree` module which can be installed using `pip install randomfiletree`.)
