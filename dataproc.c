

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

/* a=target variable, b=bit number to act upon 0-n */
#define BIT_SET(a,b)		((a) |= (1ULL<<(b)))
#define BIT_CLEAR(a,b)		((a) &= ~(1ULL<<(b)))
#define BIT_FLIP(a,b)		((a) ^= (1ULL<<(b)))
#define BIT_CHECK(a,b)		(!!((a) & (1ULL<<(b))))        // '!!' to make sure this returns 0 or 1

#define BITMASK_SET(x, mask)		((x) |= (mask))
#define BITMASK_CLEAR(x, mask)		((x) &= (~(mask)))
#define BITMASK_FLIP(x, mask)		((x) ^= (mask))
#define BITMASK_CHECK_ALL(x, mask)	(!(~(x) & (mask)))
#define BITMASK_CHECK_ANY(x, mask)	((x) & (mask))



#include <termios.h>


#define MM2S_CONTROL_REGISTER       0x00
#define MM2S_STATUS_REGISTER        0x04
#define MM2S_SRC_ADDRESS_REGISTER   0x18
#define MM2S_TRNSFR_LENGTH_REGISTER 0x28

#define S2MM_CONTROL_REGISTER       0x30
#define S2MM_STATUS_REGISTER        0x34
#define S2MM_DST_ADDRESS_REGISTER   0x48
#define S2MM_BUFF_LENGTH_REGISTER   0x58

#define IOC_IRQ_FLAG                1<<12
#define IDLE_FLAG                   1<<1

#define STATUS_HALTED               0x00000001
#define STATUS_IDLE                 0x00000002
#define STATUS_SG_INCLDED           0x00000008
#define STATUS_DMA_INTERNAL_ERR     0x00000010
#define STATUS_DMA_SLAVE_ERR        0x00000020
#define STATUS_DMA_DECODE_ERR       0x00000040
#define STATUS_SG_INTERNAL_ERR      0x00000100
#define STATUS_SG_SLAVE_ERR         0x00000200
#define STATUS_SG_DECODE_ERR        0x00000400
#define STATUS_IOC_IRQ              0x00001000
#define STATUS_DELAY_IRQ            0x00002000
#define STATUS_ERR_IRQ              0x00004000

#define HALT_DMA                    0x00000000
#define RUN_DMA                     0x00000001
#define RESET_DMA                   0x00000004
#define ENABLE_IOC_IRQ              0x00001000
#define ENABLE_DELAY_IRQ            0x00002000
#define ENABLE_ERR_IRQ              0x00004000
#define ENABLE_ALL_IRQ              0x00007000

#define M							1 //8

long long current_timestamp() {
    struct timeval te; 
    gettimeofday(&te, NULL); // get current time
    long long milliseconds = te.tv_sec*1000LL + te.tv_usec/1000; // calculate milliseconds
    // printf("milliseconds: %lld\n", milliseconds);
    return milliseconds;
}

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

unsigned int write_dma(unsigned int *virtual_addr, int offset, unsigned int value)
{
    virtual_addr[offset>>2] = value;

    return 0;
}

unsigned int read_dma(unsigned int *virtual_addr, int offset)
{
    return virtual_addr[offset>>2];
}


void dma_s2mm_status(unsigned int *virtual_addr)
{
    unsigned int status;
	status = read_dma(virtual_addr, S2MM_STATUS_REGISTER);

    //printf("Stream to memory-mapped status (0x%08x@0x%02x):", status, S2MM_STATUS_REGISTER);

    /*if (status & STATUS_HALTED) {
		printf(" Halted.\n");
	} else {
		printf(" Running.\n");
	}*/

    //if (status & STATUS_IDLE) {
	//	printf(" Idle.\n");
	//}

    if (status & STATUS_SG_INCLDED) {
		printf(" SG is included.\n");
	}

    if (status & STATUS_DMA_INTERNAL_ERR) {
		printf(" DMA internal error.\n");
	}

    if (status & STATUS_DMA_SLAVE_ERR) {
		printf(" DMA slave error.\n");
	}

    if (status & STATUS_DMA_DECODE_ERR) {
		printf(" DMA decode error.\n");
	}

    if (status & STATUS_SG_INTERNAL_ERR) {
		printf(" SG internal error.\n");
	}

    if (status & STATUS_SG_SLAVE_ERR) {
		printf(" SG slave error.\n");
	}

    if (status & STATUS_SG_DECODE_ERR) {
		printf(" SG decode error.\n");
	}

    if (status & STATUS_IOC_IRQ) {
		printf(" IOC interrupt occurred.\n");
	}

    if (status & STATUS_DELAY_IRQ) {
		printf(" Interrupt on delay occurred.\n");
	}

    if (status & STATUS_ERR_IRQ) {
		printf(" Error interrupt occurred.\n");
	}
}

