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

GLOBAL LPTREE	gl_newdesk;			/* current desktop back-*/
						/* ground pattern.	*/
GLOBAL WORD	gl_newroot;			/* current object w/in	*/
						/* gl_newdesk.		*/
GLOBAL LPTREE	desk_tree[NUM_PDS];		/* list of object trees	*/
						/* for the desktop back-*/
						/* ground pattern.	*/
GLOBAL WORD	desk_root[NUM_PDS];		/* starting object to	*/
						/* draw within desk_tree.*/

GLOBAL OBJECT	W_ACTIVE[NUM_ELEM];

#define WSTRSIZE 81
#if MULTIAPP
GLOBAL BYTE	W_NAMES[NUM_WIN][WSTRSIZE];
GLOBAL BYTE	W_INFOS[NUM_WIN][WSTRSIZE];
GLOBAL PD	*W_INACTIVE[NUM_WIN];		/* pds of inactive windows*/
GLOBAL PD	*gl_newmenu = (PD *)0x0;
GLOBAL PD	*gl_lastnpd;
GLOBAL WORD	proc_msg[8];
#endif

//WORD w_union(REG ORECT *po, REG GRECT *pt);
/*
static void epo(int orect, ORECT *o, int line)
{
        TRACE("ems_put_orect gemwmlib.c: $");
        ophex(line);
        ems_put_orect(orect, o);
}


#define ems_put_orect(a,b) epo(a,b,__LINE__)

static void epw(int win, WINDOW *w, int line)
{
        TRACE("ems_put_window gemwm1.c: $");
        ophex(line);
	TRACE("\r\n$");
        ems_put_window(win, w);
}
#define ems_put_window(a,b) epw(a,b, __LINE__)



void wsw(char *f, int line);
#define wswork() wsw(__FILE__,__LINE__)
*/


GLOBAL WORD	gl_watype[NUM_ELEM] =
{
	G_IBOX,		/* W_BOX	*/
	G_BOX,      	/* W_TITLE	*/
	G_BOXCHAR,	/* W_CLOSER	*/
	G_BOXTEXT,	/* W_NAME	*/
	G_BOXCHAR,	/* W_FULLER	*/
	G_BOXTEXT,	/* W_INFO	*/
	G_IBOX,		/* W_DATA	*/
	G_IBOX,		/* W_WORK	*/
	G_BOXCHAR,	/* W_SIZER	*/
	G_BOX,		/* W_VBAR	*/
	G_BOXCHAR,	/* W_UPARROW	*/
	G_BOXCHAR,	/* W_DNARROW	*/
	G_BOX,      	/* W_VSLIDE	*/
	G_BOX,      	/* W_VELEV	*/
	G_BOX,      	/* W_HBAR	*/
	G_BOXCHAR,	/* W_LFARROW	*/
	G_BOXCHAR,	/* W_RTARROW	*/
	G_BOX,		/* W_HSLIDE	*/
	G_BOX		/* W_HELEV	*/
} ;
/*
GLOBAL LONG	gl_waspec[NUM_ELEM] =
{
	0x00011101L,	// W_BOX		//
	0x00011101L,	// W_TITLE		//
	0x12011101L,	// W_CLOSER		//
	0x0L,			// W_NAME		//
	0x07011101L,	// W_FULLER		//
	0x0L,			// W_INFO		//
	0x00001101L,	// W_DATA		//
	0x00001101L,	// W_WORK		//
	0x06011101L,	// W_SIZER		//
	0x00011101L,	// W_VBAR		//
	0x0C011101L,	// W_UPARROW	//
	0x0D011101L,	// W_DNARROW	//
	0x00011111L,	// W_VSLIDE		//
	0x00011101L,	// W_VELEV		//
	0x00011101L,	// W_HBAR		//
	0x0F011101L,	// W_LFARROW	//
	0x0E011101L,	// W_RTARROW	//
	0x00011111L,	// W_HSLIDE		//
	0x00011101L		// W_HELEV		//
} ;
*/
/* Changed for color category support 910327WHF */
/* [JCE] but still using the GEM character set, not the ViewMAX one */

