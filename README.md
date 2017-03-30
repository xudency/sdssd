# fscftl - Full Stack Control FTL
NVMe SSD Host-Based(Linux Kernel) FLash Translation Layer       
base CNEXLabs WestLake SSD controller   

the controller need provide physical page address(PPA) interace to Host    
we use wrppa rdppa ersppa to directly manage NandFlash    

FTL is the core of SSD, traditionly,it's keep strictly in Firmware    
so the PPA is hidden on Host software's perspectively    
as FW or HW maintain FTL on devide side, so Linux Driver communication    
with SSD by typical LBA command(the same with HDD)     
in this way It's simple to Host but on another hand, with FTL firmly on device    
Host lost control of datapath, thus for a SSD controller  
whatever it's apply for(KV-store log-file media-file) 
the performance is fixed(iops latency BW, etc)   
we have no way to optimized and tuning for different apply scenario    
so the intention of this project is to Move the FTL from device to Host    
we aim at control every datapath and background operation on Host     
in this way, Host is flexible to change to meet different scenario's requirement       

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
- read data page cache     
- badblock manage   

compare device-base and host-based     

## device-based FTL storage stack     
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


## Host-based storage stack   
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