void dma_mm2s_status(unsigned int *virtual_addr)
{
    unsigned int status;
	status = read_dma(virtual_addr, MM2S_STATUS_REGISTER);

    /*
	printf("Memory-mapped to stream status (0x%08x@0x%02x):", status, MM2S_STATUS_REGISTER);

    if (status & STATUS_HALTED) {
		printf(" Halted.\n");
	} else {
		printf(" Running.\n");
	}
	*/

    /*if (status & STATUS_IDLE) {
		printf(" Idle.\n");
	}*/

    if (status & STATUS_SG_INCLDED) {
		printf(" SG is included.\n");
	}

    if (status & STATUS_DMA_INTERNAL_ERR) {
		printf(" DMA internal error.\n");
	}

    if (status & STATUS_DMA_SLAVE_ERR) {
		printf(" DMA slave error.\n");
	}

    if (status & STATUS_DMA_DECODE_ERR) {
		printf(" DMA decode error.\n");
	}

    if (status & STATUS_SG_INTERNAL_ERR) {
		printf(" SG internal error.\n");
	}

    if (status & STATUS_SG_SLAVE_ERR) {
		printf(" SG slave error.\n");
	}

    if (status & STATUS_SG_DECODE_ERR) {
		printf(" SG decode error.\n");
	}

    if (status & STATUS_IOC_IRQ) {
		printf(" IOC interrupt occurred.\n");
	}

    if (status & STATUS_DELAY_IRQ) {
		printf(" Interrupt on delay occurred.\n");
	}

    if (status & STATUS_ERR_IRQ) {
		printf(" Error interrupt occurred.\n");
	}
}

void memdump(void* virtual_address, int byte_count) {
    char *p = virtual_address;
    int offset;
    for (offset = 0; offset < byte_count; offset++) {
        printf("%02x", p[offset]);
        if (offset % 4 == 3) { printf(" "); }
    }
    printf("\n");
}


void print_mem(void *virtual_address, int count)
{
	//char *data_ptr = virtual_address;
	unsigned int *data_ptr = virtual_address;

	for(int i = 0; i < count; i ++){
		//printf("%02X ", data_ptr[i]);
		//printf("%ld", data_ptr[i]);

		// print a space every 4 bytes (0 indexed)
		//if(i%4==3){
		//	printf(" | ");
		//}

		printf("%i ", data_ptr[i]);
		
	}

	printf("\n");
}

int dma_mm2s_sync(unsigned int* dma_virtual_address) {
    unsigned int mm2s_status =  read_dma(dma_virtual_address, MM2S_STATUS_REGISTER);

	while(!(mm2s_status & IOC_IRQ_FLAG) || !(mm2s_status & IDLE_FLAG)){
        dma_s2mm_status(dma_virtual_address);
        dma_mm2s_status(dma_virtual_address);

		printf("--\r\n");

        mm2s_status =  dma_get(dma_virtual_address, MM2S_STATUS_REGISTER);
		delay(200);
    }
}

int dma_s2mm_sync(unsigned int* dma_virtual_address) {
    unsigned int s2mm_status = read_dma(dma_virtual_address, S2MM_STATUS_REGISTER);
	while(!(s2mm_status & IOC_IRQ_FLAG) || !(s2mm_status & IDLE_FLAG)){
        dma_s2mm_status(dma_virtual_address);
        dma_mm2s_status(dma_virtual_address);
		printf("--\r\n");
        s2mm_status = dma_get(dma_virtual_address, S2MM_STATUS_REGISTER);
		delay(200);
    }
}