GLOBAL LONG	gl_waspec3d[NUM_ELEM] =
{
	0x00022101L,	/* W_BOX	*/
	0x00FE1101L,	/* W_TITLE	*/
	0x1201110BL,	/* W_CLOSER	*/
	0x0L,			/* W_NAME	*/
	0x0701110BL,	/* W_FULLER	*/
	0x0L,			/* W_INFO	*/
	0x00021101L,	/* W_DATA	*/
	0x00FE1101L,	/* W_WORK	*/
	0x0601110BL,	/* W_SIZER	*/
	0x00021109L,	/* W_VBAR	*/
	0x0C01110BL,	/* W_UPARROW	*/
	0x0D01110BL,	/* W_DNARROW	*/
	0x00011109L,	/* W_VSLIDE	*/
	0x0001110BL,	/* W_VELEV	*/
	0x00021109L,	/* W_HBAR	*/
	0x0F01110BL,	/* W_LFARROW	*/
	0x0E01110BL,	/* W_RTARROW	*/
	0x00011109L,	/* W_HSLIDE	*/
	0x0001110BL		/* W_HELEV	*/
} ;



GLOBAL LONG	gl_waspec2d[NUM_ELEM] =
{
	0x00011101L,	/* W_BOX	*/
	0x00011101L,	/* W_TITLE	*/
	0x1201110BL,	/* W_CLOSER	*/
	0x0L,			/* W_NAME	*/
	0x0701110BL,	/* W_FULLER	*/
	0x0L,			/* W_INFO	*/
	0x00001101L,	/* W_DATA	*/
	0x00001101L,	/* W_WORK	*/
	0x0601110BL,	/* W_SIZER	*/
	0x00011101L,	/* W_VBAR	*/
	0x0C01110BL,	/* W_UPARROW	*/
	0x0D01110BL,	/* W_DNARROW	*/
	0x00011109L,	/* W_VSLIDE	*/
	0x0001110BL,	/* W_VELEV	*/
	0x00011101L,	/* W_HBAR	*/
	0x0F01110BL,	/* W_LFARROW	*/
	0x0E01110BL,	/* W_RTARROW	*/
	0x00011109L,	/* W_HSLIDE	*/
	0x0001110BL		/* W_HELEV	*/
} ;


GLOBAL LONG	gl_waspec[NUM_ELEM];




GLOBAL WORD	gl_waflag3d[NUM_ELEM] =		/*910326WHF*/
{
	FLAG3D,				/* W_BOX	*/
	FL3DBAK,		   	/* W_TITLE	*/
	FLAG3D+USECOLORCAT,	/* W_CLOSER	*/
	USECOLORCAT,		/* W_NAME	*/
	FLAG3D+USECOLORCAT,	/* W_FULLER	*/
	USECOLORCAT,		/* W_INFO	*/
	FL3DBAK,			/* W_DATA	*/
	FL3DBAK,			/* W_WORK	*/
	FLAG3D+USECOLORCAT,	/* W_SIZER	*/
	FL3DBAK+USECOLORCAT,/* W_VBAR	*/
	FLAG3D+USECOLORCAT,	/* W_UPARROW	*/
	FLAG3D+USECOLORCAT,	/* W_DNARROW	*/
	USECOLORCAT,		/* W_VSLIDE	*/
	FLAG3D+USECOLORCAT, /* W_VELEV	*/
	FL3DBAK+USECOLORCAT,/* W_HBAR	*/
	FLAG3D+USECOLORCAT,	/* W_LFARROW	*/
	FLAG3D+USECOLORCAT,	/* W_RTARROW	*/
	USECOLORCAT,		/* W_HSLIDE	*/
	FLAG3D+USECOLORCAT	/* W_HELEV	*/
} ;



GLOBAL WORD	gl_waflag2d[NUM_ELEM] =		/*910326WHF*/
{
	0,					/* W_BOX	*/
	0,			    	/* W_TITLE	*/
	FLAG3D+USECOLORCAT,	/* W_CLOSER	*/
	USECOLORCAT,		/* W_NAME	*/
	FLAG3D+USECOLORCAT,	/* W_FULLER	*/
	USECOLORCAT,		/* W_INFO	*/
	0,					/* W_DATA	*/
	0,					/* W_WORK	*/
	FLAG3D+USECOLORCAT,	/* W_SIZER	*/
	0,					/* W_VBAR	*/
	FLAG3D+USECOLORCAT,	/* W_UPARROW	*/
	FLAG3D+USECOLORCAT,	/* W_DNARROW	*/
	USECOLORCAT,		/* W_VSLIDE	*/
	FLAG3D+USECOLORCAT, /* W_VELEV	*/
	0,      			/* W_HBAR	*/
	FLAG3D+USECOLORCAT,	/* W_LFARROW	*/
	FLAG3D+USECOLORCAT,	/* W_RTARROW	*/
	USECOLORCAT,		/* W_HSLIDE	*/
	FLAG3D+USECOLORCAT	/* W_HELEV	*/
} ;


