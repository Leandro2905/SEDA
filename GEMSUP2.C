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


MLOCAL LPVOID	ad_rso;

/*
 * void wsw(char *f, int line);
 * #define wswork() wsw(__FILE__,__LINE__)
 */


#if SINGLAPP
EXT UWORD crysbind1(WORD opcode, REG LPWORD pglobal, REG UWORD int_in[], 
			REG UWORD int_out[], REG LPVOID addr_in[]);
#endif
#if MULTIAPP
EXT UWORD crysbind1(WORD opcode, REG LPWORD pglobal, REG UWORD int_in[], 
			REG UWORD int_out[], REG LPVOID addr_in[], 
			REG LPWORD addr_out[]);
#endif

#if SINGLAPP
	UWORD
crysbind(opcode, pglobal, int_in, int_out, addr_in)
#endif
#if MULTIAPP
	UWORD
crysbind(opcode, pglobal, int_in, int_out, addr_in, addr_out)
#endif

	WORD		opcode;
	REG LPWORD	pglobal;
	REG UWORD	int_in[];
	REG UWORD	int_out[];
	REG LPVOID	addr_in[];
#if MULTIAPP
	LONG		addr_out[];
#endif
{
	LPVOID		maddr;
	REG WORD	ret;

	ret = TRUE;
/*
 * #if DEBUG
	TRACE("GEM function called: ");
	ophex(opcode);
	TRACE("\r\n");
#endif
*/
	if (opcode < FORM_DO) return crysbind1(opcode,pglobal,int_in,int_out,addr_in
#if MULTIAPP
	,addr_out
#endif
	);
	
	switch(opcode)
	{
				/* Form Manager				*/
	  case FORM_DO:
		ret = fm_do(FM_FORM, FM_START);
		break;
	  case FORM_DIAL:
		ret = fm_dial(FM_TYPE, (GRECT *)&FM_X, (GRECT *)&FM_IX);
		break;
	  case FORM_ALERT:
		ret = fm_alert(FM_DEFBUT, FM_ASTRING);
		break;
	  case FORM_ERROR:
		ret = fm_error(FM_ERRNUM);
		break;
	  case FORM_CENTER:
		ob_center(FM_FORM, (GRECT *)&FM_XC);
		break;
	  case FORM_KEYBD:
		gsx_sclip(&gl_rfull);
		FM_OCHAR = FM_ICHAR;
		FM_ONXTOB = FM_INXTOB;
		ret = fm_keybd(FM_FORM, FM_OBJ, (WORD *)&FM_OCHAR, (WORD *)&FM_ONXTOB);
		break;
	  case FORM_BUTTON:
		gsx_sclip(&gl_rfull);
		ret = fm_button(FM_FORM, FM_OBJ, FM_CLKS, (WORD *)&FM_ONXTOB);
		break;
				/* Graphics Manager			*/
#if MULTIAPP
	  case PROC_CREATE:
	  	ret = prc_create(PR_IBEGADDR, PR_ISIZE, PR_ISSWAP, PR_ISGEM,
				 &PR_ONUM );
	  	break;
	  case PROC_RUN:
		ret = pr_run(PR_NUM, PR_ISGRAF, PR_ISOVER, PR_PCMD, PR_PTAIL);
		break;
	  case PROC_DELETE:
	  	ret = pr_abort(PR_NUM);
	  	break;
	  case PROC_INFO:
	  	ret = pr_info(PR_NUM, &PR_OISSWAP, &PR_OISGEM, &PR_OBEGADDR,
			&PR_OCSIZE, &PR_OENDMEM, &PR_OSSIZE, &PR_OINTADDR);
	  	break;
	  case PROC_MALLOC:
	  	ret = pr_malloc(PR_IBEGADDR, PR_ISIZE);
	  	break;
	  case PROC_MFREE:
	  	ret = pr_mfree(PR_NUM);
	  	break;
	  case PROC_SWITCH:
	  	ret = pr_switch(PR_NUM);
	  	break;
	  case PROC_SETBLOCK:
		ret = pr_setblock(PR_NUM);
		break;
#endif
				/* Graphics Manager			*/
	  case GRAF_RUBBOX:
		  gr_rubbox(GR_I1, GR_I2, GR_I3, GR_I4, 
					(WORD *)&GR_O1, (WORD *)&GR_O2);
		  break;
	  case GRAF_DRAGBOX:
		  gr_dragbox(GR_I1, GR_I2, GR_I3, GR_I4, (GRECT *)&GR_I5, 
					(WORD *)&GR_O1, (WORD *)&GR_O2);
		  break;
	  case GRAF_MBOX:
		  gr_movebox(GR_I1, GR_I2, GR_I3, GR_I4, GR_I5, GR_I6);
		  break;
#if GROWBOX
	  case GRAF_GROWBOX:
		gr_growbox((GRECT *)&GR_I1, (GRECT *)&GR_I5);
		break;
	  case GRAF_SHRINKBOX:
		gr_shrinkbox((GRECT *)&GR_I1, (GRECT *)&GR_I5);
		break;
#endif
	  case GRAF_WATCHBOX:
		ret = gr_watchbox(GR_TREE, GR_OBJ, GR_INSTATE, GR_OUTSTATE);
		break;
	  case GRAF_SLIDEBOX:
		ret = gr_slidebox(GR_TREE, GR_PARENT, GR_OBJ, GR_ISVERT);
		break;
	  case GRAF_HANDLE:
		GR_WCHAR = gl_wchar;
		GR_HCHAR = gl_hchar;
		GR_WBOX = gl_wbox;
		GR_HBOX = gl_hbox;
		ret = gl_handle;
		break;
	  case GRAF_MOUSE:
		if (GR_MNUMBER > 255)
		{
		  if (GR_MNUMBER == M_OFF)
		    gsx_moff();
		  if (GR_MNUMBER == M_ON)
		    gsx_mon();
		}
		else
		{
		  if (GR_MNUMBER != 255)		
		  {
		    rs_gaddr(ad_sysglo, R_BIPDATA, 3 + GR_MNUMBER, &maddr);
		    maddr = *(LPLPTR)maddr;
		  }
		  else
		    maddr = (LPVOID)GR_MADDR;
		  gsx_mfset(maddr);
		}
		break;
	  case GRAF_MKSTATE:
		ret = gr_mkstate((WORD *)&GR_MX, (WORD *)&GR_MY, (WORD *)&GR_MSTATE, (WORD *)&GR_KSTATE);
		break;
				/* Scrap Manager			*/
	  case SCRP_READ:
		ret = sc_read(SC_PATH);
		break;
	  case SCRP_WRITE:
		ret = sc_write(SC_PATH);
		break;
	  case SCRP_CLEAR:
	  	ret = sc_clear();
	  	break;
				/* File Selector Manager		*/
	  case FSEL_INPUT:
		ret = fs_exinput(FS_IPATH, FS_ISEL, (WORD *)&FS_BUTTON, ADDR(rs_str(ISELNAME)));
		break;
	  case FSEL_EXINPUT:
		ret = fs_exinput(FS_IPATH, FS_ISEL, (WORD *)&FS_BUTTON, FS_INAME);
		break;
				/* Window Manager			*/
	  case WIND_CREATE:
		ret = wm_create(WM_KIND, (GRECT *)&WM_WX);
		break;
	  case WIND_OPEN:
		wm_open(WM_HANDLE, (GRECT *)&WM_WX);
		break;
	  case WIND_CLOSE:
		wm_close(WM_HANDLE);
		break;
	  case WIND_DELETE:
		wm_delete(WM_HANDLE);
		break;
	  case WIND_GET:
		wm_get(WM_HANDLE, WM_WFIELD, (WORD *)&WM_OX);
		break;
	  case WIND_SET:
		  wm_set(WM_HANDLE, WM_WFIELD, (WORD *)&WM_IX);
		  break;
	  case WIND_FIND:
		ret = wm_find(WM_MX, WM_MY);
		break;
	  case WIND_UPDATE:
		wm_update(WM_BEGUP);
		break;
	  case WIND_CALC:
		wm_calc(WM_WCTYPE, WM_WCKIND, WM_WCIX, WM_WCIY, 
			WM_WCIW, WM_WCIH, (WORD *)&WM_WCOX, (WORD *)&WM_WCOY, 
			(WORD *)&WM_WCOW, (WORD *)&WM_WCOH);
		break;
				/* Resource Manager			*/
	  case RSRC_LOAD:
		ret = rs_load(pglobal, (LPBYTE)RS_PFNAME);
		break;
	  case RSRC_FREE:
		ret = rs_free(pglobal);
		break;
	  case RSRC_GADDR:
		ret = rs_gaddr(pglobal, RS_TYPE, RS_INDEX, &ad_rso);
		break;
	  case RSRC_SADDR:
		ret = rs_saddr(pglobal, RS_TYPE, RS_INDEX, RS_INADDR);
		break;
	  case RSRC_OBFIX:
		ret = rs_obfix(RS_TREE, RS_OBJ);
		break;
				/* Shell Manager			*/
	  case SHEL_READ:
		ret = sh_read((LPBYTE)SH_PCMD, (LPBYTE)SH_PTAIL);
		break;
	  case SHEL_WRITE:
		ret = sh_write(SH_DOEX, SH_ISGR, SH_ISCR, SH_PCMD, SH_PTAIL);
		break;
	  case SHEL_GET:
		ret = sh_get(SH_PBUFFER, SH_LEN);
		break;
	  case SHEL_PUT:
		ret = sh_put(SH_PDATA, SH_LEN);
		break;
	  case SHEL_FIND:
		ret = sh_find(SH_PATH);
		break;
	  case SHEL_ENVRN:
		ret = sh_envrn(SH_PATH, SH_SRCH);
		break;
	  case SHEL_RDEF:
		sh_rdef(SH_LPCMD, SH_LPDIR);
		break;
	  case SHEL_WDEF:
		sh_wdef(SH_LPCMD, SH_LPDIR);
		break;
	  case XGRF_STEPCALC:
	  	gr_stepcalc( XGR_I1, XGR_I2, (GRECT *)&XGR_I3, (WORD *)&XGR_O1,
		   (WORD *)&XGR_O2, (WORD *)&XGR_O3, (WORD *)&XGR_O4, (WORD *)&XGR_O5 );
		break;
	  case XGRF_2BOX:
	  	gr_2box(XGR_I4, XGR_I1, (GRECT *)&XGR_I6, XGR_I2,
			XGR_I3,	XGR_I5 );
		break;

	  /* ViewMAX calls */

	  case XGRF_COLOR:
		gsx_setclr(XGR_I1, XGR_I2, XGR_I3, XGR_I4, XGR_I5);
		break;
	  case XGRF_DTIMAGE:
		ret = change_desktop((FDB FAR *)XGR_IMAGE); 
		break;

	  /* Property get/set calls */
#if PROPLIB
	  case PROP_GET:
       ret = prop_get(PROP_PROG, PROP_SECT, PROP_BUF, PROP_BUFL, PROP_OPT);
       break;

	  case PROP_PUT:
       ret = prop_put(PROP_PROG, PROP_SECT, PROP_BUF, PROP_OPT);
       break;

	  case PROP_DEL:
       ret = prop_put(PROP_PROG, PROP_SECT, NULL, PROP_OPT);
	   break;

	  case PROP_GUI_GET:
	   ret = prop_gui_get(PROP_NUM);
	   break;

	  case PROP_GUI_SET:
	   ret = prop_gui_set(PROP_NUM, PROP_VALUE);
	   break;
	   
	   
#endif		
#if XAPPLIB
	  case XAPP_GETINFO:
        ret = xa_getinfo(XAPP_WHAT, (WORD *)&XAPP_OUT);
        break;
#endif
#if XSHELL
	  case XSHL_GETSHELL:
	  	ret = xs_getshell(XSHL_PROGRAM);
	  	break;
	  case XSHL_SETSHELL:
	  	ret = xs_setshell(XSHL_PROGRAM);
	  	break;
#endif
	  default:
		fm_show(ALNOFUNC, (UWORD *)NULLPTR, 1);
		ret = -1;
		break;
	}
/*
#if DEBUG
	TRACE("GEM function returns: ");
	ophex(opcode);
	TRACE("\r\n");
#endif
*/
	return(ret);
}