int main()
{
    //printf("- Hello World! - Running DMA transfer test application.\n");

	printf("- Opening a character device file of the Arty's DDR memeory...\n");
	int ddr_memory = open("/dev/mem", O_RDWR | O_SYNC);

	printf("- Memory map the address of the DMA AXI IP via its AXI lite control interface register block.\n");
    unsigned int *dma_virtual_addr = mmap(NULL, 65535, PROT_READ | PROT_WRITE, MAP_SHARED, ddr_memory, 0x40400000);

	printf("- Memory map the MM2S source address register block.\n");
    unsigned int *virtual_src_addr  = mmap(NULL, 65535, PROT_READ | PROT_WRITE, MAP_SHARED, ddr_memory, 0x0e000000);

	printf("- Memory map the S2MM destination address register block.\n");
    unsigned int *virtual_dst_addr = mmap(NULL, 65535, PROT_READ | PROT_WRITE, MAP_SHARED, ddr_memory, 0x0f000000);

	printf("- Writing random data to source register block...\n");
	virtual_src_addr[0]= 0x00000001;
	virtual_src_addr[1]= 0x00000002;
	virtual_src_addr[2]= 0x00000003;
	virtual_src_addr[3]= 0x00000004;

	virtual_src_addr[4]= 0x00000005;
	virtual_src_addr[5]= 0x00000006;
	virtual_src_addr[6]= 0x00000007;
	virtual_src_addr[7]= 0x00000008;

	unsigned long long i=0;
	long long  start = current_timestamp();

	
	printf("> Clearing the destination register block...\n");
	memset(virtual_dst_addr, 0, 32*M);

		//if (i < 2) {
			printf("> Source memory block data:      ");
			print_mem(virtual_src_addr, 8);

			printf("> Destination memory block data: ");
			print_mem(virtual_dst_addr, 8);
		//}


		printf("> Reset the DMA.\n");
		write_dma(dma_virtual_addr, S2MM_CONTROL_REGISTER, RESET_DMA);
		write_dma(dma_virtual_addr, MM2S_CONTROL_REGISTER, RESET_DMA);
		dma_s2mm_status(dma_virtual_addr);
		dma_mm2s_status(dma_virtual_addr);


		printf("> Halt the DMA.\n");
		write_dma(dma_virtual_addr, S2MM_CONTROL_REGISTER, HALT_DMA);
		write_dma(dma_virtual_addr, MM2S_CONTROL_REGISTER, HALT_DMA);
		dma_s2mm_status(dma_virtual_addr);
		dma_mm2s_status(dma_virtual_addr);

		write_dma(dma_virtual_addr, MM2S_CONTROL_REGISTER, ENABLE_ALL_IRQ);
		dma_s2mm_status(dma_virtual_addr);
		dma_mm2s_status(dma_virtual_addr);

		printf("> Writing source address of the data from MM2S in DDR...\n");
		write_dma(dma_virtual_addr, MM2S_SRC_ADDRESS_REGISTER, 0x0e000000);
		dma_mm2s_status(dma_virtual_addr);

		printf("> Writing the destination address for the data from S2MM in DDR...\n");
		write_dma(dma_virtual_addr, S2MM_DST_ADDRESS_REGISTER, 0x0f000000);
		dma_s2mm_status(dma_virtual_addr);

		printf("> Run the MM2S channel.\n");
		write_dma(dma_virtual_addr, MM2S_CONTROL_REGISTER, RUN_DMA);
		dma_mm2s_status(dma_virtual_addr);

		printf("> Run the S2MM channel.\n");
		write_dma(dma_virtual_addr, S2MM_CONTROL_REGISTER, RUN_DMA);		
		dma_s2mm_status(dma_virtual_addr);

		printf("\n\n\n");

	//while(1){

		printf("> Writing MM2S transfer length of 32 bytes...\n");
		write_dma(dma_virtual_addr, MM2S_TRNSFR_LENGTH_REGISTER, 32*M);
		dma_mm2s_status(dma_virtual_addr);

		printf("> Writing S2MM transfer length of 32 bytes...\n");
		write_dma(dma_virtual_addr, S2MM_BUFF_LENGTH_REGISTER, 32*M);
		dma_s2mm_status(dma_virtual_addr);

		printf("> Waiting for MM2S synchronization...\n");
		dma_mm2s_sync(dma_virtual_addr);

		printf("> Waiting for S2MM sychronization...\n");
		dma_s2mm_sync(dma_virtual_addr);

		dma_s2mm_status(dma_virtual_addr);
		dma_mm2s_status(dma_virtual_addr);

		

		if (i < 10) {
			printf("+ Destination memory block: ");
			print_mem(virtual_dst_addr, 8);
			printf("\n");
		}

/*
		if (i == 100000) {
			printf("%i done in %lld milliseconds\n\n", i, (current_timestamp()-start));			
		}

		if (i == 1000000) {
			printf("%i done in %lld milliseconds\n\n", i, (current_timestamp()-start));			
		}

		if (i == 5000000) {
			printf("%i done in %lld milliseconds\n\n", i, (current_timestamp()-start));			
		}		
		i++;	
		//delay(1);
	}*/
    return 0;
}

