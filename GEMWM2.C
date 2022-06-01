/*	GEMWMLIB.C 	4/23/84 - 07/11/85	Lee Lorenzen		*/
/*	merge High C vers. w. 2.2 		8/24/87		mdf	*/ 
/*	fix wm_delete bug			10/8/87		mdf	*/

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

#define DESKWH	0x0
#define DROP_SIZE 0		/* Windows don't have drop shadows */

#if MULTIAPP
#define NUM_MWIN NUM_WIN+1
#define INACROOT NUM_WIN
EXTERN WORD	shrunk;
EXTERN SHELL	sh[];
EXTERN LONG	ad_armice;
EXTERN LONG 	ad_sysmenu;
#endif
#if SINGLAPP
#define NUM_MWIN NUM_WIN
#endif

WORD		w_bldactive();

EXTERN PD	*fpdnm();
						/* in GEMFLAG.C		*/
EXTERN WORD	tak_flag();
						/* in GSXIF.C		*/
EXTERN VOID	gsx_sclip();
EXTERN VOID	gsx_mret();
						/* in OBLIB.C		*/
EXTERN VOID	everyobj();
EXTERN VOID	ob_delete();
EXTERN VOID	ob_draw();
EXTERN VOID	ob_order();
EXTERN WORD	ob_find();
						/* in WRECT88.C		*/
//EXTERN ORECT	*get_orect();


EXTERN GRECT	gl_rfull;

EXT LPTREE	gl_newdesk;			/* current desktop back-*/
						/* ground pattern.	*/
EXT WORD	gl_newroot;			/* current object w/in	*/
						/* gl_newdesk.		*/
EXT LPTREE	desk_tree[NUM_PDS];		/* list of object trees	*/
						/* for the desktop back-*/
						/* ground pattern.	*/
EXT WORD	desk_root[NUM_PDS];		/* starting object to	*/
						/* draw within desk_tree.*/

EXT OBJECT	W_ACTIVE[NUM_ELEM];

#define WSTRSIZE 81
#if MULTIAPP
GLOBAL BYTE	W_NAMES[NUM_WIN][WSTRSIZE];
GLOBAL BYTE	W_INFOS[NUM_WIN][WSTRSIZE];
GLOBAL PD	*W_INACTIVE[NUM_WIN];		/* pds of inactive windows*/
GLOBAL PD	*gl_newmenu = (PD *)0x0;
GLOBAL PD	*gl_lastnpd;
GLOBAL WORD	proc_msg[8];
#endif
/*
static void epw(int win, WINDOW *w, int line)
{
        TRACE("ems_put_window gemwm2.c: $");
        ophex(line);
	TRACE("\r\n$");
        ems_put_window(win, w);
}
#define ems_put_window(a,b) epw(a,b, __LINE__)


static void epo(int orect, ORECT *o, int line)
{
        TRACE("ems_put_orect gemwmlib.c: $");
        ophex(line);
        ems_put_orect(orect, o);
}


#define ems_put_orect(a,b) epo(a,b,__LINE__)
*/

EXT WORD	gl_watype[NUM_ELEM];
EXT LONG	gl_waspec3d[NUM_ELEM];
EXT LONG	gl_waspec2d[NUM_ELEM];
EXT LONG	gl_waspec[NUM_ELEM];
EXT WORD	gl_waflag3d[NUM_ELEM];
EXT WORD	gl_waflag2d[NUM_ELEM];
EXT WORD	gl_waflag[NUM_ELEM];



EXT TEDINFO	gl_aname;
EXT TEDINFO	gl_ainfo;

EXT TEDINFO	gl_asamp;


VOID wm_calc(WORD wtype, REG UWORD kind, WORD x, WORD y, WORD w, WORD h, 
			 LPWORD px, LPWORD py, LPWORD pw, LPWORD ph);

EXT VOID w_emsobadd(REG WORD parent, REG WORD child);
EXT WORD *w_getxptr(WORD which, REG WORD w_handle);
EXT VOID w_nilit(REG WORD num, REG LPTREE olist);
EXT VOID ems_nilit(REG WORD num);

/* ---------- added for metaware compiler ---------- */
GLOBAL VOID	wm_update();
EXTERN VOID	rc_copy();			/* in OPTIMOPT.A86	*/
EXTERN WORD 	rc_equal();
EXTERN VOID 	r_set();
EXTERN VOID	bfill();
EXTERN WORD 	min();
EXTERN WORD 	max();
EXTERN VOID 	movs();
EXTERN WORD 	rc_intersect();			/* in OPTIMIZE.C	*/
EXTERN VOID 	rc_union();
EXTERN WORD 	mul_div();			/* in GSX2.A86		*/
EXTERN VOID	ct_chgown();			/* in CTRL.C		*/
EXTERN VOID	ap_rdwr();			/* in APLIB.C		*/
EXTERN VOID	bb_screen();			/* in GRAF.C		*/
EXTERN VOID  	gsx_moff();			/* in GSXIF.C		*/
EXTERN VOID 	gsx_mon();
EXTERN VOID 	mn_bar();			/* in MNLIB.C		*/
EXTERN VOID	or_start();			/* in WRECT.C		*/
EXTERN VOID	fm_own();			/* in FMLIB.C		*/
/* ------------------------------------------------- */

EXTERN WORD	gl_wbox;
EXTERN WORD	gl_hbox;

EXTERN WORD	gl_width;
EXTERN WORD	gl_height;

EXTERN GRECT	gl_rscreen;
EXTERN GRECT	gl_rfull;
EXTERN GRECT	gl_rzero;


EXTERN THEGLO	D;

EXT WORD	wind_msg[8];


EXT WORD	gl_wtop;
/*
MLOCAL void sstk(int ln)
{
        LPWORD ptr = MK_FP(rlr->p_uda->u_ssuser, (WORD)rlr->p_uda->u_spuser);
        TRACE("rlr->p_uda=$");
        ophex((WORD)(rlr->p_uda));
        TRACE(" gemwm2.c:$");
        ophex(ln);
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
        TRACE("\r$");
}

#define showstk() sstk(__LINE__)

void wsw(char *f, int line)
{
	MLOCAL WINDOW w;

	ems_get_window(1, &w);
	TRACE("wswork: $"); TRACE(f);
	TRACE(" line=$"); ophex(line);
	TRACE(" rect=($"); ophex(w.w_xwork);
	TRACE(", $"); ophex(w.w_ywork);
	TRACE(", $"); ophex(w.w_wwork);
	TRACE(", $"); ophex(w.w_hwork);
	TRACE(")\r\n$");
}

#define wswork() wsw(__FILE__,__LINE__)
*/

