#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include "hi_mw_type.h"
#include "hi_xmp.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */


static const char* sSpherical =
    "   <rdf:RDF xmlns:rdf='http://www.w3.org/1999/02/22-rdf-syntax-ns#'>"
    "     <rdf:Description rdf:about=''"
    "        xmlns:GSpherical='http://ns.google.com/videos/1.0/spherical/'>"
    "<GSpherical:Spherical>true</GSpherical:Spherical>"
    "<GSpherical:Stitched>true</GSpherical:Stitched>"
    "<GSpherical:ProjectionType>equirectangular</GSpherical:ProjectionType>"
    "<GSpherical:StitchingSoftware>xxxxxx</GSpherical:StitchingSoftware>"
    "</rdf:RDF>";


static const char* kSpherical = "http://ns.google.com/videos/1.0/spherical/";



static const HI_U8 uuid_xmp[] =
{
    0xff, 0xcc, 0x82, 0x63, 0xf8, 0x55, 0x4a, 0x93,
    0x88, 0x14, 0x58, 0x7a, 0x02, 0x52, 0x1f, 0xdd
};



#define FREE_ARRAY(i,p,count)  do{for(i=0;i<count;i++){if(NULL != p[i]){free(p[i]);p[i]=NULL;}}}while(0)

HI_S32  Muxer_XMP()
{
    HI_XMP_ATTR_S stAttr;
    HI_MW_PTR hXMPhandle = NULL;
    HI_CHAR cTmp;
    HI_S32 len = 0;
    HI_S32 i = 0;
    HI_CHAR* pBuf = NULL;
    stAttr.enXMPAttrType = HI_XMP_ATTR_WRITER;
    stAttr.stWriterAttr.s32NSCnt = 1;
    stAttr.stWriterAttr.pstNameSpaces[0] = (HI_XMP_NS_S*)malloc( sizeof(HI_XMP_NS_S));

    if (NULL == stAttr.stWriterAttr.pstNameSpaces)
    {
        return HI_FAILURE;
    }

    (HI_VOID)strncpy(stAttr.stWriterAttr.pstNameSpaces[0]->aszPrefix, "GSpherical", strlen( "GSpherical"));
    (HI_VOID)strncpy(stAttr.stWriterAttr.pstNameSpaces[0]->aszURI, "http://ns.google.com/videos/1.0/spherical/", strlen( "http://ns.google.com/videos/1.0/spherical/"));

    if ( HI_SUCCESS != HI_XMP_Create(&stAttr, &hXMPhandle))
    {
        FREE_ARRAY(i, stAttr.stWriterAttr.pstNameSpaces, stAttr.stWriterAttr.s32NSCnt);
        return HI_FAILURE;
    }

    if ( HI_SUCCESS != HI_XMP_AddProperty(hXMPhandle, stAttr.stWriterAttr.pstNameSpaces[0]->aszURI, "ProjectionType", "equirectangular"))
    {
        FREE_ARRAY(i, stAttr.stWriterAttr.pstNameSpaces, stAttr.stWriterAttr.s32NSCnt);
        return HI_FAILURE;
    }

    if ( HI_SUCCESS != HI_XMP_AddProperty(hXMPhandle, stAttr.stWriterAttr.pstNameSpaces[0]->aszURI, "StitchingSoftware", "hisilicon"))
    {
        FREE_ARRAY(i, stAttr.stWriterAttr.pstNameSpaces, stAttr.stWriterAttr.s32NSCnt);
        return HI_FAILURE;
    }

    if ( HI_SUCCESS != HI_XMP_AddProperty(hXMPhandle, stAttr.stWriterAttr.pstNameSpaces[0]->aszURI, "Spherical", "true"))
    {
        FREE_ARRAY(i, stAttr.stWriterAttr.pstNameSpaces, stAttr.stWriterAttr.s32NSCnt);
        return HI_FAILURE;
    }
    if ( HI_SUCCESS != HI_XMP_AddProperty(hXMPhandle, stAttr.stWriterAttr.pstNameSpaces[0]->aszURI, "Stitched", "true"))
    {
        FREE_ARRAY(i, stAttr.stWriterAttr.pstNameSpaces, stAttr.stWriterAttr.s32NSCnt);
        return HI_FAILURE;
    }

    if ( HI_SUCCESS != HI_XMP_ToString(hXMPhandle, &cTmp, &len))
    {
        FREE_ARRAY(i, stAttr.stWriterAttr.pstNameSpaces, stAttr.stWriterAttr.s32NSCnt);
        return HI_FAILURE;
    }

    pBuf = (HI_CHAR*)malloc( (HI_U32)len + 16);

    if (NULL == pBuf)
    {
        FREE_ARRAY(i, stAttr.stWriterAttr.pstNameSpaces, stAttr.stWriterAttr.s32NSCnt);
        return HI_FAILURE;
    }

    (HI_VOID)memset(pBuf, 0, len);
    memcpy(pBuf, uuid_xmp, 16);

    if ( HI_SUCCESS != HI_XMP_ToString(hXMPhandle, &pBuf[16], &len))
    {
        FREE_ARRAY(i, stAttr.stWriterAttr.pstNameSpaces, stAttr.stWriterAttr.s32NSCnt);
        free(pBuf);
        pBuf = NULL;
        return HI_FAILURE;
    }

    if ( HI_SUCCESS != HI_XMP_Destroy(hXMPhandle))
    {
        FREE_ARRAY(i, stAttr.stWriterAttr.pstNameSpaces, stAttr.stWriterAttr.s32NSCnt);
        free(pBuf);
        pBuf = NULL;
        return HI_FAILURE;
    }

    FREE_ARRAY(i, stAttr.stWriterAttr.pstNameSpaces, stAttr.stWriterAttr.s32NSCnt);

    printf("pBuf=[%s]\n", &pBuf[16]);
    free(pBuf);
    pBuf = NULL;
    return HI_SUCCESS;
}


