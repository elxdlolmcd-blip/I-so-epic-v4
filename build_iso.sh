#!/bin/sh
set -e
make clean && make
mkdir -p build/iso
cp SYSTEM.CNF build/iso/SYSTEM.CNF
cp CHICKEN.ELF build/iso/SLUS_000.01
MK=$(command -v mkisofs || command -v genisoimage)
"$MK" -iso-level 1 -V CHICKENGUN -o ChickenGunPS2.iso build/iso
echo "Listo: ChickenGunPS2.iso"
