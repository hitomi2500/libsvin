#include <QFileDialog>
#include <QProcess>
#include <QMessageBox>
#include <QtEndian>
#include "mainwindow.h"
#include "ui_mainwindow.h"

void MainWindow::on_pushButton_Process_Sprites_clicked()
{
    QList<QByteArray> Script_Lines;
    QList<QByteArray> Script_Lines_Eng;
    QList<QByteArray> Script_Actors;
    QList<QByteArray> Script_Actors_Aliases;
    QByteArray b;
    QByteArray ba;
    QByteArray ba_pal;

    //filling actors list first
    QFile script_file_actors("actors.txt");
    script_file_actors.open(QIODevice::ReadOnly);
    do {
        b = script_file_actors.readLine();
        if (b.simplified().size() > 0)
            if (b.simplified().contains("{"))
                if (true == b.contains('\"'))
                {
                    b = b.simplified();
                    b = b.mid(b.indexOf('\"')+1);
                    b = b.left(b.indexOf('\"'));
                    Script_Actors_Aliases.append(b);
                    QByteArray b2 = script_file_actors.readLine().simplified();
                    b2 = b2.mid(b2.indexOf(':')+1);
                    b2 = b2.mid(b2.indexOf('\"')+1);
                    b2 = b2.left(b2.indexOf('\"'));
                    Script_Actors.append(b2);
                }
    } while (b.length()>0);
    script_file_actors.close();

    //load every sprite recipe from script into recipes list
    QFile script_file(ui->lineEdit_script->text());
    QFile script_file_eng("eng.txt");
    script_file.open(QIODevice::ReadOnly);
    script_file_eng.open(QIODevice::ReadOnly);

    do {
        b = script_file.readLine();
        Script_Lines.append(b);
    } while (b.length()>0);
    do {
        b = script_file_eng.readLine();
        if (b.simplified().size() > 0)
            if (false == b.simplified().startsWith("#TODO"))
                if (b.simplified().contains('\"'))
                    Script_Lines_Eng.append(b);
    } while (b.length()>0);
    ui->textEdit->append(QString("Loaded %1 strings from script, %2 english lines").arg(Script_Lines.size()).arg(Script_Lines_Eng.size()));
    script_file.close();
    script_file_eng.close();

    /*script_file_eng.setFileName("eng_out.txt");
    script_file_eng.open(QIODevice::WriteOnly);
    for (int i = 0;i<Script_Lines_Eng.size();i++)
        script_file_eng.write(Script_Lines_Eng.at(i));
    script_file_eng.close();*/

    QList<QByteArray> Script_Sprite_Recipes;
    QList<int> Script_Sprite_Recipes_Position;
    for (int i=0;i<Script_Lines.size();i++)
    {
        if (Script_Lines.at(i).simplified().startsWith("show"))
        {
            QByteArray b2 = Script_Lines.at(i).simplified();
            b2 = b2.mid(5); //remove "show"
            if (b2.contains("with"))
                b2 = b2.left(b2.indexOf(" with")); //remove appearance effect for now
            if (b2.contains("at"))
                b2 = b2.left(b2.indexOf(" at")); //remove location
            Script_Sprite_Recipes.append(b2);
            Script_Sprite_Recipes_Position.append((i));
        }
    }
    ui->textEdit->append(QString("Script: detected %1 sprite recipes").arg(Script_Sprite_Recipes.size()));

    QList<QByteArray> Script_Labels;
    QList<int> Script_Labels_Position;
    QList<int> Script_Labels_Position_Processed_Rus;
    QList<int> Script_Labels_Position_Processed_Eng;
    for (int i=0;i<Script_Lines.size();i++)
    {
        if (Script_Lines.at(i).simplified().endsWith(":"))
        {
            if (Script_Lines.at(i).simplified().startsWith("label "))
            {
                QByteArray b2 = Script_Lines.at(i).simplified();
                b2 = b2.left(b2.length()-1); //remove ":"
                b2 = b2.mid(6); //remove "label"
                Script_Labels.append(b2);
                Script_Labels_Position.append((i));
                Script_Labels_Position_Processed_Rus.append(0);
                Script_Labels_Position_Processed_Eng.append(0);
            }
        }
    }
    ui->textEdit->append(QString("Script: detected %1 labels").arg(Script_Labels.size()));

    QList<QByteArray> Script_Variables;
    for (int i=0;i<Script_Lines.size();i++)
    {
        if (Script_Lines.at(i).simplified().startsWith("$ ")) //variable or func
        {
            if ( (false == Script_Lines.at(i).contains('(')) || (false == Script_Lines.at(i).contains(')')) ) //not func
            {
                QByteArray b2 = Script_Lines.at(i).simplified();
                b2 = b2.mid(2); //remove "$ "
                b2 = b2.left(b2.indexOf(" ")); //remove everything after variable name
                if (false == Script_Variables.contains(b2))
                    Script_Variables.append(b2);
            }
        }
    }
    ui->textEdit->append(QString("Script: detected %1 variables").arg(Script_Variables.size()));


    QList<int> Script_Menus_Starts;
    QList<int> Script_Menus_Lines;
    QList<Menu_Choise> Script_Menus_Choises;
    Menu_Choise _m;
    for (int i=0;i<Script_Lines.size();i++)
    {
        if (Script_Lines.at(i).simplified().startsWith("menu:")) //variable or func
        {
            QByteArray b2 = Script_Lines.at(i);
            int iTabSize = b2.indexOf('m');
            int iTabSizeCurrent = iTabSize-1;
            Script_Menus_Starts.append(i);
            Script_Menus_Lines.append(i);
            int iChoise = 0;
            while (iTabSizeCurrent != iTabSize)
            {
                //Script_Menus_Lines.append(i);//do not append every line, only starts and choises
                i++;
                if ( Script_Lines.at(i).simplified().startsWith("\"") && (Script_Lines.at(i).simplified().endsWith("\":") ) )
                {
                     _m.menu_id = Script_Menus_Starts.size();
                     _m.choise_id = iChoise;
                     iChoise++;
                     _m.value = Script_Lines.at(i).simplified();
                     Script_Menus_Choises.append(_m);
                     Script_Menus_Lines.append(i);
                }
                b2 = Script_Lines.at(i);
                if (b2.simplified().size() == 0)
                {
                    iTabSizeCurrent = 0;
                }
                else
                {
                    char c = b2.simplified().at(0);
                    iTabSizeCurrent = b2.indexOf(c);
                }
            }
        }
    }
    ui->textEdit->append(QString("Script: detected %1 menus, %2 choises, %3 lines total").arg(Script_Menus_Starts.size()).arg(Script_Menus_Choises.size()).arg(Script_Menus_Lines.size()));


    //now find a corresponding recipe for each entry from recipe list
    QFile recipes_file(ui->lineEdit_Recipes_List->text());
    recipes_file.open(QIODevice::ReadOnly);
    QByteArray Recipes_data = recipes_file.readAll().simplified();
    QList<QByteArray> ImagePaths;
    QList<Image_Link> ImageLinks;
    QList<int> ImageTilesUsage;

    //parse used recipies
    for (int i=0;i<Script_Sprite_Recipes.size();i++)
    {
        //find exact match for our recipe
        QByteArray search = Script_Sprite_Recipes.at(i);
        search.append(" =");
        QByteArray b = Recipes_data.mid(Recipes_data.indexOf(search));
        b = b.left(b.indexOf("image ")-1);//cut until next recipe
        //cutout night and evening conditions for now
        if (b.contains("ConditionSwitch"))
        {
            b = b.mid(b.indexOf("True"));
        }
        int iBigPalette = 1; //only 1st image goes with big palette
        int iShiftedPalette = 0;
        while (b.contains("\""))
        {
            b = b.mid(b.indexOf("\"")+1);
            QByteArray b2 = b.left(b.indexOf("\""));
            //checking if the image alreaddy exists in the list
            if (ImagePaths.contains(b2))
            {
                //do not add anoter copy, just link it
            }
            else
            {
                //new file, append it
                ImagePaths.append(b2);
            }
            ImageLinks.append({i,ImagePaths.indexOf(b2),iBigPalette,iShiftedPalette});
            if (iBigPalette == 0)
                iShiftedPalette = 1;
            iBigPalette = 0;
            b = b.mid(b.indexOf("\"")+1);
        }
    }
    ui->textEdit->append(QString("Script: found %1 image files").arg(ImagePaths.size()));

    //using imagemagick for image transforms
    QProcess process;
    QStringList proc_args;
    QImage img;
    process.setProgram("C:\\Program Files\\ImageMagick-7.0.11-Q16-HDRI\\magick");

    //next step - processing every image in the list
    int iTotalTiles = 0;
    QByteArray path = ui->lineEdit_Recipes_List->text().toLatin1();
    while (false == path.endsWith("/"))
        path.chop(1);
    for (int iImageNumber=0; iImageNumber < ImagePaths.size(); iImageNumber++)
    {
        QByteArray b = ImagePaths.at(iImageNumber);
        b.prepend(path);
        QByteArray b2 = b.left(b.indexOf("."));
        QByteArray b3 = b2;
        b2.append("_processed.png");
        b3.append(".tim");
        //deciding if image should be 255 or 127 colors
        //searching image in the recipes
        int iBigPalette = 0;
        int iShiftedPalette = 0;
        for (int i=0;i<ImageLinks.size();i++)
        {
            if (ImageLinks.at(i).image == iImageNumber)
            {
                iBigPalette = ImageLinks.at(i).big_palette;
                iShiftedPalette =  ImageLinks.at(i).shifted_palette;
            }
        }
        //check if the file was already processed and a processed copy exist
        if (QFile::exists(b2))
        {
            //exists, do nothing
        }
        else
        {
            //resize sprite for height of 448
            proc_args.clear();
            proc_args.append(b);
            proc_args.append("-resize");
            proc_args.append("8x448^");
            proc_args.append("tmp1.png");
            process.setArguments(proc_args);
            process.open();
            process.waitForFinished();


            proc_args.clear();
            proc_args.append("tmp1.png");
            proc_args.append("-channel");
            proc_args.append("alpha");
            proc_args.append("-threshold");
            proc_args.append("99%");
            proc_args.append("tmp2.png");
            process.setArguments(proc_args);
            process.open();
            process.waitForFinished();

            //generate palette, VDP2 NBG palette is limited to 255 colours
            proc_args.clear();
            proc_args.append("tmp2.png");
            proc_args.append("-colors");
            if (iBigPalette == 0)
                proc_args.append("127");
            else
                proc_args.append("255");
            proc_args.append(b2);
            process.setArguments(proc_args);
            process.open();
            process.waitForFinished();
        }

        img.load(b2);

        if (img.size().width() <= 0 )
            return;

        /*if (img.size().width()>44*8)
        {
            //let's crop image to 232x448 for now. how should we deal with overlaps, another layer?
            //img = img.copy((img.size().width()-232)/2,0,232,448);
        }
        else*/
        {
            //let's crop image to tile size
            if ( (img.size().width()%8 != 0) || (img.size().height()%8 != 0) )
                img = img.copy(0,0,(img.size().width()/8)*8,(img.size().height()/8)*8);
        }


        //let's detect a transparent color
        int transp_color = img.pixelIndex(0,0);

        //let's calculate tile usage for image (no tile sharing yet)
        //writing tile usage into file as well
        QFile _tiled_image_file(b3);
        _tiled_image_file.open(QIODevice::WriteOnly|QIODevice::Truncate);
        ba.clear();
        int iSizeTiledX = img.size().width()/8;
        int iSizeTiledY = img.size().height()/8;
        _tiled_image_file.write(QByteArray(1,iSizeTiledX));
        _tiled_image_file.write(QByteArray(1,iSizeTiledY));
        int iActiveTiles = 0;
        for (int TileY = 0; TileY < iSizeTiledY; TileY++)
        {
            for (int TileX = 0; TileX < iSizeTiledX; TileX++)
            {
                //is the tile empty?
                bool bEmpty = true;
                for (int x=0;x<8;x++)
                {
                    for (int y=0;y<8;y++)
                    {
                        if (img.pixelIndex(TileX*8+x,TileY*8+y) != transp_color)
                                bEmpty = false;
                    }
                }
                //if the tile is non-empty, count its usage
                if (false == bEmpty)
                {
                    iActiveTiles++;
                    _tiled_image_file.write(QByteArray(1,1));
                    //ba.append(QByteArray(1,1));
                }
                else
                {
                    _tiled_image_file.write(QByteArray(1,0));
                    //ba.append(QByteArray(1,0));
                }
            }
        }
        //pass 2 - writing tile data
        for (int TileY = 0; TileY < iSizeTiledY; TileY++)
        {
            for (int TileX = 0; TileX < iSizeTiledX; TileX++)
            {
                //is the tile empty?
                bool bEmpty = true;
                for (int x=0;x<8;x++)
                {
                    for (int y=0;y<8;y++)
                    {
                        if (img.pixelIndex(TileX*8+x,TileY*8+y) != transp_color)
                                bEmpty = false;
                    }
                }
                //if the tile is non-empty, write it
                if (false == bEmpty)
                {
                    for (int y=0;y<8;y++)
                    {
                        for (int x=0;x<8;x++)
                        {
                            //_tiled_image_file.write(QByteArray(1,(char)img.pixelIndex(TileX*8+x,TileY*8+y)));
                            ba.append(QByteArray(1,(char)img.pixelIndex(TileX*8+x,TileY*8+y)));
                        }
                    }

                }
            }
        }


        //now write palette
        ba_pal.clear();
        //for mid layer 2 or right-left layer 1 colours are shifted
        if (iShiftedPalette > 0)
        {

            for (int j=0;j<128*3;j++)
            {
                ba_pal.append('\0');
            }
        }
        for (int j=0;j<img.colorTable().size();j++)
        {
            ba_pal.append(QColor(img.colorTable().at(j)).red());
            ba_pal.append(QColor(img.colorTable().at(j)).green());
            ba_pal.append(QColor(img.colorTable().at(j)).blue());
        }
        while (ba_pal.size()<256*3)
            ba_pal.append('\0');

        //for VDP2 sprites 0x00 (transparent) can't be used
        //imagemagick generates palettes from 0x00 to 0xFE, 0xFF being transparent
        //so we only have to move color 0x00 to color 0xFF
        /*for (int i=0;i<ba.size();i++)
            if (ba.at(i) == 0)
                ba[i]=-1;*/
        //hacking palette, moving color 0x00 to color 0xFF
        /*ba_pal[255*3] = ba_pal.at(0*3);
        ba_pal[255*3+1] = ba_pal.at(0*3+1);
        ba_pal[255*3+2] = ba_pal.at(0*3+2);*/

        //semi-transparent colors look like shit without real transparency, so let's make them 100% transparent
        /*QVector<QRgb> cltbl = img.colorTable();
        for (int i=0;i<ba.size();i++)
        {
            if (QColor(img.colorTable().at((uint8_t)ba.at(i))).alpha() < 255)
                ba[i]=0;
        }*/

        if (iShiftedPalette > 0)
        {
            //for layer 2 colours are shifted
            for (int i=0;i<ba.size();i++)
            {
                if (ba[i]!='\0')
                    ba[i] =(char) (((uint8_t)ba[i])+128);
            }
        }

        /*QColor _col;
        for (int i=0;i<256;i++)
        {
            _col.fromRgb(img.color(i));
            _tiled_image_file.write(QByteArray(1,_col.red()));
            _tiled_image_file.write(QByteArray(1,_col.green()));
            _tiled_image_file.write(QByteArray(1,_col.blue()));
        }*/

        _tiled_image_file.write(ba);
        _tiled_image_file.write(ba_pal);

        ImageTilesUsage.append(iActiveTiles);
        iTotalTiles += iActiveTiles;
        _tiled_image_file.close();
    }
    ui->textEdit->append(QString("Script: detected %1 total tiles").arg(iTotalTiles));


    //now packing every image file into tiled image
    //preprocessing should've compressed and paletted the image
    //we need to save this data:
    // 1) tile usage map, for full image size (46x56 for 373x448 sprites, 88x56 for full 704x448 sprites
    // 2) tiles data with unused tiles skipped
    // 3) palette, 768-byte fixed format
    for (int iImage=0;iImage<ImagePaths.size(); iImage++)
    {
        QString _tiled_filename = ImagePaths[iImage];
        _tiled_filename.chop(3);
        _tiled_filename.append("tim");

    }

    //creating simplified script file
    QFile script_outfile_rus("SCRIPT_RUS.TXT");
    QFile script_outfile_eng("SCRIPT_ENG.TXT");
    int iCurrentRecipe;
    int iActiveLines = 0;
    script_outfile_rus.open(QIODevice::WriteOnly|QIODevice::Truncate);
    script_outfile_eng.open(QIODevice::WriteOnly|QIODevice::Truncate);
    for (int i=0;i<Script_Lines.size();i++)
    {
        //if there is a label aimed at this position, set its processed location
        for (int j=0;j<Script_Labels.size();j++)
        {
            if (Script_Labels_Position.at(j) == i)
            {
                Script_Labels_Position_Processed_Rus[j] = script_outfile_rus.size();
                Script_Labels_Position_Processed_Eng[j] = script_outfile_eng.size();
            }
        }
        //fetch first word
        QByteArray first_word = Script_Lines.at(i).simplified().left(Script_Lines.at(i).simplified().indexOf(" "));
        if (Script_Lines.at(i).simplified().size()==0)
        {
            //skipping empty input
        }
        else if (Script_Lines.at(i).simplified().startsWith("show"))
        {
            //this is a recipe, searching the exact recipe number
            iCurrentRecipe = -1;
            int iPosition = 1;
            if (Script_Lines.at(i).simplified().contains("at left"))
                iPosition = 0;
            else if (Script_Lines.at(i).simplified().contains("at right"))
                iPosition = 2;
            for (int rec =0; rec < Script_Sprite_Recipes.size(); rec++)
            {
                if (Script_Lines.at(i).simplified().contains(Script_Sprite_Recipes.at(rec)))
                {
                    if (Script_Lines.at(i).simplified().contains(" far"))
                    {
                        if (Script_Sprite_Recipes.at(rec).contains(" far"))
                            iCurrentRecipe = rec; //for far only far should match
                    }
                    else
                        iCurrentRecipe = rec; //non-far, using as-is
                }
            }
            if (iCurrentRecipe != -1)
            {
                //valid recipe? write its contents into output file, searching thru entire list
                //but first let's issue "clear" instruction
                bool bFound = false;
                for (int f=0;((f<ImageLinks.size())&&(bFound==false));f++)
                {
                    //we found a match. add image.
                    if (ImageLinks.at(f).recipe == iCurrentRecipe)
                    {
                        bFound = true;
                        script_outfile_eng.write(QString("CLEAR POSITION %1").arg(iPosition).toLatin1());
                        script_outfile_eng.write("\r");
                        script_outfile_rus.write(QString("CLEAR POSITION %1").arg(iPosition).toLatin1());
                        script_outfile_rus.write("\r");
                    }
                }

                //now all non-zero layer because... you know, they should go first
                for (int f=0;f<ImageLinks.size();f++)
                {
                    //we found a match. add image.
                    if ((ImageLinks.at(f).recipe == iCurrentRecipe)&&(ImageLinks.at(f).big_palette == 0))
                    {
                        QString _tiled_filename = QString(ImagePaths[ImageLinks.at(f).image]);
                        _tiled_filename.chop(3);
                        _tiled_filename.append("tim");
                        int iLayer = 1;
                        int iPalette = 1;
                        if (ImageLinks.at(f).shifted_palette == 0)
                        {
                            iPalette++;
                        }
                        //if (iPosition == 1)
                        //    iLayer++;
                        script_outfile_eng.write(QString("SPRITE LAYER %1 POSITION %2 PALETTE %3 FILE %4").arg(iLayer).arg(iPosition).arg(iPalette).arg(_tiled_filename).toLatin1());
                        script_outfile_eng.write("\r");
                        script_outfile_rus.write(QString("SPRITE LAYER %1 POSITION %2 PALETTE %3 FILE %4").arg(iLayer).arg(iPosition).arg(iPalette).arg(_tiled_filename).toLatin1());
                        script_outfile_rus.write("\r");
                    }
                }

                //now all zero layer
                for (int f=0;f<ImageLinks.size();f++)
                {
                    //we found a match. add image.
                    if ((ImageLinks.at(f).recipe == iCurrentRecipe)&&(ImageLinks.at(f).big_palette == 1))
                    {
                        QString _tiled_filename = QString(ImagePaths[ImageLinks.at(f).image]);
                        _tiled_filename.chop(3);
                        _tiled_filename.append("tim");
                        int iLayer = 0;
                        //if (iPosition == 1)
                        //    iLayer++;
                        script_outfile_eng.write(QString("SPRITE LAYER %1 POSITION %2 PALETTE 0 FILE %3").arg(iLayer).arg(iPosition).arg(_tiled_filename).toLatin1());
                        script_outfile_eng.write("\r");
                        script_outfile_rus.write(QString("SPRITE LAYER %1 POSITION %2 PALETTE 0 FILE %3").arg(iLayer).arg(iPosition).arg(_tiled_filename).toLatin1());
                        script_outfile_rus.write("\r");
                    }
                }

                bFound = true;
                script_outfile_eng.write(QString("ENABLE POSITION %1").arg(iPosition).toLatin1());
                script_outfile_eng.write("\r");
                script_outfile_rus.write(QString("ENABLE POSITION %1").arg(iPosition).toLatin1());
                script_outfile_rus.write("\r");
            }
        }
        else if (Script_Lines.at(i).simplified().startsWith("scene bg"))
        {
            QByteArray _tmp = Script_Lines.at(i).simplified();
            _tmp = _tmp.mid(_tmp.indexOf(' ')+1);
            _tmp = _tmp.mid(_tmp.indexOf(' ')+1);
            script_outfile_eng.write(_tmp.prepend("BG images/bg/").append(".bg"));
            script_outfile_eng.write("\r");
            script_outfile_rus.write(_tmp.prepend("BG images/bg/").append(".bg"));
            script_outfile_rus.write("\r");
        }
        else if (Script_Lines.at(i).simplified().startsWith("scene cg"))
        {
            //CG is same as BG, but no sprites
            script_outfile_eng.write(QString("CLEAR POSITION 0 \r").toLatin1());
            script_outfile_eng.write(QString("CLEAR POSITION 1 \r").toLatin1());
            script_outfile_eng.write(QString("CLEAR POSITION 2 \r").toLatin1());
            script_outfile_rus.write(QString("CLEAR POSITION 0 \r").toLatin1());
            script_outfile_rus.write(QString("CLEAR POSITION 1 \r").toLatin1());
            script_outfile_rus.write(QString("CLEAR POSITION 2 \r").toLatin1());
            QByteArray _tmp = Script_Lines.at(i).simplified();
            _tmp = _tmp.mid(_tmp.indexOf(' ')+1);
            _tmp = _tmp.mid(_tmp.indexOf(' ')+1);
            script_outfile_eng.write(_tmp.prepend("BG images/cg/").append(".bg"));
            script_outfile_eng.write("\r");
            script_outfile_rus.write(_tmp.prepend("BG images/cg/").append(".bg"));
            script_outfile_rus.write("\r");
        }
        else if (Script_Lines.at(i).simplified().startsWith("jump "))
        {
            //search label in da list
            bool bFound = 0;
            for (int j=0;j<Script_Labels.size();j++)
            {
                if (0 == Script_Labels.at(j).compare(Script_Lines.at(i).simplified().mid(5)))
                {
                    bFound = true;
                    script_outfile_eng.write(QString("JUMP %1 \r").arg(j).toLatin1());
                    script_outfile_rus.write(QString("JUMP %1 \r").arg(j).toLatin1());
                }
            }
            if (false == bFound)
                ui->textEdit->append(QString("Script ERROR: unknown jump %1 at line %2").arg(QString::fromLatin1(Script_Lines.at(i).simplified())).arg(i));
        }
        else if (Script_Lines.at(i).simplified().startsWith("$ "))
        {
            //search variable in da list
            if ( (false == Script_Lines.at(i).contains('(')) || (false == Script_Lines.at(i).contains(')')) ) //not func
            {
                bool bFound = false;
                for (int j=0;j<Script_Variables.size();j++)
                {
                    if (Script_Lines.at(i).simplified().mid(2).startsWith(Script_Variables.at(j)))
                    {
                        bFound = true;
                        script_outfile_eng.write(QString("SET VAR%1 = ").arg(j).toLatin1());
                        script_outfile_rus.write(QString("SET VAR%1 = ").arg(j).toLatin1());
                        //parse equation
                        QByteArray b2 = Script_Lines.at(i).simplified();
                        b2 = b2.mid(b2.indexOf("=")+2);
                        if (b2.contains("+"))
                        {
                            //simple summ
                            QByteArray b3 = b2.left(b2.indexOf("+")-1).simplified();
                            if (Script_Variables.contains(b3))
                            {
                                script_outfile_eng.write(QString("VAR%1 + ").arg(Script_Variables.indexOf(b3)).toLatin1());
                                script_outfile_rus.write(QString("VAR%1 + ").arg(Script_Variables.indexOf(b3)).toLatin1());
                            }
                            else
                            {
                                script_outfile_eng.write(QString("%1 + ").arg(b3.toInt()).toLatin1());
                                script_outfile_rus.write(QString("%1 + ").arg(b3.toInt()).toLatin1());
                            }
                            b3 = b2.mid(b2.indexOf("+")+1).simplified();
                            if (Script_Variables.contains(b3))
                            {
                                script_outfile_eng.write(QString("VAR%1").arg(Script_Variables.indexOf(b3)).toLatin1());
                                script_outfile_rus.write(QString("VAR%1").arg(Script_Variables.indexOf(b3)).toLatin1());
                            }
                            else
                            {
                                script_outfile_eng.write(QString("%1").arg(b3.toInt()).toLatin1());
                                script_outfile_rus.write(QString("%1").arg(b3.toInt()).toLatin1());
                            }
                        }
                        else if (b2.contains("-"))
                        {
                            //simple difference
                            QByteArray b3 = b2.left(b2.indexOf("-")-1).simplified();
                            if (Script_Variables.contains(b3))
                            {
                                script_outfile_eng.write(QString("VAR%1 - ").arg(Script_Variables.indexOf(b3)).toLatin1());
                                script_outfile_rus.write(QString("VAR%1 - ").arg(Script_Variables.indexOf(b3)).toLatin1());
                            }
                            else
                            {
                                script_outfile_eng.write(QString("%1 - ").arg(b3.toInt()).toLatin1());
                                script_outfile_rus.write(QString("%1 - ").arg(b3.toInt()).toLatin1());
                            }
                            b3 = b2.mid(b2.indexOf("-")+1).simplified();
                            if (Script_Variables.contains(b3))
                            {
                                script_outfile_eng.write(QString("VAR%1").arg(Script_Variables.indexOf(b3)).toLatin1());
                                script_outfile_rus.write(QString("VAR%1").arg(Script_Variables.indexOf(b3)).toLatin1());
                            }
                            else
                            {
                                script_outfile_eng.write(QString("%1").arg(b3.toInt()).toLatin1());
                                script_outfile_rus.write(QString("%1").arg(b3.toInt()).toLatin1());
                            }
                        }
                        else
                        {
                            //constant or variable
                            QByteArray b3 = b2.simplified();
                            if (Script_Variables.contains(b3))
                            {
                                script_outfile_eng.write(QString("VAR%1").arg(Script_Variables.indexOf(b3)).toLatin1());
                                script_outfile_rus.write(QString("VAR%1").arg(Script_Variables.indexOf(b3)).toLatin1());
                            }
                            else
                            {
                                script_outfile_eng.write(QString("%1").arg(b3.toInt()).toLatin1());
                                script_outfile_rus.write(QString("%1").arg(b3.toInt()).toLatin1());
                            }
                        }
                        script_outfile_eng.write("\r");
                        script_outfile_rus.write("\r");
                    }

                }
                if (false == bFound)
                    ui->textEdit->append(QString("Script ERROR: unknown variable %1 at line %2").arg(QString::fromLatin1(Script_Lines.at(i).simplified())).arg(i));
            }
        }
        else if (Script_Menus_Lines.contains(i))
        {
            //menu start moving, going to the end, parse later
            //TODO: do somethin with menus

        }
        else if (Script_Lines.at(i).simplified().startsWith("\""))
        {
            //narrator text
            QByteArray b_rus = Script_Lines.at(i).simplified().prepend("TEXT ACTOR=255 ").append("\r");
            script_outfile_rus.write(b_rus);
            QByteArray b_eng =Script_Lines_Eng[iActiveLines].simplified().prepend("TEXT ACTOR=255 ").append("\r");
            script_outfile_eng.write(b_eng);
            script_outfile_eng.write("\r");
            if (false == Script_Lines_Eng[iActiveLines].startsWith("\""))
                ui->textEdit->append(QString("Script: ERROR, active (narrator) line mismatch, script %1 english %2").arg(i).arg(iActiveLines));
            iActiveLines++;
            while (Script_Lines_Eng.size() <= iActiveLines)
                Script_Lines_Eng.append("ERROR: DUMMY LINE\r");
        }
        else if (Script_Lines.at(i).simplified().startsWith("th \""))
        {
            //protagonist thoughts text
            QByteArray b_rus = Script_Lines.at(i).simplified().prepend("TEXT ACTOR=254 ").append("\r");
            script_outfile_rus.write(b_rus);
            QByteArray b_eng =Script_Lines_Eng[iActiveLines].simplified().prepend("TEXT ACTOR=254 ").append("\r");
            script_outfile_eng.write(b_eng);
            if (false == Script_Lines_Eng[iActiveLines].startsWith("th"))
                ui->textEdit->append(QString("Script: ERROR, active (th) line mismatch, script %1 english %2").arg(i).arg(iActiveLines));
            iActiveLines++;
            while (Script_Lines_Eng.size() <= iActiveLines)
                Script_Lines_Eng.append("ERROR: DUMMY LINE\r");
        }
        else if (Script_Actors_Aliases.contains(first_word))
        {
            int iActorID = Script_Actors_Aliases.indexOf(first_word);
            QByteArray b_rus = Script_Lines.at(i).simplified();
            b_rus = b_rus.mid(b_rus.indexOf(' ')+1).prepend(QString("TEXT ACTOR=%1 ").arg(iActorID).toLatin1()).append("\r");
            script_outfile_rus.write(QString("TEXT ACTOR=%1 ").arg(iActorID).toLatin1());
            QByteArray b_eng = Script_Lines_Eng[iActiveLines].simplified();
            b_eng = b_eng.mid(b_eng.indexOf(' ')+1).prepend(QString("TEXT ACTOR=%1 ").arg(iActorID).toLatin1()).append("\r");
            script_outfile_eng.write(b_eng);
            if (false == Script_Lines_Eng[iActiveLines].startsWith(first_word))
                ui->textEdit->append(QString("Script: ERROR, active (%3) line mismatch, script %1 english %2").arg(i).arg(iActiveLines).arg(QString::fromLatin1(first_word)));
            iActiveLines++;
            while (Script_Lines_Eng.size() <= iActiveLines)
                Script_Lines_Eng.append("ERROR: DUMMY LINE\r");
        }
        else
        {
            //write anything unparsed as commentary for now
            script_outfile_eng.write(Script_Lines.at(i).simplified().prepend("REM "));
            script_outfile_eng.write("\r");
            script_outfile_rus.write(Script_Lines.at(i).simplified().prepend("REM "));
            script_outfile_rus.write("\r");
        }

    }
    script_outfile_eng.write("END\r");
    script_outfile_eng.close();
    script_outfile_rus.write("END\r");
    script_outfile_rus.close();
    ui->textEdit->append(QString("Script: processed %1 active lines").arg(iActiveLines));

    QFile script_outfile_labels("SCRIPT_RUS.LBL");
    script_outfile_labels.open(QIODevice::WriteOnly|QIODevice::Truncate);
    for (int i=0;i<Script_Labels_Position_Processed_Rus.size();i++)
    {
        int tmp[2];
        tmp[0] = Script_Labels_Position_Processed_Rus.at(i);
        uint8_t * c = (uint8_t *)tmp;
        script_outfile_labels.write(QByteArray(1,c[3]));
        script_outfile_labels.write(QByteArray(1,c[2]));
        script_outfile_labels.write(QByteArray(1,c[1]));
        script_outfile_labels.write(QByteArray(1,c[0]));
    }
    script_outfile_labels.close();
    script_outfile_labels.setFileName("SCRIPT_ENG.LBL");
    script_outfile_labels.open(QIODevice::WriteOnly|QIODevice::Truncate);
    for (int i=0;i<Script_Labels_Position_Processed_Eng.size();i++)
    {
        int tmp[2];
        tmp[0] = Script_Labels_Position_Processed_Eng.at(i);
        uint8_t * c = (uint8_t *)tmp;
        script_outfile_labels.write(QByteArray(1,c[3]));
        script_outfile_labels.write(QByteArray(1,c[2]));
        script_outfile_labels.write(QByteArray(1,c[1]));
        script_outfile_labels.write(QByteArray(1,c[0]));
    }
    script_outfile_labels.close();
}

/*void MainWindow::Process_Sprite(QString filename)
{
    //preprocessing should've compressed and paletted the image
    //we need to save this data:
    // 1) tile usage map, for full image size (46x56 for 373x448 sprites, 88x56 for full 704x448 sprites
    // 2) tiles data with unused tiles skipped
    // 3) palette, 768-byte fixed format

}*/