VOID ap_sendmsg(ap_msg, type, towhom, w3, w4, w5, w6, w7)
	REG WORD	ap_msg[];
	WORD		type;
	PD		*towhom;
	WORD		w3, w4, w5, w6, w7;
{
	ap_msg[0] = type;
	ap_msg[1] = rlr->p_pid;
	ap_msg[2] = 0;
	ap_msg[3] = w3;
	ap_msg[4] = w4;
	ap_msg[5] = w5;
	ap_msg[6] = w6;
	ap_msg[7] = w7;
	ap_rdwr(MU_SDMSG, towhom, 16, ADDR(&ap_msg[0]));
}


VOID w_redraw(REG WORD w_handle, GRECT *pt)
{
	GRECT		t, d;
	PD		*ppd;
	int		sendit = 0;

	ems_get_window(w_handle, &D.w_win0);
	ppd = D.w_win0.w_owner;
	
#if MULTIAPP
	if ( !sh[ppd->p_pid].sh_isgem )		/* get out if not a gemapp*/
	  return;
#endif
						/* make sure work rect	*/
						/*   and word rect	*/
						/*   intersect		*/
	rc_copy(pt, &t);
	w_getsize(WS_WORK, w_handle, &d);
	if ( rc_intersect(&t, &d) )
	{
						/* make sure window has	*/
						/*   owns a rectangle	*/
	  if ( w_union(D.w_win0.w_nrlist, &d) )
	  {
						/* intersect redraw	*/
						/*   rect with union	*/
						/*   of owner rects	*/
	    if ( rc_intersect(&d, &t) )
	      sendit = 1;
	  }
        }
	if (sendit) ap_sendmsg(wind_msg, WM_REDRAW, ppd,
			w_handle, t.g_x, t.g_y, t.g_w, t.g_h);
}


/*
*	Routine to fix rectangles in preparation for a source to
*	destination blt.  If the source is at -1, then the source
*	and destination left fringes need to be realigned.
*/
	WORD
w_mvfix(ps, pd)
	REG GRECT	*ps;
	REG GRECT	*pd;
{
	REG WORD	tmpsx;
	
	tmpsx = ps->g_x;
	rc_intersect(&gl_rfull, ps);
	if (tmpsx == -1)
	{
	  pd->g_x++;
	  pd->g_w--;
	  return(TRUE);
	}
	return(FALSE);
}

/*
*	Call to move top window.  This involves BLTing the window if
*	none of it that is partially off the screen needs to be redraw,
*	else the whole desktop to just updated.  All uncovered portions
*	of the desktop are redrawn by later by calling w_update.
*/
	WORD	
w_move(w_handle, pstop, prc)
	REG WORD	w_handle;
	REG WORD	*pstop;
	GRECT		*prc;
{
	GRECT		s;			/* source		*/
	GRECT		d;			/* destination		*/
	REG GRECT	*pc;
	REG WORD	sminus1, dminus1;

	w_getsize(WS_PREV, w_handle, &s);
#if DROP_SIZE
	s.g_w += DROP_SIZE;
	s.g_h += DROP_SIZE;
#endif
	w_getsize(WS_TRUE, w_handle, &d);
						/* set flags for when	*/
						/*   part of the source	*/
						/*   is off the screen	*/
	if ( ( (s.g_x + s.g_w > gl_width) && (d.g_x < s.g_x) )  ||
	     ( (s.g_y + s.g_h > gl_height) && (d.g_y < s.g_y) )   )
	{
	  rc_union(&s, &d);
	  *pstop = DESKWH;
	}
	else
	{
	  *pstop = w_handle;
	}
						/* intersect with full	*/
						/*   screen and align	*/
						/*   fringes if -1 xpos	*/
	sminus1 = w_mvfix(&s, &d);
	dminus1 = w_mvfix(&d, &s);
						/* blit what we can	*/
	if ( *pstop == w_handle )
	{
	  gsx_sclip(&gl_rfull);
	  bb_screen(S_ONLY, s.g_x, s.g_y, d.g_x, d.g_y, s.g_w, s.g_h);
						/* cleanup left edge	*/
	  if (sminus1 != dminus1)
	  {
	    if (dminus1)
	      s.g_x--;
	    if (sminus1)
	    {
	      d.g_x--;
	      d.g_w = 1;
	      gsx_sclip(&d);
	      w_clipdraw(gl_wtop, 0, 0, 0);
	    }
	  }
	  pc = &s;
	}
	else
	{
	  pc = &d;
	}
						/* clean up the rest	*/
						/*   by returning	*/
						/*   clip rect		*/
	rc_copy(pc, prc);
	return( (*pstop == w_handle) );
}


/*
*	Draw windows from top to bottom.  If top is 0, then start at
*	the topmost window.  If bottom is 0, then start at the 
*	bottomost windwo.  For the first window drawn, just do the
*	insides, since DRAW_CHANGE has already drawn the outside
*	borders.
*/

VOID w_update(REG WORD bottom, REG GRECT *pt, REG WORD top, 
		WORD moved, WORD usetrue)
{
	REG WORD	i, ni;
	REG WORD	done;	

	ems_get_wtree(ROOT, &D.g_tree0);
						/* limit to screen	*/
	rc_intersect(&gl_rfull, pt);
	gsx_moff();
						/* update windows from	*/
						/*   top to bottom	*/
	if (bottom == DESKWH)
	{
		bottom = D.g_tree0.ob_head;
	}
						/* if there are windows	*/
	if (bottom != NIL)
	{
						/* start at the top	*/ 
	  if (top == DESKWH) 
	  {
		top = D.g_tree0.ob_tail;
	  }
						/* draw windows from	*/
						/*   top to bottom	*/
	  do
	  {
						
	    if ( !((moved) && (top == gl_wtop)) )
	    {
						/* set clip and draw	*/
						/*   a window's border	*/
	      gsx_sclip(pt);
	      					/* CHANGED 1/10/86 LKW	*/
/*	      w_clipdraw(top, 0, MAX_DEPTH, 2);	!* from FALSE to 2	*!
*/
	      w_cpwalk(top, 0, MAX_DEPTH, usetrue);
						/* let appl. draw inside*/
	      w_redraw(top, pt);
	    }
						/* scan to find prev	*/
	    i = bottom;
	    done = (i == top);
	    while (i != top)
	    {
	      ems_get_wtree(i, &D.g_tree0);
	      ni = D.g_tree0.ob_next;
	      if (ni == top)
		top = i;
	      else
		i = ni;
	    }
	  }
	  while( !done );
	}
	gsx_mon();
}

#if SINGLAPP
VOID w_setmen(WORD pid)
{
	WORD		npid;

	npid = menu_tree[pid] ? pid : 0;
	if ( gl_mntree != menu_tree[npid] )
	  mn_bar(menu_tree[npid], TRUE, npid);

	npid = desk_tree[pid] ? pid : 0;
	if (gl_newdesk != desk_tree[npid] )
	{
	  gl_newdesk = desk_tree[npid];
	  gl_newroot = desk_root[npid];
	  w_drawdesk(&gl_rscreen);
	}
}

/*
*	Routine to draw menu of top most window as the current menu bar.
*/
VOID w_menufix()
{
	WORD		pid;

	ems_get_window(w_top(), &D.w_win0);
	pid = D.w_win0.w_owner->p_pid;
	w_setmen(pid);
}
#endif

