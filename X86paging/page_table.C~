/* Page Table creation, enabling and fault handling
    Author: Shravya Kunamalla
    	 CSCE-611 Spring 2014
         Department of Computer Science
         Texas A&M University
    Date  : 02/14/2014
*/

#include "page_table.H"
#include "console.H"
#include "paging_low.H"

PageTable     * PageTable::current_page_table = NULL; // Pointer to currently loaded page table object     
FramePool     * PageTable::kernel_mem_pool = NULL;    //Pointer to the kernel memory pool   
FramePool     * PageTable::process_mem_pool = NULL;   //Pointer to the process memory pool
unsigned long   PageTable::shared_size = 0;          //Shared address space 
unsigned int    PageTable::paging_enabled = 0;       //Flag showing if paging is turned on.      

PageTable::PageTable()
{

	/* Implements the two Implements the two level paging scheme of x-86 architecture. The CR3 register points to the top-level table called the page directory.*/
 
	Console::puts("In page_table.c \n");
	
    /* Get a free frame from kernel memory pool and set up page directory there
	page_directory_frame_no is the number of the free frame.*/ 
	unsigned long page_directory_frame_no = kernel_mem_pool -> get_frame();

	/*Since frames are at 4k boundaries, frame address can be obtained by multiplying 
	page_directory_frame_no with 4k.*/ 
	page_directory = (unsigned long *)(4096 * page_directory_frame_no); 
	
    unsigned long page_table_frame_no = kernel_mem_pool -> get_frame(); //Get a free frame from kernel memory pool for page table
	
	unsigned long *page_table = (unsigned long *) (4096 * page_table_frame_no);
	
	//Put the address of the page table in the first row of page directory. 
	page_directory[0] = (unsigned long) page_table;
	
	//Set the lower 3 bits i.e, P: present bit, R/W : read write and U/S : User/Supervisor bits(binary 011)
	page_directory[0] = page_directory[0] | 3 ; 

	//There are total 1024 page directory entries, Mark all the remaining 1023 entries as not present
	//index is the index into page directory.
	unsigned int index;
	for(index = 1 ; index < 1024; index++) 
	{
		page_directory[index] = 0 | 2; //set to : supervisor, read/write, not present (010 in binary)
	}
	
	//"address" holds the physical address of where the page is 
	unsigned long address = 0;

	//Map first 4MB of this page table 
	for (index = 0; index < 1024; index++)
	{
		page_table[index] = address | 3; // attribute set to: supervisor level, read/write, present(011 in binary)
		address = address + 4096; // jump to next page table entry
	}

}

void PageTable::init_paging(FramePool * _kernel_mem_pool,
                            FramePool * _process_mem_pool,
                            const unsigned long _shared_size)
{
	kernel_mem_pool = _kernel_mem_pool;
	process_mem_pool = _process_mem_pool;
	//Shared size is constant and is equal to 4MB. 
	shared_size = _shared_size; 
}
       
/* Makes the given page table the current table. This must be done once during
   system startup and whenever the address space is switched (e.g. during
   process switching). 
*/   

void PageTable::load(){
	PageTable::current_page_table = this;
}	

/* Enable paging on the CPU. Typically, a CPU start with paging disabled, and
   memory is accessed by addressing physical memory directly. After paging is
   enabled, memory is addressed logically. 
*/
void PageTable::enable_paging()
{
	
    // put the page directory address into CR3.
    write_cr3((long unsigned int) current_page_table -> addressPageDirectory());
	
	//Set the paging bit in CR0 to 1. 	    
	write_cr0(read_cr0() | 0x80000000);

	//Set the paging_enabled bit to 1.
	paging_enabled = 1; 

	Console::puts("Page Table Enabled\n");
}

/* The page fault handler. */
void PageTable::handle_fault(REGS * _r)
{
	Console::puts("Inside page fault handler\n");
	
	//Check error code
	unsigned long error_code = _r -> err_code;

	//reading CR2 get the address of the page where page fault occured. Print this on the console. 
	Console::putui(read_cr2());
	unsigned long address_faulty_page = read_cr2();
    
	//Get the location of the page directory
	unsigned long* current_page_directory = PageTable::current_page_table -> addressPageDirectory();

	//Page directory index is in the first 10 most significant bits so shift page directory index by 22 bits to the right. 
  	unsigned long page_directory_index = address_faulty_page >> 22; 

	//Get the corresponding page table entry. 0x3FF is 1023 in hexa decimal. 
	//0x3FF << 12 gives 1111111111000000000000. 
	// Anding this with the faulty page address gives the 13th to 22nd bits of the address_faulty_page followed by 12 zeroes.
	//Shift it right by 12 bits to get the page table entry. 
  	unsigned long page_table_entry = (((0x3FF << 12) & address_faulty_page) >> 12); 
  	unsigned long  page_table_address = 0;
	
	// check if it's a page directory fault or page fault
	
	if(!(error_code & 0x1)) // last bit not set to 1 implies page fault
	{
		// get 2 new frames, one for new page table, the other one for new page and fill corresponding entries
		//if there is no page directory entry for that page table,allocate one from kernel mem pool
		if((current_page_directory [ page_directory_index ] & 0x1 ) == 0)
		{	
			//Allocate frame for page directory index
			unsigned long new_dir_frame = PageTable::kernel_mem_pool -> get_frame();
  			current_page_directory[ page_directory_index ] = ( new_dir_frame * 4096 );
			current_page_directory[ page_directory_index ] = current_page_directory[ page_directory_index ] | 7;
                        
  			//Allocate frame for that page table entry from process mem pool and mark it as present
  			unsigned long new_page_frame = PageTable::process_mem_pool -> get_frame();
  			unsigned long page_index = (unsigned long) (4096 * new_page_frame);
  			((unsigned long*)page_table_address)[ page_table_entry ] = ( page_index | 7); 
		}
		 
		//if there is page directory, but no page table entry in that for page (No Page table)
		else 
		{	
			//Page fault 
			//Allocate frame for that page table entry from process mem pool
			page_table_address = (current_page_directory[ page_directory_index ] ) & (0xFFFFF000);	
  			unsigned long new_page_frame = PageTable::process_mem_pool -> get_frame();

  			unsigned long page_index = (unsigned long) ( 4096 * new_page_frame );
			((unsigned long*)page_table_address)[ page_table_entry ] = page_index;
  			((unsigned long*)page_table_address)[ page_table_entry ] =((unsigned long*)page_table_address)[ page_table_entry ] | 7; 
		}
	
	}
	else
	{
		Console::puts("Some Unknown error happened");
		for(;;);
	}	
	
	
}
  