GLOBAL WORD	gl_waflag[NUM_ELEM];



GLOBAL TEDINFO	gl_aname;
GLOBAL TEDINFO	gl_ainfo;

GLOBAL TEDINFO	gl_asamp =
{
	0x0L, 0x0L, 0x0L, IBM, MD_REPLACE, TE_LEFT, 0x1100, 0x0, 1, 80, 80
};


VOID wm_calc(WORD wtype, REG UWORD kind, WORD x, WORD y, WORD w, WORD h, 
			 LPWORD px, LPWORD py, LPWORD pw, LPWORD ph);

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

GLOBAL WORD	wind_msg[8];


GLOBAL WORD	gl_wtop;

/*
MLOCAL void sstk(int ln)
{
        LPWORD ptr = MK_FP(rlr->p_uda->u_ssuser, (WORD)rlr->p_uda->u_spuser);
        TRACE("rlr->p_uda=$");
        ophex((WORD)(rlr->p_uda));
        TRACE(" gemwm1.c:$");
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
*/

VOID w_nilit(REG WORD num, REG LPTREE olist)
{
	while( num-- )
	{
	  olist[num].ob_next = olist[num].ob_head = olist[num].ob_tail = NIL;
	}
}

VOID ems_nilit(REG WORD num)
{
	while( num-- )
	{
	  ems_get_wtree(num, &D.g_tree0);
	  D.g_tree0.ob_next = D.g_tree0.ob_head = D.g_tree0.ob_tail = NIL;
	  ems_put_wtree(num, &D.g_tree0);
	}
}


/*
*	Routine to add a child object to a parent object.  The child
*	is added at the end of the parent's current sibling list.
*	It is also initialized.
*/
VOID w_obadd(LPTREE olist, REG WORD parent, REG WORD child)
{
	REG WORD	lastkid;

	if ( (parent != NIL) &&
	     (child != NIL) )
	{
	  olist[child].ob_next = parent;

	  lastkid = olist[parent].ob_tail;
	  if (lastkid == NIL)
	    olist[parent].ob_head = child;
	  else
	    olist[lastkid].ob_next = child;
	  olist[parent].ob_tail = child;
	}
}

/*
*	Routine to add a child object to a parent object.  The child
*	is added at the end of the parent's current sibling list.
*	It is also initialized.
*/
VOID w_emsobadd(REG WORD parent, REG WORD child)
{
	REG WORD	lastkid;
	OBJECT		obj, lk;
/*
	TRACE("w_emsobadd parent=$");
	ophex(parent);
	TRACE(" child=$");
	ophex(child);
	TRACE("\r\n$");
*/
	if ( (parent != NIL) && (child != NIL) )
	{
		ems_get_wtree(child, &obj);
		obj.ob_next = parent;
		ems_put_wtree(child, &obj);

		ems_get_wtree(parent, &obj);
		lastkid = obj.ob_tail;

		if (lastkid == NIL) obj.ob_head = child;
		else
		{
			ems_get_wtree(lastkid, &lk);
			lk.ob_next = child;
			ems_put_wtree(lastkid, &lk);
		}	
		obj.ob_tail = child;
		ems_put_wtree(parent, &obj);
	}
}

VOID w_setup(PD *ppd, WORD w_handle, WORD kind)
{
	ems_get_window(w_handle, &D.w_win0);

	D.w_win0.w_owner = ppd;
	D.w_win0.w_flags = VF_INUSE;
	D.w_win0.w_cotop = NIL;
	D.w_win0.w_kind = kind;
	D.w_win0.w_hslide = D.w_win0.w_vslide = 0;	/* slider at left/top	*/
	D.w_win0.w_hslsiz = D.w_win0.w_vslsiz = -1;	/* use default size	*/

	ems_put_window(w_handle, &D.w_win0);
}

//MLOCAL char *w_which[] = { "WS_FULL$", "WS_CURR$", "WS_PREV$", "WS_WORK$", "WS_TRUE$" };

