S2MM data receiving application for Petalinux

MM2S is not in use.
To build use arm-linux-gnueabi-gcc dataproc.c -o dataproc -static

Script for quick upload to board
```console
#!/bin/bash

cd my
#petalinux-build -c dataproc
cd /home/bulkin/FPGA/petalinux/my/project-spec/meta-user/recipes-apps/dataproc/files
arm-linux-gnueabi-gcc dataproc.c -o dataproc -static

#read  -n 1 -p "----------READY FOR UPLOAD---------" mainmenuinput

#ssh-keygen -f "/home/bulkin/.ssh/known_hosts" -R "192.168.1.4"

sshpass -p root scp -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null -r ./dataproc root@192.168.1.3:/dataproc

sshpass -p root ssh -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null root@192.168.1.3
```


Vivado design memory map:

| Master Segment Name | Address Space | Slave Segment | Offset	| Range |
| --- | --- | --- | --- | --- |
| SEG_axi_dma_0_Reg | /PSys/Data | /axi_dma_0/S_AXI_LITE/Reg | 0x40400000 | 64K |
| SEG_PSys_HP0_DDR_LOWOCM | /axi_dma_0/Data_S2MM | /PSys/S_AXI_HP0/HP0_DDR_LOWOCM | 0x00000000 | 512M |
| SEG_PSys_HP0_DDR_LOWOCM | /axi_dma_0/Data_SG | /PSys/S_AXI_HP0/HP0_DDR_LOWOCM | 0x00000000 | 512M | 



Console output:

```console
root@my:/# ./dataproc 
Please, enter number of descriptors: 4
FOR 0 set BUFFER start 0x12020000
FOR 1 set BUFFER start 0x1281ffff
FOR 2 set BUFFER start 0x1301fffe
FOR 3 set BUFFER start 0x1381fffd
Stream to memory-mapped status (0x00010008@0x34):
S2MM_STATUS_REGISTER status register values:
 running SGIncld
0) Descriptor Status: 0x8c0000e0
        BUFFER ADDR: 0x12020000;
        Cmplt: 0;       RXSOF: 8000000;
        RXEOF: 4000000; BFLEN: 224 bytes;
1) Descriptor Status: 0x880000f9
        BUFFER ADDR: 0x1281ffff;
        Cmplt: 0;       RXSOF: 8000000;
        RXEOF: 0;       BFLEN: 249 bytes;
2) Descriptor Status: 0x800001f3
        BUFFER ADDR: 0x1301fffe;
        Cmplt: 0;       RXSOF: 0;
        RXEOF: 0;       BFLEN: 499 bytes;
3) Descriptor Status: 0x800002ee
        BUFFER ADDR: 0x1381fffd;
        Cmplt: 0;       RXSOF: 0;
        RXEOF: 0;       BFLEN: 750 bytes;
1352534998 694875763 694875764 694875765 694875766 694850482 694916004 694916005 694916006 694916007 694916008 694916009 694916010 694916011 694916012 694916013 694916014 694916015 694916016 694916017 694916018 694916019 694916020 694916021 694916022 694916023 694916024 694916025 694916026 694916027 694916028 694916029 
Stream to memory-mapped status (0x0001100a@0x34):
S2MM_STATUS_REGISTER status register values:
 running idle SGIncld IOC_Irq
0) Descriptor Status: 0x8c0000e0
        BUFFER ADDR: 0x12020000;
        Cmplt: 0;       RXSOF: 8000000;
        RXEOF: 4000000; BFLEN: 224 bytes;
1) Descriptor Status: 0x880000f9
        BUFFER ADDR: 0x1281ffff;
        Cmplt: 0;       RXSOF: 8000000;
        RXEOF: 0;       BFLEN: 249 bytes;
2) Descriptor Status: 0x800001f3
        BUFFER ADDR: 0x1301fffe;
        Cmplt: 0;       RXSOF: 0;
        RXEOF: 0;       BFLEN: 499 bytes;
3) Descriptor Status: 0x800002ee
        BUFFER ADDR: 0x1381fffd;
        Cmplt: 0;       RXSOF: 0;
        RXEOF: 0;       BFLEN: 750 bytes;
1352534998 694875763 694875764 694875765 694875766 694850482 694916004 694916005 694916006 694916007 694916008 694916009 694916010 694916011 694916012 694916013 694916014 694916015 694916016 694916017 694916018 694916019 694916020 694916021 694916022 694916023 694916024 694916025 694916026 694916027 694916028 694916029 
```

We can see increment counter from datasource of FPGA

Vivado blockdesign:
![image](https://user-images.githubusercontent.com/2908600/219812215-5c07d015-7dae-495d-91a6-dd16068c52da.png)

axi_mem_intercon3 can be removed as it is not necessary