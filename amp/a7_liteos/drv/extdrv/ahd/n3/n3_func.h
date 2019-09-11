#ifndef _N3_H_
#define _N3_H_


int N3_Init( n3_init_param *pInitParam, int arg_int );
void N3_SetInputChannel( n3_init_param *pInitParam, int ch );

void N3_DetectFmt( n3_fmt_detect_param *pFmtDetectParam, int ch);
void N3_NoVideoCheck( n3_fmt_detect_param *pFmtDetectParam );
void N3_NovideoRead( n3_novideo_param *pNoVideoParam );

#endif
