# SSD Flash translation Layer prototype


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
    (FTL, Checkpoint, gc, WL, error-handle)        
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
    (FTL Checkpoint gc WL error-handle)    
++++++++++++++++++++++++++++++++    
**Official NVMe Driver**    
      NVMe (dma_map irq hwqueue)      
++++++++++++++++++++++++++++++++    
**Controller**    
      FW+Logic    
++++++++++++++++++++++++++++++++  
**NandFlash**     
++++++++++++++++++++++++++++++++     
