

// FOUND operating project
// https://github.com/GOOD-Stuff/dma-sg-udp
// watch hexdump -s 0x12020000 -n 608 -o /dev/mem


#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <errno.h>


//#define SG_CTL                      0x2C    // CACHE CONTROL

// S2MM CONTROL
#define S2MM_CONTROL_REGISTER       0x30    // S2MM_DMACR
#define S2MM_STATUS_REGISTER        0x34    // S2MM_DMASR
#define S2MM_CURDESC                0x38    // must align 0x40 addresses
#define S2MM_CURDESC_MSB            0x3C    // unused with 32bit addresses
#define S2MM_TAILDESC               0x40    // must align 0x40 addresses
#define S2MM_TAILDESC_MSB           0x44    // unused with 32bit addresses

// DESCRIPTORS FIELDS
#define NXTDESC						0x00
#define BUFFER_ADDRESS				0x08
#define CONTROL						0x18
#define STATUS						0x1C

// ADDRESSES
#define	AXI_DMA_BASEADDR			0x40400000 //AXI DMA Register Address Map
#define	DESCRIPTOR_REGISTERS_SIZE	0x10000
#define	SG_DMA_DESCRIPTORS_WIDTH	0x1FFFF

#define	MEMBLOCK_WIDTH				0x1FFFFFF  //?size? of mem used by s2mm 
#define	BUFFER_BLOCK_WIDTH			0x7FFFFF   // size of memory block per descriptor in bytes
#define DEST_MEM_BLOCK				0x6400000 

#define	HP0_DMA_BUFFER_MEM_ADDRESS       0x10000000
#define	HP0_S2MM_DMA_BASE_MEM_ADDRESS    (HP0_DMA_BUFFER_MEM_ADDRESS + MEMBLOCK_WIDTH + 1)
#define	HP0_S2MM_DMA_DESCRIPTORS_ADDRESS (HP0_S2MM_DMA_BASE_MEM_ADDRESS)
#define	HP0_S2MM_TARGET_MEM_ADDRESS      (HP0_S2MM_DMA_BASE_MEM_ADDRESS + SG_DMA_DESCRIPTORS_WIDTH + 1)

void delay(int number_of_ms)
{
    // Converting time into milli_seconds
    int mk_seconds = 1000 * number_of_ms;
 
    // Storing start time
    clock_t start_time = clock();
 
    // looping till required time is not achieved
    while (clock() < start_time + mk_seconds)
        ;
}

void print_mem(void *virtual_address, int count)
{
	
	unsigned int * data_ptr = virtual_address;

	printf("PRINTMEM %p\r\n", data_ptr);

	for(int i = 0; i < count; i ++){
		//printf("%02X ", data_ptr[i]);
		//printf("%ld", data_ptr[i]);

		// print a space every 4 bytes (0 indexed)
		//if(i%4==3)	printf(" | ");

		printf("\t%u", (uint32_t)data_ptr[i]);
		
	}

	printf("\n");
}

static void dma_set(unsigned int *address, off_t offset, unsigned int value);
static int  dma_get(unsigned int *address, off_t offset);

/**
 * @brief  Settings s2mm descriptors;
 *
 * @param  s2mm_descr_vrt_addr - pointer to s2mm descriptors;
 * 		   num_descr   - number of descriptors;
 *
 * @return none
 * @note   This function set NXTDESC, Buffer address and size for Buffer;
 * 		   Offset between descriptors is 0x40
 */