#if MULTIAPP
VOID w_setmen(WORD pid)
{
	GRECT		c;
	WORD		npid;
	LONG 		tree;
	
			/* code to find best available menu goes here */
	npid = pid;
	tree = menu_tree[npid];
	
	if (sh[npid].sh_isacc && !tree)
	{
	  tree = gl_mntree;
	  npid = gl_mnppd->p_pid;
	}
	if (!tree)
	{
	  tree = ad_sysmenu;
	  npid = 1; /* scrmgr */
	}
	
	mn_bar(tree, TRUE, npid);

	if (desk_tree[npid])
	{
	  gl_newdesk = desk_tree[npid];
	  gl_newroot = desk_root[npid];
	}
	else
	  gl_newdesk = 0x0L;

	w_drawdesk(&gl_rscreen);
}


	VOID
w_newmenu(owner)
	PD 	*owner;
{
	gl_newmenu = owner;
}

/*
*	Routine to draw menu of top most window as the current menu bar.
*/
VOID w_menufix(PD *rlr)
{
	WORD		pid;

	if (gl_newmenu && (rlr == gl_newmenu))  /* is there a possible menu */
	{					/* change and is the new ap */
	   gl_newmenu = (PD *)0x0;		/* loaded		    */
	   w_setmen(rlr->p_pid);
	}
}

WORD w_clswin()
{
	WORD		i;
						/* close any open winds	*/
	wm_update(TRUE);

	for(i=1; i < NUM_WIN; i++)
	{
	  ems_get_window(i, &D.w_win0);
	  if (D.w_win0.w_owner == rlr)
	  {
	    if (D.w_win0.w_flags & VF_INTREE)
	      ob_emsdelete(i);
	    wm_delete(i);
	  }
	}
	wm_update(FALSE);
}

/*
*	Routine to take all windows belonging to a particular
*	process, save them on a list, and delete them from
*	the window tree.
*	IF deletions first,
*/
VOID oldwfix(PD *npd, WORD isdelete)
/* PD = pd of old process 	*/
{
	WORD		ii, next;
	PD		*owner;

	ems_get_wtree(ROOT, &D.g_tree0);
	for(ii = D.g_tree0.ob_head; ii > ROOT; ii=next)
	{
	  ems_get_wtree(ii, &D.g_tree0);
	  next = D.g_tree0.ob_next;
	  ems_get_window(ii, &D.w_win0);
	  owner = D.w_win0.w_owner;
				/* if not an acc or dosnext	   */
				/* then make window inactive      */
	  if ( (!sh[owner->p_pid].sh_isacc) &&
	       (sh[owner->p_pid].sh_isgem) &&
	       !(D.w_win0.w_flags & VF_KEEPWIN) )
	  {
	    if (isdelete)
	    {
	      ob_emsdelete(ii);
	      w_emsobadd(INACROOT, ii);
  	      D.w_win0.w_flags &= ~VF_INTREE;
	      ems_put_window(ii, &D.w_win0);
	    }
	    else
	      ap_sendmsg(proc_msg, WM_UNTOPPED, owner, ii, 0, 0, 0, 0);
	  }
	}
}



VOID newwfix(PD *npd) /* pd of new process 	*/
{
	WORD		ii, next;

	ems_get_wtree(INACCROOT, &D.g_tree0);
	for(ii=D.g_tree0.ob_head; (ii != INACROOT) && (ii != NIL); ii=next)
	{
	  ems_get_wtree (ii, &D.g_tree0);
	  ems_get_window(ii, &D.g_win0);
	  next = D.g_tree0.ob_next;
	  if(D.w_win0.w_owner == npd)
	  {
	    D.w_win0.w_flags |= VF_INTREE;
	    ob_emsdelete(ii);
	    w_emsobadd(ROOT, ii);
	  }
	}
}

	WORD
w_windfix(npd)
	PD		*npd;		/* pd of new process 	*/
{
	WORD		ii, jj;
	WORD		wh, old;

/* to send all untopped	*/
	oldwfix(npd, FALSE);
	for (ii=0; ii<NUM_ACCS; ii++)
	  dsptch();
/* */
	oldwfix(npd, TRUE);
	if (npd)
	  newwfix(npd);

	ems_get_wtree(0, &D.g_tree0);
	gl_wtop = D.g_tree0.ob_tail;
	for (ii=0; ii<NUM_ACCS; ii++)
	  dsptch();

						/* if not a dos app	*/
						/* don't draw borders	*/
	if (! sh[npd->p_pid].sh_isgem)
	  return(TRUE);
	else
	  gl_newmenu = npd;

	wm_update(TRUE);
	ems_everyobj(ROOT, NIL, newrect, 0, 0, MAX_DEPTH);
	wh = gl_wtop;
	gsx_sclip(&gl_rfull);
	ob_emsdraw(ROOT, 0);
	w_setactive();
	wm_update(FALSE);

	for (ii=0; ii<NUM_ACCS; ii++)
	  dsptch();
	wm_update(TRUE);
	while (wh != NIL)
	{
	  w_cpwalk(wh, 0, MAX_DEPTH, TRUE);
	  w_redraw(wh, &gl_rfull);
	  old=wh;
	  ems_get_wtree(ROOT, &D.g_tree0);
	  wh=D.g_tree0.ob_head;
	  if (wh==old)
	    break;	  
	  D.g_tree0.ob_next = D.g_tree0.ob_head;
	  do
	  {
		wh = D.g_tree0.ob_next;
		ems_get_wtree(wh, &D.g_tree0);
	  } while (D.g_tree0.ob_next != old);
	}
	gsx_mfset(ad_armice);
	wm_update(FALSE);
	return(TRUE);
}
#endif

/*
*	Draw the tree of windows given a major change in the some 
*	window.  It may have been sized, moved, fulled, topped, or closed.
*	An attempt should be made to minimize the amount of
*	redrawing of other windows that has to occur.  W_REDRAW()
*	will actually issue window redraw requests based on
*	the rectangle that needs to be cleaned up.
*/

