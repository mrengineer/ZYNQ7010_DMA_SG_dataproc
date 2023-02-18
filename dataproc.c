#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>


        
/*********************************************************************/
/*                       Define mmap locations                       */
/*********************************************************************/

#define	AXI_DMA_REGISTER_LOCATION          0x40400000	//AXI DMA Register Address Map
#define	DESCRIPTOR_REGISTERS_SIZE          0xFFFF
#define	SG_DMA_DESCRIPTORS_WIDTH           0xFFFF
#define	MEMBLOCK_WIDTH                     0x1FFFFFF	//size of mem used by s2mm and mm2s
#define	BUFFER_BLOCK_WIDTH                 0x7D0000		//size of memory block per descriptor in bytes
#define	NUM_OF_DESCRIPTORS                 0x7			//number of descriptors for each direction

#define	HP0_DMA_BUFFER_MEM_ADDRESS         0x20000000
#define	HP0_MM2S_DMA_BASE_MEM_ADDRESS      (HP0_DMA_BUFFER_MEM_ADDRESS)
#define	HP0_S2MM_DMA_BASE_MEM_ADDRESS      (HP0_DMA_BUFFER_MEM_ADDRESS + MEMBLOCK_WIDTH + 1)
#define	HP0_MM2S_DMA_DESCRIPTORS_ADDRESS   (HP0_MM2S_DMA_BASE_MEM_ADDRESS)
#define	HP0_S2MM_DMA_DESCRIPTORS_ADDRESS   (HP0_S2MM_DMA_BASE_MEM_ADDRESS)
#define	HP0_MM2S_SOURCE_MEM_ADDRESS        (HP0_MM2S_DMA_BASE_MEM_ADDRESS + SG_DMA_DESCRIPTORS_WIDTH + 1)
#define	HP0_S2MM_TARGET_MEM_ADDRESS        (HP0_S2MM_DMA_BASE_MEM_ADDRESS + SG_DMA_DESCRIPTORS_WIDTH + 1)


/*********************************************************************/
/*                   define all register locations                   */
/*********************************************************************/

        // MM2S CONTROL
#define MM2S_CONTROL_REGISTER       0x00    // MM2S_DMACR
#define MM2S_STATUS_REGISTER        0x04    // MM2S_DMASR
#define MM2S_CURDESC                0x08    // must align 0x40 addresses
#define MM2S_CURDESC_MSB            0x0C    // unused with 32bit addresses
#define MM2S_TAILDESC               0x10    // must align 0x40 addresses
#define MM2S_TAILDESC_MSB           0x14    // unused with 32bit addresses

#define SG_CTL                      0x2C    // CACHE CONTROL

// S2MM CONTROL
#define S2MM_CONTROL_REGISTER       0x30    // S2MM_DMACR
#define S2MM_STATUS_REGISTER        0x34    // S2MM_DMASR
#define S2MM_CURDESC                0x38    // must align 0x40 addresses
#define S2MM_CURDESC_MSB            0x3C    // unused with 32bit addresses
#define S2MM_TAILDESC               0x40    // must align 0x40 addresses
#define S2MM_TAILDESC_MSB           0x44    // unused with 32bit addresses

#define ENABLE_IOC_IRQ              0x00001000
#define ENABLE_DELAY_IRQ            0x00002000
#define ENABLE_ERR_IRQ              0x00004000
#define ENABLE_ALL_IRQ              0x00007000

void delay(int number_of_seconds)
{
    // Converting time into milli_seconds
    int milli_seconds = 1000 * number_of_seconds;
 
    // Storing start time
    clock_t start_time = clock();
 
    // looping till required time is not achieved
    while (clock() < start_time + milli_seconds)
        ;
}

