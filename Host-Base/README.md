# fscftl - Full Stack Control FTL

## brief
NVMe SSD Host-Based(Linux Kernel) FLash Translation Layer       
base CNEXLabs WestLake controller and NVM++ PPA command set   

the controller need provide physical page address(PPA) interace to Host          
we use CNEXLabs Vendor specified PPA command to manage NandFlash    
these ppa command is needed for io request   
- wrppa(Write PPA)    
- rdppa(Read PPA)        
- ersppa(Erase PPA)   
- wrpparaw(write raw)        
- rdpparaw(read raw)   


## Motivation
FTL is the core component of SSD, It's the determing factor of SSD's performance     
traditionly,it's keep strictly in Firmware, on Host view, it just like a black Box       
namely detail info and restriction of NandFlash is beyond Host software perspective    
in this way Host communication with SSD by typical LBA command(just like HDD)  
It's simple and also can keep Host storage software remained(VFS Block-Layer Driver)   
but on another hand, Host lost control of datapath, for a given SSD controller  
whatever scenario it's apply for(Key-Value log-file media-file etc)   
the performance is fixed(iops latency BW, etc) without Firmware update       
so it is the price we have to paied for the simply solution which mention above        
we have no way to optimized and tuning FTL strategy for different apply scenarios                 
so the intention of this project is to Move the FTL from device to Host    
we aim at control every datapath and background operation on Host side        
in this way, Host is flexible to change to meet different requirement       

this project will implement a whole FTL module base Linux NVMe Driver and LightNVM    
below components in FTL will implemneted    
- L2P mapping   
- write data buffering   
- unstable page incached       
- Multi Luns parallel   
- Power on/down    
- Crash Recovery  
- MCP(Manu factory init)  
- XOR Engine    
- Threshold Tuning/Read Retry      
- Garbage Collection   
- read data page-cache     
- badblock manage    
- User Ioctl interface to manage SSD   

## compare device-base and host-based     

### device-based FTL storage stack     
++++++++++++++++++++++++++++++++   
**Page Cache**      
    buffer_head         
++++++++++++++++++++++++++++++++  
**Generic Block Layer**       
    bio/request            
++++++++++++++++++++++++++++++++    
**Official NVMe Driver**  
    /dev/nvme0n1   
    make_request_fn bio->LBA CMD          
++++++++++++++++++++++++++++++++    
**Controller**    
      FW+Logic    
    (FTL, l2p, gc, error-handle)        
++++++++++++++++++++++++++++++++  
**NandFlash**     


### Host-based FTL storage stack    
++++++++++++++++++++++++++++++++   
**Page Cache**      
    buffer_head         
++++++++++++++++++++++++++++++++  
**Generic Block Layer**       
    bio        
++++++++++++++++++++++++++++++++  
**FSCFTL**       
      /dev/nvme0n1_exp1       
      make_request_fn    
      bio->request    
      fullfill ppa cmd    
    (l2p gc error-handle)    
++++++++++++++++++++++++++++++++    
**Official NVMe Driver**    
      NVMe (dma_map irq hwqueue)      
++++++++++++++++++++++++++++++++    
**Controller**    
      FW+Logic    
++++++++++++++++++++++++++++++++  
**NandFlash**     
++++++++++++++++++++++++++++++++     