/* Populates D.w_win0 and D.g_tree0 */
WORD *w_getxptr(WORD which, REG WORD w_handle)
{
	WORD *rv = NULL;
/*
	TRACE("w_getxptr: which=$");
	TRACE(w_which[which]);
	TRACE(" w_handle=$");
	ophex(w_handle);
*/
	ems_get_window(w_handle, &D.w_win0);
	ems_get_wtree (w_handle, &D.g_tree0);
	switch(which)
	{
	  case WS_CURR:
	  case WS_TRUE:
		rv = ( &D.g_tree0.ob_x );
		break;
	  case WS_PREV:
		rv = ( &D.w_win0.w_xprev );
		break;
	  case WS_WORK:
		rv = ( &D.w_win0.w_xwork );
		break;
	  case WS_FULL:
		rv = ( &D.w_win0.w_xfull );
		break;
	}
/*	TRACE("  rv=($");
	ophex(rv[0]);
	TRACE(",$");
	ophex(rv[1]);
	TRACE(",$");
	ophex(rv[2]);
	TRACE(",$");
	ophex(rv[3]);
	TRACE(")\r\n$");
	wswork();
*/
	return rv;
}

/* Get the size (x,y,w,h) of the window				*/

VOID w_getsize(REG WORD which, WORD w_handle, GRECT *pt)
{
/*
	TRACE("w_getsize which=$");
	TRACE(w_which[which]);
	TRACE(" w_handle=$");
	ophex(w_handle);
	TRACE("\r\n$");
*/
	rc_copy((GRECT *)w_getxptr(which, w_handle), pt);

#if DROP_SIZE
	if ( (which == WS_TRUE) && pt->g_w && pt->g_h)
	{
	  pt->g_w += DROP_SIZE;
	  pt->g_h += DROP_SIZE;
	}
#endif
}


VOID w_setsize(WORD which, WORD w_handle, GRECT *pt)
{
/*
	WORD *pw;
	TRACE("w_setsize which=$");
	TRACE(w_which[which]);
	TRACE(" w_handle=$");
	ophex(w_handle);
	TRACE(" rect=($");
	ophex(pt->g_x); TRACE(",$");	
	ophex(pt->g_y); TRACE(",$");	
	ophex(pt->g_w); TRACE(",$");	
	ophex(pt->g_h); TRACE(")\r\n$");
*/	
	rc_copy(pt, (GRECT *)w_getxptr(which, w_handle));
	ems_put_window(w_handle, &D.w_win0);
	ems_put_wtree(w_handle, &D.g_tree0);

	/* For debugging: Read back the rectangle to confirm that it took. 
	pw = w_getxptr(which, w_handle);
	if (pw[0] != pt->g_x || pw[1] != pt->g_y || 
	    pw[2] != pt->g_w || pw[3] != pt->g_h)
	{
		TRACE("!!! Rectangle didn't take !!!\r\n$");
	}
*/
}


        VOID
w_adjust(parent, obj, x, y, w, h)
	WORD		parent;
	REG WORD	obj;
	WORD		x, y, w, h;
{
	rc_copy((GRECT *)&x, (GRECT *)&W_ACTIVE[obj].ob_x);
	W_ACTIVE[obj].ob_head = W_ACTIVE[obj].ob_tail = NIL;
	w_obadd(&W_ACTIVE[ROOT], parent, obj);
}



VOID w_hvassign3d(WORD isvert, REG WORD parent, WORD obj, WORD vx, WORD vy, 
                  WORD hx, WORD hy, WORD w, WORD h, WORD xa, WORD ya)
{
	if ( isvert ) w_adjust(parent, obj, vx, vy, gl_wbox + xa, h);
	else 	  	  w_adjust(parent, obj, hx, hy, w, gl_hbox + ya);
}


	VOID
w_hvassign(isvert, parent, obj, vx, vy, hx, hy, w, h)
	WORD		isvert;
	REG WORD	parent, obj;
	WORD		vx, vy, hx, hy, w, h;
{
	if ( isvert )
 	  w_adjust(parent, obj, vx, vy, gl_wbox , h);
	else
 	  w_adjust(parent, obj, hx, hy, w, gl_hbox);
}


/*
*	Walk the list and draw the parts of the window tree owned by
*	this window.
*/