/*
*	Routine that copies input parameters into local buffers,
*	calls the appropriate routine via a case statement, copies
*	return parameters from local buffers, and returns to the
*	routine.
*/

VOID xif(LPLPTR pcrys_blk)
{
	UWORD		control[C_SIZE];
	UWORD		int_in[I_SIZE];
	UWORD		int_out[O_SIZE];
	LPVOID		addr_in[AI_SIZE];
#if MULTIAPP
	LONG		addr_out[AO_SIZE];
#endif

	LWCOPY(ADDR(&control[0]), CONTROL, C_SIZE);
/*
{
	LPWORD ptr = MK_FP(rlr->p_uda->u_ssuser, (WORD)rlr->p_uda->u_spuser);
//	TRACE("rlr->p_uda=$");
//	ophex((WORD)(rlr->p_uda));
	TRACE(" GEM call $");
	ophex(control[0]);
	TRACE(" ss:sp=$");
	ophex(rlr->p_uda->u_ssuser);
	TRACE(":$");
	ophex((WORD)(rlr->p_uda->u_spuser));
	TRACE(" -> $");
	ophex(ptr[0]);
	TRACE(",$");
	ophex(ptr[1]);
	TRACE(",$");
	ophex(ptr[2]);
	TRACE("\r\n$");
	wswork();
	//TRKEY();
} */
	if (IN_LEN)
	  LWCOPY(ADDR(&int_in[0]), INT_IN, IN_LEN);
	if (AIN_LEN)
	  LWCOPY(ADDR(&addr_in[0]), ADDR_IN, AIN_LEN*2);
	else
	  addr_in[0] = (LPVOID)0L; // [JCE] Conditional parameter to appl_init()
	  
#if SINGLAPP
	int_out[0] = crysbind(OP_CODE, GGLOBAL, &int_in[0], &int_out[0], 
				&addr_in[0]);
#endif
#if MULTIAPP
	int_out[0] = crysbind(OP_CODE, GGLOBAL, &int_in[0], &int_out[0], 
				&addr_in[0], &addr_out[0]);
#endif
	if (OUT_LEN)
	  LWCOPY(INT_OUT, ADDR(&int_out[0]), OUT_LEN);
	if (OP_CODE == RSRC_GADDR)
	 *((LPVOID FAR *)(ADDR_OUT)) = ad_rso;
#if MULTIAPP
	if (OP_CODE == PROC_INFO)
	  LWCOPY(ADDR_OUT, ADDR(&addr_out[0]), AOUT_LEN * 2);
#endif
}


/*
*	Supervisor entry point.  Stack frame must be exactly like
*	this if supret is to work.
*/
VOID super(LONG pcrys_blk)
{
	xif((LPLPTR)pcrys_blk);
	
	if ( (gl_dspcnt++ % 10) == 0 )
	  dsptch();

/*	TRACE("GEM call returns\r\n$");
	wswork(); */
	supret(0);
}

	VOID
nsuper(pcrys_blk)
	LONG		pcrys_blk;
{
#if MULTIAPP
	supret(0);
#endif
}