void dma_descr_set(unsigned int *s2mm_descr_vrt_addr, int num_descr) {
    off_t				offset_nxtdesc;
    off_t				offset_buff_addr;
    const unsigned int	offset = 0x40;
    int					i;

    for (i = 0; i < num_descr; i++) {
		offset_nxtdesc   = offset * i;
		offset_buff_addr = BUFFER_BLOCK_WIDTH * i;

		dma_set(s2mm_descr_vrt_addr, NXTDESC + offset_nxtdesc, HP0_S2MM_DMA_DESCRIPTORS_ADDRESS + offset_nxtdesc + offset);

		printf("FOR %i set BUFFER start 0x%08lx\r\n", i, HP0_S2MM_TARGET_MEM_ADDRESS + offset_buff_addr);

		dma_set(s2mm_descr_vrt_addr, BUFFER_ADDRESS + offset_nxtdesc, HP0_S2MM_TARGET_MEM_ADDRESS + offset_buff_addr);

		if (i == 0) {
			dma_set(s2mm_descr_vrt_addr, CONTROL + offset_nxtdesc, 0x0000000 | BUFFER_BLOCK_WIDTH);
		}
		else if (i == (num_descr - 1)) {
			dma_set(s2mm_descr_vrt_addr, NXTDESC + offset_nxtdesc, HP0_S2MM_DMA_DESCRIPTORS_ADDRESS);
			dma_set(s2mm_descr_vrt_addr, CONTROL + offset_nxtdesc, 0x0000000 | BUFFER_BLOCK_WIDTH);
		}
		else
			dma_set(s2mm_descr_vrt_addr, CONTROL + offset_nxtdesc, BUFFER_BLOCK_WIDTH);
    } // end for
}

/**
 * @brief  Read descriptor status;
 *
 * @param  s2mm_descr_vrt_address - pointer to s2mm descriptors;
 * 	   num_descr              - number of descriptors;
 *
 * @return none;
 */
void dma_descr_scan(unsigned int *s2mm_descr_vrt_addr, int num_descr) {
    unsigned int s2mm_descr_st = 0;

	off_t offset_status;
    int          i;

	off_t buffer_addr = 0;
	unsigned int cmplt = 0;


	for (i = 0; i < num_descr; i++) {
        offset_status = 0x40 * i;
		s2mm_descr_st = dma_get(s2mm_descr_vrt_addr, STATUS + offset_status);

		cmplt = s2mm_descr_st & 0x80000000;
		buffer_addr = dma_get(s2mm_descr_vrt_addr, BUFFER_ADDRESS + offset_status);

		/*
		buffer_addr = dma_get(s2mm_descr_vrt_addr, BUFFER_ADDRESS + offset_status);
		printf("%d) Descriptor Status: 0x%08x"
			"\tBUFFER ADDR: 0x%08lx;\r\n"
			"\tCmplt: %x;"
			"\tRXSOF: %x;"
			"\tRXEOF: %x;"
			"\tBFLEN: %d bytes;\r\n", 
			i, s2mm_descr_st,  
			buffer_addr, cmplt,  s2mm_descr_st & 0x8000000,
			s2mm_descr_st & 0x4000000,   s2mm_descr_st & 0x7FFFFF
			);
		*/
		
		printf("%d)"
			"\tBUFFER ADDR: 0x%08lx; "
			"\tCmplt: %x;"
			"\tBFLEN: %d bytes;\r\n", 
			i, buffer_addr, cmplt, s2mm_descr_st & 0x7FFFFF
			);
    }
}

/**
 * @brief  Reset S2MM register;
 *
 * @param  dma_vrt_address - pointer to AXI DMA controller;
 *
 * @return none;
 */
void dma_reset(unsigned int *dma_vrt_address) {
    dma_set(dma_vrt_address, S2MM_CONTROL_REGISTER, 0x04);		// reset DMA
    dma_set(dma_vrt_address, S2MM_CONTROL_REGISTER, 0x00);		// clear all bits
}

/**
 * @brief  Write into some AXI address;
 *
 * @param  address - pointer to AXI DMA register or descriptor;
 * 	   offset  - address space offset;
 * 	   value   - value for writing;
 *
 * @return none;
 */
void dma_set(unsigned int *address, off_t offset, unsigned int value) {
    address[offset >> 2] = value;
}

/**
 * @brief  Read from some AXI address;
 *
 * @param  address - pointer to AXI DMA register or descriptor;
 * 	   offset  - address space offset;
 *
 * @return Value which was read from address;
 */
int dma_get(unsigned int *address, off_t offset) {
    return address[offset >> 2];
}