do_walk(wh, tree, obj, depth, pc)
	REG WORD	wh;
	LPTREE		tree;
	WORD		obj;
	WORD		depth;
	REG GRECT	*pc;
{
	WORD		npo;
	ORECT		po;
	GRECT		t;
	
/*	TRACE("do_walk: wh=$");
	ophex(wh);
	TRACE(" grect=$");
	if (pc == NULL) TRACE("NULL");
	else
	{
		TRACE("($");
		ophex(pc->g_x);	TRACE(",$");
		ophex(pc->g_y);	TRACE(",$");
		ophex(pc->g_w);	TRACE(",$");
		ophex(pc->g_h);	TRACE(")$");
	}
	TRACE("\r\n$");
*/
	if ( wh == NIL )
	  return(TRUE);
						/* clip to screen	*/
	if (pc)
	  rc_intersect(&gl_rfull, pc);
	else
	  pc = &gl_rfull;

	ems_get_window(wh, &D.w_win0);
						/* walk owner rect list	*/
	for(npo=D.w_win0.w_nrlist; npo != 0; npo=po.o_link)
	{
/*	  TRACE("do_walk: npo=$");
	  ophex(npo);
	  TRACE("\r\n$"); */
	  ems_get_orect(npo, &po);
	  rc_copy((GRECT *)&po.o_x, &t);
						/* intersect owner rect	*/
						/*   with clip rect's	*/
	  if ( rc_intersect(pc, &t) )
	  {
						/*  set clip and draw	*/
	    gsx_sclip(&t);
	    ob_draw(tree, obj, depth);
	  }
	}

	return (TRUE);
}


/*
*	Draw the desktop background pattern underneath the current
*	set of windows.
*/
VOID w_drawdesk(REG GRECT *pc)
{
	REG LPTREE	tree;
	REG WORD	depth;
	REG WORD	root;
	GRECT		pt;
	OBJECT		wdesk;
	
	rc_copy(pc, &pt);
/*	TRACE("w_drawdesk: $");
	ophex(gl_newdesk);
	TRACE("\r\n$"); */
	if (gl_newdesk)
	{
	  tree = gl_newdesk;
	  depth = MAX_DEPTH;
	  root = gl_newroot;
	}
	else
	{
	/* When drawing the default desktop, create a mini-tree with one 
	 * object in it. This prevents having to call ob_emsdraw */
	  ems_get_wtree(ROOT, &wdesk);
	  wdesk.ob_head = NIL;
	  wdesk.ob_tail = NIL;
	  wdesk.ob_next = NIL;
	  tree = &wdesk;
	  depth = 0;
	  root = ROOT;
	}
						/* account for drop	*/
						/*   shadow		*/
						/* BUGFIX in 2.1	*/
#if DROP_SIZE
	pt.g_w += DROP_SIZE;
	pt.g_h += DROP_SIZE;
#endif
	
	do_walk(DESKWH, tree, root, depth, pc);
}


VOID w_cpwalk(REG WORD wh, WORD obj, WORD depth, WORD usetrue)
{
	GRECT		c;
/*
	TRACE("w_cpwalk: wh=$");
	ophex(wh);
	TRACE(" obj=$");
	ophex(obj);
	TRACE(" depth=$");
	ophex(depth);
	TRACE(" usetrue=$");
	ophex(usetrue);
	TRACE("\r\n$");
*/	
						/* start with window's	*/
						/*   true size as clip	*/
	if ( usetrue )  w_getsize(WS_TRUE, wh, &c);
	else
	{
						/* use global clip	*/
	  gsx_gclip(&c);
						/* add in drop shadow	*/
#if DROP_SIZE
	  c.g_w += DROP_SIZE;
	  c.g_h += DROP_SIZE;
#endif
	}
	w_bldactive(wh);
	do_walk(wh, gl_awind, obj, depth, &c);
}


/*
*	Build an active window and draw the all parts of it but clip
*	these parts with the owner rectangles and the passed in
*	clip rectangle.
*/

VOID w_clipdraw(WORD wh, WORD obj, WORD depth, WORD usetrue)
{
	WORD		i;

/*	TRACE("w_clipdraw: wh=$");
	ophex(wh);
	TRACE(" obj=$");
	ophex(obj);
	TRACE(" depth=$");
	ophex(depth);
	TRACE(" usetrue=$");
	ophex(usetrue);
	TRACE("\r\n$");
*/
	ems_get_window(wh, &D.w_win0);
	if ( (usetrue == 2) || (usetrue == 0) )
	{
	  ems_get_wtree(ROOT, &D.g_tree0);
	  for(i = D.g_tree0.ob_head; i > ROOT; )
	  {
		ems_get_window(i, &D.w_win1);
	    	if ( (i != wh) 
		&&   (D.w_win1.w_owner == D.w_win0.w_owner)
		&&   (D.w_win1.w_flags & VF_SUBWIN)
		&&   (D.w_win0.w_flags & VF_SUBWIN))
		{
			w_cpwalk(i, obj, depth, TRUE);
		}
		i = D.g_tree0.ob_next;
		ems_get_wtree(i, &D.g_tree0);
	  }
	}
						/* build active tree	*/
	w_cpwalk(wh, obj, depth, usetrue);
}


