/* Manages the frame pools 
    Author: Shravya Kunamalla
    	 CSCE-611 Spring 2014
         Department of Computer Science
         Texas A&M University
    Date  : 02/14/2014
*/

#include "frame_pool.H"
#include "utils.H"
#include "console.H"

FramePool::FramePool(unsigned long _base_frame_no,
             		 unsigned long _nframes,
             		 unsigned long _info_frame_no)

{
	Console::puts("Inside FramePool\n");
	
	base_frame_no = _base_frame_no;
	nframes = _nframes;
	info_frame_no = _info_frame_no;
	//each frame of size 4096 bytes
	unsigned long base_address = base_frame_no * 4096;

	//_info_frame_no is the frame number (within the directly mapped region) of
        //the frame that should be used to store the management information of the frame pool.
	// if info_frame_no is given , then the management information of the frame pool is stored there.
	if(info_frame_no)
	{ 		
		bit_map_address = (unsigned long*) (info_frame_no * 4096);
		// Make all the bits unallocated
		memset(bit_map_address,0,4096); 
	} 
	// else store the management information at the first free frame i.e, the base frame no. 
	else
	{
		bit_map_address = (unsigned long*) (base_frame_no * 4096);
		// Make all the bits unallocated
		memset(bit_map_address, 0 , 4096); 
		get_frame();
	}
 
}   
        
/* Allocates a frame from the frame pool. If successful, returns the frame
    * number of the frame. If fails, returns 0. */
 
unsigned long FramePool::get_frame()
{
	
	//Total available number of frames in _nframes . In order scan for an unallocated frame , we have to check each bit of the bitmap.
	//get the starting address of bit map
	unsigned long *startAddress = bit_map_address;
	
	//first check for each byte and then for each bit of bytes
	//each long has 32 bit and each bit can be used for each frame
	int max = nframes/32; 
	int i,j;
	for(i = 0;i < max ;i++)
	{
		if(startAddress[i] == 0xFFFFFFFF) 
			//means all allocated so just continue and check for next 
			continue; 
			
		//else found an unallocated one and now scan through each of the 32 to find the free frame
		for(j = 0;j < 32;j++)
		{	
			
			int mask = 0x1 << j;
			//Anding startAddress with mask enables us to check each bit. If 1 , means the jth bit is already allocated so continue
			if(startAddress[i] & mask)
				continue; 
				
			//Else we found the unallocated bit so return the frame number corresponding to this bit and mark it unaccessible.
			startAddress[i] = startAddress[i] | mask; 
			return (base_frame_no + (i * 32 + j));	
		}
		//if no free frame, return 0
		return 0;
	}						
}

/* Releases frame back to the given frame pool.
      The frame is identified by the frame number. 
      NOTE: This function is static because there may be more than one frame pool
      defined in the system, and it is unclear which one this frame belongs to.
*/

void FramePool::release_frame(unsigned long _frame_no)
{	
	unsigned long *bit_map_address;
	int ByteNo = _frame_no / 32;
	int BitNo  = _frame_no % 32; 
	//Make a particular bit to 0 and all others 1 . Anding it with the bit map address, just unallocates that bit.
	int mask = ~(0x1 << BitNo); 
	bit_map_address[ByteNo] = bit_map_address[ByteNo] & mask; 	
}

/* Mark the area of physical memory as inaccessible. The arguments have the
* same semanticas as in the constructor.
*/

void FramePool::mark_inaccessible(unsigned long _base_frame_no,
                                  unsigned long _nframes)
{
	//Scan through all the frames then and it with the appropriate mask to unallocate
	int i=0;
	for(i = 0;i < _nframes ; i++)
	{	
		int ByteNo = (_base_frame_no + i) / 32;
		int BitNo  = (_base_frame_no + i) % 32;
		int mask = (0x1 << BitNo);
		//Set the bit to 0 to unallocate it 
	        bit_map_address[ByteNo] = bit_map_address[ByteNo] & mask; 
	        _base_frame_no++;	
	}


}                                  
                        

