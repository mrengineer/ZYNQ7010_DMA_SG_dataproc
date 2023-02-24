

// FOUND operating project
// https://github.com/GOOD-Stuff/dma-sg-udp



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

// DESCRIPTORS FIELDS
#define NXTDESC						0x00
#define BUFFER_ADDRESS				0x08
#define CONTROL						0x18
#define STATUS						0x1C

// ADDRESSES
#define	AXI_DMA_BASEADDR            0x40400000 //AXI DMA Register Address Map
#define	DESCRIPTOR_REGISTERS_SIZE   0x10000
#define	SG_DMA_DESCRIPTORS_WIDTH    0x1FFFF
#define	MEMBLOCK_WIDTH              0x1FFFFFF  //?size? of mem used by s2mm and mm2s
#define	BUFFER_BLOCK_WIDTH          0x7FFFFF   // size of memory block per descriptor in bytes
#define DEST_MEM_BLOCK				      0x6400000 

#define	HP0_DMA_BUFFER_MEM_ADDRESS       0x10000000
#define	HP0_MM2S_DMA_BASE_MEM_ADDRESS    (HP0_DMA_BUFFER_MEM_ADDRESS)
#define	HP0_S2MM_DMA_BASE_MEM_ADDRESS    (HP0_DMA_BUFFER_MEM_ADDRESS +    \
										                      MEMBLOCK_WIDTH + 1)
#define	HP0_MM2S_DMA_DESCRIPTORS_ADDRESS (HP0_MM2S_DMA_BASE_MEM_ADDRESS)
#define	HP0_S2MM_DMA_DESCRIPTORS_ADDRESS (HP0_S2MM_DMA_BASE_MEM_ADDRESS)
#define	HP0_MM2S_SOURCE_MEM_ADDRESS      (HP0_MM2S_DMA_BASE_MEM_ADDRESS + \
										                      SG_DMA_DESCRIPTORS_WIDTH + 1)
#define	HP0_S2MM_TARGET_MEM_ADDRESS      (HP0_S2MM_DMA_BASE_MEM_ADDRESS + \
										                      SG_DMA_DESCRIPTORS_WIDTH + 1)


static void dma_set(unsigned int *address, off_t offset, unsigned int value);
static int  dma_get(unsigned int *address, off_t offset);