HI_S32 Proc_KeyValue(const HI_CHAR* pszkey, const HI_CHAR* pszValue)
{
    printf("pszkey=[%s] pszValue=[%s]\n", pszkey, pszValue);
    return HI_SUCCESS;
}


HI_S32  DeMuxer_XMP()
{
    HI_XMP_ATTR_S stAttr;
    HI_MW_PTR hXMPhandle = NULL;
    stAttr.enXMPAttrType = HI_XMP_ATTR_READER;
    stAttr.stReaderAttr.s32InputBufLen = strlen(sSpherical);
    stAttr.stReaderAttr.pszInputBuf = (char*)malloc(stAttr.stReaderAttr.s32InputBufLen + 1);
    if (NULL == stAttr.stReaderAttr.pszInputBuf )
    {
        return HI_FAILURE;
    }
    strncpy(stAttr.stReaderAttr.pszInputBuf, sSpherical, strlen(sSpherical));
    stAttr.stReaderAttr.pszInputBuf[strlen(sSpherical)] = '\0';
    if ( HI_SUCCESS != HI_XMP_Create(&stAttr, &hXMPhandle))
    {
        printf("HI_XMP_Create fail \n");
        free(stAttr.stReaderAttr.pszInputBuf);
        stAttr.stReaderAttr.pszInputBuf= NULL;
        return HI_FAILURE;
    }

    if ( HI_SUCCESS != HI_XMP_ForeachKeyValue(hXMPhandle, kSpherical, Proc_KeyValue))
    {
        printf("HI_XMP_Create fail \n");
        free(stAttr.stReaderAttr.pszInputBuf);
        stAttr.stReaderAttr.pszInputBuf= NULL;
        return HI_FAILURE;
    }


    if ( HI_SUCCESS != HI_XMP_Destroy(hXMPhandle))
    {
        free(stAttr.stReaderAttr.pszInputBuf);
        stAttr.stReaderAttr.pszInputBuf= NULL;
        return HI_FAILURE;
    }
    free(stAttr.stReaderAttr.pszInputBuf);
    stAttr.stReaderAttr.pszInputBuf= NULL;
    return HI_SUCCESS;
}



#ifdef __HuaweiLite__
int app_main(int argc, char* argv[])
#else
int main(int argc, char* argv[])
#endif
{
    HI_S32 s32ret = 0;
    s32ret = HI_XMP_Init();

    if (HI_SUCCESS != s32ret)
    {
        printf("HI_XMP_Init fail s32ret=[%d]\n", s32ret);
    }

    s32ret = Muxer_XMP();

    if (HI_SUCCESS != s32ret)
    {
        printf("Muxer_XMP fail s32ret=[%d]\n", s32ret);
    }

    s32ret = DeMuxer_XMP();

    if (HI_SUCCESS != s32ret)
    {
        printf("DeMuxer_XMP fail s32ret=[%d]\n", s32ret);
    }

    s32ret = HI_XMP_DeInit();

    if (HI_SUCCESS != s32ret)
    {
        printf("HI_XMP_DeInit fail s32ret=[%d]\n", s32ret);
    }

    return HI_SUCCESS;
}












#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

