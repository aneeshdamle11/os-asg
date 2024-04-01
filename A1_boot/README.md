## Problem
Triple boot a computer (own: beware, vm: smart, lab pc: hahaha)

### Installation of Operating Systems using VirtualBox
--------------------------------------------------

Virtualbox: Virtual machine Manager, Virtualisation Software

```
Q. What does a virtualbox do?
A. It creates virtual "machine"
* creates virtual PC h/w:   processor, RAM(memory), secondary storage(HDD etc),
                            network interface, I/O interface, etc.
```

After creating virtual machine: you have only the bare-metal h/w.
No s/w (OS on h/w)

Next step: Install OS

Host: the Physical desktop/laptop that you have with its OS installed on the
physical hard disk

Guest: the virtual machine + os installed on it

VBox - will create a file on HOST computer, to simulate the virtual harddisk inside it
Physical hard disk: actual HDD of the laptop
Virtual hard disk: a file (acting as harddisk) on your laptop

```
What does it mean by installation of an OS?
* OS is a program, that is supposed to get "loaded" in the computer's
  RAM(memory) when the computer starts.
* OS is normally present on your Hard Disk, as one/more files
* When the computer boots(starts), the OS gets "loaded" in RAM.
  Who loads it? Ans. another program, hardwired into the h/w called BIOS: Basic Input Output System
* But, when you buy a laptop (bare h/w), there is no OS as such on your hard disk!
* Installation - the task of transferring the OS image from installation
  medium onto the hard disk.
```

Installation medium:
- Bootable pen drive with the OS image on it
- Bootable CD/DVD
- Bootable ISO file (particularly for s/w like VirtualBox)

ISO file: image of a CD/DVD (image: byte-by-byte sequential dump of the
                                    contents, in a file)

Partition: Logical division of a physical hard disk.
Eg. If you have a 200 GB HDD, then it can be divided into multiple partitions
of different sizes,
eg. [50 GB, 100 GB, 50 GB] (or) [40 GB, 60 GB, 20 GB, 80 GB]

partitions - what's their use?
* An operating system gets installed in a partition. Most typically only one
  OS within  a partition.
* You can logically separate your different types of data on different
  partitions. Eg. C:\ D:\ on windows are nothing but partitions

Format: Creating a logical "empty", "initialised" data structure for storing the files/folders
        Most typically some type of a n-ary tree
--> there are different types of file system formats.
    Eg. NTFS(default on Windows), FAT32 (default on windows), Ext4 (default on Linux)

Bootloader?
Is a s/w that is loaded by the BIOS into RAM.
Bootloader loads the OS image from Hard disk into RAM, and starts the OS
Eg. GRUB (used by Ubuntu)
End users see boot loader, in the form of a menu showing "boot menu options" - choice of OS to boot from

Sequence: When you turn on the computer, the BIOS runs automatically.
The BIOS program was put inside your computer by the manufacturer.

```
FAQ
Q. If we were to do a dual boot or a triple boot, will we follow the same
procedure just the difference will be that while creating the 2nd VM, I will
choose the existing Hard Disk and not create a new one right?
A. Yes, that is correct

Q. Why is the /home necessary? Can't we just use / and swap partitions?
A. Suppose you want to triple boot Fedora, Debian and Ubuntu OSs and want them
to share the same folders. So then, you create a /home while installing the
first OS, but use the same /home for others by mounting their /home there
```


#### Notes of VM
- VM : bare-h/w (default option)
- HDD: /home/$user/VBox/
- CD/DVD: [Ubuntu]/home/media/

