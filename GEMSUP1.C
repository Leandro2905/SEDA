/*	GEMSUPER.C	7/09/84 - 08/14/85	Lee Jay Lorenzen	*/
/*	2.0		11/06/85 - 12/12/85	Lowell Webster		*/
/*	APPLLIB.C	4/11/84 - 10/22/84	Lee Jay Lorenzen	*/
/*	EVNTLIB.C	4/11/84 - 5/16/84	Lee Jay Lorenzen	*/
/*	MENULIB.C	04/26/84 - 09/29/84	Lowell Webster		*/
/*	OBJCLIB.C	03/15/84 - 09/10/84	Gregg Morris		*/
/*	FORMLIB.C	03/15/84 - 06/16/85	Gregg Morris		*/
/*	GRAFLIB.C	05/05/84 - 09/10/84	Lee Lorenzen		*/
/*	SCRPLIB.C	05/05/84 - 02/02/85	Lee Lorenzen		*/
/*	FSELLIB.C	05/05/84 - 09/09/84	Lee Lorenzen		*/
/*	WINDLIB.C 	4/23/84 - 8/18/84	Lee Lorenzen		*/
/*	RSRCLIB.C	05/05/84 - 10/23/84	Lowell Webster		*/
/*	SHELLIB.C	4/11/84 - 01/29/85	Lee Lorenzen		*/
/*	merge High C vers. w. 2.2 		8/24/87		mdf	*/ 

/*
*       Copyright 1999, Caldera Thin Clients, Inc.                      
*       This software is licenced under the GNU Public License.         
*       Please see LICENSE.TXT for further information.                 
*                                                                       
*                  Historical Copyright                                 
*	-------------------------------------------------------------
*	GEM Application Environment Services		  Version 2.3
*	Serial No.  XXXX-0000-654321		  All Rights Reserved
*	Copyright (C) 1987			Digital Research Inc.
*	-------------------------------------------------------------
*/

#include "aes.h"

#define CONTROL  pcrys_blk[0]
#define GGLOBAL  pcrys_blk[1]
#define INT_IN   pcrys_blk[2]
#define INT_OUT  pcrys_blk[3]
#define ADDR_IN  pcrys_blk[4]
#define ADDR_OUT pcrys_blk[5]




#if SINGLAPP
UWORD crysbind1(WORD opcode, REG LPWORD pglobal, REG UWORD int_in[],
                        REG UWORD int_out[], REG LPVOID addr_in[])
#endif
#if MULTIAPP
UWORD crysbind1(WORD opcode, REG LPWORD pglobal, REG UWORD int_in[],
                        REG UWORD int_out[], REG LPVOID addr_in[],
                        REG LPWORD addr_out[])