int main(void) {
	int 	      fd;
	int           num_descr    = 0;

	printf("Please, enter number of descriptors: ");
	scanf("%d", &num_descr);

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
	unsigned int *s2mm_descr_reg_mmap = mmap(NULL, DESCRIPTOR_REGISTERS_SIZE,
						 PROT_READ | PROT_WRITE,
						 MAP_SHARED, fd,
						 HP0_S2MM_DMA_DESCRIPTORS_ADDRESS);
	if (s2mm_descr_reg_mmap == MAP_FAILED) {
		fprintf(stderr, "<E>: Couldn't mapped memory for AXI DMA: %s\r\n",
				strerror(errno));
		close(fd);
		return -1;
	}

	unsigned int *dest_mem_map = mmap(NULL, DEST_MEM_BLOCK,
					  PROT_READ | PROT_WRITE, MAP_SHARED, fd,
					  (off_t) (HP0_S2MM_TARGET_MEM_ADDRESS));
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
	dma_set(axi_dma_vrt, S2MM_CONTROL_REGISTER, 0x01);
	dma_set(axi_dma_vrt, S2MM_TAILDESC, s2mm_tail_descr_addr);

	int ctrl_reg_ok = 0, s2mm_status;
	while (!ctrl_reg_ok) {
		s2mm_status = dma_get(axi_dma_vrt, S2MM_STATUS_REGISTER);
		ctrl_reg_ok = (s2mm_status & 0x00001000);
		printf("Stream to memory-mapped status (0x%08x@0x%02x):\n", s2mm_status, S2MM_STATUS_REGISTER);
		printf("S2MM_STATUS_REGISTER status register values:\n");

		if (s2mm_status & 0x00000001) printf(" halted");
		else		    	          printf(" running");
		if (s2mm_status & 0x00000002) printf(" idle");
		if (s2mm_status & 0x00000008) printf(" SGIncld");
		if (s2mm_status & 0x00000010) printf(" DMAIntErr");
		if (s2mm_status & 0x00000020) printf(" DMASlvErr");
		if (s2mm_status & 0x00000040) printf(" DMADecErr");
		if (s2mm_status & 0x00000100) printf(" SGIntErr");
		if (s2mm_status & 0x00000200) printf(" SGSlvErr");
		if (s2mm_status & 0x00000400) printf(" SGDecErr");
		if (s2mm_status & 0x00001000) printf(" IOC_Irq");
		if (s2mm_status & 0x00002000) printf(" Dly_Irq");
		if (s2mm_status & 0x00004000) printf(" Err_Irq");
		printf("\n");
		dma_descr_scan(s2mm_descr_reg_mmap, num_descr);
	}

	munmap(axi_dma_vrt,  DESCRIPTOR_REGISTERS_SIZE);
	munmap(s2mm_descr_reg_mmap, DESCRIPTOR_REGISTERS_SIZE);
	munmap(dest_mem_map, DEST_MEM_BLOCK);

	close(fd);

	return 0;
}


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
    off_t 	       offset_nxtdesc;
    off_t 	       offset_buff_addr;
    const unsigned int offset = 0x40;
    int                i;

    for (i = 0; i < num_descr; i++) {
	offset_nxtdesc   = offset * i;
	offset_buff_addr = BUFFER_BLOCK_WIDTH * i;

	dma_set(s2mm_descr_vrt_addr, NXTDESC + offset_nxtdesc,
		HP0_S2MM_DMA_DESCRIPTORS_ADDRESS + offset_nxtdesc + offset);
	dma_set(s2mm_descr_vrt_addr, BUFFER_ADDRESS + offset_nxtdesc,
		HP0_S2MM_TARGET_MEM_ADDRESS + offset_buff_addr);
	if (i == 0) {
	    dma_set(s2mm_descr_vrt_addr, CONTROL + offset_nxtdesc,
	            0x0000000 | BUFFER_BLOCK_WIDTH);
	}
	else if (i == (num_descr - 1)) {
	    dma_set(s2mm_descr_vrt_addr, NXTDESC + offset_nxtdesc,
	  	    HP0_S2MM_DMA_DESCRIPTORS_ADDRESS);
	    dma_set(s2mm_descr_vrt_addr, CONTROL + offset_nxtdesc,
		    0x0000000 | BUFFER_BLOCK_WIDTH);
	}
	else
	    dma_set(s2mm_descr_vrt_addr, CONTROL + offset_nxtdesc,
	            BUFFER_BLOCK_WIDTH);
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
    off_t        offset_status;
    int          i;

    for (i = 0; i < num_descr; i++) {
        offset_status = 0x40 * i;
	s2mm_descr_st = dma_get(s2mm_descr_vrt_addr, STATUS + offset_status);
	printf("%d) Descriptor Status: 0x%08x \r\n"
		"\tCmplt: %x;\r\n"
		"\tRXSOF: %x;\r\n"
		"\tRXEOF: %x;\r\n"
		"\tBFLEN: %d bytes;\r\n", i, s2mm_descr_st,
		s2mm_descr_st & 0x80000000,  s2mm_descr_st & 0x8000000,
		s2mm_descr_st & 0x4000000,   s2mm_descr_st & 0x7FFFFF);
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
    dma_set(dma_vrt_address, S2MM_CONTROL_REGISTER, 0x04);
    dma_set(dma_vrt_address, S2MM_CONTROL_REGISTER, 0x00);
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
static void dma_set(unsigned int *address, off_t offset, unsigned int value) {
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
static int dma_get(unsigned int *address, off_t offset) {
    return address[offset >> 2];
}





/*
        
//from AXI DMA v7.1 - LogiCORE IP Product Guide http://www.xilinx.com/support/documentation/ip_documentation/axi_dma/v7_1/pg021_axi_dma.pdf

//1. Write the address of the starting descriptor to the Current Descriptor register. If AXI DMA is configured for an address space greater than 32, then also program the MSB 32 bits of the current descriptor.
//2. Start the MM2S channel running by setting the run/stop bit to 1 (MM2S_DMACR.RS =1). The Halted bit (DMASR.Halted) should deassert indicating the MM2S channel is running.
//3. If desired, enable interrupts by writing a 1 to MM2S_DMACR.IOC_IrqEn and MM2S_DMACR.Err_IrqEn.
//4. Write a valid address to the Tail Descriptor register. If AXI DMA is configured for an address space greater than 32, then also program the MSB 32 bits of the tail descriptor.
//5. Writing to the Tail Descriptor register triggers the DMA to start fetching the descriptors from the memory. In case of multichannel configuration, the fetching of descriptors starts when the packet arrives on the S2MM channel.
//6. The fetched descriptors are processed, Data is read from the memory and then output to the MM2S streaming channel.

//                       define mmap locations          
//          consult the README for the exact memory layout


#define clear() printf("\033[H\033[J")

#define	AXI_DMA_REGISTER_LOCATION          0x40400000		//AXI DMA Register Address Map
#define	DESCRIPTOR_REGISTERS_SIZE          0x10000	//0xFFFF
#define	SG_DMA_DESCRIPTORS_WIDTH           0x1FFFF	//0xFFFF
#define	MEMBLOCK_WIDTH                     0x1FFFFFF		//size of mem used by s2mm and mm2s
#define	BUFFER_BLOCK_WIDTH                 0x7F0000 //0x7D0000		//size of memory block per descriptor in bytes
#define	NUM_OF_DESCRIPTORS                 0x7		//number of descriptors for each direction

#define	HP0_DMA_BUFFER_MEM_ADDRESS         0x10000000 //0x20000000
#define	HP0_S2MM_DMA_BASE_MEM_ADDRESS      (HP0_DMA_BUFFER_MEM_ADDRESS + MEMBLOCK_WIDTH + 1)
#define	HP0_S2MM_DMA_DESCRIPTORS_ADDRESS   (HP0_S2MM_DMA_BASE_MEM_ADDRESS)
#define	HP0_S2MM_TARGET_MEM_ADDRESS        (HP0_S2MM_DMA_BASE_MEM_ADDRESS + SG_DMA_DESCRIPTORS_WIDTH + 1)


//                   define all register locations
//               based on "LogiCORE IP Product Guide"



#define SG_CTL                      0x2C    // CACHE CONTROL

// S2MM CONTROL
#define S2MM_CONTROL_REGISTER       0x30    // S2MM_DMACR
#define S2MM_STATUS_REGISTER        0x34    // S2MM_DMASR
#define S2MM_CURDESC                0x38    // must align 0x40 addresses
#define S2MM_CURDESC_MSB            0x3C    // unused with 32bit addresses
#define S2MM_TAILDESC               0x40    // must align 0x40 addresses
#define S2MM_TAILDESC_MSB           0x44    // unused with 32bit addresses



#define S2MM_STATUS_DMA_INT_ERR_BIT			1<<28
#define S2MM_STATUS_DMA_SLV_ERR_BIT			1<<29
#define S2MM_STATUS_DMA_DEC_ERR_BIT			1<<30
#define S2MM_STATUS_CMPLT_BIT       		1<<31
#define S2MM_STATUS_BYTES_TRANSFERED		0x1FFFFFF


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

void bits(int n, int len)
{
	char v = 0; 
    for (unsigned i = 0; i < len; ++i) {
        v = (n & (1 << i)) ? '1' : '0';
		printf("%02i=%c ", i, v);
		if(i%8==7) printf("\n");
    }
	printf("\n");
}

void print_mem(void *virtual_address, int count)
{
	//char *data_ptr = virtual_address;
	u_int32_t *data_ptr = virtual_address;

	for(int i = 0; i < count; i ++){
		//printf("%02X ", data_ptr[i]);
		//printf("%ld", data_ptr[i]);

		// print a space every 4 bytes (0 indexed)
		//if(i%4==3){
		//	printf(" | ");
		//}

		printf("%li ", data_ptr[i]);
		
	}

	printf("\n");
}

int main() {

	setbuf(stdout, NULL);
	printf("START dataproc\n");

	#pragma pack (1)
	typedef struct  {
		uint64_t 	next_descriptor;
		uint64_t	buffer_address;
		uint32_t	reserved1;
		uint32_t	reserved2;
		uint32_t	control;
		uint32_t	status;
		uint32_t	app1;
		uint32_t	app2;
		uint32_t	app3;
		uint32_t	app4;
		uint64_t	fake1;		// because descriptors step must be 0x40h. Tus we have to make struct 64 bytes
		uint64_t	fake2;		// Address Space Offset is relative to 16 - 32-bit word alignment in system memory, that is, 0x00, 0x40, 0x80 and so forth.
	} s2mm_descriptor;

	unsigned int* axi_dma_register_mmap;
	unsigned int* s2mm_descriptor_register_mmap;
	unsigned int* source_mem_map;
	unsigned int* dest_mem_map;

	int controlregister_ok = 0;
	int s2mm_status;	
	
	uint32_t s2mm_current_descriptor_address;
	uint32_t s2mm_tail_descriptor_address;


	int dh = open("/dev/mem", O_RDWR | O_SYNC);
	axi_dma_register_mmap 			= mmap(NULL, DESCRIPTOR_REGISTERS_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, dh, AXI_DMA_REGISTER_LOCATION);
	s2mm_descriptor_register_mmap 	= mmap(NULL, DESCRIPTOR_REGISTERS_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, dh, HP0_S2MM_DMA_DESCRIPTORS_ADDRESS);
	dest_mem_map 					= mmap(NULL, BUFFER_BLOCK_WIDTH * NUM_OF_DESCRIPTORS, PROT_READ | PROT_WRITE, MAP_SHARED, dh, (off_t)(HP0_S2MM_TARGET_MEM_ADDRESS));
	
	int i;
	

	// fill s2mm-register memory with zeros
    for (i = 0; i < DESCRIPTOR_REGISTERS_SIZE; i++) {
		char *p = (char *)s2mm_descriptor_register_mmap;
		p[i] = 0x00000000;
	}
	
	printf("s2mm-register memory with zeros is DONE\n");

       
	//                 reset and halt all dma operations 


	axi_dma_register_mmap[S2MM_CONTROL_REGISTER >> 2] =  0x4;
	axi_dma_register_mmap[S2MM_CONTROL_REGISTER >> 2] =  0x0;

	printf("Reset and halt all dma operations are DONE\n"); // save current descriptor address

	s2mm_descriptor_register_mmap[0x0 >> 2] = HP0_S2MM_DMA_DESCRIPTORS_ADDRESS + 0x40; 	// set next descriptor address
	s2mm_descriptor_register_mmap[0x8 >> 2] = HP0_S2MM_TARGET_MEM_ADDRESS + 0x0; 		// set target buffer address
	s2mm_descriptor_register_mmap[0x18 >> 2] = 0x87D0000; 								// set mm2s/s2mm buffer length to control register


	
	printf ("Register set:  next address  %i\n", s2mm_descriptor_register_mmap[0x0 >> 2]);
	printf ("Register set: target buffer  %i\n", s2mm_descriptor_register_mmap[0x8 >> 2]);
	printf ("Register set: buffer  length %i\n", s2mm_descriptor_register_mmap[0x18 >> 2]);

	s2mm_descriptor * d = (s2mm_descriptor *)s2mm_descriptor_register_mmap;

	//printf ("Size of s2mm_descriptor %i\n\n", sizeof(s2mm_descriptor));


	printf ("Descriptor:  next address  %lli\n", d[0].next_descriptor);
	printf ("Descriptor:  buffer address  %lli\n", d[0].buffer_address);
	printf ("Descriptor:  control  %li\n\n", d[0].control);
	bits(d[0].control, 32);
	printf("\n\n\n");
    	

	s2mm_descriptor_register_mmap[0x40 >> 2] = HP0_S2MM_DMA_DESCRIPTORS_ADDRESS + 0x80; // set next descriptor address
	s2mm_descriptor_register_mmap[0x48 >> 2] = HP0_S2MM_TARGET_MEM_ADDRESS + 0x7D0000; 	// set target buffer address
	s2mm_descriptor_register_mmap[0x58 >> 2] = 0x7D0000; 								// set mm2s/s2mm buffer length to control register


	printf ("Register set:  next address  %i\n", s2mm_descriptor_register_mmap[0x40 >> 2]);
	printf ("Register set: target buffer  %i\n", s2mm_descriptor_register_mmap[0x48 >> 2]);
	printf ("Register set: buffer  length %i\n\n", s2mm_descriptor_register_mmap[0x58 >> 2]);

	printf ("Descriptor:  next address  %lli\n", 	d[1].next_descriptor);
	printf ("Descriptor:  buffer address  %lli\n", 	d[1].buffer_address);
	printf ("Descriptor:  control  %li\n\n", 		d[1].control);
	printf ("Descriptor:  status  %li\n\n", 		d[1].status);
	bits(d[1].control, 32);
	printf("\n\n\n");
	

	print_mem(s2mm_descriptor_register_mmap, 32);


	s2mm_descriptor_register_mmap[0x80 >> 2] = HP0_S2MM_DMA_DESCRIPTORS_ADDRESS + 0xC0; // set next descriptor address
	s2mm_descriptor_register_mmap[0x88 >> 2] = HP0_S2MM_TARGET_MEM_ADDRESS + 0xFA0000; 	// set target buffer address
	s2mm_descriptor_register_mmap[0x98 >> 2] = 0x7D0000; 								// set mm2s/s2mm buffer length to control register

	s2mm_descriptor_register_mmap[0xC0 >> 2] = HP0_S2MM_DMA_DESCRIPTORS_ADDRESS + 0x100; // set next descriptor address
	s2mm_descriptor_register_mmap[0xC8 >> 2] = HP0_S2MM_TARGET_MEM_ADDRESS + 0x1770000; // set target buffer address
	s2mm_descriptor_register_mmap[0xD8 >> 2] = 0x7D0000; // set mm2s/s2mm buffer length to control register

	s2mm_descriptor_register_mmap[0x100 >> 2] = HP0_S2MM_DMA_DESCRIPTORS_ADDRESS + 0x140; // set next descriptor address
	s2mm_descriptor_register_mmap[0x108 >> 2] = HP0_S2MM_TARGET_MEM_ADDRESS + 0x1F40000; // set target buffer address
	s2mm_descriptor_register_mmap[0x118 >> 2] = 0x7D0000; // set mm2s/s2mm buffer length to control register

	s2mm_descriptor_register_mmap[0x140 >> 2] = HP0_S2MM_DMA_DESCRIPTORS_ADDRESS + 0x180; // set next descriptor address
	s2mm_descriptor_register_mmap[0x148 >> 2] = HP0_S2MM_TARGET_MEM_ADDRESS + 0x2710000; // set target buffer address
	s2mm_descriptor_register_mmap[0x158 >> 2] = 0x7D0000; // set mm2s/s2mm buffer length to control register

	s2mm_descriptor_register_mmap[0x180 >> 2] = 0x00; // set next descriptor address (unused?)
	s2mm_descriptor_register_mmap[0x188 >> 2] = HP0_S2MM_TARGET_MEM_ADDRESS + 0x2EE0000; // set target buffer address
	s2mm_descriptor_register_mmap[0x198 >> 2] = 0x47D0000; // set mm2s/s2mm buffer length to control register

	s2mm_tail_descriptor_address = HP0_S2MM_DMA_DESCRIPTORS_ADDRESS + 0x180 ; // save tail descriptor address




	printf("build s2mm stream and control stream is DONE\n");


	//                 set current descriptor addresses
	//           and start dma operations (S2MM_DMACR.RS = 1)


	printf("S2MM_CURDESC is 0x%08x\n", axi_dma_register_mmap[S2MM_CURDESC>>2]);
	axi_dma_register_mmap[S2MM_CURDESC>>2] =  s2mm_current_descriptor_address;
	axi_dma_register_mmap[S2MM_CONTROL_REGISTER >> 2] =  0x1;

	
	//                          start transfer                          
	
	

	axi_dma_register_mmap[S2MM_TAILDESC>>2] =  s2mm_tail_descriptor_address;

	
	//                 wait until all transfers finished                
	

	printf("wait until all transfers finished\n");
	
	i = 0;

	//int j;

	while (!controlregister_ok)
    {
		clear();

		i ++;
		printf("ITER %i\n", i);
		s2mm_status = axi_dma_register_mmap[S2MM_STATUS_REGISTER >> 2];

		controlregister_ok = (s2mm_status & 0x00001000);

		//printf("Stream to memory-mapped status (0x%08x@0x%02x):\n", s2mm_status, S2MM_STATUS_REGISTER);
		printf("S2MM_STATUS_REGISTER status register values:\n");
		if (s2mm_status & 0x00000001) printf(" halted"); else printf(" running");
		if (s2mm_status & 0x00000002) printf(" idle");
		if (s2mm_status & 0x00000008) printf(" SGIncld");
		if (s2mm_status & 0x00000010) printf(" DMAIntErr");
		if (s2mm_status & 0x00000020) printf(" DMASlvErr");
		if (s2mm_status & 0x00000040) printf(" DMADecErr");
		if (s2mm_status & 0x00000100) printf(" SGIntErr");
		if (s2mm_status & 0x00000200) printf(" SGSlvErr");
		if (s2mm_status & 0x00000400) printf(" SGDecErr");
		if (s2mm_status & 0x00001000) printf(" IOC_Irq");
		if (s2mm_status & 0x00002000) printf(" Dly_Irq");
		if (s2mm_status & 0x00004000) printf(" Err_Irq");
		
		s2mm_current_descriptor_address = (uint32_t) axi_dma_register_mmap[S2MM_CURDESC>>2];
		printf("\nCurdesc 0x%08x\n", s2mm_current_descriptor_address);

		
		for (unsigned char j = 0; j < 7; j++) {
				printf(" Descriptor %i:  TRANSFERED   %li\n", j, (d[j].status & S2MM_STATUS_BYTES_TRANSFERED));
				printf("                COMPLETED    %i\n", (d[j].status & (S2MM_STATUS_CMPLT_BIT)));
				printf("                DMA_DEC_ERR  %i\n", (d[j].status & (S2MM_STATUS_DMA_DEC_ERR_BIT)));
				printf("                DMA_SLV_ERR  %i\n", (d[j].status & (S2MM_STATUS_DMA_SLV_ERR_BIT)));
		}

		//S2MM_STATUS_BYTES_TRANSFERED
		//for (j = 0; j < 8; j++) {
		//	char *p = (char *)dest_mem_map;
		//	printf(" %i@0x%08x@0", (unsigned int)p, *p);
		//}
		
		printf("\n");
		delay(100);
    }
        

	return 0;
}
*/