WORD draw_change(REG WORD w_handle, REG GRECT *pt)
{
	GRECT		c, pprev;
	REG GRECT	*pw;
	REG WORD	start;
	WORD		stop, moved, topflags;
	WORD		oldtop, clrold, diffbord, wasclr;
/*	
	TRACE("draw_change w_handle=$");
	ophex(w_handle);
	TRACE(" rect=($");
	ophex(pt->g_x);
	TRACE(",$");
	ophex(pt->g_y);
	TRACE(",$");
	ophex(pt->g_w);
	TRACE(",$");
	ophex(pt->g_h);
	TRACE(")\r\n$");
*/
	ems_get_window(w_handle, &D.w_win0);

	wasclr = !(D.w_win0.w_flags & VF_BROKEN);
						/* save old size	*/
	w_getsize(WS_CURR, w_handle, &c);
	w_setsize(WS_PREV, w_handle, &c);
						/* set new size's	*/
	//TRACE("draw_change: Setting size\r\n$");
	w_setsize(WS_CURR, w_handle, pt);
	pw = (GRECT *)w_getxptr(WS_WORK, w_handle); /* populates D.w_win0 */
	wm_calc(WC_WORK, D.w_win0.w_kind, 
			pt->g_x, pt->g_y, pt->g_w, pt->g_h, 
			&pw->g_x, &pw->g_y, &pw->g_w, &pw->g_h);
	/*TRACE("draw_change: Work area is ($");
	ophex(pw->g_x);
	TRACE(",$");
	ophex(pw->g_y);
	TRACE(",$");
	ophex(pw->g_w);
	TRACE(",$");
	ophex(pw->g_h);
	TRACE(")\r\n$"); */
	/* Write back changes */
	ems_put_window(w_handle, &D.w_win0);	
	/* And do this to confirm that it took. */
	w_getxptr(WS_WORK, w_handle); 

	//TRACE("Calling ems_everyobj\r\n$");
						/* update rect. lists	*/
	ems_everyobj(ROOT, NIL, newrect, 0, 0, MAX_DEPTH);
						/* remember oldtop	*/
	oldtop = gl_wtop;
	ems_get_wtree(ROOT, &D.g_tree0);
	gl_wtop = D.g_tree0.ob_tail;
						/* if new top then	*/
						/*   change men		*/
	if (gl_wtop != oldtop)
#if SINGLAPP
	  w_menufix();
#endif
#if MULTIAPP
	{
	  ems_get_window(gl_wtop, &D.g_win1);
	  if (gl_wtop == NIL) w_setmen(1);
	  else w_newmenu(D.g_win1.w_owner);
	}
#endif
	//wswork();
						/* set ctrl rect and	*/
						/*   mouse owner	*/
	w_setactive();
	//wswork();
						/* init. starting window*/
	start = w_handle;
						/* stop at the top	*/
	stop = DESKWH;
						/* set flag to say we	*/
						/*   haven't moved 	*/
						/*   the top window	*/
	moved = FALSE;
						/* if same upper left	*/
						/*   corner and not	*/
						/*   zero size window	*/
						/*   then its a size or	*/
						/*   top request, else	*/
						/*   its a move, grow,	*/
						/*   open or close.	*/
	//wswork();
	if ( (!rc_equal(&gl_rzero, pt)) &&
	      (pt->g_x == c.g_x) && 
	      (pt->g_y == c.g_y) )
	{
						/* size or top request	*/
	  if ( (pt->g_w == c.g_w) && (pt->g_h == c.g_h) )
	  {
	  					/* sizes of prev and 	*/
						/*  current are the same*/
						/*  so its a top request*/

						/* return if this isn't	*/
						/*   a top request 	*/
	    ems_get_wtree(ROOT, &D.g_tree0);
	    if ( (w_handle != D.g_tree0.ob_tail) ||
		 (w_handle == oldtop) )

	      goto finish; //return(TRUE);
						/* say when borders will*/
						/*   change		*/
	//wswork();
	    ems_get_window(gl_wtop, &D.w_win1);
	    topflags = D.w_win1.w_flags;
	    ems_get_window(oldtop, &D.w_win1);
	    diffbord = D.w_win1.w_cotop==gl_wtop ? FALSE : 
			  !( (D.w_win1.w_flags & VF_SUBWIN) &&
		          (topflags & VF_SUBWIN) );
						/* draw oldtop covered	*/
						/*   with deactivated 	*/
						/*   borders		*/
	//wswork();
	    if (oldtop != NIL)
	    {
	      if (diffbord)
	      {
		 w_clipdraw(oldtop, 0, MAX_DEPTH, 2);
		 if (D.w_win1.w_cotop != NIL)
		 {
		      w_clipdraw(D.w_win1.w_cotop, 0, MAX_DEPTH, 2);
	        }
	      }
	      clrold = !(D.w_win1.w_flags & VF_BROKEN);
	    }
	    else
	      clrold = TRUE;
						/* if oldtop isn't 	*/
						/*   overlapped and new	*/
						/*   top was clear then	*/
						/*   just draw activated*/
						/*   borders		*/
	    if ( (clrold) && 
		 (wasclr) )
	    {
	      w_clipdraw(gl_wtop, 0, MAX_DEPTH, 1);
	      goto finish; //return(TRUE);
	    }
	  }
	  else
	  					/* size change		*/
	  {
						/* stop before current	*/
						/*   window if shrink	*/
						/*   was a pure subset	*/
	//wswork();
	    if ( (pt->g_w <= c.g_w) && (pt->g_h <= c.g_h) )
	    {
	      stop = w_handle;
	      w_clipdraw(gl_wtop, 0, MAX_DEPTH, 2);
	      moved = TRUE;
	    }
						/* start at bottom if	*/
						/*   a shrink occurred	*/
	    if ( (pt->g_w < c.g_w) || (pt->g_h < c.g_h) )
	      start = DESKWH;
						/* update rect. is the	*/
						/*   union of two sizes	*/
						/*   + the drop shadow	*/
	    c.g_w = max(pt->g_w, c.g_w) + DROP_SIZE; 
	    c.g_h = max(pt->g_h, c.g_h) + DROP_SIZE; 
	  }
	}
	else
	{
						/* move or grow or open	*/
						/*   or close		*/
	//wswork();
	  if ( !(c.g_w && c.g_h) ||
		( (pt->g_x <= c.g_x) && 
		  (pt->g_y <= c.g_y) &&
		  (pt->g_x+pt->g_w >= c.g_x+c.g_w) && 
		  (pt->g_y+pt->g_h >= c.g_y+c.g_h)))
	  {
						/* a grow that is a 	*/
						/*  superset or an open	*/
	    rc_copy(pt, &c);
	  }
	  else
	  {
						/* move, close or shrink*/
						/* do a move of top guy	*/
	    if ( (pt->g_w == c.g_w) && 
		 (pt->g_h == c.g_h) &&
		 (gl_wtop == w_handle) )
	    {
	//wswork();
	      moved = w_move(w_handle, &stop, &c);
	      start = DESKWH;
	    }
						/* check for a close	*/
	    if ( !(pt->g_w && pt->g_h) )
	      start = DESKWH;
						/* handle other moves	*/
						/*   and shrinks	*/
	    if ( start != DESKWH )
	    {
	      rc_union(pt, &c);
	      if ( !rc_equal(pt, &c) )
	        start = DESKWH;
	    }
	  }
	}
						/* update gl_wtop	*/
						/*   after close,	*/
						/*   or open		*/
	//wswork();
	ems_get_wtree(ROOT, &D.g_tree0);
	if ( oldtop != D.g_tree0.ob_tail )
	{
	  if (gl_wtop != NIL)
	  {
						/* open or close with	*/
						/*   other windows open	*/
	    w_getsize(WS_CURR, gl_wtop, pt);
	    rc_union(pt, &c);
						/* if it was an open	*/
						/*   then draw the	*/
						/*   old top guy	*/
						/*   covered		*/
	    if ( (oldtop != NIL ) &&
		 (oldtop != w_handle) )
	    {
	    					/* BUGFIX 2/20/86 LKW	*/
						/* only an open if prev	*/
						/*  size was zero.	*/
	      w_getsize(WS_PREV, gl_wtop, &pprev);
	      if ( !rc_equal( &pprev, &gl_rzero) )
	      {
	        w_clipdraw(oldtop, 0, MAX_DEPTH, 2);	/* */
		ems_get_window(oldtop, &D.w_win1);
		if ( D.w_win1.w_cotop != NIL)
		{
		  w_clipdraw(D.w_win1.w_cotop, 0, MAX_DEPTH, 2);
		}
	      }
	    }
	  }
	}
#if DROP_SIZE
	c.g_w += DROP_SIZE;		/* account for drop shadow*/
	c.g_h += DROP_SIZE;		/* BUGFIX in 2.1	*/
#endif						
	//wswork();
						/* update the desktop	*/
						/*   background		*/
	if (start == DESKWH)
	  w_drawdesk(&c);
	//wswork();

						/* start the redrawing	*/
	w_update(start, &c, stop, moved, TRUE);
finish:
	//wswork();
	return TRUE;
}


