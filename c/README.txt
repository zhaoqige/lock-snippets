HOWTO CREATE A APP IN OPENWRT
-----------------------------
1. Create a dir named "gws5k";
2. Copy "Makefile/cross-compile/Makefile" to "gws5k";
3. Create a sub dir named "src" in "gws5k";
4. Copy "Makefile/cross-compile/Makefile.cross-compile" to "gws5k/src/";
5. Copy "app.*", "task.*" to "gws5k/src/";
6. Copy "gws5k" to "<openwrt_root>/packages/";
7. Run "make menuconfig", select "gws5k";
8. Run "make package/gws5k/compile V=s";
9. Find "gws5k_*.ipk" in "<openwrt_root>/bin/<arch>/packages/".