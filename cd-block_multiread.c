#include <cd-block.h>
#include <cpu/instructions.h>
#include <smpc/smc.h>
#include <assert.h>

#define CD_STATUS_TIMEOUT 0xAA

int
cd_block_multiple_sectors_read(uint32_t fad, uint32_t number, uint8_t *output_buffer)
{
        const int32_t num_sectors = number;
        uint8_t *out_buffer = output_buffer;

        assert(output_buffer != NULL);
        assert(fad >= 150);

        int ret;
        int num;
        int counter;

        if ((ret = cd_block_cmd_set_sector_length(SECTOR_LENGTH_2048)) != 0) {
                return ret;
        }

        if ((ret = cd_block_cmd_reset_selector(0, 0)) != 0) {
                return ret;
        }

        if ((ret = cd_block_cmd_set_cd_device_connection(0)) != 0) {
                return ret;
        }

        /*static int seed = 1;
        for (int i=0;i<number*2048;i++)
        {
                output_buffer[i] = seed;
                seed = seed * 117;
        }*/

        /* Start reading */
        if ((ret = cd_block_cmd_play_disk(0, fad, num_sectors)) != 0) {
                return ret;
        }

        /* A bit rude - waiting until every sector requested arrived */
        counter = 0;

        while (counter < num_sectors)
        {
            /* If at least one sector has transferred, we copy it */
            num = 0;
            while (num == 0) {
                    num = cd_block_cmd_get_sector_number(0);
            }

            while (num > 0)
            {
                ret = cd_block_transfer_data(0, 0, out_buffer);

                if (ret == 0)
                {
                        out_buffer += 2048;
                        counter++;
                        num--;
                }
                else if (ret == CD_STATUS_TIMEOUT)
                {
                        //do nothing
                }
                /*else
                {
                        //abort 
                        return ret;
                }*/
            }
            
        }

        return 0;
}

int
cd_block_sector_read_request(uint32_t fad)
{
        const int32_t num_sectors = 1;

        assert(fad >= 150);

        int ret;

        if ((ret = cd_block_cmd_set_sector_length(SECTOR_LENGTH_2048)) != 0) {
                return ret;
        }

        if ((ret = cd_block_cmd_reset_selector(0, 0)) != 0) {
                return ret;
        }

        if ((ret = cd_block_cmd_set_cd_device_connection(0)) != 0) {
                return ret;
        }

        /* Start reading */
        if ((ret = cd_block_cmd_play_disk(0, fad, num_sectors)) != 0) {
                return ret;
        }

        return 0;
}

int
cd_block_sector_read_process(uint8_t *output_buffer)
{
        assert(output_buffer != NULL);

        int ret;

        /* If at least one sector has transferred, we copy it */
        while ((cd_block_cmd_get_sector_number(0)) == 0) {
        }

        if ((ret = cd_block_transfer_data(0, 0, output_buffer)) != 0) {
                return ret;
        }

        return 0;
}


int
cd_block_sector_read_flush(uint8_t *output_buffer)
{
        assert(output_buffer != NULL);

        /* If at least one sector has transferred, we copy it */
        while ((cd_block_cmd_get_sector_number(0)) != 0) {
                cd_block_transfer_data(0, 0, output_buffer);
        }

        return 0;
}