/*
int main() {
    int dh = open("/dev/mem", O_RDWR | O_SYNC); // Open /dev/mem which represents the whole physical memory
    unsigned int* virtual_address = mmap(NULL, 65535, PROT_READ | PROT_WRITE, MAP_SHARED, dh, 0x40400000); // Memory map AXI Lite register block
    unsigned int* virtual_source_address  = mmap(NULL, 65535, PROT_READ | PROT_WRITE, MAP_SHARED, dh, 0x0e000000); // Memory map source address
    unsigned int* virtual_destination_address = mmap(NULL, 65535, PROT_READ | PROT_WRITE, MAP_SHARED, dh, 0x0f000000); // Memory map destination address

    virtual_source_address[0]= 0x11223344; // Write random stuff to source block
    memset(virtual_destination_address, 0, 32); // Clear destination block

    printf("Source memory block:      "); memdump(virtual_source_address, 32);
    printf("Destination memory block: "); memdump(virtual_destination_address, 32);

    printf("Resetting DMA\n");
    dma_set(virtual_address, S2MM_CONTROL_REGISTER, 4);
    dma_set(virtual_address, MM2S_CONTROL_REGISTER, 4);
    dma_s2mm_status(virtual_address);
    dma_mm2s_status(virtual_address);

    printf("Halting DMA\n");
    dma_set(virtual_address, S2MM_CONTROL_REGISTER, 0);
    dma_set(virtual_address, MM2S_CONTROL_REGISTER, 0);
    dma_s2mm_status(virtual_address);
    dma_mm2s_status(virtual_address);

    printf("Writing destination address\n");
    dma_set(virtual_address, S2MM_DESTINATION_ADDRESS, 0x0f000000); // Write destination address
    dma_s2mm_status(virtual_address);

    printf("Writing source address...\n");
    dma_set(virtual_address, MM2S_START_ADDRESS, 0x0e000000); // Write source address
    dma_mm2s_status(virtual_address);

    printf("Starting S2MM channel with all interrupts masked...\n");
    dma_set(virtual_address, S2MM_CONTROL_REGISTER, 0xf001);
    dma_s2mm_status(virtual_address);

    printf("Starting MM2S channel with all interrupts masked...\n");
    dma_set(virtual_address, MM2S_CONTROL_REGISTER, 0xf001);
    dma_mm2s_status(virtual_address);

    printf("Writing S2MM transfer length...\n");
    dma_set(virtual_address, S2MM_LENGTH, 32);
    dma_s2mm_status(virtual_address);

    printf("Writing MM2S transfer length...\n");
    dma_set(virtual_address, MM2S_LENGTH, 32);
    dma_mm2s_status(virtual_address);

    printf("Waiting for MM2S synchronization...\n");
    dma_mm2s_sync(virtual_address);

    printf("Waiting for S2MM sychronization...\n");
    dma_s2mm_sync(virtual_address); // If this locks up make sure all memory ranges are assigned under Address Editor!

    dma_s2mm_status(virtual_address);
    dma_mm2s_status(virtual_address);

    printf("Destination memory block: "); memdump(virtual_destination_address, 32);
}
*/


