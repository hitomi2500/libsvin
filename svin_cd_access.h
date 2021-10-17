#ifndef _SVIN_CD_ACCESS_H_
#define _SVIN_CD_ACCESS_H_

#include <stdint.h>
#include <cd-block/cmd.h>
#include "iso9660-internal-local.h"

extern int _svin_cd_block_sectors_read(uint32_t fad, uint8_t *output_buffer, uint32_t length);
extern int _svin_cd_block_sector_read(uint32_t fad, uint8_t *output_buffer);

extern int _svin_cd_block_sector_read_request(uint32_t fad);
extern int _svin_cd_block_sector_read_process(uint8_t *output_buffer);
extern int _svin_cd_block_sector_read_flush(uint8_t *output_buffer);

extern void iso9660_rom_filelist_read(const iso9660_filelist_entry_t root_entry,
    iso9660_filelist_t *filelist, int32_t count);

#endif /* !_SVIN_CD_ACCESS_H_ */
