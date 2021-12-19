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
    char * script_buffer;
    char * tmp_buffer;
    char * tmp_buffer2;
    bool bFinished = false;
    char * pDebug = (char*)0x20280000;
    int i,j,k;
    int iActor;
    int iActorColor;
    int iLayer;
    int iPosition;
    int iPalette;
    //char *sprite_filename;

    //first let's find script FAD, browsing root folder
    //-------------- Getting FAD and index for background pack binary -------------------
    fad_t _script_fad;
    assert(true == _svin_filelist_search(filename,&_script_fad,&i));

    script_buffer = malloc(4096);
    tmp_buffer = malloc(2048);
    tmp_buffer2 = malloc(2048);
    _svin_cd_block_sector_read(_script_fad, (uint8_t*)script_buffer);
    _script_fad++;
    int iDataInBuffer=2048;
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
                case 255: 
                    strcpy(tmp_buffer2,"<narrative>");
                    iActorColor = 7;
                    break;
                case 254: 
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
            iDataInBuffer -= (i+1);
            //wait for keypress
            _svin_wait_for_key_press_and_release();
        }
        else if (strncmp(script_buffer,"BG ",3)==0)
        {
            sprintf(&pDebug[iStringNumber*32],"BG at line %i",iStringNumber);
            //temporary measures - removing all sprites when changing BG
            for (i=0;i<3;i++)
                _svin_sprite_clear(i);
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
            iDataInBuffer -= (i+1);
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
            iDataInBuffer -= (i+1);
        }
        else if (strncmp(script_buffer,"SPRITE ",7)==0)
        {
            //draw sprite
            i = (int)strchr(script_buffer,'\r') - (int)script_buffer;
            if (i>2048) i=2048;
            if (i<4) i=4;
            j = (int)strstr(script_buffer,"LAYER ") - (int)script_buffer;
            j+=6;
            iLayer=atoi(&script_buffer[j]);
            if (iLayer<0) iLayer = 0;
            if (iLayer>2) iLayer = 2;
            j = (int)strstr(script_buffer,"POSITION ") - (int)script_buffer;
            j+=9;
            iPosition=atoi(&script_buffer[j]);
            if (iPosition<0) iPosition = 0;
            if (iPosition>2) iPosition = 2;
            j = (int)strstr(script_buffer,"PALETTE ") - (int)script_buffer;
            j+=8;
            iPalette=atoi(&script_buffer[j]);
            if (iPalette<0) iPalette = 0;
            if (iPalette>2) iPalette = 2;
            j = (int)strstr(script_buffer,"FILE ") - (int)script_buffer;
            j+=5;
            memcpy(tmp_buffer,&(script_buffer[j]),i-j);
            tmp_buffer[i-j]=0;
            //_svin_sprite_draw(tmp_buffer,0,0);
            sprintf(&pDebug[iStringNumber*32],"SPRITE pos%ilayer%ipal%iline%i",iPosition,iLayer,iPalette,iStringNumber);
            //memcpy(&pDebug[iStringNumber*32],tmp_buffer,32); //copy sprite name for debug
            _svin_sprite_draw(tmp_buffer,iLayer,iPosition,iPalette);
            
            //remove command from buffer
            for (j=i+1;j<4096;j++)
                script_buffer[j-i-1] = script_buffer[j];
            iDataInBuffer -= (i+1);
        }
        else if (strncmp(script_buffer,"CLEAR ",6)==0)
        {
            //draw sprite
            i = (int)strchr(script_buffer,'\r') - (int)script_buffer;
            if (i>2048) i=2048;
            if (i<4) i=4;
            j = (int)strstr(script_buffer,"POSITION ") - (int)script_buffer;
            j+=9;
            iPosition=atoi(&script_buffer[j]);
            if (iPosition<0) iPosition = 0;
            if (iPosition>2) iPosition = 2;
            _svin_sprite_clear(iPosition);
            sprintf(&pDebug[iStringNumber*32],"CLEAR %i at line %i",iPosition,iStringNumber);
           
            //remove command from buffer
            for (j=i+1;j<4096;j++)
                script_buffer[j-i-1] = script_buffer[j];
            iDataInBuffer -= (i+1);
        }
        else if (strncmp(script_buffer,"ENABLE ",6)==0)
        {
            //draw sprite
            i = (int)strchr(script_buffer,'\r') - (int)script_buffer;
            if (i>2048) i=2048;
            if (i<4) i=4;
            j = (int)strstr(script_buffer,"POSITION ") - (int)script_buffer;
            j+=9;
            iPosition=atoi(&script_buffer[j]);
            if (iPosition<0) iPosition = 0;
            if (iPosition>2) iPosition = 2;
            _svin_set_cycle_patterns_nbg();//position ignored
            sprintf(&pDebug[iStringNumber*32],"ENABLE %i at line %i",iPosition,iStringNumber);
           
            //remove command from buffer
            for (j=i+1;j<4096;j++)
                script_buffer[j-i-1] = script_buffer[j];
            iDataInBuffer -= (i+1);
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
            i = (int)strchr(script_buffer,'\r') - (int)script_buffer;
            if (i>2048) i=2048;
            if (i<4) i=4;
            //remove command from buffer
            for (j=i+1;j<4096;j++)
                script_buffer[j-i-1] = script_buffer[j];
            iDataInBuffer -= (i+1);
        }
        //should we load more data?
        if (iDataInBuffer<2048)
        {
            _svin_cd_block_sector_read(_script_fad, (uint8_t*)tmp_buffer2);
            _script_fad++;
            for (j=0;j<2048;j++)
            {
                script_buffer[iDataInBuffer] = tmp_buffer2[j];
                iDataInBuffer++;
            }
        }
    }   
}