/*
*	Walk down ORECT list looking for the next rect that still has
*	size when clipped with the passed in clip rectangle.
*/
VOID	w_owns(WORD w_handle, WORD npo, GRECT *pt, REG WORD *poutwds)
{
	int found = 0;

	ems_get_window(w_handle, &D.w_win0);
	while (npo)
	{
	  ems_get_orect(npo, &D.g_olist0);
	  rc_copy((GRECT *)&D.g_olist0.o_x, (GRECT *)&poutwds[0]);
	  D.w_win0.w_nrnext = npo = D.g_olist0.o_link;
	  ems_put_window(w_handle, &D.w_win0);

	  if ( (rc_intersect(pt, (GRECT *)&poutwds[0])) &&
	       (rc_intersect(&gl_rfull, (GRECT *)&poutwds[0]))  )
	  {
		found = 1;
		break;
	  }
	}
	if (!found) poutwds[2] = poutwds[3] = 0;
}


/*
*	Walk down ORECT list and accumulate the union of all the owner
*	rectangles.
*/
WORD	w_union(WORD npo, GRECT *pt)
{
	ORECT po;

	if (!npo) return(FALSE);

	ems_get_orect(npo, &po);
	rc_copy((GRECT *)&po.o_x, pt);
	//ems_put_orect,(npo, &po);

	npo = po.o_link;
	while (npo)
	{
	  ems_get_orect(npo, &po);
	  rc_union((GRECT *)&po.o_x, pt);
	  //ems_put_orect,(npo, &po);
	  npo = po.o_link;
	}
	return(TRUE);
}



/*
*	Start the window manager up by initializing internal variables.
*/
VOID wm_start()
{
	REG WORD	i;
	ORECT		po;
	WORD		npo;
	REG LPTREE	tree;
	PD		*ppd;

#if MULTIAPP
	mn_init();
#endif
						/* init default owner	*/
						/*  to be screen mgr.	*/
	ppd = fpdnm(NULLPTR, SCR_MGR);
						/* init owner rects.	*/
	or_start();
						/* init window extent	*/
						/*   objects		*/
	bfill(sizeof(OBJECT), 0, &D.g_tree0);
	for (i = 0; i < NUM_MWIN; i++)
	{
		ems_put_wtree(i, &D.g_tree0);
	}
	ems_nilit(NUM_MWIN);

	for(i=0; i<NUM_MWIN; i++)
	{
		ems_get_window(i, &D.w_win0);
		ems_get_wtree (i, &D.g_tree0);
		D.w_win0.w_flags  = 0;
		D.w_win0.w_nrlist = 0;
		D.g_tree0.ob_type = G_IBOX;
#if MULTIAPP
	  W_NAMES[i][0] = 0;
#endif
		ems_put_window(i, &D.w_win0);
		ems_put_wtree(i, &D.g_tree0);
	}

	ems_get_wtree (ROOT, &D.g_tree0);
	D.g_tree0.ob_type = G_BOX;
	tree = ad_stdesk;

#if 1	/* RSF: No longer needed */
	/* 910327WHF: modify standard desktop tree to use color categories */
	tree->ob_flags |= USECOLORCAT ;
			/* the assignment below is byte order dependent */
	*((BYTE FAR *)(&(tree+ROOT)->ob_spec)) = CC_DESKTOP ;
	D.g_tree0.ob_flags = USECOLORCAT;
	/* 910327WHF: end */
#endif
	D.g_tree0.ob_spec = tree[ROOT].ob_spec;

#if EMSDESK	
	if (gl_ems_avail && gl_emm_inuse)	/* Desktop image loaded in EMS */
	{
		D.g_tree0.ob_type = tree[ROOT].ob_type = G_DTMFDB;
		D.g_tree0.ob_spec = tree[ROOT].ob_spec = DTMFDB_EMS_SPEC; 
	}
#endif
	ems_put_wtree (ROOT, &D.g_tree0);

						/* init window element	*/
						/*   objects		*/
	bfill(NUM_ELEM * sizeof(OBJECT), 0, &W_ACTIVE[ROOT]);
	w_nilit(NUM_ELEM, &W_ACTIVE[0]);
	for(i=0; i<NUM_ELEM; i++)
	{
	  W_ACTIVE[i].ob_type  = gl_watype[i];
	  W_ACTIVE[i].ob_spec  = gl_waspec[i];
	  W_ACTIVE[i].ob_flags = gl_waflag[i]; /*910326WHF*/
	}
// Commented out in ViewMAX
//	
//	W_ACTIVE[ROOT].ob_state = SHADOWED;
						/* init rect. list	*/
	ems_get_window(0, &D.w_win0);
	D.w_win0.w_nrlist = npo = get_orect();
	ems_put_window(0, &D.w_win0);
	ems_get_orect(npo, &po);
	po.o_link = 0;
	po.o_x = XFULL;
	po.o_y = YFULL;
	po.o_w = WFULL;
	po.o_h = HFULL;
	ems_put_orect(npo, &po);
	w_setup(ppd, DESKWH, NONE);
	w_setsize(WS_CURR, DESKWH, &gl_rscreen);
	w_setsize(WS_PREV, DESKWH, &gl_rscreen);
	w_setsize(WS_FULL, DESKWH, &gl_rfull);
	w_setsize(WS_WORK, DESKWH, &gl_rfull);
						/* init global vars	*/
	gl_wtop = NIL;
	gl_awind = (LPTREE)ADDR(&W_ACTIVE[0]);
	gl_newdesk = 0x0L;
						/* init tedinfo parts	*/
						/*   of title and info	*/
						/*   lines		*/
	movs(sizeof(TEDINFO), &gl_asamp, &gl_aname);
	movs(sizeof(TEDINFO), &gl_asamp, &gl_ainfo);
	gl_aname.te_just = TE_CNTR;
	gl_aname.te_color = (sys_fg&0xFF80)+CC_NAME;	/*910328WHF*/
	gl_ainfo.te_color = (syi_fg&0xFF80)+CC_INFO;
	
	W_ACTIVE[W_NAME].ob_spec = (LONG)ADDR(&gl_aname);
	W_ACTIVE[W_INFO].ob_spec = (LONG)ADDR(&gl_ainfo);
}


