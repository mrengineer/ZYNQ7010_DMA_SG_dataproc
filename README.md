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
S2MM_CURDESC is 0x22000000
S2MM_CURDESC SET 0x22000000
S2MM_TAILDESC 0x22000180
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
