#include <errno.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include "hi_appcomm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define COMMAND_PARAM_CNT (5)
#define COLUMN_WIDTH (16)

#define FG_SIGN ("#")
#define BG_SIGN ("-")

static FT_Library s_ftLibrary;
static FT_Face s_ftFace;

static inline HI_VOID usage()
{
    printf("usage: ./ttf2bmp xxx.ttf w h output\n");
}

static HI_S32 FreeTypeFontInit(HI_CHAR* pszFontFile, HI_U32 FontSizeWidth ,HI_U32 FontSizeHeight)
{
    HI_S32 s32Ret = 0;
    /* initialize freetype library */
    s32Ret = FT_Init_FreeType(&s_ftLibrary );

    if (s32Ret)
    {
        MLOGE("FT_Init_FreeType failed\n");
        return -1;
    }

    /* open ttf face */
    s32Ret = FT_New_Face(s_ftLibrary, pszFontFile, 0, &s_ftFace);

    if (s32Ret)
    {
        MLOGE("FT_Init_FreeType failed\n");
        return -1;
    }

    /* set pixel size*/
    s32Ret = FT_Set_Pixel_Sizes(s_ftFace, 0, FontSizeHeight);

    if (s32Ret)
    {
        MLOGE("FT_Set_Pixel_Sizes (%d)failed\n", FontSizeHeight);
        return -1;
    }

    return 0;
}

static HI_VOID FreeTypeFontDeinit(HI_VOID)
{
    FT_Done_Face(s_ftFace);
    FT_Done_FreeType(s_ftLibrary);
}

static HI_VOID DrawMatrix(FT_GlyphSlot glyph, HI_S32 FontSizeWidth ,HI_S32 FontSizeHeight,HI_U8* pu8BmpData)
{
    HI_S32 i, j, k, counter;
    HI_U8 temp;
    printf("width=%d rows=%d\n", glyph->bitmap.width, glyph->bitmap.rows);

    for (j = 0; j < (FontSizeHeight * 26) / 32 - glyph->bitmap_top; j++)
    {
        for (i = 0; i < FontSizeWidth; i++)
        {
            printf(BG_SIGN);
            pu8BmpData[j*FontSizeWidth+i]=0;
        }
        printf("\n");
    }
    for (; j < glyph->bitmap.rows + (FontSizeHeight * 26) / 32 - glyph->bitmap_top; j++)
    {
        for (i = 0; i < glyph->bitmap_left; i++)
        {
            printf(BG_SIGN);
            pu8BmpData[j*FontSizeWidth+i]=0;
        }
        for (k = 0; k < glyph->bitmap.pitch; k++)
        {
            temp = glyph->bitmap.buffer[glyph->bitmap.pitch * (j + glyph->bitmap_top - (FontSizeHeight * 26) / 32) + k];
            for (counter = 0; counter < 8; counter++)
            {
                if (i >= FontSizeWidth)
                {
                    break;
                }
                if (temp & 0x80)
                {
                    printf(FG_SIGN);
                    pu8BmpData[j*FontSizeWidth+i]=1;
                }
                else
                {
                    printf(BG_SIGN);
                    pu8BmpData[j*FontSizeWidth+i]=0;
                }
                temp <<= 1;
                i++;
            }
        }
        for (; i < FontSizeWidth; i++)
        {
            printf(BG_SIGN);
            pu8BmpData[j*FontSizeWidth+i]=0;
        }
        printf("\n");
    }
    for (; j < FontSizeHeight; j++)
    {
        for (i = 0; i < FontSizeWidth; i++)
        {
            printf(BG_SIGN);
            pu8BmpData[j*FontSizeWidth+i]=0;
        }
        printf("\n");
    }
}

static HI_VOID Text2Bitmap(HI_S32 FontSizeWidth ,HI_S32 FontSizeHeight, HI_CHAR* pszText,HI_U8* pu8BmpData)
{
    HI_S32 i = 0;
    HI_S32 s32Textlen = strlen(pszText);

    for (i = 0; i < s32Textlen; i++)
    {
        FT_Load_Glyph(s_ftFace, FT_Get_Char_Index(s_ftFace, pszText[i]), FT_LOAD_DEFAULT);
        FT_Render_Glyph(s_ftFace->glyph, FT_RENDER_MODE_MONO);
        printf("Char[%d]:[%c](ASC:%d)",i,pszText[i],pszText[i]);
        DrawMatrix(s_ftFace->glyph, FontSizeWidth,FontSizeHeight, pu8BmpData+i*FontSizeWidth*FontSizeHeight);
    }

}