/* SG VARIANT */
/*
// S2MM CONTROL
#define S2MM_CONTROL_REGISTER       0x30    // S2MM_DMACR
#define MDACR_Run_Stop_BIT			0
#define MDACR_IOC_IRqEn_BIT			12
#define MDACR_Dly_IrqEn_BIT			13
#define MDACR_Err_IrqEn_BIT			14


#define S2MM_STATUS_REGISTER        0x34    // S2MM_DMASR
#define STATUS_Halted_BIT			0
#define STATUS_Idle_BIT				1
#define STATUS_SGIncld_BIT			3
#define STATUS_DMAIntErr_BIT		4
#define STATUS_DMASlvErr_BIT		5
#define STATUS_DMADecErr_BIT		6
#define STATUS_SGIntErr_BIT			8
#define STATUS_SGSlvErr_BIT			9
#define STATUS_SGDecErr_BIT			10
#define STATUS_IOC_Irq_BIT			12
#define STATUS_Dly_Irq_BIT			13
#define STATUS_Err_Irq_BIT			14


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


void print_mem(void *virtual_address, unsigned int count)
{
	
	unsigned int * data_ptr = virtual_address;

	unsigned int val = 0;
	unsigned int prev = 0;
	signed int delta = 0;

	for(unsigned int i = 0; i < count; i ++){
		val = (uint32_t)data_ptr[i];				//Maybe I am wrong here?????

		if (val != prev + 1) {
			delta = val - prev;
			printf("%i\t%u\t prev=%u\t delta=%i\r\n", i, val, prev, delta);
		}
		prev = val;
	}

	printf("\n");
}

void print_binary(unsigned int number){
    int i,n;
    
    n = sizeof(int)*8;

    for(i=n-1;i>=0;i--) {
      printf("%d",(number >> i) & 1);
      if(i % 8 == 0) printf(" ");
    }
}

static void dma_set(unsigned int *address, off_t offset, unsigned int value);
static int  dma_get(unsigned int *address, off_t offset);


void dma_descr_set(unsigned int *s2mm_descr_vrt_addr, int num_descr) {
    off_t				offset_nxtdesc;
    off_t				offset_buff_addr;
    const unsigned int	offset = 0x40;
    int					i;

    for (i = 0; i < num_descr; i++) {
		offset_nxtdesc   = offset * i;
		offset_buff_addr = BUFFER_BLOCK_WIDTH * i + 1*i;


		printf("Descriptor %i, descriptor address start is 0x%08lx \r\n", i, HP0_S2MM_DMA_DESCRIPTORS_ADDRESS + offset_nxtdesc);

		dma_set(s2mm_descr_vrt_addr, NXTDESC + offset_nxtdesc, HP0_S2MM_DMA_DESCRIPTORS_ADDRESS + offset_nxtdesc + offset);	//NB! Because mmap is used, s2mm_descr_vrt_addr is shifted for HP0_S2MM_DMA_DESCRIPTORS_ADDRESS
		printf("\tNXTDESC (0x%02lx)\t= 0x%08lx\r\n", NXTDESC + offset_nxtdesc, HP0_S2MM_DMA_DESCRIPTORS_ADDRESS + offset_nxtdesc + offset);

		dma_set(s2mm_descr_vrt_addr, BUFFER_ADDRESS + offset_nxtdesc, HP0_S2MM_TARGET_MEM_ADDRESS + offset_buff_addr);
		printf("\tBUFFER_ADDRESS (0x%02lx)\t = 0x%02lx\r\n", BUFFER_ADDRESS + offset_nxtdesc, HP0_S2MM_TARGET_MEM_ADDRESS + offset_buff_addr);

		if (i == 0) {			
			dma_set(s2mm_descr_vrt_addr, CONTROL + offset_nxtdesc, 0x0000000 | BUFFER_BLOCK_WIDTH);
			printf("\tCONTROL eq LEN (0x%02lx)\t = 0x%02x\r\n", CONTROL + offset_nxtdesc, 0x0000000 | BUFFER_BLOCK_WIDTH);
		}
		else if (i == (num_descr - 1)) {
			dma_set(s2mm_descr_vrt_addr, NXTDESC + offset_nxtdesc, HP0_S2MM_DMA_DESCRIPTORS_ADDRESS);						//1st descriptor
			printf("\tUPD NXTDESC (0x%02lx)\t 0x%08x\r\n", NXTDESC + offset_nxtdesc, HP0_S2MM_DMA_DESCRIPTORS_ADDRESS);

			dma_set(s2mm_descr_vrt_addr, CONTROL + offset_nxtdesc, 0x0000000 | BUFFER_BLOCK_WIDTH);
			printf("\tCONTROL eq LEN (0x%02lx)\t = 0x%02x\r\n", CONTROL + offset_nxtdesc, 0x0000000 | BUFFER_BLOCK_WIDTH);
		}
		else
			dma_set(s2mm_descr_vrt_addr, CONTROL + offset_nxtdesc, BUFFER_BLOCK_WIDTH);
    } // end for
}


void dma_descr_scan(unsigned int *s2mm_descr_vrt_addr, int num_descr, int fd) {
    unsigned int s2mm_descr_st = 0;

	off_t offset_status;
    int          i;

	off_t buffer_addr			= 0;
	unsigned int cmplt			= 0;
	unsigned long buffer_len	= 0;
	unsigned int *buf_mem_map;

	unsigned char complete_bit	= 0;

	for (i = 0; i < num_descr; i++) {
        offset_status = 0x40 * i;
		s2mm_descr_st = dma_get(s2mm_descr_vrt_addr, STATUS + offset_status);
		
		cmplt		= s2mm_descr_st & 0x80000000;
		complete_bit = BIT_CHECK(s2mm_descr_st, 31);

		buffer_len	= s2mm_descr_st & 0x7FFFFF;
		buffer_addr	= dma_get(s2mm_descr_vrt_addr, BUFFER_ADDRESS + offset_status);
		
		
		printf("%d)\tBUFFER ADDR: 0x%08lx;\tCmplt: %d (bit=%i);\tBFLEN: %ld bytes;\r\n", 
					i, buffer_addr, cmplt, complete_bit, buffer_len);


		if (complete_bit) {
			buf_mem_map 	= mmap(NULL, buffer_len, PROT_READ, MAP_SHARED, fd, buffer_addr);

			print_mem(buf_mem_map, buffer_len/4);

			printf(" s2mm_descr_st WAS ");
			print_binary(s2mm_descr_st);
			printf("\r\n s2mm_descr_st UPD ");
			
			BIT_CLEAR(s2mm_descr_st, 31);

			dma_set(s2mm_descr_vrt_addr, STATUS + offset_status, s2mm_descr_st);
			s2mm_descr_st = dma_get(s2mm_descr_vrt_addr, STATUS + offset_status);
			print_binary(s2mm_descr_st);
			printf("\r\n\r\n");
		
			if (buf_mem_map == MAP_FAILED) {
				fprintf(stderr, "<E>: Couldn't mapped memory for descriptor buffer block: %s\r\n", strerror(errno));
			} else {
				munmap(buf_mem_map,  buffer_len);
			}
		}
    }
}


void dma_reset(unsigned int *dma_vrt_address) {
    dma_set(dma_vrt_address, S2MM_CONTROL_REGISTER, 0x04);		// reset DMA
    dma_set(dma_vrt_address, S2MM_CONTROL_REGISTER, 0x00);		// clear all bits
}


void dma_set(unsigned int *address, off_t offset, unsigned int value) {
    address[offset >> 2] = value;
}

int dma_get(unsigned int *address, off_t offset) {
    return address[offset >> 2];
}


*/

