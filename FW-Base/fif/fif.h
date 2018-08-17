#ifndef __FIF_H__
#define __FIF_H__

#define NAND_QUAD_PLANE 2
#define NAND_DUAL_PLANE 1
#define NAND_SGNL_PLANE 0

#define NAND_MULT_PLANE PL_BITS

erase_ppa_nowait(ppa_t *ppalist, u16 num, u8 plmode);
erase_ppa_wait(ppa_t *ppalist, u16 num, u8 plmode);

write_ppa_nowait(ppa_t *ppalist, u16 num, void *buff, u8 plmode);
write_ppa_wait(ppa_t *ppalist, u16 num, void *buff, u8 plmode);

read_ppa_nowait(ppa_t *ppalist, u16 num, void *buff, u8 plmode);
read_ppa_wait(ppa_t *ppalist, u16 num, void *buff, u8 plmode);


#endif
