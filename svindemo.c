#include <yaul.h>

#include <assert.h>
#include <stdlib.h>

#include "svin.h"

#define MENU_ENTRY_COUNT 16


extern vdp1_cmdt_list_t *_svin_cmdt_list;

int
main(void)
{



        /* Load the maximum number */
#ifdef _SVIN_DIRTY_STATIC_LINKING
        //for dirty linking, fill the filelist manually
        _filelist.entries[0].name[0] = 'B';
        _filelist.entries[0].name[1] = 'G';
        _filelist.entries[0].name[2] = '.';
        _filelist.entries[0].name[3] = 'P';
        _filelist.entries[0].name[4] = 'A';
        _filelist.entries[0].name[5] = 'K';
        _filelist.entries[0].name[6] = '\0';
        _filelist.entries[0].starting_fad = 128; //BG.PAK starts at 1M in CS0
        _filelist.entries[0].size = 636928; // DIRTY UPDATE ME!
        _filelist.entries[0].sector_count = (_filelist.entries[0].size - 1)/2048 + 1;
        _filelist.entries[1].name[0] = 'S';
        _filelist.entries[1].name[1] = 'L';
        _filelist.entries[1].name[2] = '.';
        _filelist.entries[1].name[3] = 'P';
        _filelist.entries[1].name[4] = 'A';
        _filelist.entries[1].name[5] = 'K';
        _filelist.entries[1].name[6] = '\0';
        _filelist.entries[1].starting_fad = 480; //SL.PAK starts at 2M in CS0
        _filelist.entries[1].size = 47104; // DIRTY UPDATE ME!
        _filelist.entries[1].sector_count = (_filelist.entries[1].size - 1)/2048 + 1;
        _filelist.entries_count = 2;
        _filelist.entries_pooled_count = 0;
#else
#endif


        _svin_filelist_fill(); //move this to init probably

        _svin_init();

        //_svin_hang_test(9);   
        //while(1);   

        MEMORY_WRITE(32, SCU(ASR0), 0x23301FF0);

        _svin_background_load_index("BG.PAK");
        //_svin_actor_debug_load_index(&_filelist);

        //load logo
        _svin_clear_palette(0);
        _svin_background_set("yaul_logo");
        _svin_delay(1000);
        _svin_background_fade_to_black();

        //_svin_background_set("int_dining_hall_people_day");
        //_svin_background_set_by_index(0);
        //_svin_actor_debug_load_test(&_filelist,"SL.PAK",0);
        //_svin_actor_debug_load_test(&_filelist,"US.PAK",1);

        //_svin_background_set_by_index(0);

        //vdp1_cmdt_t *cmdts;
        //cmdts = &_svin_cmdt_list->cmdts[0]; 
        //vdp1_cmdt_t *cmdt_sprite;

        //_svin_background_set("ext_camp_entrance_day");
        //_svin_textbox_clear();
        //_svin_textbox_print("Helena","Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.","Lato_Black12",0,0);
        //_svin_background_update("bus_stop");
        _svin_run_script("SCRIPT.TXT");

        while(1);

        while(1)
        {
                for (unsigned int i=0; i< 100; i++)
                {
                //_svin_background_set _by_index(i);
                _svin_background_update_by_index(i);
                //_svin_background_set("bus_stop");

                //render some text
                _svin_textbox_clear();
                switch(i)
                {
                case 0:
                        _svin_textbox_print("Helena","Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.","Lato_Black12",0,0);
                        break;
                case 1:
                        _svin_textbox_print("Slavya","Hey!","Lato_Black12",6,6);
                        break;
                case 2:
                        _svin_textbox_print("Helena","Sed ut perspiciatis unde omnis iste natus error sit voluptatem accusantium doloremque laudantium, totam rem aperiam, eaque ipsa quae ab illo inventore veritatis et quasi architecto beatae vitae dicta sunt explicabo. Nemo enim ipsam voluptatem quia voluptas sit aspernatur aut odit aut fugit, sed quia consequuntur magni dolores eos qui ratione voluptatem sequi nesciunt. Neque porro quisquam est, qui.","Lato_Black12",0,0);
                        break;
                case 3:
                        _svin_textbox_print("Slavya","Hey, Lena! Will you stop it already? Are you trying to summon a devil?","Lato_Black12",6,6);
                        break;
                case 4:
                        _svin_textbox_print("Helena","At vero eos et accusamus et iusto odio dignissimos ducimus....","Lato_Black12",0,0);
                        break;               
                case 5:
                        _svin_textbox_print("Slavya","I SAID STOP!","DejaVuSerif32",6,6);
                        break;
                case 6:
                        _svin_textbox_print("Helena","Wow! How did you do that?","Lato_Black12",0,0);
                        break;
                case 7:
                        _svin_textbox_print("Slavya","That's... I'd rather not talk about it.","Lato_Black12",6,6);
                        break;
                case 8:
                        _svin_textbox_print("Helena","C'mon, you can tell me. Was it Times New Roman? You lucky...","Lato_Black12",0,0);
                        break;
                case 9:
                        _svin_textbox_print("Slavya","No! Why in the world... No, i'm not telling you anything.","Lato_Black12",6,6);
                        break;
                case 10:
                        _svin_textbox_print("Ульяна","Привет, девчонки! Чего ругаетесь? Можно мне тоже?","Lato_Black12",1,1);
                        break;
                case 11:
                        _svin_textbox_print("Славя","Да мы тут с Леной... Ульяна, почему ты не на субботнике?","Lato_Black12",6,6);
                        break;
                case 12:
                        _svin_textbox_print("Ульяна","Да ну, скукотища! Я вот французкий учу, смотрите","Lato_Black12",1,1);
                        break;
                case 13:
                        _svin_textbox_print("Ulyana","Lorem superposés valise pourparlers rêver chiots rendez-vous naissance Eiffel myrtille. Grèves Arc de Triomphe encore pourquoi sentiments baguette pédiluve une projet sentiments saperlipopette vachement le. Brume éphémère baguette Bordeaux en fait sommet avoir minitel.","Lato_Black12",1,1);
                        break;
                case 14:
                        _svin_textbox_print("Slavyana","Et toi aussi?","Lato_Black12",6,6);
                        break;
                case 15:
                        _svin_textbox_print("Alice","Quel est le bruit sans combat?","Lato_Black12",5,5);
                        break;
                case 16:
                        _svin_textbox_print("Slavyana","Qu'est-ce que vous, un polyglotte?","Lato_Black12",6,6);
                        break;
                case 17:
                        _svin_textbox_print("Αλίκη","Ναί. Δύσκολο να φανταστείς;","Lato_Black12",5,5);
                        break;
                case 18:
                        _svin_textbox_print("Slavyana","Minusta se näyttää tarpeeksi.","Lato_Black12",6,6);
                        break; 
                default:               
                        _svin_textbox_print("","<END OF SCRIPT>","Lato_Black12",0,0);
                        break;
                }

                _svin_delay(1000);

                //while(1);
                }
        }


        while(1);
}
