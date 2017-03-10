# XUDCFTL
NVMe SSD Host-Based(Linux Kernel) FLash Translation Layer       
split as a Module xudcftl.ko base Linux 4.0+       

## Framework(storage software stack)

++++++++++++++++++++++++++++++++   
**Page Cache**      
    buffer_head         
++++++++++++++++++++++++++++++++  
**Generic Block Layer**       
    bio        
++++++++++++++++++++++++++++++++  
**XUDCFTL**       
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
