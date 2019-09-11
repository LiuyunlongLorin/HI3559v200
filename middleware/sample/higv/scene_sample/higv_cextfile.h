#ifndef __HIGV_CEXTFILE_H__
#define __HIGV_CEXTFILE_H__

#include "hi_type.h"

#include "hi_gv.h"

#ifdef __cplusplus
#if __cplusplus
    extern "C"{
#endif
#endif /*  __cplusplus  */

#define trs_inout_top_out (1)
#define trs_inout_top_in (2)
#define trs_inout_left_out (3)
#define trs_inout_left_in (4)
#define trs_inout_right_out (5)
#define trs_inout_right_in (6)
#define trs_inout_bottom_out (7)
#define trs_inout_bottom_in (8)
#define trs_win_in (9)
#define trs_win_out (10)
#define trs_anim_test (11)
#define animalpha_change (12)
#define animroll_btn (13)
#define animroll_listbox_left (14)
#define animroll_listbox_up (15)
#define animroll_listbox_up_down (16)
#define anim_test_roll_up_down (17)
#define animroll_listbox_down (18)
#define animroll_listbox_right (19)
#define trs_tab_122 (20)
#define trs_tab_123 (21)
#define trs_tab_223 (22)
#define trs_tab_221 (23)
#define trs_tab_321 (24)
#define trs_tab_322 (25)
#define trs_tab_left (26)
#define trs_tab_right (27)
#define trs_test_move (28)
#define animroll_1s (29)
#define translate_outside_up_to_down (30)
#define translate_outside_left_to_right (31)
#define translate_left_to_right (32)
#define ANIM_IN_OUT (33)
#define ANIM_IN_OUT_HIDE (35)
#define ANIM_GRP_TOP (37)
#define ANIM_BTN_TOP_1 (38)
#define ANIM_BTN_TOP_2 (42)
#define ANIM_GRP_RIGHT (43)
#define ANIM_BTN_RIGHT_1 (44)
#define ANIM_BTN_RIGHT_2 (45)
#define ANIM_BTN_RIGHT_3 (46)
#define ANIM_GRP_BOTTON (47)
#define ANIM_BTN_BOTTON_1 (48)
#define ANIM_BTN_BOTTON_2 (49)
#define ANIM_BTN_BOTTON_3 (50)
#define ANIM_BTN_BOTTON_4 (51)
#define ANIM_GRP_LEFT (52)
#define ANIM_BTN_LEFT_1 (53)
#define ANIM_BTN_LEFT_2 (54)
#define ANIM_TAB_INOUT (55)
#define ANIM_TAB_INOUT_HIDE (56)
#define ANIM_TAB_INOUT_LABEL1 (57)
#define ANIM_TAB_INOUT_TAB1_BTN (59)
#define ANIM_TAB_INOUT_TAB1_LBL (60)
#define ANIM_TAB_INOUT_TAB2_BTN (62)
#define ANIM_TAB_INOUT_TAB3_BTN (63)
#define ANIM_TAB_INOUT_TAB1 (64)
#define ANIM_TAB_INOUT_TAB1_BTN1 (65)
#define ANIM_TAB_INOUT_TAB2 (67)
#define ANIM_TAB_INOUT_TAB2_BTN1 (68)
#define ANIM_TAB_INOUT_TAB2_BTN2 (69)
#define ANIM_TAB_INOUT_TAB3 (70)
#define ANIM_TAB_INOUT_TAB3_BTN1 (71)
#define ANIM_TAB_INOUT_TAB3_BTN2 (72)
#define ANIM_TAB_INOUT_TAB3_BTN3 (73)
#define ANIM_TEST (74)
#define ANIM_TEST_HIDE (75)
#define ANIM_TEST_LABEL1 (76)
#define ANIM_TEST_TEST (77)
#define SHOW_IMAGE (78)
#define ANIM_WIN_MOVE (79)
#define ANIM_WIN_MOVE_IMAGE1 (80)
#define ANIM_WIN_MOVE_LABEL1 (81)
#define ANIM_WIN_MOVE_GRP2 (82)
#define ANIM_WIN_MOVE_BTN_2 (83)
#define ANIM_WIN_MOVE_BTN_3 (85)
#define ANIM_WIN_MOVE_GRP1 (86)
#define ANIM_WIN_MOVE_BTN_1 (87)
#define MSG_SCENE_WIN (88)
#define MSG_SCENE_WIN_IMAGE1 (89)
#define MSG_SCENE_WIN_LABEL1 (90)
#define MSG_SCENE_WIN_LABEL2 (91)
#define MSG_SCENE_BUTTON1 (93)
#define MSG_SCENE_BUTTON2 (95)
#define MSG_SCENE_BUTTON3 (96)
#define MSG_SCENE_BUTTON4 (97)
#define BUTTON_SCENE_WIN (98)
#define BUTTON_WIN_IMAGE1 (99)
#define BUTTON_WIN_LABEL1 (100)
#define BUTTON_GROUPBOX_TOP (101)
#define BUTTON_GROUPBOX_BTM (102)
#define BUTTON_WIN_BUTTON1 (103)
#define WIN_CLOCK (105)
#define WIN_CLOCK_HIDE (106)
#define WIN_CLOCK_LABEL1 (107)
#define CLOCK_WIN_CLOCK_TIME (108)
#define ADM_SET_STRINGID (109)
#define ADM_SET_STRINGID2 (110)
#define ADM_IMAGE (111)
#define FLING_SCENE_WIN (112)
#define FLING_WIN_IMAGE1 (113)
#define FLING_WIN_LABEL1 (114)
#define FLING_GROUPBOX_TOP (115)
#define FLING_GROUPBOX_BTM (116)
#define FOCUS_CHANGE_WIN (117)
#define FOCUS_CHANGE_WIN_IMAGE1 (118)
#define FOCUS_CHANGE_WIN_LABEL1 (119)
#define FOCUS_CHANGE_WIN_BUTTON1 (120)
#define FOCUS_CHANGE_WIN_BUTTON2 (123)
#define FOCUS_CHANGE_WIN2 (124)
#define FOCUS_CHANGE_WIN2_BUTTON1 (125)
#define simhei_font_text_10 (126)
#define simhei_font_text_20 (41)
#define simhei_font_text_24 (127)
#define simhei_font_text_28 (128)
#define simhei_font_text_32 (129)
#define simhei_font_text_36 (130)
#define simhei_font_text_40 (131)
#define notosans_font_text_20 (132)
#define msuighur_font_text_30 (133)
#define HISTOGRAM_SCENE_WIN (134)
#define HISTOGRAM_SCENE_WIN_IMAGE1 (135)
#define HISTOGRAM_SCENE_WIN_LABEL1 (136)
#define HISTOGRAM_SCENE_WIN_GROUPBOX (137)
#define IMAGEEX_SCENE_WIN (139)
#define IMAGEEX_SCENE_WIN_IMAGE1 (140)
#define IMAGE_WIN_LABEL1 (141)
#define BTN_TEST (142)
#define SHOW_IMAGE_GIF (143)
#define MAIN_WIN (144)
#define MAIN_WIN_LABEL1 (145)
#define MAIN_WIN_LISTBOX1_ITEM_SCEOLLBAR (146)
#define MAIN_WIN_LISTBOX1 (149)
#define MAIN_WIN_LISTBOX1_LISTCOL1 (153)
#define MULTI_LANGUAGE_SCENE_WIN (154)
#define MULTI_LANGUAGE_WIN_IMAGE1 (155)
#define MULTI_LANGUAGE_WIN_LABEL1 (156)
#define MULTI_LANGUAGE_WIN_LABEL2 (157)
#define MULTI_LANGUAGE_BUTTON1 (159)
#define MULTI_LANGUAGE_BUTTON2 (161)
#define MULTI_LANGUAGE_BUTTON3 (162)
#define MULTI_LANGUAGE_BUTTON4 (163)
#define MULTI_LANGUAGE_BUTTON5 (164)
#define MULTI_LANGUAGE_BUTTON6 (165)
#define MULTI_LANGUAGE_BUTTON7 (166)
#define MULTI_LANGUAGE_BUTTON8 (167)
#define MULTI_LANGUAGE_BUTTON9 (168)
#define MULTI_LANGUAGE_BUTTON10 (169)
#define MULTI_LANGUAGE_BUTTON11 (170)
#define MULTI_LANGUAGE_BUTTON12 (171)
#define SCROLLBAR_SKIN_SCENE_WIN (172)
#define SCROLLBAR_SKIN_WIN_IMAGE1 (173)
#define SCROLLBAR_SKIN_WIN_LABEL1 (174)
#define SCROLLBAR_SKIN_SCROLLBAR (175)
#define SCROLLBAR_SKIN_LISTBOX (176)
#define SCROLLBAR_SKIN_LISTBOX_LISTCOL (179)
#define SCROLLGRID_WIN (180)
#define SCROLLGRID_WIN_IMAGE1 (181)
#define SCROLLGRID_WIN_LABEL1 (182)
#define scrollgrid_scrollbar (183)
#define SCROLLGRID_WIN_SCROLLGRID (184)
#define GRID_IMG (185)
#define GRID_ICON (186)
#define MENU_WIN_LISTBOX1 (187)
#define MENU_WIN_LISTBOX1_LISTCOL1 (188)
#define SCROLLVIEW_SCENE1_WIN (189)
#define SCROLLVIEW_SCENE1_WIN_IMAGE1 (190)
#define SCROLLVIEW_SCENE1_WIN_LABEL1 (191)
#define SCROLLVIEW_SCENE1_WIN_SCROLLVIEW (192)
#define SCROLLVIEW_SCENE1_WIN_SCROLLVIEW_BUTTON_LEFT (194)
#define SCROLLVIEW_SCENE1_WIN_SCROLLVIEW_BUTTON_1 (195)
#define SCROLLVIEW_SCENE1_WIN_SCROLLVIEW_BUTTON_2 (196)
#define SCROLLVIEW_SCENE1_WIN_SCROLLVIEW_BUTTON_3 (197)
#define SCROLLVIEW_SCENE1_WIN_SCROLLVIEW_BUTTON_4 (198)
#define SCROLLVIEW_SCENE1_WIN_SCROLLVIEW_BUTTON_5 (199)
#define SCROLLVIEW_SCENE1_WIN_SCROLLVIEW_BUTTON_6 (200)
#define SCROLLVIEW_SCENE1_WIN_SCROLLVIEW_BUTTON_RIGHT (201)
#define SCROLLVIEW_SCENE2_WIN (202)
#define SCROLLVIEW_SCENE2_WIN_IMAGE1 (203)
#define SCROLLVIEW_SCENE2_WIN_LABEL1 (204)
#define SCROLLVIEW_SCENE2_WIN_SCROLLVIEW (205)
#define SCROLLVIEW_SCENE2_WIN_BUTTON_LEFT0 (206)
#define SCROLLVIEW_SCENE2_WIN_BUTTON_LEFT (208)
#define SCROLLVIEW_SCENE2_WIN_BUTTON_1 (209)
#define SCROLLVIEW_SCENE2_WIN_BUTTON_2 (210)
#define SCROLLVIEW_SCENE2_WIN_BUTTON_3 (211)
#define SCROLLVIEW_SCENE2_WIN_BUTTON_4 (212)
#define SCROLLVIEW_SCENE2_WIN_BUTTON_5 (213)
#define SCROLLVIEW_SCENE2_WIN_BUTTON_6 (214)
#define SCROLLVIEW_SCENE2_WIN_BUTTON_RIGHT (215)
#define SCROLLVIEW_SCENE2_WIN_BUTTON_RIGHT0 (216)
#define common_skin_white (36)
#define common_skin_red (61)
#define win_title_skin (58)
#define list_normal_skin (66)
#define list_active_skin (150)
#define group_skin (84)
#define mainwin_listbox_select_skin (151)
#define mainwin_listbox_row_normal (152)
#define common_skin (147)
#define button_scene_button_skin (104)
#define scorllbar_res_skin (148)
#define listbox_normal_skin (177)
#define listbox_select_normal_skin (178)
#define msg_scene_button_normal_skin (94)
#define msg_scene_label_skin (92)
#define multi_language_label_skin (158)
#define multi_language_button_skin (160)
#define histogram_skin (138)
#define scrollview_skin (193)
#define button_focus_skin (217)
#define button_normal_skin (207)
#define scrollview_button_focus_skin (218)
#define button_normal (39)
#define button_normal_focus (40)
#define wnd_backgroud_transparent (34)
#define focus_change_normal_skin (121)
#define focus_change_active_skin (122)

extern HIGV_MSG_PROC g_pfunHIGVAppEventFunc[99];

extern HI_S32 ANIM_WIN_ontouchaction(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 ANIM_IN_OUT_HIDE_ongesturetap(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 ANIM_TAB_INOUT_onshow(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 ANIM_TAB_INOUT_onhide(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 ANIM_TAB_INOUT_HIDE_ongesturetap(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 ANIM_TAB_INOUT_TAB1_BTN_ongesturetap(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 ANIM_TAB_INOUT_TAB2_BTN_ongesturetap(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 ANIM_TAB_INOUT_TAB3_BTN_ongesturetap(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 ANIM_TEST_ontouchaction(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 ANIM_TEST_ongesturetap(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 ANIM_WIN_MOVE_onshow(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 ANIM_WIN_MOVE_IMAGE1_ongesturetap(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 ANIM_WIN_MOVE_ongesturetap(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 ANIM_WIN_MOVE_EXIT_ongesturetap(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 ANIM_WIN_MOVE_GRP1_onanimstart(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 ANIM_WIN_MOVE_GRP1_onanimupdate(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 ANIM_WIN_MOVE_GRP1_onanimstop(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 ANIM_WIN_MOVE_BTN_1_ongesturetap(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 MSG_SCENE_WIN_onevent(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 MSG_SCENE_WIN_ongesturescroll(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 MSG_SCENE_WIN_IMAGE1_ongesturetap(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 MSG_SCENE_BUTTON1_ongesturetap(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 MSG_SCENE_BUTTON2_ongesturetap(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 MSG_SCENE_BUTTON3_ongesturetap(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 MSG_SCENE_BUTTON4_ongesturetap(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 BUTTON_WIN_onshow(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 BUTTON_WIN_onhide(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 BUTTON_SCENE_WIN_ontouchaction(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 BUTTON_WIN_IMAGE1_ongesturetap(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 BUTTON_WIN_BUTTON1_ontouchaction(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 WIN_CLOCK_HIDE_ongesturetap(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 OnTimeUndateListener(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 GridGetCount(HIGV_HANDLE DBSource, HI_U32 *RowCnt);
extern HI_S32 GridGetValue(HIGV_HANDLE DBSource, HI_U32 Row, HI_U32 Num, HI_VOID *pData, HI_U32 *pRowNum);
extern HI_S32 ADM_IMAGE_Register(HIGV_HANDLE DBSource, HIGV_HANDLE hADM);
extern HI_S32 ADM_IMAGE_unRegister(HIGV_HANDLE DBSource, HIGV_HANDLE hADM);
extern HI_S32 FLING_SCENE_WIN_ontouchaction(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 FLING_WIN_IMAGE1_ongesturetap(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 FOCUS_CHANGE_WIN_onshow(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 FOCUS_CHANGE_WIN_onhide(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 FOCUS_CHANGE_WIN_onevent(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 FOCUS_CHANGE_WIN_ontimer(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 FOCUS_CHANGE_WIN_IMAGE1_ongesturetap(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 FOCUS_CHANGE_WIN_BUTTON1_ongesturetap(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 FOCUS_CHANGE_WIN_BUTTON2_ongesturetap(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 FOCUS_CHANGE_WIN2_onshow(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 FOCUS_CHANGE_WIN2_onhide(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 FOCUS_CHANGE_WIN2_BUTTON1_ongesturetap(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 HISTOGRAM_SCENE_WIN_onshow(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 HISTOGRAM_SCENE_WIN_onrefresh(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 HISTOGRAM_SCENE_WIN_IMAGE1_ongesturetap(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 IMAGEEX_SCENE_WIN_ontouchaction(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 IMAGEEX_SCENE_WIN_IMAGE1_ontouchaction(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 IMAGEEX_SCENE_WIN_onrepeatfinish(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 MAIN_WIN_onshow(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 Main_Win_Listbox1_OnSelect(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 Main_Win_Listbox1_OnCellSelect(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 MULTI_LANGUAGE_SCENE_WIN_onshow(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 MULTI_LANGUAGE_SCENE_WIN_onhide(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 MULTI_LANGUAGE_WIN_IMAGE1_ongesturetap(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 MULTI_LANGUAGE_BUTTON1_ongesturetap(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 MULTI_LANGUAGE_BUTTON2_ongesturetap(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 MULTI_LANGUAGE_BUTTON3_ongesturetap(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 MULTI_LANGUAGE_BUTTON4_ongesturetap(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 MULTI_LANGUAGE_BUTTON5_ongesturetap(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 MULTI_LANGUAGE_BUTTON6_ongesturetap(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 MULTI_LANGUAGE_BUTTON7_ongesturetap(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 MULTI_LANGUAGE_BUTTON8_ongesturetap(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 MULTI_LANGUAGE_BUTTON9_ongesturetap(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 MULTI_LANGUAGE_BUTTON10_ongesturetap(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 MULTI_LANGUAGE_BUTTON11_ongesturetap(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 MULTI_LANGUAGE_BUTTON12_ongesturetap(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 SCROLLBAR_SKIN_SCENE_WIN_onshow(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 SCROLLBAR_SKIN_IMAGE1_ongesturetap(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 SCROLLGRID_WIN_onshow(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 SCROLLGRID_WIN_onhide(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 SCROLLGRID_WIN_IMAGE1_ontouchaction(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 SCROLLGRID_WIN_LABEL1_ontouchaction(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 GridIconConv(HIGV_HANDLE hList, HI_U32 Col, HI_U32 Item, const HI_CHAR *pSrcStr, HI_CHAR *pDstStr, HI_U32 Length);
extern HI_S32 SCROLLGRID_WIN_LABEL1_OnAnimStop(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 SCROLLVIEW_SCENE1_WIN_onshow(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 SCROLLVIEW_SCENE1_WIN_onhide(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 SCROLLVIEW_SCENE1_WIN_onrefresh(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 SCROLLVIEW_SCENE1_WIN_IMAGE1_ongesturetap(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 SCROLLVIEW_SCENE1_WIN_SCROLLVIEW_ongesturefling(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 SCROLLVIEW_SCENE1_WIN_SCROLLVIEW_onmouseout(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 SCROLLVIEW_SCENE1_WIN_SCROLLVIEW_ontouchaction(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 SCROLLVIEW_SCENE1_WIN_SCROLLVIEW_onfinishfling(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 SCROLLVIEW_SCENE2_WIN_onshow(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 SCROLLVIEW_SCENE2_WIN_onrefresh(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 SCROLLVIEW_SCENE2_WIN_onevent(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 SCROLLVIEW_SCENE2_WIN_ontimer(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 SCROLLVIEW_SCENE2_WIN_ongesturefling(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 SCROLLVIEW_SCENE2_WIN_IMAGE1_ongesturetap(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 SCROLLVIEW_SCENE2_WIN_SCROLLVIEW_ongesturescroll(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 SCROLLVIEW_SCENE2_WIN_SCROLLVIEW_ongesturefling(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 SCROLLVIEW_SCENE2_WIN_SCROLLVIEW_onmouseout(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 SCROLLVIEW_SCENE2_WIN_SCROLLVIEW_ontouchaction(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 SCROLLVIEW_SCENE2_WIN_SCROLLVIEW_onfinishfling(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /*  __cplusplus  */


#endif  /* __HIGV_CEXTFILE_H__ */

