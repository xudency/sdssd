# fscftl - Full Stack Control FTL
NVMe SSD Host-Based(Linux Kernel) FLash Translation Layer       
base CNEXLabs WestLake SSD controller   

the controller need provide physical page address(PPA) interace to Host    
we use wrppa rdppa ersppa to directly manage NandFlash    


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
