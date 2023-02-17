S2MM data receiving application for Petalinux

MM2S is not in use.
There is some problem because after start output has no descriptor change as expected

Vivado design memory map:

| Master Segment Name | Address Space | Slave Segment | Offset	| Range |
| --- | --- | --- | --- | --- |
| SEG_axi_dma_0_Reg | /PSys/Data | /axi_dma_0/S_AXI_LITE/Reg | 0x40400000 | 64K |
| SEG_PSys_HP0_DDR_LOWOCM | /axi_dma_0/Data_S2MM | /PSys/S_AXI_HP0/HP0_DDR_LOWOCM | 0x00000000 | 512M |
| SEG_PSys_HP0_DDR_LOWOCM | /axi_dma_0/Data_SG | /PSys/S_AXI_HP0/HP0_DDR_LOWOCM | 0x00000000 | 512M | 




Console output:

```console
START
nmaps are done
s2mm-register memory with zeros is DONE
reset and halt all dma operations is DONE
build s2mm stream and control stream is DONE
S2MM_CURDESC is 0x00000000
S2MM_CURDESC SET 0x22000000
S2MM_TAILDESC 0x00000000
S2MM_TAILDESC SET 0x22000180
wait until all transfers finished
Stream to memory-mapped status (0x00014509@0x34):
S2MM_STATUS_REGISTER status register values:
 halted SGIncld SGIntErr SGDecErr Err_Irq curdesc 0x22000000
Stream to memory-mapped status (0x00014509@0x34):
S2MM_STATUS_REGISTER status register values:
 halted SGIncld SGIntErr SGDecErr Err_Irq curdesc 0x22000000
Stream to memory-mapped status (0x00014509@0x34):
S2MM_STATUS_REGISTER status register values:
 halted SGIncld SGIntErr SGDecErr Err_Irq curdesc 0x22000000
Stream to memory-mapped status (0x00014509@0x34):
S2MM_STATUS_REGISTER status register values:
 halted SGIncld SGIntErr SGDecErr Err_Irq curdesc 0x22000000
Stream to memory-mapped status (0x00014509@0x34):
S2MM_STATUS_REGISTER status register values:
 halted SGIncld SGIntErr SGDecErr Err_Irq curdesc 0x22000000
Stream to memory-mapped status (0x00014509@0x34):
S2MM_STATUS_REGISTER status register values:
 halted SGIncld SGIntErr SGDecErr Err_Irq curdesc 0x22000000
Stream to memory-mapped status (0x00014509@0x34):
S2MM_STATUS_REGISTER status register values:
 halted SGIncld SGIntErr SGDecErr Err_Irq curdesc 0x22000000
Stream to memory-mapped status (0x00014509@0x34):
S2MM_STATUS_REGISTER status register values:
 halted SGIncld SGIntErr SGDecErr Err_Irq curdesc 0x22000000
 ```

Vivado messages:
```
[BD 41-1289] Cannot find slave segment </PSys/S_AXI_HP1/HP1_DDR_LOWOCM>. Cannot create segment <SEG_PSys_HP1_DDR_LOWOCM> at <0x0000_0000 [ 512M ]> in Address Space </axi_dma_0/Data_MM2S>.
```

Vivado blockdesign:
![image](https://user-images.githubusercontent.com/2908600/219812215-5c07d015-7dae-495d-91a6-dd16068c52da.png)