int main(void) {
	int 	      fd;
	int           num_descr    = 4;


	//printf("Please, enter number of descriptors: ");
	//scanf("%d", &num_descr);


	fd = open("/dev/mem", O_RDWR | O_SYNC);
	if (fd < 0) {
		fprintf(stderr, "<E>: Couldn't open /dev/mem: %s\r\n", strerror(errno));		
		exit(-1);
	}

	// Mapped memory from phy to virt for work with DMA without Linux drivers
	unsigned int *axi_dma_vrt = mmap(NULL, DESCRIPTOR_REGISTERS_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, AXI_DMA_BASEADDR);
	if (axi_dma_vrt == MAP_FAILED) {
		fprintf(stderr, "<E>: Couldn't mapped memory for AXI DMA: %s\r\n", strerror(errno));
		close(fd);
		return -1;
	}

	// mapped memory for S2MM descriptors
	unsigned int *s2mm_descr_reg_mmap = mmap(NULL, DESCRIPTOR_REGISTERS_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, HP0_S2MM_DMA_DESCRIPTORS_ADDRESS);

	if (s2mm_descr_reg_mmap == MAP_FAILED) {
		fprintf(stderr, "<E>: Couldn't mapped memory for AXI DMA: %s\r\n",
				strerror(errno));
		close(fd);
		return -1;
	}

	unsigned int *dest_mem_map = mmap(NULL, DEST_MEM_BLOCK,	PROT_READ | PROT_WRITE, MAP_SHARED, fd,	(off_t) (HP0_S2MM_TARGET_MEM_ADDRESS));
	
	if (dest_mem_map == MAP_FAILED) {
		fprintf(stderr, "<E>: Couldn't mapped memory for destination block: %s\r\n",
				strerror(errno));
		close(fd);
		return -1;
	}



	// fill s2mm-register memory with zeros
	memset(s2mm_descr_reg_mmap, 0, DESCRIPTOR_REGISTERS_SIZE);
	memset(dest_mem_map, 0, DEST_MEM_BLOCK);

	dma_reset(axi_dma_vrt);		// Reset AXI DMA


	uint32_t s2mm_curr_descr_addr = HP0_S2MM_DMA_DESCRIPTORS_ADDRESS;
	dma_descr_set(s2mm_descr_reg_mmap, num_descr);

	uint32_t s2mm_tail_descr_addr = HP0_S2MM_DMA_DESCRIPTORS_ADDRESS + (num_descr * 0x40 - 0x40);

	dma_set(axi_dma_vrt, S2MM_CURDESC,  s2mm_curr_descr_addr);	
	dma_set(axi_dma_vrt, S2MM_CONTROL_REGISTER, 0x01);	//RUN
	dma_set(axi_dma_vrt, S2MM_TAILDESC, s2mm_tail_descr_addr);

	
	//dma_set(axi_dma_vrt, S2MM_CONTROL_REGISTER, 0x11);	//RUN + CYCLIC
	//dma_set(axi_dma_vrt, S2MM_CONTROL_REGISTER, 0x5001);	//RUN IOC_IRqEn Err_IrqEn

	int ctrl_reg_ok = 0, s2mm_status, f_err = 0;
	int cnt = 0;



	while (!ctrl_reg_ok && !f_err && cnt < 5) {
		s2mm_status = dma_get(axi_dma_vrt, S2MM_STATUS_REGISTER);
		ctrl_reg_ok = (s2mm_status & 0x00001000);

		printf("Stream to memory-mapped status (0x%08x@0x%02x):\n", s2mm_status, S2MM_STATUS_REGISTER);
		printf("ctrl_reg_ok = %i\n", ctrl_reg_ok);
		printf("S2MM_STATUS_REGISTER status register values:\n");

		if (s2mm_status & 0x00000001) printf(" HALTED");
		else	printf(" running");

		if (s2mm_status & 0x00000002) printf(" idle");

		if (s2mm_status & 0x00000200) { printf(" SGSlvErr"); f_err = 1; }
		if (s2mm_status & 0x00000400) { printf(" SGDecErr"); f_err = 1; }
		if (s2mm_status & 0x00001000) { printf(" IOC_Irq");}
		if (s2mm_status & 0x00002000) { printf(" Dly_Irq");}
		if (s2mm_status & 0x00004000) { printf(" Err_Irq"); f_err = 1; }
		printf("\n");
		
		dma_descr_scan(s2mm_descr_reg_mmap, num_descr);

		cnt ++;
		printf("\r\n");

	}

	munmap(axi_dma_vrt,  DESCRIPTOR_REGISTERS_SIZE);
	munmap(s2mm_descr_reg_mmap, DESCRIPTOR_REGISTERS_SIZE);
	munmap(dest_mem_map, DEST_MEM_BLOCK);

	close(fd);

	return 0;
}