/*
*	Allocates a window for the calling application of the appropriate
*	size and returns a window handle.
*
*/

WORD wm_create(WORD kind, GRECT *pt)
{
	REG WORD	i;
	WORD rv = -1;

	//TRACE("wm_create: $");

	for(i = 0 ; i < NUM_WIN; i++)
	{
		ems_get_window(i, &D.w_win0);
		if (!(D.w_win0.w_flags & VF_INUSE)) break;
	}
	if ( i < NUM_WIN )
	{
	  w_setup(rlr, i, kind);
	  w_setsize(WS_CURR, i, &gl_rzero);
	  w_setsize(WS_PREV, i, &gl_rzero);
	  w_setsize(WS_FULL, i, pt);
	  rv = i;
	}
	//ophex(rv);
	//TRACE("\r\n$");
	return rv;
}


/*
*	Opens or closes a window.
*/
VOID wm_opcl(REG WORD wh, REG GRECT *pt, WORD isadd)
{
 	GRECT		t;
/*
	TRACE("wm_opcl wh=$");
	ophex(wh);
	TRACE(" isadd=$");
	ophex(isadd);
	TRACE("\r\n$");
*/
	rc_copy(pt, &t);
	wm_update(TRUE);

	if (isadd)
	{
	  ems_get_window(wh, &D.w_win0);
	  D.w_win0.w_flags |= VF_INTREE;
	  ems_put_window(wh, &D.w_win0);
	  w_emsobadd(ROOT, wh);
	}
	else
	{
	  ob_emsdelete(wh);
	  ems_get_window(wh, &D.w_win0);
	  D.w_win0.w_flags &= ~VF_INTREE;
	  ems_put_window(wh, &D.w_win0);
	}

	draw_change(wh, &t);
	if (isadd)
	  w_setsize(WS_PREV, wh, pt);
	wm_update(FALSE);
}

/*
*	Opens a window from a created but closed state.
*/
	VOID
wm_open(w_handle, pt)
	WORD		w_handle;
	GRECT		*pt;
{
//	TRACE("wm_open: $");
//	ophex(w_handle);
//	TRACE("\r\n$");
	wm_opcl(w_handle, pt, TRUE);
}


/*
*	Closes a window from an open state.
*/

	VOID
wm_close(w_handle)
	WORD		w_handle;
{
//	TRACE("wm_closw: $");
//	ophex(w_handle);
//	TRACE("\r\n$");
	wm_opcl(w_handle, &gl_rzero, FALSE);
}


/*
*	Frees a window and its handle up for use by 
*	by another application or by the same application.
*/

VOID wm_delete(WORD w_handle)
{
	ems_newrect(w_handle, 0, 0);		/* give back recs.	*/
	w_setsize(WS_CURR, w_handle, &gl_rscreen);
	w_setsize(WS_PREV, w_handle, &gl_rscreen);
	w_setsize(WS_FULL, w_handle, &gl_rfull);
	w_setsize(WS_WORK, w_handle, &gl_rfull);
	ems_get_window(w_handle, &D.w_win0);

	D.w_win0.w_flags = 0x0;	/*&= ~VF_INUSE;	*/
	D.w_win0.w_owner = (PD *)NULLPTR;
#if MULTIAPP
	W_INACTIVE[w_handle] = NULLPTR;
#endif
	ems_put_window(w_handle, &D.w_win0);
}


/*
*	Gives information about the current window to the application
*	that owns it.
*/
VOID wm_get(REG WORD w_handle, WORD w_field, REG WORD *poutwds)
{
	REG WORD	which;
	GRECT		t;
	WORD		npo;

/*
#if DEBUG
	TRACE("wm_get ");
	ophex(w_handle); TRACE(", ");
	ophex(w_field); TRACE("\r\n");
#endif
*/
	which = -1;
	ems_get_window(w_handle, &D.w_win0);

	switch(w_field)
	{
	  case WF_WXYWH:
		which = WS_WORK;
		break;
	  case WF_CXYWH:
		which = WS_CURR;
		break;
	  case WF_PXYWH:
		which = WS_PREV;
		break;
	  case WF_FXYWH:
		which = WS_FULL;
		break;
	  case WF_HSLIDE:
		poutwds[0] = D.w_win0.w_hslide;
		break;
	  case WF_VSLIDE:
		poutwds[0] = D.w_win0.w_vslide;
		break;
	  case WF_HSLSIZ:
		poutwds[0] = D.w_win0.w_hslsiz;
		break;
	  case WF_VSLSIZ:
		poutwds[0] = D.w_win0.w_vslsiz;
		break;
	  case WF_TOP:
		poutwds[0] = w_top();
		break;
	  case WF_FIRSTXYWH:
	  case WF_NEXTXYWH:
	        w_getsize(WS_WORK, w_handle, &t);
		npo = (w_field == WF_FIRSTXYWH) ? D.w_win0.w_nrlist : 
						  D.w_win0.w_nrnext;
		w_owns(w_handle, npo, &t, &poutwds[0]);
		break;
	  case WF_SCREEN:
		gsx_mret((LPBYTE *)&poutwds[0], (LONG *)&poutwds[2]);
		break;
	  case WF_TATTRB:
		poutwds[0] = D.w_win0.w_flags >> 3;
		break;

/* [JCE 23-8-1999] Allow dynamic modification of the window gadgets */
		
	  case WF_OBFLAG:
	  	if (w_handle == W_SIZER) poutwds[0] = sizer_flag;
		else                     poutwds[0] = gl_waflag[w_handle];
		break;

	  case WF_OBTYPE:
	    poutwds[0] = gl_watype[w_handle];
	    break;

	  case WF_OBSPEC:
		if (w_handle == W_SIZER)
		{
			poutwds[0] = FP_OFF(sizer_spec);
			poutwds[1] = FP_SEG(sizer_spec);
			break;
		}
	    poutwds[0] = FP_OFF(gl_waspec[w_handle]);
	    poutwds[1] = FP_SEG(gl_waspec[w_handle]);
	    break;
	}
	if (which != -1)
	  w_getsize(which, w_handle, (GRECT *)&poutwds[0]);
}

