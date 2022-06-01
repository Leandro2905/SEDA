/*	GEMOBLIB.C	03/15/84 - 05/27/85	Gregg Morris		*/
/*	merge High C vers. w. 2.2 		8/21/87		mdf	*/ 
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


						/* in GSXBIND.C		*/
#define vsf_color( x )		gsx_1code(S_FILL_COLOR, x)
						/* in GLOBE.C		*/
						/* in GSXIF.C		*/
EXTERN VOID	gsx_moff();
EXTERN VOID	gsx_mon();
EXTERN WORD	gsx_chkclip();

/* ------------- added for metaware compiler --------- */
GLOBAL VOID	ob_actxywh();
GLOBAL VOID 	ob_relxywh();
GLOBAL VOID	ob_offset();
EXTERN VOID 	r_set();
EXTERN WORD 	strlen();
EXTERN BYTE 	ob_sst();			/* in OBJOP.C 		*/
EXTERN VOID 	everyobj();
EXTERN WORD 	get_par();
EXTERN WORD 	ems_getpar();
EXTERN VOID 	gsx_gclip();			/* in GRAF.C		*/
EXTERN VOID 	gsx_cline();
EXTERN VOID 	gsx_attr();
EXTERN VOID	gsx_blt();
EXTERN VOID 	gr_inside();
EXTERN VOID 	gr_crack();
EXTERN VOID	gr_box();
EXTERN VOID	gr_rect();
EXTERN VOID 	gr_gtext();
EXTERN VOID	gr_gicon();
EXTERN VOID 	gsx_tblt();
EXTERN VOID	bb_fill();
EXTERN VOID     gsx_1code();			/* in GSXIF.C		*/
/* ------------------------------------------- */

EXTERN THEGLO	D;

#define EMSMAP1 (( (LPBYTE)(D.g_emsmap)) )
#define EMSMAP2 (( (LPBYTE)(D.g_emsmap)) + gl_emm_mapsize)

/* Wrappers for object functions when the object is somewhere in EMS */
WORD ob_emsfind(REG WORD currobj, REG WORD depth, WORD mx, WORD my)
{
	WORD w, rv;

        EMS_PageMap(0, NULL, EMSMAP1); 
	for (w = 0; w < gl_emm_size; w++) EMS_Map(D.g_emshandle, w, w);
	rv = ob_find(D.g_emmtree, currobj, depth, mx, my);
	EMS_PageMap(1, EMSMAP1, NULL); 
        return rv;
}


/* Wrappers for object functions when the object is somewhere in EMS */
VOID ob_emsdelete(WORD obj)
{
	WORD w;

        EMS_PageMap(0, NULL, EMSMAP1); 
	for (w = 0; w < gl_emm_size; w++) EMS_Map(D.g_emshandle, w, w);
        ob_delete(D.g_emmtree, obj);
        EMS_PageMap(1, EMSMAP1, NULL); 
}


VOID ob_emsorder(WORD mov_obj, WORD new_pos)
{
	WORD w;

        EMS_PageMap(0, NULL, EMSMAP1); 
	for (w = 0; w < gl_emm_size; w++) EMS_Map(D.g_emshandle, w, w);
        ob_order(D.g_emmtree, mov_obj, new_pos);
        EMS_PageMap(1, EMSMAP1, NULL); 
}


VOID ob_emsdraw(WORD obj, WORD depth)
{
	WORD w;

        EMS_PageMap(0, NULL, EMSMAP1);
	for (w = 0; w < gl_emm_size; w++) EMS_Map(D.g_emshandle, w, w);
        ob_draw(D.g_emmtree, obj, depth);
        EMS_PageMap(1, EMSMAP1, NULL);
}


/* Note that everyobj stores the EMS mapping in the second temporary 
 * slot. This is because it can call other functions, and they will
 * use the first temporary slot. The same goes for ems_newrect below. */
VOID ems_everyobj(WORD this, WORD last,
              EVERYOP routine, WORD startx, WORD starty, WORD maxdep)
{
	WORD w;

        EMS_PageMap(0, NULL, EMSMAP2);
	for (w = 0; w < gl_emm_size; w++) EMS_Map(D.g_emshandle, w, w);
        everyobj(D.g_emmtree, this, last, routine, startx, starty, maxdep);
        EMS_PageMap(1, EMSMAP2, NULL);
}

VOID ems_newrect(WORD wh, WORD d1, WORD d2)
{
	WORD w;

        EMS_PageMap(0, NULL, EMSMAP2);
	for (w = 0; w < gl_emm_size; w++) EMS_Map(D.g_emshandle, w, w);
        newrect(D.g_emmtree, wh, d1, d2);
        EMS_PageMap(1, EMSMAP2, NULL);
}

