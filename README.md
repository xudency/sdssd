# fscftl - Full Stack Control FTL
NVMe SSD Host-Based(Linux Kernel) FLash Translation Layer       
split as a Module fscftl.ko base Linux 4.0+       

## Framework(storage software stack)

++++++++++++++++++++++++++++++++   
**Page Cache**      
    buffer_head         
++++++++++++++++++++++++++++++++  
**Generic Block Layer**       
    bio        
++++++++++++++++++++++++++++++++  
**FSCFTL**       
      make_request_fn    
++++++++++++++++++++++++++++++++    
**Official NVMe Driver**    
      NVMe SQE/CQE     
++++++++++++++++++++++++++++++++    
**Controller**    
      FW+Logic    
++++++++++++++++++++++++++++++++  
**NandFlash**     
++++++++++++++++++++++++++++++++     