WORD wm_gsizes(WORD w_field, WORD *psl, WORD *psz)
{
	if ( (w_field == WF_HSLSIZ) ||
	     (w_field == WF_HSLIDE) )
	{
	  *psl = W_ACTIVE[W_HELEV].ob_x;
	  *psz = W_ACTIVE[W_HELEV].ob_width; 
	  return(W_HBAR);
	}
	if ( (w_field == WF_VSLSIZ) ||
	     (w_field == WF_VSLIDE) )
	{
	  *psl = W_ACTIVE[W_VELEV].ob_y;
	  *psz = W_ACTIVE[W_VELEV].ob_height; 
	  return(W_VBAR);
	}
	return(0);
}


/*
*	Routine to top a window and then make the right redraws happen
*/
	VOID
wm_mktop(w_handle)
	REG WORD	w_handle;
{
	GRECT		t, p;

	if ( w_handle != gl_wtop )
	{
	  ob_emsorder(w_handle, NIL);
	  w_getsize(WS_PREV, w_handle, &p);
	  w_getsize(WS_CURR, w_handle, &t);
	  draw_change(w_handle, &t);
	  w_setsize(WS_PREV, w_handle, &p);
	}
}


/*
*	Allows application to set the attributes of
*	one of the windows that it currently owns.  Some of the
*	information includes the name, and the scroll bar elevator
*	positions. 
*/

VOID wm_set(WORD w_handle, WORD w_field, WORD *pinwds)
{
	WORD		which, liketop, i;
	WORD		osl, osz, nsl, nsz;
	WORD		wbar;
	LONG		lspec;
	GRECT		t;
	BOOLEAN		dirty = FALSE;

	/* [JCE 23-8-1999] Set window frame controls. The w_handle in these
	 * calls isn't a real window handle; so we don't do any of the
	 * editing below */
/*
	TRACE("wm_set (w_handle=$");
	ophex(w_handle);
	TRACE(", w_field=$");
	ophex(w_field);
	TRACE(", pinwds=$");
	ophex(pinwds[0]);
	TRACE(":$");
	ophex(pinwds[1]);
	TRACE(":$");
	ophex(pinwds[2]);
	TRACE(":$");
	ophex(pinwds[3]);
	TRACE(")\r\n$");
*/	
	switch(w_field)
	{
	  case WF_OBFLAG:
		if (w_handle > W_HELEV)
		{
			return;	/* Off the end of the tree */
		}
	  
	    if (w_handle == W_SIZER) sizer_flag = pinwds[0];
		gl_waflag[w_handle]          = 
		gl_waflag3d[w_handle]        = 
		gl_waflag2d[w_handle]        = 
		W_ACTIVE [w_handle].ob_flags = pinwds[0];
		return;
/*
 * Don't let the type be changed dynamically *
 *
 *	  case WF_OBTYPE:
 *	    gl_watype[w_handle]         = pinwds[0];
 *      W_ACTIVE [w_handle].ob_type = pinwds[0];
 *	    return;
 */
	  case WF_OBSPEC:
		if (w_handle > W_HELEV)
		{
			return;	/* Off the end of the tree */
		}
		lspec = (LONG)MK_FP(pinwds[1], pinwds[0]);
		gl_waspec[w_handle]         =
		gl_waspec3d[w_handle]       =
		gl_waspec2d[w_handle]       = 
		W_ACTIVE [w_handle].ob_spec = lspec;
	    if (w_handle == W_SIZER) sizer_spec = gl_waspec[W_SIZER];
	    return;
	}
	
	which = -1;
						/* grab the window sync	*/
	wm_update(TRUE);
	wbar = wm_gsizes(w_field, &osl, &osz); 

	/* [JCE] The meaningless little dance below gets us round what I assume 
	 *      is a bug in the Pacific optimiser; it tries to allocate "pinwds"
	 *      and "wbar" to the same register, with nasty consequences.
	 *
	 *      The optimiser optimises it away, so no extra code is generated;
	 *      but its presence makes the code generator store wbar on the 
	 *      stack rather than in a register, solving the problem. 
	 *       
	 */       
	{
		WORD *pData = &wbar;
		if (!*pData) ++pData;
	}

	ems_get_window(w_handle, &D.w_win0);
	ems_get_window(gl_wtop,  &D.w_win1);
	
	if (wbar)
	{
	  pinwds[0] = max(-1, pinwds[0]);
	  pinwds[0] = min(1000, pinwds[0]);
	}
	liketop = ( ( w_handle == gl_wtop ) || ( D.w_win0.w_cotop == gl_wtop) ||
		    ( D.w_win0.w_flags & VF_SUBWIN ) );
	
	switch(w_field)
	{
	  case WF_NAME:
		which = W_NAME;
		break;
	  case WF_INFO:
		which = W_INFO;
		break;
	  case WF_SIZTOP:
		ob_emsorder(w_handle, NIL);
						/* fall thru	*/
	  case WF_CXYWH:
		draw_change(w_handle, (GRECT *)&pinwds[0]);
		break;
	  case WF_COTOP:
		D.w_win1.w_cotop = w_handle;
		ems_put_window(gl_wtop, &D.w_win1);
		if (w_handle != NIL)
		{
				/* Fall thru and make this the new top */
			D.w_win0.w_cotop = gl_wtop;
			ems_put_window(w_handle, &D.w_win0);
		}
		else
			break;
	  case WF_TOP:
		if (w_handle != gl_wtop)
		{
		  ems_get_wtree(ROOT, &D.g_tree0);
		  for(i=D.g_tree0.ob_head; i>ROOT; i=D.g_tree0.ob_next)
		  {
		    ems_get_window(i, &D.w_win1);
		    if ( (i != w_handle) &&
		         (D.w_win1.w_owner == rlr) &&
		         (D.w_win1.w_flags & VF_SUBWIN) &&
		         (D.w_win0.w_flags & VF_SUBWIN) ) wm_mktop(i);
		    ems_get_wtree(i, &D.g_tree0);
		  }
		  wm_mktop(w_handle);
		}
		break;
	  case WF_NEWDESK:
		D.w_win0.w_owner = rlr;
		desk_tree[rlr->p_pid] = gl_newdesk = *(LPTREE *) &pinwds[0];
		desk_root[rlr->p_pid] = gl_newroot = pinwds[2];
		dirty = TRUE;
		break;
	  case WF_HSLSIZ:
		D.w_win0.w_hslsiz = pinwds[0];
		dirty = TRUE;
		break;
	  case WF_VSLSIZ:
		D.w_win0.w_vslsiz = pinwds[0];
		dirty = TRUE;
		break;
	  case WF_HSLIDE:
		D.w_win0.w_hslide = pinwds[0];
		dirty = TRUE;
		break;
	  case WF_VSLIDE:
		D.w_win0.w_vslide = pinwds[0];
		dirty = TRUE;
		break;
	  case WF_TATTRB:
		if (pinwds[0] & WA_SUBWIN)
		  D.w_win0.w_flags |= VF_SUBWIN;
		else
		  D.w_win0.w_flags &= ~VF_SUBWIN;
		if (pinwds[0] & WA_KEEPWIN)
		  D.w_win0.w_flags |= VF_KEEPWIN;
		else
		  D.w_win0.w_flags &= ~VF_KEEPWIN;
		dirty = TRUE;
		break;
	}
	/* Only write back the window if told to. Some things (like 
	 * changing CXYWH, for example) do their own thing with 
	 * w_win0, so we end up with a different window struct in there. 
	 * Reload the window struct otherwise. */
	if (dirty) ems_put_window(w_handle, &D.w_win0);
	else	   ems_get_window(w_handle, &D.w_win0);	
	if ( (wbar) &&
	     (liketop) )
	{
	  w_bldactive(w_handle);
	  wm_gsizes(w_field, &nsl, &nsz); 
	  if ( (osl != nsl) ||
	       (osz != nsz) ||
	       (D.w_win0.w_flags & VF_SUBWIN) )
	  {
	    w_getsize(WS_TRUE, w_handle, &t);
	    do_walk(w_handle, gl_awind, wbar + 3, MAX_DEPTH, &t);
	  }
	}
	if (which != -1)
	{
		w_strchg(w_handle, which, MK_FP(pinwds[1],pinwds[0]));
	}
	wm_update(FALSE); 		/* give up the sync	*/
}


