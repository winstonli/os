#!/bin/bash

if [ "$#" -ne 1 ]; then
  echo
  echo "Usage: $0 /dev/sdX"
  echo
  exit 1
fi

make

drive_label="OSDev-$RANDOM"
mount_dir="/media/os-$RANDOM"

# wipe the existing drive and make a new partition table
sudo fdisk "$1" <<EOF
o
n
p
1


w
EOF

# format the partition as FAT32 and mount it
sudo mkdir -p "$mount_dir"
sudo mkfs.vfat -F 32 -n "$drive_label" -I "$1"1
sudo mount "$1"1 "$mount_dir"

# install grub to the drive
sudo grub-install --root-directory="$mount_dir" --no-floppy --recheck --force "$1"

# copy in kernel and grub config
sudo cp start *.bin "$mount_dir/boot/"
sudo cp grub.cfg "$mount_dir/boot/grub/"

# ensure data is written, then unmount and eject device
sudo sync
sudo umount "$1"1
sudo udisksctl power-off --block-device "$1"

# cleanup
sudo rm -rf "$mount_dir"