unsigned int write_dma(unsigned int *virtual_addr, int offset, unsigned int value)
{
    virtual_addr[offset>>2] = value;

    return 0;
}

    int main() {

	unsigned int* axi_dma_register_mmap;
	unsigned int* mm2s_descriptor_register_mmap;
	unsigned int* s2mm_descriptor_register_mmap;
	unsigned int* source_mem_map;
	unsigned int* dest_mem_map;
	int controlregister_ok = 0,mm2s_status,s2mm_status;
	uint32_t mm2s_current_descriptor_address;
	uint32_t s2mm_current_descriptor_address;
	uint32_t mm2s_tail_descriptor_address;
	uint32_t s2mm_tail_descriptor_address;

	setbuf(stdout, NULL);

	printf("START\n");


	/*********************************************************************/
	/*               mmap the AXI DMA Register Address Map               */
	/*********************************************************************/

	int dh = open("/dev/mem", O_RDWR | O_SYNC); 
	axi_dma_register_mmap = mmap(NULL, DESCRIPTOR_REGISTERS_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, dh, AXI_DMA_REGISTER_LOCATION);
	s2mm_descriptor_register_mmap = mmap(NULL, DESCRIPTOR_REGISTERS_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, dh, HP0_S2MM_DMA_DESCRIPTORS_ADDRESS);
	dest_mem_map = mmap(NULL, BUFFER_BLOCK_WIDTH * NUM_OF_DESCRIPTORS, PROT_READ | PROT_WRITE, MAP_SHARED, dh, (off_t)(HP0_S2MM_TARGET_MEM_ADDRESS));
	
	int i;
	

	printf("nmaps are done\n");

	// fill s2mm-register memory with zeros
    for (i = 0; i < DESCRIPTOR_REGISTERS_SIZE; i++) {
		char *p = (char *)s2mm_descriptor_register_mmap;
		p[i] = 0x00000000;
	}
	
	printf("s2mm-register memory with zeros is DONE\n");

        
	/*********************************************************************/
	/*                 reset and halt all dma operations                 */
	/*********************************************************************/

	axi_dma_register_mmap[S2MM_CONTROL_REGISTER >> 2] =  0x4;
	axi_dma_register_mmap[S2MM_CONTROL_REGISTER >> 2] =  0x0;

	printf("reset and halt all dma operations is DONE\n");

	/*********************************************************************/
	/*           build mm2s and s2mm stream and control stream           */
	/* chains will be filled with next desc, buffer width and registers  */
	/*                         [0]: next descr                           */
	/*                         [1]: reserved                             */
	/*                         [2]: buffer addr                          */
	/*********************************************************************/


	s2mm_current_descriptor_address = HP0_S2MM_DMA_DESCRIPTORS_ADDRESS; // save current descriptor address

	s2mm_descriptor_register_mmap[0x0 >> 2] = HP0_S2MM_DMA_DESCRIPTORS_ADDRESS + 0x40; // set next descriptor address
	s2mm_descriptor_register_mmap[0x8 >> 2] = HP0_S2MM_TARGET_MEM_ADDRESS + 0x0; // set target buffer address
	s2mm_descriptor_register_mmap[0x18 >> 2] = BUFFER_BLOCK_WIDTH; //before in code 0x87D0000; // set mm2s/s2mm buffer length to control register

	s2mm_descriptor_register_mmap[0x40 >> 2] = HP0_S2MM_DMA_DESCRIPTORS_ADDRESS + 0x80; // set next descriptor address
	s2mm_descriptor_register_mmap[0x48 >> 2] = HP0_S2MM_TARGET_MEM_ADDRESS + BUFFER_BLOCK_WIDTH; //0x7D0000; // set target buffer address
	s2mm_descriptor_register_mmap[0x58 >> 2] = BUFFER_BLOCK_WIDTH; //0x7D0000; // set mm2s/s2mm buffer length to control register

	s2mm_descriptor_register_mmap[0x80 >> 2] = HP0_S2MM_DMA_DESCRIPTORS_ADDRESS + 0xC0; // set next descriptor address
	s2mm_descriptor_register_mmap[0x88 >> 2] = HP0_S2MM_TARGET_MEM_ADDRESS + (BUFFER_BLOCK_WIDTH*2); // set target buffer address
	s2mm_descriptor_register_mmap[0x98 >> 2] = BUFFER_BLOCK_WIDTH; //0x7D0000; // set mm2s/s2mm buffer length to control register


	s2mm_descriptor_register_mmap[0xC0 >> 2] = HP0_S2MM_DMA_DESCRIPTORS_ADDRESS + 0x100; // set next descriptor address
	s2mm_descriptor_register_mmap[0xC8 >> 2] = HP0_S2MM_TARGET_MEM_ADDRESS + (BUFFER_BLOCK_WIDTH*3); // set target buffer address
	s2mm_descriptor_register_mmap[0xD8 >> 2] = BUFFER_BLOCK_WIDTH; //0x7D0000; // set mm2s/s2mm buffer length to control register

	s2mm_descriptor_register_mmap[0x100 >> 2] = HP0_S2MM_DMA_DESCRIPTORS_ADDRESS + 0x140; // set next descriptor address
	s2mm_descriptor_register_mmap[0x108 >> 2] = HP0_S2MM_TARGET_MEM_ADDRESS + (BUFFER_BLOCK_WIDTH*4); // set target buffer address
	s2mm_descriptor_register_mmap[0x118 >> 2] = BUFFER_BLOCK_WIDTH; //0x7D0000; // set mm2s/s2mm buffer length to control register

	s2mm_descriptor_register_mmap[0x140 >> 2] = HP0_S2MM_DMA_DESCRIPTORS_ADDRESS + 0x180; // set next descriptor address
	s2mm_descriptor_register_mmap[0x148 >> 2] = HP0_S2MM_TARGET_MEM_ADDRESS + (BUFFER_BLOCK_WIDTH*5); // set target buffer address
	s2mm_descriptor_register_mmap[0x158 >> 2] = BUFFER_BLOCK_WIDTH; //0x7D0000; // set mm2s/s2mm buffer length to control register

	s2mm_descriptor_register_mmap[0x180 >> 2] = 0x200; // set next descriptor address (unused?)
	s2mm_descriptor_register_mmap[0x188 >> 2] = HP0_S2MM_TARGET_MEM_ADDRESS + (BUFFER_BLOCK_WIDTH*6); // set target buffer address
	s2mm_descriptor_register_mmap[0x198 >> 2] = BUFFER_BLOCK_WIDTH; // before in code 0x47D0000; // set mm2s/s2mm buffer length to control register
	


	s2mm_tail_descriptor_address = HP0_S2MM_DMA_DESCRIPTORS_ADDRESS + 0x180 ; // save tail descriptor address

	printf("build s2mm stream and control stream is DONE\n");

	/*********************************************************************/
	/*                 set current descriptor addresses                  */
	/*           and start dma operations (S2MM_DMACR.RS = 1)            */
	/*********************************************************************/

	printf("S2MM_CURDESC is 0x%08x\n", axi_dma_register_mmap[S2MM_CURDESC>>2]);

	axi_dma_register_mmap[S2MM_CURDESC>>2] =  s2mm_current_descriptor_address;

	printf("S2MM_CURDESC SET 0x%08x\n", axi_dma_register_mmap[S2MM_CURDESC>>2]);
	axi_dma_register_mmap[S2MM_CONTROL_REGISTER >> 2] =  0x1;


	/*********************************************************************/
	/*                          start transfer                           */
	/*                 (by setting the taildescriptors)                  */
	/*********************************************************************/


	printf("S2MM_TAILDESC 0x%08x\n", axi_dma_register_mmap[S2MM_TAILDESC>>2]);
	axi_dma_register_mmap[S2MM_TAILDESC>>2] =  s2mm_tail_descriptor_address;
	printf("S2MM_TAILDESC SET 0x%08x\n", axi_dma_register_mmap[S2MM_TAILDESC>>2]);


	printf("wait until all transfers finished\n");

	/*********************************************************************/
	/*                 wait until all transfers finished                 */
	/*********************************************************************/

	char j = 0;

	while ((!controlregister_ok && j < 8))
    {
		j ++;

		s2mm_status = axi_dma_register_mmap[S2MM_STATUS_REGISTER >> 2];
		controlregister_ok = ((s2mm_status & 0x00001000));


		printf("Stream to memory-mapped status (0x%08x@0x%02x):\n", s2mm_status, S2MM_STATUS_REGISTER);
		printf("S2MM_STATUS_REGISTER status register values:\n");
		if (s2mm_status & 0x00000001) {
			printf(" halted"); 			
		} 	else printf(" running");
		if (s2mm_status & 0x00000002) printf(" idle");
		if (s2mm_status & 0x00000008) printf(" SGIncld");
		if (s2mm_status & 0x00000010) {
			printf(" DMAIntErr");
			return 0;
		}
		if (s2mm_status & 0x00000020) printf(" DMASlvErr");
		if (s2mm_status & 0x00000040) printf(" DMADecErr");
		if (s2mm_status & 0x00000100) printf(" SGIntErr");
		if (s2mm_status & 0x00000200) printf(" SGSlvErr");
		if (s2mm_status & 0x00000400) printf(" SGDecErr");
		if (s2mm_status & 0x00001000) printf(" IOC_Irq");
		if (s2mm_status & 0x00002000) printf(" Dly_Irq");
		if (s2mm_status & 0x00004000) printf(" Err_Irq");

		s2mm_current_descriptor_address = (uint32_t) axi_dma_register_mmap[S2MM_CURDESC>>2];
		printf(" curdesc 0x%08x\n", s2mm_current_descriptor_address);
    }
    
	
	return 0;
}