/*
int main(void) {
	int 	      fd;
	int           num_descr    = 4;


	//printf("Please, enter number of descriptors: ");
	//scanf("%d", &num_descr);

	printf("Define HP0_S2MM_DMA_BASE_MEM_ADDRESS\t= 0x%08x\r\n", HP0_S2MM_DMA_BASE_MEM_ADDRESS);
	printf("Define HP0_S2MM_DMA_DESCRIPTORS_ADDRESS\t= 0x%08x\r\n", HP0_S2MM_DMA_DESCRIPTORS_ADDRESS);
	printf("Define HP0_S2MM_TARGET_MEM_ADDRESS\t= 0x%08x\r\n\r\n", HP0_S2MM_TARGET_MEM_ADDRESS);



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
	unsigned int *s2mm_descr_reg_mmap = mmap(NULL, DESCRIPTOR_REGISTERS_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, (off_t) HP0_S2MM_DMA_DESCRIPTORS_ADDRESS);

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

//	dma_reset(axi_dma_vrt);		// Reset AXI DMA

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

		//printf("Stream to memory-mapped status (0x%08x@0x%02x):\n", s2mm_status, S2MM_STATUS_REGISTER);
		printf("ctrl_reg_ok = %i\n", ctrl_reg_ok);
		//printf("S2MM_STATUS_REGISTER status register values:\n");

		if (s2mm_status & 0x00000001) printf(" HALTED");
		else	printf(" running");

		if (s2mm_status & 0x00000002) printf(" idle");

		if (s2mm_status & 0x00000200) { printf(" SGSlvErr"); f_err = 1; }
		if (s2mm_status & 0x00000400) { printf(" SGDecErr"); f_err = 1; }
		if (s2mm_status & 0x00001000) { printf(" IOC_Irq");}


		if (s2mm_status & 0x00002000) { printf(" Dly_Irq");}
		if (s2mm_status & 0x00004000) { printf(" Err_Irq"); f_err = 1; }
		printf("\n");
		
		dma_descr_scan(s2mm_descr_reg_mmap, num_descr, fd);
		cnt ++;


		if BIT_CHECK(s2mm_status, 12) {
			printf(" IOC_Irq >>>");


			//re-read
			printf("\r\n s2mm_status BEFORE UPDATE ");
			print_binary(s2mm_status);
			

			//Writhe 1 to this bit will clear it. page 18 Thus write back it with '1' inside
			//dma_set(axi_dma_vrt, S2MM_STATUS_REGISTER, s2mm_status);

			//printf("\r\n s2mm_status AFTER         ");
			//print_binary(s2mm_status);

			//s2mm_status = dma_get(axi_dma_vrt, S2MM_STATUS_REGISTER);
			//ctrl_reg_ok = (s2mm_status & 0x00001000);
			//printf("\r\nUPD s2mm_status = %i, ctrl_reg_ok = %i\n", s2mm_status, ctrl_reg_ok);

			printf("\n");
			//dma_descr_scan(s2mm_descr_reg_mmap, num_descr, fd);

			//break;	

			//dma_set(axi_dma_vrt, S2MM_CONTROL_REGISTER, 0x01);	//RUN
		}

		printf("\r\n");
		delay(1);

	}

	munmap(axi_dma_vrt,  DESCRIPTOR_REGISTERS_SIZE);
	munmap(s2mm_descr_reg_mmap, DESCRIPTOR_REGISTERS_SIZE);
	munmap(dest_mem_map, DEST_MEM_BLOCK);

	close(fd);

	return 0;
}*/