/*
*	Given an x and y location this call will figure out which window
*	the mouse is in.
*/

	WORD
wm_find(x, y)
	WORD		x, y;
{
	return(	ob_emsfind(0, 2, x, y) );
}


/*
*	Locks or unlocks the current state of the window tree while an 
*	application is responding to a window update message in his message
*	pipe or is making some other direct screen update based on his current
*	rectangle list.
*/
	VOID
wm_update(beg_update)
	REG WORD	beg_update;
{

	if ( beg_update < 2)
	{
	  if ( beg_update )
	  {
	    if ( !tak_flag(&wind_spb) )
		    ev_block(MU_MUTEX, ad_windspb);

	  }
	  else
	    unsync(&wind_spb);
	}
	else
	{
	  beg_update -= 2;
	  fm_own( beg_update );
	}
}

/*
*	Given a width and height of a Work Area and the Kind of window
*	desired calculate the required window size including the 
*	Border Area.  or...  Given the width and height of a window
*	including the Border Area and the Kind of window desired, calculate
*	the result size of the window Work Area.
*/
VOID wm_calc(WORD wtype, REG UWORD kind, WORD x, WORD y, WORD w, WORD h, 
			 LPWORD px, LPWORD py, LPWORD pw, LPWORD ph)
{
	REG WORD	tb, bb, lb, rb;

	tb = bb = rb = lb = (gl_opts.frame3d) ? 4 : 1;

	if ( kind & (NAME | CLOSER | FULLER) ) 
		tb += (gl_hbox + (gl_opts.frame3d ? 4 : -1));
	if ( kind & INFO )	  				   
		tb += (gl_hbox - 1);

	if ( kind & (UPARROW | DNARROW | VSLIDE | SIZER) )
	  rb += (gl_wbox + (gl_opts.frame3d ? 4 : - 1));
	if ( kind & (LFARROW | RTARROW | HSLIDE | SIZER) )
	  bb += (gl_hbox + (gl_opts.frame3d ? 4 : - 1));
						/* negate values to calc*/
						/*   Border Area	*/
	if ( wtype == WC_BORDER )
	{
	  lb = -lb;
	  tb = -tb;
	  rb = -rb;
	  bb = -bb;						
	}
	*px = x + lb;
	*py = y + tb;
	*pw = w - lb - rb;
	*ph = h - tb - bb;
}



/*----------------------------------------------------------------------
*	This routine changes the background image as requested.  
*	If bmp is null, then we revert to a simple colored background.
*/
WORD change_desktop( FDB FAR * bmp )
{
	LPTREE	tree;
	GRECT	t;
	WORD	done = 0;
	
	tree = ad_stdesk;

	ems_get_wtree(ROOT, &D.g_tree0);
	if (bmp)
	{
#if EMSDESK
		/* If EMS is available, pop the image in there */
		if (gl_ems_avail)
		{
			WORD pg, npgs, hndl;
			FDB  FAR *emm_fdb;
			LONG bmsize = (2L * (long)bmp->fd_wdwidth * 
			                   (long)bmp->fd_h * bmp->fd_nplanes)
			            + sizeof(FDB);

			npgs = (bmsize + 16383L) / 16384;
			
			if (tree[ROOT].ob_spec == DTMFDB_EMS_SPEC)
			{
				EMS_Free( gl_emm_handle );
				tree[ROOT].ob_spec = 0;
				tree[ROOT].ob_type = G_BOX;
				gl_emm_inuse  = 0;
				gl_emm_handle = 0;
			}
			
			if ((npgs < gl_emm_ppgs) &&
			    (npgs < gl_emm_lpgs) &&
			    (hndl = EMS_Alloc(npgs)))
			{
				for (pg = 0; pg < npgs; pg++)
				{
					if (!EMS_Map(hndl, pg, pg)) { npgs = 0; break; }
				}
				gl_emm_handle = hndl;
				gl_emm_inuse = npgs;
				if (npgs)
				{
					emm_fdb = (FDB FAR *)MK_FP(gl_emm_seg, 0);
					
					LBCOPY(emm_fdb,     bmp,          sizeof(FDB));
					LBCOPY(&emm_fdb[1], bmp->fd_addr, bmsize - sizeof(FDB));
					emm_fdb->fd_addr = MK_FP(gl_emm_seg, sizeof(FDB));
					/* Got it */
					tree[ROOT].ob_spec   = DTMFDB_EMS_SPEC;
					tree[ROOT].ob_type   = G_DTMFDB;
					D.g_tree0.ob_type = G_DTMFDB;
					done = 1;
				}
			}
		}
#endif
		if (!done)
		{
			(tree+ROOT)->ob_type = G_DTMFDB;
			(tree+ROOT)->ob_spec = (LONG)bmp;
			D.g_tree0.ob_type = G_DTMFDB;
		}
	}
	else
	{
		if ((tree+ROOT)->ob_type == G_DTMFDB)
		{
			(tree+ROOT)->ob_spec = 0L;
		}

		/* revert to colored background */
		(tree+ROOT)->ob_type = G_BOX;
		D.g_tree0.ob_type = G_BOX;

		tree->ob_flags |= USECOLORCAT ;
			/* the assignment below is byte order dependent */
		*((BYTE FAR *)(&(tree+ROOT)->ob_spec)) = CC_DESKTOP ;
		D.g_tree0.ob_flags = USECOLORCAT;
	}
	
	D.g_tree0.ob_spec = (tree+ROOT)->ob_spec ;
	ems_put_wtree(ROOT, &D.g_tree0);	
	LBCOPY( (char far *)&t, (char far *)&tree->ob_x, sizeof(GRECT));
	w_drawdesk(&t);

    return done;
}

/* gemwmlib.c */