#endif
{
	LONG		maddr, mbv;
	LPTREE		tree;
	REG WORD	ret;

	ret = TRUE;

// [JCE] This function is too big for PPD's optimiser, so it's split in two:

	switch(opcode)
	{	
				/* Application Manager			*/
	  case APPL_INIT:
#if SINGLAPP
	  	pglobal[0] = 0x0300;		/* version number	*/
	  	pglobal[1] = 0x0001;		/* num of concurrent procs*/
#endif
#if MULTIAPP
	  	pglobal[0] = 0x0110;		/* version number	*/
	  	pglobal[1] = NUM_DESKACC-1;	/* num of concurrent procs*/
#endif
/*		LLSET(pglobal, 0x00010200L);
*/
		pglobal[2]  = rlr->p_pid;
		sh_desk(0, (LPLONG)(&pglobal[3]));
		pglobal[10] = gl_nplanes;
		pglobal[11] = LLOWD((LONG)ADDR(&D));
		pglobal[12] = LHIWD((LONG)ADDR(&D));
		pglobal[13] = gl_bvdisk >> 16;
		pglobal[14] = gl_bvhard >> 16;
						/* reset dispatcher 	*/
						/*  count to let the app*/
						/*  run a while.	*/
		gl_dspcnt = NULL;
		ret = ap_init(addr_in[0]);
		break;
	  case APPL_READ:
	  case APPL_WRITE:
		ap_rdwr(opcode == APPL_READ ? MU_MESAG : MU_SDMSG, 
			fpdnm(NULLPTR, AP_RWID), AP_LENGTH, AP_PBUFF);
		break;
	  case APPL_FIND:
		ret = ap_find( AP_PNAME );
		break;
	  case APPL_TPLAY:
		ap_tplay(AP_TBUFFER, AP_TLENGTH, AP_TSCALE);
		break;
	  case APPL_TRECORD:
		ret = ap_trecd(AP_TBUFFER, AP_TLENGTH);
		break;
	  case APPL_BVSET:
		gl_bvdisk = ((ULONG)AP_BVDISK) << 16;
		gl_bvhard = ((ULONG)AP_BVHARD) << 16;
		break;
	  case APPL_YIELD:
		dsptch();
		break;
	  case APPL_BVEXT:
	    switch (AP_XBVMODE)
		{ 
			case 0: AP_XBVDISKL = LLOWD(gl_bvdisk);
					AP_XBVDISKH = LHIWD(gl_bvdisk);
					AP_XBVHARDL = LLOWD(gl_bvhard);
					AP_XBVHARDH = LHIWD(gl_bvhard);
					break;
			case 1: gl_bvdisk  = (LONG)AP_XBVDISK;
					gl_bvhard  = (LONG)AP_XBVHARD;
					break;
		}
		break;
	  case APPL_EXIT:
		ap_exit( TRUE );
		break;
				/* Event Manager			*/
	  case EVNT_KEYBD:
		  ret = ev_block(MU_KEYBD, 0x0L);
		break;
	  case EVNT_BUTTON:
		ret = ev_button(B_CLICKS, B_MASK, B_STATE, (WORD *)&EV_MX);
		break;
	  case EVNT_MOUSE:
		ret = ev_mouse((MOBLK *)&MO_FLAGS, (WORD *)&EV_MX);
		break;
	  case EVNT_MESAG:
#if MULTIAPP
						/* standard 16 byte read */
		ev_mesag(MU_MESAG, rlr, ME_PBUFF);
#endif
#if SINGLAPP
		ap_rdwr(MU_MESAG, rlr, 16, ME_PBUFF);
#endif
		break;
	  case EVNT_TIMER:
		ev_timer( HW(T_HICOUNT) + LW(T_LOCOUNT) );
		break;
	  case EVNT_MULTI:
		  if (MU_FLAGS & MU_TIMER)
		  maddr = HW(MT_HICOUNT) + LW(MT_LOCOUNT);
if ((MB_MASK == 3) && (MB_STATE == 1))
{
  MB_STATE = 0;

}
	  mbv = HW(MB_CLICKS) | LW((MB_MASK << 8) | MB_STATE);
		ret = ev_multi(MU_FLAGS, (MOBLK *)&MMO1_FLAGS, (MOBLK *)&MMO2_FLAGS, 
			maddr, mbv, MME_PBUFF, (WORD *)&EV_MX);
		break;
	  case EVNT_DCLICK:
		ret = ev_dclick(EV_DCRATE, EV_DCSETIT);
		break;
				/* Menu Manager				*/
	  case MENU_BAR:
	  /* GEM/2 and GEM/3 only draw the menu bar if it's for the 
	   * app that owns the menu, as defined by gl_mnppd. However,
	   * gl_mnppd is only set when a new top window is set. 
	   *
       * This, in turn, stops the desktop's menu bar appearing 
       * until it opens one or more windows. The call to w_menufix() below
       * forces a recheck of gl_mnppd and a redraw of the menu bar.
	   */
        if (gl_mnppd == rlr)
		  mn_bar(MM_ITREE, SHOW_IT, rlr->p_pid);
		else
		{
		  menu_tree[rlr->p_pid] = (SHOW_IT) ? MM_ITREE : ((LPTREE)0x0L);
	  	  /* Fix for GEM/1 and FreeGEM desktops */
		  w_menufix();
		}
		break;
	  case MENU_ICHECK:
		do_chg(MM_ITREE, ITEM_NUM, CHECKED, CHECK_IT, FALSE, FALSE);
		break;
	  case MENU_IENABLE:
		do_chg(MM_ITREE, (ITEM_NUM & 0x7fff), DISABLED, 
			!ENABLE_IT, ((ITEM_NUM & 0x8000) != 0x0), FALSE);
		break;
	  case MENU_TNORMAL:
		if (gl_mntree == menu_tree[rlr->p_pid])
		  do_chg(MM_ITREE, TITLE_NUM, SELECTED, !NORMAL_IT, 
				TRUE, TRUE);
		break;
	  case MENU_TEXT:
		tree = (LPTREE)MM_ITREE;
		if (LHIWD((LONG)tree))
		  LSTCPY((LPBYTE)tree[ITEM_NUM].ob_spec, (LPBYTE)MM_PTEXT);	
		else
		  LSTCPY(desk_acc[ gl_mnpds[ LLOWD(tree) ] ], MM_PTEXT);
		break;
	  case MENU_REGISTER:
		ret = mn_register(MM_PID, MM_PSTR);
		break;
	  case MENU_UNREGISTER:
		if (MM_MID == -1) 
		  MM_MID = gl_mnpds[rlr->p_pid];
		mn_unregister( MM_MID );
		break;
				/* Object Manager			*/
	  case MENU_CLICK:
		if (MN_SETIT)
		  gl_mnclick = MN_CLICK;
		ret = gl_mnclick;
		break;

	  case OBJC_ADD:
		ob_add(OB_TREE, OB_PARENT, OB_CHILD);
		break;
	  case OBJC_DELETE:
		ob_delete(OB_TREE, OB_DELOB);
		break;
	  case OBJC_DRAW:
		gsx_sclip((GRECT *)&OB_XCLIP);
		ob_draw(OB_TREE, OB_DRAWOB, OB_DEPTH);
		break;
	  case OBJC_FIND:
		ret = ob_find(OB_TREE, OB_STARTOB, OB_DEPTH, 
				OB_MX, OB_MY);
		break;
	  case OBJC_OFFSET:
		ob_offset(OB_TREE, OB_OBJ, (WORD *)&OB_XOFF, (WORD *)&OB_YOFF);
		break;
	  case OBJC_ORDER:
		ob_order(OB_TREE, OB_OBJ, OB_NEWPOS);
		break;
	  case OBJC_EDIT:
		gsx_sclip(&gl_rfull);
		OB_ODX = OB_IDX;
		ret = ob_edit(OB_TREE, OB_OBJ, OB_CHAR, (WORD *)&OB_ODX, OB_KIND);
		break;
	  case OBJC_CHANGE:
		gsx_sclip((GRECT *)&OB_XCLIP);
		ob_change(OB_TREE, OB_DRAWOB, OB_NEWSTATE, OB_REDRAW);
		break;
	}


	return (ret);
}