VOID w_strchg(REG WORD w_handle, REG WORD obj, REG LPBYTE pstring)
{

	ems_get_window(w_handle, &D.w_win0);
/*
	LPBYTE ps1;

	TRACE("w_strchg w_handle=$");
	ophex(w_handle);
	TRACE(" obj=$");
	ophex(obj);
	TRACE(" value='$");
	for (ps1 = pstring; ps1[0]; ++ps1)
	{
		TRCHAR(*ps1);
	}
	TRACE("'\r\n$");
*/
	if ( obj == W_NAME )
	{
	  LBCOPY(D.w_win0.w_name, pstring, WSTRSIZE);
	  gl_aname.te_ptext = &D.w_win0.w_name[0];
	}
	else
	{
	  LBCOPY(D.w_win0.w_info, pstring, WSTRSIZE);
	  gl_ainfo.te_ptext = &D.w_win0.w_info[0];
	}
	ems_put_window(w_handle, &D.w_win0);

	w_clipdraw(w_handle, obj, MAX_DEPTH, 1);
}


	VOID
w_barcalc(isvert, space, sl_value, sl_size, min_sld, ptv, pth)
	WORD		isvert;
	REG WORD	space;
	REG WORD	sl_value, sl_size;
	REG WORD	min_sld;
	GRECT		*ptv, *pth;
{
	if (sl_size == -1)
	  sl_size = min_sld;
	else
	  sl_size = max(min_sld, mul_div(sl_size, space, 1000) );

	sl_value = mul_div(space - sl_size, sl_value, 1000);
#if APPLE_COMPLIANT
	if (isvert)
	  r_set(ptv, 3, sl_value, gl_wbox-6, sl_size);
	else
	  r_set(pth, sl_value, 2, sl_size, gl_hbox-4);
#else
	if (isvert)
	  r_set(ptv, 0, sl_value, gl_wbox, sl_size);
	else
	  r_set(pth, sl_value, 0, sl_size, gl_hbox);

#endif
}



VOID
w_bldbar(kind, istop, w_bar, sl_value, sl_size, x, y, w, h)
	UWORD		kind;
	WORD		istop;
	WORD		w_bar, sl_value, sl_size;
	REG WORD	x, y, w, h;
{
	WORD		isvert, obj;
	UWORD		upcmp, dncmp, slcmp;		
	REG WORD	w_up;
	WORD		w_dn, w_slide, space, min_sld;

	isvert = (w_bar == W_VBAR);
	if ( isvert )
	{
	  upcmp = UPARROW;
	  dncmp = DNARROW;
	  slcmp = VSLIDE;
	  w_up = W_UPARROW;
	  w_dn = W_DNARROW;
	  w_slide = W_VSLIDE;
	  min_sld = gl_hbox;
	}
	else
	{
	  upcmp = LFARROW;
	  dncmp = RTARROW;
	  slcmp = HSLIDE;
	  w_up = W_LFARROW;
	  w_dn = W_RTARROW;
	  w_slide = W_HSLIDE;
	  min_sld = gl_wbox;
	}

	if (gl_opts.frame3d) w_hvassign3d(isvert, W_DATA, w_bar, x, y, x, y, w, h, 4, 4);
	else		     w_hvassign(isvert, W_DATA, w_bar, x, y, x, y, w, h);

	x = y = (gl_opts.frame3d) ? 2 : 0;

	if ( istop )
	{
	  if (kind & upcmp)
	  {
	    w_adjust(w_bar, w_up, x, y, gl_wbox, gl_hbox);
	    if ( isvert )
	    {
	      y += (gl_hbox - 1);
	      h -= (gl_hbox - 1);
	    }
	    else
	    {
	      x += (gl_wbox - 1);
	      w -= (gl_wbox - 1);
	    }
	  }
	  if ( kind & dncmp )
	  {
	    if (gl_opts.frame3d) 
	    {
	      w -= (gl_wbox + 3);
	      h -= (gl_hbox + 3);
           }
            else
            {
	      w -= (gl_wbox - 1);
	      h -= (gl_hbox - 1);
            }
	    w_hvassign(isvert, w_bar, w_dn, x, y + h - 1, 
			x + w - 1, y, gl_wbox, gl_hbox);
	  }
	  if ( kind & slcmp )
	  {
	    if (gl_opts.frame3d)
            {
		if (isvert) 
		{
			y++; h-=2; 
		}
		else 
		{
			x++; w-=2;
		}

            }
	    w_hvassign(isvert, w_bar, w_slide, x, y, x, y, w, h);
	    space = (isvert) ? h : w;

	    w_barcalc(isvert, space, sl_value, sl_size, min_sld, 
		  (GRECT *)&W_ACTIVE[W_VELEV].ob_x, (GRECT *)&W_ACTIVE[W_HELEV].ob_x);

	    obj = (isvert) ? W_VELEV : W_HELEV;
	    W_ACTIVE[obj].ob_head = W_ACTIVE[obj].ob_tail = NIL;
	    w_obadd(&W_ACTIVE[ROOT], w_slide, obj);
	  }
	}
}


	WORD
