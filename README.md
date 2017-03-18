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
    bio        
++++++++++++++++++++++++++++++++    
**Official NVMe Driver**  
    /dev/nvme0n1   
    make_request_fn bio->LBA CMD          
++++++++++++++++++++++++++++++++    
**Controller**    
      FW+Logic    
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
      /dev/cnexssd    
      make_request_fn bio->PPA CMD         
++++++++++++++++++++++++++++++++    
**Official NVMe Driver**    
      NVMe PPA R/W/E API   
++++++++++++++++++++++++++++++++    
**Controller**    
      FW+Logic    
++++++++++++++++++++++++++++++++  
**NandFlash**     
++++++++++++++++++++++++++++++++     
