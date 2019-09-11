#ifndef __HIGV_CEXTFILE_H__
#define __HIGV_CEXTFILE_H__

#include "hi_type.h"

#include "hi_gv.h"

#ifdef __cplusplus
#if __cplusplus
    extern "C"{
#endif
#endif /*  __cplusplus  */

#define btn_move (1)
#define btn_move2 (2)
#define btn_cancel (3)
#define btn_onoff (4)
#define btn_end (5)
#define btn_ok_move1 (6)
#define btn_cancel_move1 (7)
#define btn_off_move1 (8)
#define btn_end_move1 (9)
#define btn_ok_move0 (10)
#define btn_cancel_move0 (11)
#define btn_off_move0 (12)
#define btn_end_move0 (13)
#define animalpha_change (14)
#define animroll_btn (15)
#define animroll_listbox_left (16)
#define animroll_listbox_up (17)
#define animroll_listbox_up_down (18)
#define animroll_listbox_down (19)
#define animroll_listbox_right (20)
#define ANIM_WIN (21)
#define ANIM_WIN_IMAGE1 (23)
#define ANIM_WIN_LABEL1 (24)
#define ANIM_BUTTON (27)
#define BUTTON_WIN (29)
#define BUTTON_WIN_IMAGE1 (30)
#define BUTTON_WIN_LABEL1 (31)
#define BUTTON_WIN_BUTTON1 (32)
#define BUTTON_WIN_BUTTON2 (35)
#define BUTTON_SWITCH (36)
#define BUTTON_toggle (39)
#define ADM_SET_STRINGID (41)
#define ADM_SET_STRINGID2 (42)
#define ADM_IMAGE (43)
#define ADM_MONTH (44)
#define ADM_DAY (45)
#define ADM_YEAR (46)
#define simhei_font_text_10 (47)
#define simhei_font_text_20 (26)
#define simhei_font_text_40 (48)
#define IMAGE_WIN (49)
#define IMAGE_WIN_IMAGE1 (51)
#define IMAGE_WIN_LABEL1 (52)
#define PNG_IMAGE (53)
#define JPG_IMAGE (54)
#define BMP_IMAGE (55)
#define SHOW_IMAGE (56)
#define LISTBOX_WIN (57)
#define LISTBOX_WIN_IMAGE1 (58)
#define LISTBOX_WIN_LABEL1 (59)
#define listbox_item_scrollbar (60)
#define MENUSET_LISTBOX (63)
#define MENUSET_LISTBOX_LISTCOL1 (68)
#define MENUSET_LISTBOX_LISTCOL2 (69)
#define MENUSET_LISTBOX_LISTCOL3 (70)
#define MAIN_WIN (71)
#define MAIN_WIN_LABEL1 (72)
#define MAIN_WIN_LISTBOX1_ITEM_SCEOLLBAR (73)
#define MAIN_WIN_LISTBOX1 (75)
#define MAIN_WIN_LISTBOX1_LISTCOL1 (78)
#define PROGRESSBAR_WIN (79)
#define PROGRESSBAR_WIN_IMAGE1 (80)
#define PROGRESSBAR_WIN_LABEL1 (81)
#define HORIZONTAL_PROGRESSBAR (82)
#define FORWARD_BUTTON (85)
#define BACK_BUTTON (86)
#define SCROLLGRID_WIN (87)
#define SCROLLGRID_WIN_IMAGE1 (88)
#define SCROLLGRID_WIN_LABEL1 (89)
#define scrollgrid_scrollbar (90)
#define SCROLLGRID_WIN_SCROLLGRID (91)
#define GRID_IMG (94)
#define GRID_ICON (95)
#define GRID_TEXT (96)
#define SCROLLTEXT_WIN (97)
#define SCROLLTEXT_WIN_IMAGE1 (98)
#define SCROLLTEXT_WIN_LABEL1 (99)
#define SCROLLTEXT_WIN_SCROLLTEXT (100)
#define SCROLLVIEW_WIN (102)
#define SCROLLVIEW_WIN_IMAGE1 (103)
#define SCROLLVIEW_WIN_LABEL1 (104)
#define scrollview_scrollbar (105)
#define SCROLLVIEW_WIN_SCROLLVIEW1 (107)
#define ChildButton_01 (109)
#define ChildButton_02 (113)
#define ChildButton_03 (112)
#define ChildButton_04 (118)
#define ChildButton_05 (121)
#define ChildButton_06 (124)
#define ChildButton_07 (128)
#define ChildButton_08 (132)
#define ChildButton_09 (135)
#define ChildButton_10 (136)
#define ChildButton_11 (137)
#define ChildButton_12 (138)
#define ChildButton_13 (139)
#define ChildButton_14 (140)
#define ChildButton_15 (141)
#define ChildButton_16 (142)
#define group_skin (28)
#define common_skin (74)
#define rec_status_red (37)
#define common_skin_white (22)
#define common_skin_gold (50)
#define win_title_skin (25)
#define mainwin_listbox_select_skin (76)
#define mainwin_listbox_row_normal (77)
#define button_normal (33)
#define button_normal_focus (34)
#define status_bar_mode_sel (38)
#define set_listbox_select_skin (40)
#define button_check_no_skin (143)
#define button_check_skin (144)
#define trackbar_normal_skin (145)
#define trackbar_ver_normal_skin (146)
#define scrollview_skin (108)
#define scrollgrid_normal_skin (92)
#define scrollgrid_active_skin (93)
#define button1_normal_skin (110)
#define button1_active_skin (111)
#define button2_normal_skin (114)
#define button2_active_skin (115)
#define button3_normal_skin (116)
#define button3_active_skin (117)
#define button4_normal_skin (119)
#define button4_active_skin (120)
#define button5_normal_skin (122)
#define button5_active_skin (123)
#define button6_normal_skin (126)
#define button6_active_skin (127)
#define button7_normal_skin (130)
#define button7_active_skin (131)
#define button8_normal_skin (133)
#define button8_active_skin (134)
#define scrollbar_slide_skin (147)
#define scrollbar_skin (106)
#define slideunlock_normal_skin (148)
#define slideunlock_win_skin (149)
#define scorllbar_bg_skin (61)
#define scorllbar_res_skin (62)
#define list_normal_skin (64)
#define list_active_skin (65)
#define setting_listbox_select_skin (66)
#define setting_listbox_row_normal (67)
#define normal_skin_date (150)
#define normal_skin_time (151)
#define normal_skin_win1 (152)
#define label_skin (153)
#define down_focus_cover_skin (154)
#define up_focus_cover_skin (155)
#define down_no_focus_cover_skin (156)
#define up_no_focus_cover_skin (157)
#define progressbar_normal_skin (83)
#define progressbar_progress_skin (84)
#define scrolltext_normal_skin (101)
#define SLIDEUNLOCK_WIN (158)
#define SLIDEUNLOCK_WIN_IMAGE1 (159)
#define SLIDEUNLOCK_WIN_LABEL1 (160)
#define SLID_BG_IMG (161)
#define IMAGEEX_BG (162)
#define LOCK_IMG (163)
#define HORIZONTAL_SLIDEUNLOCK (164)
#define TRACKBAR_WIN (165)
#define TRACKBAR_WIN_IMAGE1 (166)
#define TRACKBAR_WIN_LABEL1 (167)
#define HORIZONTAL_TRACKBAR (168)
#define VERTICAL_TRACKBAR (169)
#define WHEELVIEW_WIN (170)
#define WHEELVIEW_WIN_IMAGE1 (171)
#define WHEELVIEW_WIN_LABEL1 (172)
#define WHEELVIEW_MONTH (173)
#define WHEELVIEW_DAY (174)
#define WHEELVIEW_YEAR (175)

extern HIGV_MSG_PROC g_pfunHIGVAppEventFunc[75];

extern HI_S32 AnimOnShowListener(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 ANIM_WIN_IMAGE1_ontouchaction(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 BUTTON_WIN_OnShowListener(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 BUTTON_WIN_OnHideListener(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 BUTTON_WIN_ontouchaction(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 BUTTON_WIN_IMAGE1_ontouchaction(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 BUTTON_WIN_BUTTON1_ontouchaction(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 BUTTON_WIN_BUTTON2_ontouchaction(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 BUTTON_SWITCH_ontouchaction(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 BUTTON_TOGGLE_ontouchaction(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 GridGetCount(HIGV_HANDLE DBSource, HI_U32 *RowCnt);
extern HI_S32 GridGetValue(HIGV_HANDLE DBSource, HI_U32 Row, HI_U32 Num, HI_VOID *pData, HI_U32 *pRowNum);
extern HI_S32 GridRegister(HIGV_HANDLE DBSource, HIGV_HANDLE hADM);
extern HI_S32 GridUnregister(HIGV_HANDLE DBSource, HIGV_HANDLE hADM);
extern HI_S32 IMAGE_WIN_IMAGE1_ontouchaction(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 LISTBOX_WIN_onshow(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 LISTBOX_WIN_onhide(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 LISTBOX_WIN_IMAGE1_ontouchaction(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 listboxOnCellSelectListener(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 listboxOnSelectListener(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 listboxOnDateChangeListener(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 Main_Win_Listbox1_OnSelect(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 Main_Win_Listbox1_OnCellSelect(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 PROGRESSBAR_WIN_IMAGE1_ontouchaction(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 onProgressBarSelectListener(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 OnProgressBarForwardListener(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 OnProgressBarBackListener(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 SCROLLGRID_WIN_onshow(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 SCROLLGRID_WIN_onhide(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 SCROLLGRID_WIN_ontimer(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 SCROLLGRID_WIN_onrefresh(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 SCROLLGRID_WIN_IMAGE1_ontouchaction(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 SCROLLGRID_WIN_SCROLLGRID_oncellcolselect(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 SCROLLGRID_WIN_SCROLLGRID_onfocusmove(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 SCROLLGRID_WIN_SCROLLGRID_onfocusreachtop(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 SCROLLGRID_WIN_SCROLLGRID_onfocusreachbtm(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 SCROLLGRID_WIN_SCROLLGRID_onfocusreachleft(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 SCROLLGRID_WIN_SCROLLGRID_onfocusreachright(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 GridIconConv(HIGV_HANDLE hList, HI_U32 Col, HI_U32 Item, const HI_CHAR *pSrcStr, HI_CHAR *pDstStr, HI_U32 Length);
extern HI_S32 GridTextConv(HIGV_HANDLE hList, HI_U32 Col, HI_U32 Item, const HI_CHAR *pSrcStr, HI_CHAR *pDstStr, HI_U32 Length);
extern HI_S32 SCROLLTEXT_WIN_onshow(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 SCROLLTEXT_WIN_IMAGE1_ontouchaction(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 SCROLLTEXT_WIN_SCROLLTEXT_onetimeover(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 SCROLLVIEW_WIN_onshow(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 SCROLLVIEW_WIN_onrefresh(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 SCROLLVIEW_WIN_onhide(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 SCROLLVIEW_WIN_ontimer(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 SCROLLVIEW_WIN_IMAGE1_ontouchaction(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 SCROLLVIEW_WIN_SCROLLVIEW1_onviewmove(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 SCROLLVIEW_WIN_SCROLLVIEW1_onfinishfling(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 ChildButton_01_onkeydown(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 ChildButton_02_onkeydown(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 OnShowListener(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 SLIDEUNLOCK_WIN_onrefresh(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 SLIDEUNLOCK_WIN_onhide(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 SLIDEUNLOCK_WIN_IMAGE1_ontouchaction(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 OnUnlockListener(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 OnMoveListener(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 OnKickBackListener(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 TRACKBAR_WIN_onshow(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 TRACKBAR_WIN_onhide(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 TRACKBAR_WIN_IMAGE1_ontouchaction(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 HORIZONTAL_TRACKBAR_onvaluechange(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 HORIZONTAL_TRACKBAR_onmouseout(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 HORIZONTAL_TRACKBAR_ontouchaction(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 VERTICAL_TRACKBAR_onvaluechange(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 WheelViewOnShowListener(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 WHEELVIEW_WIN_IMAGE1_ontouchaction(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 OnMonthListener(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 OnMonthScrollListener(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 OnMonthFocusSelectListener(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 OnDayListener(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 OnDayFocusSelectListener(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 OnYearListener(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);
extern HI_S32 OnYearFocusSelectListener(HIGV_HANDLE hWidget,HI_PARAM wParam, HI_PARAM lParam);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /*  __cplusplus  */


#endif  /* __HIGV_CEXTFILE_H__ */