w_top()
{
	return( (gl_wtop != NIL) ? gl_wtop : DESKWH );
}

VOID w_setactive()
{
	GRECT		d;
	REG WORD	wh;
	PD		*ppd;

	wh = w_top();
	w_getsize(WS_WORK, wh, &d);
	ems_get_window(wh, &D.w_win0);
	ppd = D.w_win0.w_owner;
						/* BUGFIX 2.1		*/
						/*  don't chg own if null*/
	if (ppd != (PD *)NULLPTR)
	  ct_chgown(ppd, &d);
}

WORD w_bldactive(REG WORD w_handle)
{
	WORD		istop, issub;
	REG WORD	kind;
	REG WORD	havevbar;
	REG WORD	havehbar;
	GRECT		t;
	REG WORD	tempw;
	WORD		offx, offy;

	if (w_handle == NIL)
	  return(TRUE);

	ems_get_window(w_handle, &D.w_win0);
	ems_get_window(gl_wtop, &D.w_win1);
						/* set if it is on top	*/
	istop = (gl_wtop == w_handle || D.w_win0.w_cotop == gl_wtop);
						/* get the kind of windo*/
	kind = D.w_win0.w_kind;
	w_nilit(NUM_ELEM, &W_ACTIVE[0]);
						/* start adding pieces	*/
						/*   & adjusting sizes	*/
#if MULTIAPP
	gl_aname.te_ptext = ADDR(W_NAMES[w_handle]);
	gl_ainfo.te_ptext = ADDR(W_INFOS[w_handle]);
#endif
#if SINGLAPP
	gl_aname.te_ptext = &D.w_win0.w_name[0];
	gl_ainfo.te_ptext = &D.w_win0.w_info[0];
#endif
	gl_aname.te_just = TE_CNTR;
	issub = ( (D.w_win0.w_flags & VF_SUBWIN) &&
		  (D.w_win1.w_flags & VF_SUBWIN) );
	w_getsize(WS_CURR, w_handle, &t);
	rc_copy(&t, (GRECT *)&W_ACTIVE[W_BOX].ob_x);
	offx = t.g_x;
	offy = t.g_y;
						/* do title area	*/
	if (gl_opts.frame3d)
	{
		t.g_x = t.g_y = 4;
		t.g_w -= 4;
	}
	else t.g_x = t.g_y = 0;

	if ( kind & (NAME | CLOSER | FULLER) )
	{
	  if (gl_opts.frame3d) t.g_w -= 4;
	  w_adjust(W_BOX, W_TITLE, t.g_x, t.g_y, t.g_w, gl_hbox);
	  if (gl_opts.frame3d) t.g_x = t.g_y = 0;
	  
	  tempw = t.g_w;
	  if ( (kind & CLOSER) &&
	       ( istop || issub ) )
	  {
	    w_adjust(W_TITLE, W_CLOSER, t.g_x, t.g_y, gl_wbox, gl_hbox);
	    t.g_x += gl_wbox;
	    tempw -= gl_wbox;
	  }
	  if ( (kind & FULLER) &&
	       ( istop || issub ) )
	  {
	    tempw -= gl_wbox;
	    w_adjust(W_TITLE, W_FULLER, t.g_x + tempw, t.g_y, 
			gl_wbox, gl_hbox);
	  }
	  if ( kind & NAME )
	  {
	    w_adjust(W_TITLE, W_NAME, t.g_x, t.g_y, tempw, gl_hbox);
	    W_ACTIVE[W_NAME].ob_state = (istop || issub) ? NORMAL : DISABLED;

// APPLE  no pattern in window title //
// nb: ViewMAX always draws the pattern //

#if (!APPLE_COMPLIANT)
 	  gl_aname.te_color = (istop && (!issub)) ? WTS_FG : WTN_FG;
#endif
		
	  }
	  if (gl_opts.frame3d)
	  {	
		t.g_y += (gl_hbox + 6);
	  	t.g_h -= (gl_hbox + 8);
	  	t.g_w += 4;
 	   }
	   else
          {
	  	t.g_x = 0;
	  	t.g_y += (gl_hbox - 1);
	  	t.g_h -= (gl_hbox - 1);
          }
	}
						/* do info area		*/
	if (gl_opts.frame3d) t.g_x = 2;
	if ( kind & INFO )
	{
 	  gl_ainfo.te_color = WTI_FG;
	  w_adjust(W_BOX, W_INFO, t.g_x, t.g_y, t.g_w, gl_hbox);
	  t.g_y += (gl_hbox - 1);
	  t.g_h -= (gl_hbox - 1);
	}
						/* do data area		*/
	w_adjust(W_BOX, W_DATA, t.g_x, t.g_y, t.g_w, t.g_h);
						/* do work area		*/
	if (!gl_opts.frame3d)
	{
		t.g_x++;
		t.g_y++;
		t.g_w -= 2;
		t.g_h -= 2;
	}
	havevbar = kind & (UPARROW | DNARROW | VSLIDE | SIZER);
	havehbar = kind & (LFARROW | RTARROW | HSLIDE | SIZER);
	if (gl_opts.frame3d)
	{
		if ( havevbar ) t.g_w -= (gl_wbox + 4);
		if ( havehbar ) t.g_h -= (gl_hbox + 4);
		t.g_x = 2;
		t.g_y = 2;
		t.g_h -= 4;
		t.g_w -= 4;
	}
	else
	{
		if ( havevbar ) t.g_w -= (gl_wbox - 1);
		if ( havehbar ) t.g_h -= (gl_hbox - 1);
		t.g_x = t.g_y = 1;
	}
	w_adjust(W_DATA, W_WORK, t.g_x, t.g_y, t.g_w, t.g_h);
						/* do vert. area	*/
	if ( havevbar )
	{
	  t.g_x += t.g_w;
	  if (gl_opts.frame3d) 
	       w_bldbar(kind, istop || issub, W_VBAR, D.w_win0.w_vslide, 
			D.w_win0.w_vslsiz, t.g_x + 2, 0, 
			t.g_w + 4, t.g_h + 4);

	  else w_bldbar(kind, istop || issub, W_VBAR, D.w_win0.w_vslide, 
			D.w_win0.w_vslsiz, t.g_x, 0, 
			t.g_w + 2, t.g_h + 2);
	}
						/* do horiz area	*/
	if ( havehbar )
	{
	  t.g_y += t.g_h;
	  if (gl_opts.frame3d)
	       w_bldbar(kind, istop || issub, W_HBAR, D.w_win0.w_hslide,
			D.w_win0.w_hslsiz, 0, t.g_y + 2, 
			t.g_w + 4, t.g_h);
	  else w_bldbar(kind, istop || issub, W_HBAR, D.w_win0.w_hslide,
			D.w_win0.w_hslsiz, 0, t.g_y, 
			t.g_w + 2, t.g_h + 2);
	}
						/* do sizer area	*/
	if ( (havevbar) &&
	     (havehbar) )
	{
	  if (gl_opts.frame3d) 
	       w_adjust(W_DATA, W_SIZER, t.g_x+1, t.g_y+1, gl_wbox+3, gl_hbox+3);
	  else w_adjust(W_DATA, W_SIZER, t.g_x, t.g_y, gl_wbox, gl_hbox);
	  
	  if (istop && (kind & SIZER))
	  {
		W_ACTIVE[W_SIZER].ob_spec = sizer_spec;
		W_ACTIVE[W_SIZER].ob_flags |= sizer_flag;
	  }
	  else
	  {
	    W_ACTIVE[W_SIZER].ob_spec = 0x00011100L;
		W_ACTIVE[W_SIZER].ob_flags &= ~sizer_flag;
	  }
	}
	if (gl_opts.frame3d) for (istop = 0; istop < NUM_ELEM; istop++)
	{
		if (W_ACTIVE[istop].ob_flags & (FLAG3D | FL3DBAK))
		{
			W_ACTIVE[istop].ob_state  |= WHITEBAK;
		}
	}

	return TRUE;
}