static HI_VOID bit_set(HI_U8 *pu8Data, HI_U8 u8Position, HI_U8 u8flag)
{
    HI_S32 a = 1<<(u8Position-1);
    if (u8flag)
    {
        *pu8Data |= a;
    }
    else
    {
        *pu8Data &= ~a;
    }
}

#if 0
static HI_VOID bit_print(HI_U8 u8Data)
{
    HI_S32 i;
	for (i = 7; i >= 0; i--)
	{
    	if(u8Data & (1 << i))
    	{
            printf("1");
        }
    	else
    	{
            printf("0");
        }
	}
}
#endif

static HI_VOID Bit2Hex(HI_U8* pu8BitData,HI_U32 u32BitSize,HI_U8* pu8HexData)
{
    HI_U32 i,j = 0;
    HI_U32 n=0;
    for(i=0;i<u32BitSize/8;i++)
    {
        for(j=8;j>0;j--)
        {
            bit_set(&pu8HexData[i],j,pu8BitData[n++]);
        }
    }
}

static HI_S32 SaveBin2Text(const HI_U8* pu8DataBuf,HI_U32 u32DataSize,const HI_CHAR* pszFilePath)
{
    FILE *fp = NULL;
    fp = fopen(pszFilePath, "w");
    if(!fp)
    {
        MLOGE("open [%s] failed %s\n", pszFilePath,strerror(errno));
        return HI_FAILURE;
    }
    HI_U32 i=0;
    HI_CHAR acOutStr[COLUMN_WIDTH*6+1]={0};
    HI_U8* pcBuffer = (HI_U8*)(pu8DataBuf);
    for(i=0;i<(u32DataSize);i++)
    {
        snprintf(&acOutStr[(i % COLUMN_WIDTH)*6],7,"0x%02x ,",pcBuffer[i]);
        if((i+1)%COLUMN_WIDTH==0)
        {
            fputs(acOutStr,fp);
            fwrite("\n", 1, 1, fp);
            memset(acOutStr,0x00,sizeof(acOutStr));
        }
    }
    if((i%COLUMN_WIDTH!=0)&&(i!=0))
    {
        fputs(acOutStr,fp);
    }
    fflush(fp);
    fclose(fp);
    return HI_SUCCESS;
}

HI_S32 main(HI_S32 argc, HI_CHAR** argv)
{
    HI_S32 s32Idx = 0;

    /* check command line param */
    if (COMMAND_PARAM_CNT > argc)
    {
        MLOGE("invalid input param number(%d)!\n", argc);

        for (s32Idx = 0; s32Idx < argc; ++s32Idx)
        {
            MLOGE("param[%d] %s\n", s32Idx, argv[s32Idx]);
        }

        usage();
        exit(1);
    }

    for (s32Idx = 0; s32Idx < argc; ++s32Idx)
    {
        MLOGD("param[%d] %s\n", s32Idx, argv[s32Idx]);
    }

    HI_CHAR* pszFontFile = argv[1];
    HI_S32 s32FontW = atoi(argv[2]);
    HI_S32 s32FontH = atoi(argv[3]);
    HI_CHAR* Output = argv[4];
    //HI_CHAR* Text = argv[5];
    HI_CHAR* Text = " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";

    /*step 1.Load ttf*/
    FreeTypeFontInit(pszFontFile, s32FontW, s32FontH);

    /*step 2.Convert text to bitmap*/
    HI_U8* pu8BmData = calloc(strlen(Text),s32FontW*s32FontH);
    Text2Bitmap(s32FontW,s32FontH,Text, pu8BmData);

    /*step 3.Convert bitmap to hex*/
    HI_U32 u32BitmapSize = strlen(Text)* s32FontW * s32FontH;
    HI_U8* pu8HexData = calloc(1,u32BitmapSize/8);
    Bit2Hex(pu8BmData,u32BitmapSize,pu8HexData);
    HI_APPCOMM_SAFE_FREE(pu8BmData);

    /*step 4.save hex to text*/
    SaveBin2Text(pu8HexData, u32BitmapSize/8, Output);
    HI_APPCOMM_SAFE_FREE(pu8HexData);

    MLOGI("ttf family name:%s\n", s_ftFace->family_name);
    MLOGI("ttf style name:%s\n", s_ftFace->style_name);
    FreeTypeFontDeinit();
    MLOGI("Char number:%ld\n", strlen(Text));
    MLOGI("Char list:%s\n", Text);
    MLOGI("Char size:%d X %d\n", s32FontW,s32FontH);
    MLOGI("Hex data size:%d(Bytes)\n", u32BitmapSize/8);
    return 0;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

