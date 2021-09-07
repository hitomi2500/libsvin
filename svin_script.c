#include <yaul.h>

#include <assert.h>
#include <stdlib.h>

#include "svin.h"
#include "svin_textbox.h"

#include <mcufont.h>

#define UNUSED(x) (void)(x)

void 
_svin_run_script(char * filename)
{
    char script_buffer[4096];
    char tmp_buffer[2048];
    char tmp_buffer2[32];
    bool bFinished = false;
    char * pDebug = (char*)0x20280000;
    int i,j,k;
    int iActor;
    int iActorColor;

    //first let's find script FAD, browsing root folder
    //-------------- Getting FAD and index for background pack binary -------------------
    fad_t _script_fad = _svin_filelist_search(filename);
    /*for (unsigned int i = 0; i < _filelist->entries_count; i++)
    {
        file_entry = &(_filelist->entries[i]);
        if (strcmp(file_entry->name, filename) == 0)
        {
            _script_fad = file_entry->starting_fad;
        }
    }*/
    assert(_script_fad > 0);
    //reading 1st block
    cd_block_sector_read(_script_fad, (uint8_t*)script_buffer);
    //starting parse cycle 
    bFinished = false;
    int iStringNumber = 0;
    strcpy(pDebug,"start 1");
    _svin_textbox_clear();
    while (false == bFinished)
    {
        iStringNumber++;
        if (strncmp(script_buffer,"TEXT ",5)==0)
        {
            sprintf(&pDebug[iStringNumber*32],"TEXT at line %i",iStringNumber);
            //print text on panel
            i = (int)strchr(script_buffer,'\r') - (int)script_buffer;
            if (i>2048) i=2048;
            if (i<4) i=4;
            //getting actor number
            j=4;
            while (strncmp(&(script_buffer[j]),"ACTOR=",6)!=0)
                j++;
            j+=6;
            iActor=0;
            while (script_buffer[j]!=' ')
            {
                iActor *= 10;
                iActor += script_buffer[j]-'0';
                j++;
            }
            switch (iActor)
            {
                case 0: 
                    strcpy(tmp_buffer2,"<narrative>");
                    iActorColor = 7;
                    break;
                case 1: 
                    strcpy(tmp_buffer2,"<think>");
                    iActorColor = 7;
                    break;
                case 2: 
                    strcpy(tmp_buffer2,"<me>");
                    iActorColor = 7;
                    break;
                case 3: 
                    strcpy(tmp_buffer2,"Славя");
                    iActorColor = 6;
                    break;
                case 4: 
                    strcpy(tmp_buffer2,"Лена");
                    iActorColor = 0;
                    break;
                case 5: 
                    strcpy(tmp_buffer2,"Алиса");
                    iActorColor = 2;
                    break;
                default:
                    strcpy(tmp_buffer2,"<it's a bug>");
                    iActorColor = 7;
                    break;
            }
            //moving on to text
            while (script_buffer[j]!='"')
                j++;
            j++; //skipping colon
            k=0; 
            while (script_buffer[j]!='"')
            {
                tmp_buffer[k] = script_buffer[j];
                j++;
                k++;
            }
            tmp_buffer[k] = 0;
            _svin_textbox_print(tmp_buffer2,tmp_buffer,"Lato_Black12",iActorColor,iActorColor);
            //remove command from buffer
            for (j=i+1;j<4096;j++)
                script_buffer[j-i-1] = script_buffer[j];
            //wait for keypress
            _svin_wait_for_key_press_and_release();
        }
        else if (strncmp(script_buffer,"BG ",3)==0)
        {
            sprintf(&pDebug[iStringNumber*32],"BG at line %i",iStringNumber);
            //set bg
            i = (int)strchr(script_buffer,'\r') - (int)script_buffer;
            if (i>2048) i=2048;
            if (i<4) i=4;
            for (j=3;j<i;j++)
                tmp_buffer[j-3] = script_buffer[j];
            tmp_buffer[i-3]=0;
            _svin_background_update(tmp_buffer);
            //remove command from buffer
            for (j=i+1;j<4096;j++)
                script_buffer[j-i-1] = script_buffer[j];
        }
        else if (strncmp(script_buffer,"REM ",4)==0)
        {
            sprintf(&pDebug[iStringNumber*32],"REM at line %i",iStringNumber);
            //comment
            i = (int)strchr(script_buffer,'\r') - (int)script_buffer;
            if (i>2048) i=2048;
            if (i<4) i=4;
            //remove command from buffer
            for (j=i+1;j<4096;j++)
                script_buffer[j-i-1] = script_buffer[j];
        }
        else if (strncmp(script_buffer,"SPRITE ",7)==0)
        {
            sprintf(&pDebug[iStringNumber*32],"SPRITE at line %i",iStringNumber);
            //draw sprite
            i = (int)strchr(script_buffer,'\r') - (int)script_buffer;
            if (i>2048) i=2048;
            if (i<4) i=4;
            //remove command from buffer
            for (j=i+1;j<4096;j++)
                script_buffer[j-i-1] = script_buffer[j];
        }
        else if (strncmp(script_buffer,"END",3)==0)
        {
            sprintf(&pDebug[iStringNumber*32],"END at line %i",iStringNumber);
            //end of script
            bFinished = true;
        }
        else
        {
            sprintf(&pDebug[iStringNumber*32],"UNKNOWN at line %i",iStringNumber);
        }
    }   
}