/* SG MAIN

int main(void) {
	int 	      fd;
	int           num_descr    = 5;


	//printf("Please, enter number of descriptors: ");
	//scanf("%d", &num_descr);

	printf("Define HP0_S2MM_DMA_BASE_MEM_ADDRESS\t= 0x%08x\r\n", HP0_S2MM_DMA_BASE_MEM_ADDRESS);
	printf("Define HP0_S2MM_DMA_DESCRIPTORS_ADDRESS\t= 0x%08x\r\n", HP0_S2MM_DMA_DESCRIPTORS_ADDRESS);
	printf("Define HP0_S2MM_TARGET_MEM_ADDRESS\t= 0x%08x\r\n\r\n", HP0_S2MM_TARGET_MEM_ADDRESS);


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

	dma_reset(axi_dma_vrt);		// Reset AXI DMA

	// fill s2mm-register memory with zeros
	memset(s2mm_descr_reg_mmap, 0, DESCRIPTOR_REGISTERS_SIZE);
	memset(dest_mem_map, 0, DEST_MEM_BLOCK);

	dma_reset(axi_dma_vrt);		// Reset AXI DMA
	printf("DMA reset...\r\n");

	uint32_t s2mm_curr_descr_addr = HP0_S2MM_DMA_DESCRIPTORS_ADDRESS;
	dma_descr_set(s2mm_descr_reg_mmap, num_descr);

	uint32_t s2mm_tail_descr_addr = HP0_S2MM_DMA_DESCRIPTORS_ADDRESS + (num_descr * 0x40 - 0x40);

	dma_set(axi_dma_vrt, S2MM_CURDESC,  s2mm_curr_descr_addr);
	printf("DMA S2MM_CURDESC (0x%08x)=0x%08x\r\n", S2MM_CURDESC, s2mm_curr_descr_addr);


	uint32_t run = 0;
	BIT_SET(run, MDACR_Run_Stop_BIT);
	//BIT_SET(run, MDACR_IOC_IRqEn_BIT);
	//BIT_SET(run, MDACR_Dly_IrqEn_BIT);
	//BIT_SET(run, MDACR_Err_IrqEn_BIT);


	dma_set(axi_dma_vrt, S2MM_CONTROL_REGISTER, run);	//RUN
	
	
	//dma_set(axi_dma_vrt, S2MM_CONTROL_REGISTER, 0x11);	//RUN + CYCLIC
	//dma_set(axi_dma_vrt, S2MM_CONTROL_REGISTER, 0x5001);	//RUN IOC_IrqEn Err_IrqEn
	dma_set(axi_dma_vrt, S2MM_TAILDESC, s2mm_tail_descr_addr);

	
	printf("DMA run\r\n");
	printf("DMA S2MM_TAILDESC (0x%08x)=0x%08x\r\n", S2MM_TAILDESC, s2mm_tail_descr_addr);


	s2mm_curr_descr_addr = dma_get(axi_dma_vrt, S2MM_CURDESC);
	printf("DMA S2MM_CURDESC (0x%08x)=0x%08x\r\n", S2MM_CURDESC, s2mm_curr_descr_addr);
	

	int ctrl_reg_ok = 0, s2mm_status, f_err = 0;
	unsigned int cnt = 0;


//!ctrl_reg_ok &&
	while ( !f_err) {
		s2mm_status = dma_get(axi_dma_vrt, S2MM_STATUS_REGISTER);
		//ctrl_reg_ok = (s2mm_status & 0x00001000);


		//printf("Stream to memory-mapped status (0x%08x@0x%02x):\n", s2mm_status, S2MM_STATUS_REGISTER);
		printf("\r\nLOOP %i) s2mm_status = %i, ctrl_reg_ok = %i ", cnt, s2mm_status, ctrl_reg_ok);
		//printf("S2MM_STATUS_REGISTER status register values:\n");

		s2mm_curr_descr_addr = dma_get(axi_dma_vrt, S2MM_CURDESC);
		printf("DMA S2MM_CURDESC (0x%08x)=0x%08x\r\n", S2MM_CURDESC, s2mm_curr_descr_addr);	

		if BIT_CHECK(s2mm_status, STATUS_Halted_BIT) printf(" HALTED"); else printf(" running");
		if BIT_CHECK(s2mm_status, STATUS_Idle_BIT) printf(" idle");

		if BIT_CHECK(s2mm_status, STATUS_SGIncld_BIT) printf(" SG_mode_EN");
		
		if BIT_CHECK(s2mm_status, STATUS_IOC_Irq_BIT) printf(" IOC_Irq");
		if BIT_CHECK(s2mm_status, STATUS_Dly_Irq_BIT) printf(" Dly_Irq");

		if BIT_CHECK(s2mm_status, STATUS_DMAIntErr_BIT) { printf(" DMAIntErr"); f_err = 1; }
		if BIT_CHECK(s2mm_status, STATUS_DMASlvErr_BIT) { printf(" SGSlvErr"); f_err = 1; }
		if BIT_CHECK(s2mm_status, STATUS_DMADecErr_BIT) { printf(" SGDecErr"); f_err = 1; }
		if BIT_CHECK(s2mm_status, STATUS_SGIntErr_BIT) { printf(" SGIntErr"); f_err = 1; }
		if BIT_CHECK(s2mm_status, STATUS_SGSlvErr_BIT) { printf(" SGSlvErr"); f_err = 1; }
		if BIT_CHECK(s2mm_status, STATUS_SGDecErr_BIT) { printf(" SGDecErr"); f_err = 1; }
		if BIT_CHECK(s2mm_status, STATUS_Err_Irq_BIT) { printf(" Err_Irq"); f_err = 1; }
				
		printf("\r\n");


		//if (s2mm_status & 0x00001000) printf(" IOC_Irq");
		if BIT_CHECK(s2mm_status, STATUS_IOC_Irq_BIT) {
			printf("\r\nIOC_Irq proccessing...");


			dma_descr_scan(s2mm_descr_reg_mmap, num_descr, fd);

			//re-read
			printf("\r\n s2mm_status IOC BIT IS %i\r\n", BIT_CHECK(s2mm_status, STATUS_IOC_Irq_BIT));
			

			//Write 1 to this bit will clear it. page 18 Thus write back it with '1' inside
			BIT_SET(s2mm_status, STATUS_IOC_Irq_BIT);
			dma_set(axi_dma_vrt, S2MM_STATUS_REGISTER, s2mm_status);

			s2mm_status = dma_get(axi_dma_vrt, S2MM_STATUS_REGISTER);
			printf("\r\n s2mm_status IOC BIT AFTER %i\r\n", BIT_CHECK(s2mm_status, STATUS_IOC_Irq_BIT));


			

			dma_set(axi_dma_vrt, S2MM_CONTROL_REGISTER, 0x0);	//STOP

			dma_set(axi_dma_vrt, S2MM_CURDESC,  HP0_S2MM_DMA_DESCRIPTORS_ADDRESS);
			
			printf("DMA S2MM_CURDESC (0x%08x)=0x%08x\r\n", S2MM_CURDESC, dma_get(axi_dma_vrt, S2MM_CURDESC));

			
			dma_set(axi_dma_vrt, S2MM_CONTROL_REGISTER, run);	//RUN


			dma_set(axi_dma_vrt, S2MM_TAILDESC, s2mm_tail_descr_addr);			//continue according p. 16 about idle

			s2mm_status = dma_get(axi_dma_vrt, S2MM_STATUS_REGISTER);

				if BIT_CHECK(s2mm_status, STATUS_Halted_BIT) printf(" HALTED"); else printf(" running");
				if BIT_CHECK(s2mm_status, STATUS_Idle_BIT) printf(" idle");

				if BIT_CHECK(s2mm_status, STATUS_SGIncld_BIT) printf(" SG_mode_EN");
				
				if BIT_CHECK(s2mm_status, STATUS_IOC_Irq_BIT) printf(" IOC_Irq");
				if BIT_CHECK(s2mm_status, STATUS_Dly_Irq_BIT) printf(" Dly_Irq");

				if BIT_CHECK(s2mm_status, STATUS_DMAIntErr_BIT) { printf(" DMAIntErr"); f_err = 1; }
				if BIT_CHECK(s2mm_status, STATUS_DMASlvErr_BIT) { printf(" SGSlvErr"); f_err = 1; }
				if BIT_CHECK(s2mm_status, STATUS_DMADecErr_BIT) { printf(" SGDecErr"); f_err = 1; }
				if BIT_CHECK(s2mm_status, STATUS_SGIntErr_BIT) { printf(" SGIntErr"); f_err = 1; }
				if BIT_CHECK(s2mm_status, STATUS_SGSlvErr_BIT) { printf(" SGSlvErr"); f_err = 1; }
				if BIT_CHECK(s2mm_status, STATUS_SGDecErr_BIT) { printf(" SGDecErr"); f_err = 1; }
				if BIT_CHECK(s2mm_status, STATUS_Err_Irq_BIT) { printf(" Err_Irq"); f_err = 1; }			
		}


		delay(10);

		cnt ++;
		

	}

	munmap(axi_dma_vrt,  DESCRIPTOR_REGISTERS_SIZE);
	munmap(s2mm_descr_reg_mmap, DESCRIPTOR_REGISTERS_SIZE);
	munmap(dest_mem_map, DEST_MEM_BLOCK);

	close(fd);

	printf("Programm is finished\r\n");

	return 0;
}
*/