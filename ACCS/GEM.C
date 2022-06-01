/*  	GEMFSLIB.C	5/14/84 - 07/16/85	Lee Lorenzen		*/
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

#include "ppdgem.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include "rclib.h"
#include "gemfslib.h"    

/* [JCE 28-6-2005] Picking this up after not touching it for over a year...
 *
 * The major problems seem to be caused by GEM's weird reverse-semantics
 * versions of the standard string functions. So far I've had strcpy() 
 * with reversed arguments, and strcmp() returning 1 for equal, 0 for not.
 */

#define F_SUBDIR 0x10
#define NM_FILES 100
#define NM_NAMES (F9NAME-F1NAME+1)
#define NAME_OFFSET F1NAME
#define LEN_FTITLE 18				/* BEWARE, requires change*/
#define LEN_FSNAME 16
						/*  in GEM.RSC		*/
						/* in DOS.C		*/
EXTERN	BYTE	*scasb(BYTE *p, WORD b);

WORD	gem_handle;			/* GEM vdi handle		*/
WORD	vdi_handle;			/* acc1 vdi handle		*/
WORD	work_out[57];			/* open virt workstation values	*/
GRECT	work_area;			/* current window work area	*/
WORD	gl_apid;			/* application ID		*/
WORD	gl_rmsg[8];			/* message buffer		*/
LPBYTE	ad_rmsg;			/* LONG pointer to message bfr	*/
WORD	gl_itemacc1 = 0;		/* hello menu item		*/
WORD	gl_xfull;			/* full window 'x'		*/
WORD	gl_yfull;			/* full window 'y'		*/
WORD	gl_wfull;			/* full window 'w' width	*/
WORD	gl_hfull;			/* full window 'h' height	*/
WORD	ev_which;			/* event message returned value	*/
WORD	acc1_whndl = 0;		/* hello window handle		*/

/* Offsets of the filenames in the array of filenames */
MLOCAL LPBYTE	  g_fslist[NM_FILES];

MLOCAL BYTE	  g_dir[130];

/* ---------- added for metaware compiler ---------- */
EXTERN WORD 	dos_gdrv();
EXTERN WORD 	dos_sfirst();
EXTERN WORD 	dos_snext();
EXTERN VOID 	ins_char();
EXTERN WORD 	strchk(BYTE s[], BYTE t[]);
EXTERN WORD 	mul_div(WORD m1, UWORD m2, WORD d1);
/* ----------------------------------------------------- */


GLOBAL BYTE	gl_dta[128];

GLOBAL BYTE	gl_fsobj[4] = {FTITLE, FILEBOX, SCRLBAR, 0x0};
MLOCAL GRECT	gl_rfs;

GLOBAL WORD	gl_shdrive;
GLOBAL WORD	gl_fspos;
MLOCAL WORD	gl_wchar, gl_hchar, gl_wbox1, gl_hbox1;
MLOCAL BYTE 	gl_tmp1[LEN_FSNAME];
MLOCAL BYTE 	gl_tmp2[LEN_FSNAME];
MLOCAL BYTE	gl_fsnames[ (LEN_FSNAME * NM_FILES) ];
MLOCAL ULONG	gl_bvdisk, gl_bvhard;
MLOCAL LPTREE	gl_fstree;
/*
*	LONG string compare, TRUE == strings the same
*/

#define W_CLOSER   2
#define W_UPARROW 10
#define W_DNARROW 11
#define W_VELEV   13

static WORD gl_working = 0;
static WORD gl_xclip, gl_yclip, gl_wclip, gl_hclip;
static MFDB gl_src, gl_dst;

/* static char dbuff[500]; */

int issep(char c)
{
	return (c == '\\' || c == '/');
}
/*
void debug(char *s, ...)
{
	va_list ap;
	FILE *fp = fopen("e:/gemapp.log", "a");
	
	va_start(ap, s);
	vfprintf(fp, s, ap);
	fprintf(fp, "\n");
	va_end(ap);
	fclose(fp);
}
*/


VOID bb_screen(WORD scrule, WORD scsx, WORD scsy, WORD scdx, WORD scdy, 
		WORD scw, WORD sch)
{
	WORD	pts[8];

	v_hide_c(vdi_handle);
	memset(&gl_src, 0, sizeof(gl_src));
	memset(&gl_dst, 0, sizeof(gl_dst));

	gl_dst.fwp = gl_src.fwp = gl_wfull;
	gl_dst.fww = gl_src.fww = gl_wfull / 16;
	gl_dst.fh  = gl_src.fh  = gl_hfull;

	pts[0] = scsx;
	pts[1] = scsy;
	pts[2] = scsx + scw - 1;
	pts[3] = scsy + sch - 1;
	pts[4] = scdx;
	pts[5] = scdy;
	pts[6] = scdx + scw - 1;
	pts[7] = scdy + sch - 1;

	vro_cpyfm(vdi_handle, scrule, &pts[0], &gl_src, &gl_dst);
	v_show_c(vdi_handle, 0);
}


/*
 *       Routine to insert a character in a string by
 */
        VOID
ins_char(str, pos, chr, tot_len)
        REG BYTE        *str;
        WORD            pos;
        BYTE            chr;
        REG WORD        tot_len;
{
        REG WORD        ii, len;

        len = strlen(str);

        for (ii = len; ii > pos; ii--)
          str[ii] = str[ii-1];
        str[ii] = chr;
        if (len+1 < tot_len)
          str[len+1] = NULL;
        else
          str[tot_len-1] = NULL;
}


/*
*	Routine to see if the test filename matches one of a set of 
*	comma delimited wildcard strings.
*		e.g.,	pwld = "*.COM,*.EXE,*.BAT"
*		 	ptst = "MYFILE.BAT"
*/
WORD wildcmp(BYTE *pwld, BYTE *ptst)
{
	BYTE		*pwild;
	BYTE		*ptest;
						/* skip over *.*, and	*/
						/*   *.ext faster	*/
	while(*pwld)
	{
	  ptest = ptst;
	  pwild = pwld;
						/* move on to next 	*/
						/*   set of wildcards	*/
	  pwld = scasb(pwld, ',');
	  if (*pwld)
	    pwld++;
						/* start the checking	*/
	  if (pwild[0] == '*')
	  {
	    if (pwild[2] == '*')
	      return(TRUE);
	    else
	    {
	      pwild = &pwild[2];
	      ptest = scasb(ptest, '.');
	      if (*ptest)
	        ptest++;
	    }
	  }
						/* finish off comparison*/
	  while( (*ptest) && 
	         (*pwild) &&
		 (*pwild != ',') )
	  {
	    if (*pwild == '?')
	    {
	       pwild++;
	       if (*ptest != '.')
	         ptest++;
	    }
	    else
	    {
	      if (*pwild == '*')
	      {
	        if (*ptest != '.')
		  ptest++;
	        else		
		  pwild++;
	      }
	      else
	      {
	        if (*ptest == *pwild)
	        {
	          pwild++;
	          ptest++;
	        }
	        else
	          break;
	      }
	    }
	  }
						/* eat up remaining 	*/
						/*   wildcard chars	*/
	  while( (*pwild == '*') ||
	         (*pwild == '?') ||
	         (*pwild == '.') )
	    pwild++;
						/* if any part of wild-	*/
						/*   card or test is	*/
						/*   left then no match	*/
	  if ( ((*pwild == NULL) || (*pwild == ',')) && 
	       (!*ptest) )
	    return( TRUE );
	}
	return(FALSE);
}


/*
 *       Strip out period and turn into raw data.
 */
VOID fmt_str(BYTE *instr, BYTE *outstr)
{
        WORD            count;
        BYTE            *pstr;

        pstr = instr;
        while( (*pstr) && (*pstr != '.') )
          *outstr++ = *pstr++;
        if (*pstr)
        {
          count = 8 - (pstr - instr);
          while ( count-- )
            *outstr++ = ' ';
          pstr++;
          while (*pstr)
            *outstr++ = *pstr++;
        }
        *outstr = NULL;
}


VOID unfmt_str(BYTE *instr, BYTE *outstr)
{
        BYTE            *pstr, temp;

        pstr = instr;
        while( (*pstr) && ((pstr - instr) < 8) )
        {
          temp = *pstr++;
          if (temp != ' ')
            *outstr++ = temp;
        }
        if (*pstr)
        {
          *outstr++ = '.';
          while (*pstr)
            *outstr++ = *pstr++;
        }
        *outstr = NULL;
}



WORD inf_gindex(LPTREE tree, WORD baseobj, WORD numobj)
{
        WORD            retobj;

        for (retobj=0; retobj < numobj; retobj++)
        {
          if (tree[baseobj+retobj].ob_state & SELECTED)
            return(retobj);
        }
        return(-1);
}


#asm
;
;       copy from src to dest until and including a null in the
;       source string
;
;       len = LSTCPY(pdstoff, pdstseg, psrcoff, psrcseg)
;
;
_LSTCPY:
        push    bp
        mov             bp,sp
        push    ds
        push    di
        push    si
        cld                     ; assume forward
        les             di,4[bp]        ; dest off & seg
        lds             si,8[bp]        ; src off & seg
        mov             cx,#0           ; count
        mov             ah,#0
lsc_loop:
        lodsb
        cmp             al,#0
	je              lsc_done
        inc             cx
        stosb
        jmp             lsc_loop
lsc_done:
        stosb
        pop             si
        pop             di
        mov             ax,cx
        pop             ds
        pop             bp
        ret             #8


;
;***************************************************************************
;
; MUL_DIV (m1,m2,d1)
;
;        ( ( ( m1 * 2 * m2 ) / d1 ) + 1 ) / 2
;       m1 = signed 16 bit integer
;       m2 = unsigned 15 bit integer
;       d1 = signed 16 bit integer
;
;***************************************************************************
_mul_div:
        push    bp              ;save the callers bp
        mov             bp,sp
        mov             bx,dx   ; DX = Parameter 1
                                        ; AX = Parameter 2
        shl             ax,#1   ; m2 * 2
        imul    bx              ; m2 * 2 * m1
        mov             bx,04[bp]
        idiv    bx              ; m2 * 2 * m1 / d1
        and             ax,ax           ; set flags
        js              mul_div_neg
        inc             ax
        shr             ax,#1           ; m2 * 2 * m1 / d1 +1 / 2
        pop             bp
        ret             #2

mul_div_neg:
        add             ax,#-1
        neg             ax
        shr             ax,#1
        neg             ax              ; m2 * 2 * m1 / d1 -1 / 2
        pop             bp
        ret             #2
;
;
;       BYTE
;*scasb(p, b)
;       BYTE            *p;
;       BYTE            b;
;
_scasb:
        push    bp
        mov             bp,sp
        push    si
        cld
        mov             si,dx   ;Source
        mov             bx,ax   ;Byte
        and             bx,#00ffh
lpscas:
        lodsb
        cmp             al,bh
        jz              scasret
        cmp             al,bl
        jnz             lpscas
scasret:
        mov             ax,si
        dec             ax
        pop             si
        pop             bp
        ret
;
;
;/*
;*      Return <0 if s<t, 0 if s==t, >0 if s>t
;*/
;
;       WORD
;strchk(s, t)
;       BYTE            s[], t[];
;
_strchk:
        push    bp
        mov     bp,sp

        push    es
        push    si
        push    di

        cld
        mov             si,dx           ;si -> s
        mov             di,ax           ;di -> t
        mov             ax,ds
        mov             es,ax
        xor             ax,ax           ; clear ah,al
lpschk:
        lodsb                           ; pick up source
        scasb                           ; compare with destination
        jne             scplmn          ; if not same then false
        cmp             al,ah           ; if no more string
        jnz             lpschk          ;   then exit true
scplmn:
        mov             bl,-1[di]
        mov             bh,#0
        sub             ax,bx

        pop             di
        pop             si
        pop             es

        pop             bp
        ret
;



#endasm


/*
 *       Return 0 if cancel was selected, 1 if okay was selected, -1 if
 *       nothing was selected.
 */

WORD inf_what(LPTREE tree, WORD ok, WORD cncl)
{
        WORD            field;

        field = inf_gindex(tree, ok, 2);

        if (field != -1)
        {
          tree[ok + field].ob_state = NORMAL;
          field = (field == 0);
        }
        return(field);
}


VOID fs_sset(LPTREE tree, WORD obj, LPBYTE pstr, LPBYTE *ptext, WORD *ptxtlen)
{
        LPTEDI          spec;
        WORD            len;

        spec      = (LPTEDI)(tree[obj].ob_spec);
        *ptext    = spec->te_ptext;
        *ptxtlen  = spec->te_txtlen;
        len       = LSTRLEN(pstr);      /* allow for null       */  
        len       = min(len, *ptxtlen - 1);
        LBCOPY(*ptext, pstr, len);
        (*ptext)[len] = '\0';           /* add null             */
}


VOID inf_sset(LPTREE tree, WORD obj, BYTE *pstr)
{
        LPBYTE          text;
        WORD            txtlen;

        fs_sset(tree, obj, ADDR(pstr), &text, &txtlen);
}

void ob_relxywh(LPTREE tree, WORD obj, GRECT *pt)
{
	LWCOPY(ADDR(pt), ADDR(&tree[obj].ob_x), sizeof(GRECT)/2);
}

void ob_actxywh(LPTREE tree, WORD obj, GRECT *pt)
{
	objc_offset(tree, obj, &pt->g_x, &pt->g_y);
	pt->g_w = tree[obj].ob_width;
	pt->g_h = tree[obj].ob_height;	
}



void gsx_gclip(GRECT *pt)
{
	pt->g_x = gl_xclip;
	pt->g_y = gl_yclip;
	pt->g_w = gl_wclip;
	pt->g_h = gl_hclip;
}

void gsx_sclip(GRECT *pt)
{
	WORD pts[4];
	
	gl_xclip = pt->g_x;	
	gl_yclip = pt->g_y;	
	gl_wclip = pt->g_w;	
	gl_hclip = pt->g_h;	

	if (gl_wclip && gl_hclip)
	{
		pts[0] = gl_xclip;
		pts[1] = gl_yclip;
		pts[2] = gl_xclip + gl_wclip - 1;
		pts[3] = gl_yclip + gl_hclip - 1;
		vs_clip(vdi_handle, TRUE, pts);
	}
	else
	{
		vs_clip(vdi_handle, FALSE, pts);
	}	
}


void ob_draw(LPTREE tree, WORD obj, WORD depth)
{
	objc_draw(tree, obj, depth, gl_rfs.g_x, gl_rfs.g_y, gl_rfs.g_w,
		gl_rfs.g_h);
}

void ob_change(LPTREE tree, WORD obj, WORD state, WORD redraw)
{
	objc_change(tree, obj, 1, gl_rfs.g_x, gl_rfs.g_y, gl_rfs.g_w,
		gl_rfs.g_h, state, redraw);
}


VOID fs_sget(LPTREE tree, WORD obj, LPBYTE pstr)
{
        LPBYTE          ptext;

        ptext = ((LPTEDI)(tree[obj].ob_spec))->te_ptext;
        LSTCPY(pstr, ptext);
}



WORD LSTCMP(LPBYTE lst, LPBYTE rst)
{
	WORD		i;
	BYTE		l;

	i = 0;
	while ((l = lst[i]) != 0)
	{ 
	  if 	(l != rst[i])
	    return(FALSE);
	  i++;
	}
	if (rst[i]) return(FALSE);
	return(TRUE);
}


/*
*	Routine to back off the end of a file string.
*/
BYTE *fs_back(REG BYTE *pstr, REG BYTE *pend)
{
						/* back off to last	*/
						/*   slash		*/
	while ( (*pend != ':') &&
		!issep(*pend)  &&
		(pend != pstr) )
	{
	  pend--;
	}
						/* if a : then insert	*/
						/*   a backslash	*/
	if (*pend == ':')
	{
	  pend++;
	  ins_char(pend, 0, '/', 64);
	}
	return(pend);
}


/*
*	Routine to back up a path and return the pointer to the beginning
*	of the file specification part
*/
BYTE *fs_pspec(REG BYTE *pstr, REG BYTE *pend)
{
	pend = fs_back(pstr, pend);
	if (issep(*pend))
	{
	  pend++;
	}
	else
	{
	  strcpy(pstr, "A:/*.*");	//, pstr);
	  pstr[0] += (BYTE) dos_gdrv();
	  pend = pstr + 3;
	}
	return(pend);
}

/*
*	Routine to compare based on type and then on name if its a file
*	else, just based on name
*/

WORD fs_comp()
{
	WORD		chk;

/*
	strcpy(dbuff, gl_tmp1);
	strcpy(dbuff+200, gl_tmp2);
	if (dbuff[0] == 7) dbuff[0] = '*';
	if (dbuff[200] == 7) dbuff[200] = '*';
	debug("fs_comp '%s' '%s'", dbuff, dbuff + 200);
*/
	if ( (gl_tmp1[0] == ' ') &&
	     (gl_tmp2[0] == ' ') )
	{
	  chk = strchk( scasb(&gl_tmp1[0], '.'), 
			scasb(&gl_tmp2[0], '.') );

	  if ( chk ) return( chk );
	}
	return ( strchk(&gl_tmp1[0], &gl_tmp2[0]) );
}

/* This would be used in qsort(), except that Pacific's qsort() does not
 * appear to be trustworthy 
int fs_qcomp(const void *p1, const void *p2)
{
	LPBYTE *l1 = (LPBYTE *)p1;
	LPBYTE *l2 = (LPBYTE *)p2;

	LSTCPY(gl_tmp1, *l1);
	LSTCPY(gl_tmp2, *l2);

	return fs_comp();
} */


WORD fs_add(WORD thefile, WORD fs_index)
{
	WORD		len;

/*
	LSTCPY(dbuff, ((LPBYTE)gl_dta) + 29);
	if (dbuff[0] == 7) dbuff[0] = '*';
	debug("fs_add[%d]: '%s' at %d", thefile, dbuff, fs_index);
*/
	len = LSTCPY(gl_fsnames + (LONG) fs_index, ((LPBYTE)gl_dta) + 29);
	g_fslist[thefile] = gl_fsnames + (LONG)fs_index;
	fs_index += len + 2;

	return(fs_index);
}


/*
*	Make a particular path the active path.  This involves
*	reading its directory, initializing a file list, and filling
*	out the information in the path node.  Then sort the files.
*/
WORD fs_active(LPBYTE ppath, BYTE *pspec, WORD *pcount)
{
	WORD		ret, thefile, len;
	WORD		fs_index;
	REG WORD	i, j, gap; 
	LPBYTE		temp; 
	LONG		vec;
/*	void		*ptr; */

	graf_mouse(HOURGLASS, 0x0L);

/* DEBUG CODE 
	LSTCPY(dbuff, ppath);
	debug("fs_active: ppath=%s gl_shdrive=0x%x", dbuff, gl_shdrive);
*/
	thefile = 0;
	fs_index = 0;
	len = 0;

	if (gl_shdrive)
	{
	  strcpy(gl_dta + 29, "\007 A:"); //, &gl_dta[29]);
	  vec = gl_bvdisk;
	  
	/* [JCE] Support for >16 drives... */
	  for(i=0; i<32; i++)
	  {
	    if ( vec & 0x80000000L )
	    {
	      gl_dta[31] = 'A' + i;
	      fs_index = fs_add(thefile, fs_index);
	      thefile++;
	    }
	    vec = vec << 1;
	  }
	}
	else
	{
	  dos_sdta(gl_dta);
	  ret = dos_sfirst(ppath, F_SUBDIR);
	  while ( ret )
	  {
						/* if it is a real file	*/
						/*   or directory then	*/
						/*   save it and set	*/
						/*   first byte to tell	*/
						/*   which		*/
	    if (gl_dta[30] != '.')
	    {
	      gl_dta[29] = (gl_dta[21] & F_SUBDIR) ? 0x07 : ' ';
	      if ( (gl_dta[29] == 0x07) ||
		   (wildcmp(pspec, &gl_dta[30])) )
	      {
		fs_index = fs_add(thefile, fs_index);
	        thefile++;
	      }
	    }
	    ret = dos_snext();

	    if (thefile >= NM_FILES)
	    {
	      ret = FALSE;
	      v_sound(vdi_handle, 660, 4);
	    }
	  }
	}
	*pcount = thefile;
/* Version using qsort() -- sadly, Pacific's qsort() does not appear to 
 * be trustworthy. 
	{
		int k;
		for (k = 0; k < thefile; k++)
		{
			LSTCPY(gl_tmp1, g_fslist[k]);
			if (gl_tmp1[0] == 7) gl_tmp1[0] = '*';
			debug("file %d: '%s' ", k, gl_tmp1);
		}
	}
	ptr = g_fslist;
	qsort(ptr, thefile, sizeof(g_fslist[0]), fs_qcomp);
*/
						/* sort files using shell*/
						/*   sort on page 108 of */
						/*   K&R C Prog. Lang.	*/

	for(gap = thefile/2; gap > 0; gap /= 2)
	{
	  for(i = gap; i < thefile; i++)
	  {
	    for (j = i-gap; j >= 0; j -= gap)
	    {
	      LSTCPY(gl_tmp1, g_fslist[j]);
	      LSTCPY(gl_tmp2, g_fslist[j+gap]);
	      if ( fs_comp() <= 0 )
		break;
	      temp = g_fslist[j];
	      g_fslist[j] = g_fslist[j+gap];
	      g_fslist[j+gap] = temp;
	    }
	  }
	}
	graf_mouse( ARROW, 0x0L);
	return(TRUE);
}


/*
*	Routine to adjust the scroll counters by one in either
*	direction, being careful not to overrun or underrun the
*	tail and heads of the list
*/
WORD fs_1scroll(REG WORD curr, REG WORD count, WORD touchob)
{
	REG WORD	newcurr;

	newcurr = (touchob == FUPAROW) ? (curr - 1) : (curr + 1);
	if (newcurr < 0)
	  newcurr++;
	if ( (count - newcurr) < NM_NAMES )
	  newcurr--;
	return( (count > NM_NAMES) ? newcurr : curr );
}


/*
*	Routine to take the filenames that will appear in the window, 
*	based on the current scrolled position, and point at them 
*	with the sub-tree of G_STRINGs that makes up the window box.
*/
VOID fs_format(REG LPTREE tree, WORD currtop, WORD count)
{
	REG WORD	i, cnt;
	REG WORD	y, h, th;
	LPBYTE		adtext;
	WORD		tlen;

						/* build in real text	*/
						/*   strings		*/
	gl_fspos = currtop;			/* save new position	*/
	cnt = min(NM_NAMES, count - currtop);
	for(i=0; i<NM_NAMES; i++)
	{
	  if (i < cnt)
	  {
	    LSTCPY(gl_tmp2,  g_fslist[currtop+i]);
	    fmt_str(&gl_tmp2[1], &gl_tmp1[1]);
	    gl_tmp1[0] = gl_tmp2[0];
	  }
	  else
	  {
	    gl_tmp1[0] = ' ';
	    gl_tmp1[1] = 0;
	  }

/* Defensive programming */
	  
#if DEBUG
	  if (tree == (LPTREE)0)
	  {
#asm
		  int	#3
#endasm
	  }

#endif
	  
	  fs_sset(tree, NAME_OFFSET+i, gl_tmp1, &adtext, &tlen);
	  tree[NAME_OFFSET+i].ob_type  = ((gl_shdrive) ? G_BOXTEXT : G_FBOXTEXT);
	  tree[NAME_OFFSET+i].ob_state = NORMAL;
	}
						/* size and position the*/
						/*   elevator		*/
	y = 0;
	th = h = tree[FSVSLID].ob_height;
	if ( count > NM_NAMES)
	{
	  h = mul_div(NM_NAMES, h, count);
	  h = max(gl_hbox1/2, h);		/* min size elevator	*/
	  y = mul_div(currtop, th-h, count - NM_NAMES);
	}
	tree[FSVELEV].ob_y = y;
	tree[FSVELEV].ob_height = h;
}


/*
*	Routine to select or deselect a file name in the scrollable 
*	list.
*/
VOID fs_sel(WORD sel, WORD state)
{
	if (sel)
	{
		gl_fstree [F1NAME + sel - 1].ob_state = state;
		ob_draw(gl_fstree, F1NAME + sel - 1, 0);
	}
}


/*
*	Routine to handle scrolling the directory window a certain number
*	of file names.
*/
WORD fs_nscroll(REG LPTREE tree, REG WORD *psel, 
		WORD curr, WORD count, WORD touchob, WORD n)
{
	REG WORD	i, newcurr, diffcurr;
	WORD		sy, dy, neg;
	GRECT		r[2];
						/* single scroll n times*/
	newcurr = curr;
	for (i=0; i<n; i++)
	  newcurr = fs_1scroll(newcurr, count, touchob);
						/* if things changed 	*/
						/*   then redraw	*/
	diffcurr = newcurr - curr;
	if (diffcurr)
	{
	  curr = newcurr;
	  fs_sel(*psel, NORMAL);
	  *psel = 0;
	  fs_format(tree, curr, count);
	  gsx_gclip((GRECT *)&r[1].g_x);
	  ob_actxywh(tree, F1NAME, (GRECT *)&r[0].g_x);

	  if (( neg = (diffcurr < 0)) != 0 )
	    diffcurr = -diffcurr;

	  if (diffcurr < NM_NAMES)
	  {
	    sy = r[0].g_y + (r[0].g_h * diffcurr);
	    dy = r[0].g_y;

	    if (neg)
	    {
	      dy = sy;
	      sy = r[0].g_y;
	    }

	    bb_screen(S_ONLY, r[0].g_x, sy, r[0].g_x, dy, r[0].g_w, 
				r[0].g_h * (NM_NAMES - diffcurr) );
	    if ( !neg )
	      r[0].g_y += r[0].g_h * (NM_NAMES - diffcurr);
	  }
	  else
	    diffcurr = NM_NAMES;

	  r[0].g_h *= diffcurr;
	  for(i=0; i<2; i++)
	  {
	    gsx_sclip((GRECT *)&r[i].g_x);
	    ob_draw(tree, ((i) ? FSVSLID : FILEBOX), MAX_DEPTH);
	  }
	}
	return(curr);
}


/*
*	Routine to call when a new directory has been specified.  This
*	will activate the directory, format it, and display ir[0].
*/
WORD fs_newdir(LPBYTE ftitle, LPBYTE fpath, BYTE *pspec, LPTREE tree, 
			   WORD *pcount, WORD pos)
{
	BYTE		*ptmp;
	WORD		len;

/** DEBUG CODE *
	LSTCPY(dbuff, ftitle);
	LSTCPY(dbuff + 250, fpath);
	debug("fs_newdir: Title=%s path=%s", dbuff, dbuff + 250);
*/
					/* BUGFIX 2.1 added len calculation*/
					/*  so FTITLE doesn't run over into*/
					/*  F1NAME.			*/
	ob_draw(tree, FSDIRECT, MAX_DEPTH);
	fs_active(fpath, pspec, pcount);
	if (pos+ NM_NAMES > *pcount)	/* in case file deleted		*/
	  pos = max(0, *pcount - NM_NAMES);
	fs_format(tree, pos, *pcount);
	len = LSTRLEN(ADDR(pspec));
	len = (len > LEN_FTITLE) ? LEN_FTITLE : len;
	*ftitle = ' ';
	ftitle++;
	LBCOPY(ftitle, ADDR(pspec), len);
	ftitle += len;
	*ftitle = ' ';
	ftitle++;
	*ftitle = 0;
	
	ptmp = &gl_fsobj[0];
	while(*ptmp)
	  ob_draw(tree, *ptmp++, MAX_DEPTH);
	return(TRUE);
}



MLOCAL VOID tidy_tree(LPTREE tree)
{
	LPTEDI ptitle;
	WORD n, m, l, h;
	LONG obspec;
	
/* [JCE 5-4-1999] Set the close and scroll buttons to look like 
 *                those on the windows */
MLOCAL WORD ctl[4] = { FCLSBOX,  FUPAROW,   FSVELEV, FDNAROW };
MLOCAL WORD wa [4] = { W_CLOSER, W_UPARROW, W_VELEV, W_DNARROW };
 	for (n = 0; n < 4; n++)
 	{
		l = h = 0;
		wind_get(wa[n], WF_OBFLAG, &m, NULL, NULL, NULL);
		wind_get(wa[n], WF_OBSPEC, &l, &h, NULL, NULL);
 		tree[ctl[n]].ob_flags &= ~FLAG3D;
 		tree[ctl[n]].ob_flags |= (m & FLAG3D);
		obspec = (long)MK_FP(h, l);
		obspec &= 0xFF000000L;
		obspec |= (long)(tree[ctl[n]].ob_spec) & 0xFFFFFFL;
		tree[ctl[n]].ob_spec = (LPVOID)(obspec);
 	}

/* [JCE 20-10-1999] Make all the scrollbar buttons a sensible size,
 * regardless of screen resolution */

	tree[FDNAROW].ob_height = tree[FUPAROW].ob_height;
	tree[FDNAROW].ob_y      = tree[SCRLBAR].ob_height - tree[FDNAROW].ob_height;
	tree[FSVSLID].ob_y      = tree[FUPAROW].ob_height;
	tree[FSVSLID].ob_height = tree[FDNAROW].ob_y - tree[FSVSLID].ob_y + 1;

	
	ptitle = (LPTEDI)(tree[FTITLE].ob_spec);

/*	ptitle->te_color = WTS_FG; */
}

/*
*	File Selector input routine that takes control of the mouse
*	and keyboard, searchs and sort the directory, draws the file 
*	selector, interacts with the user to determine a selection
*	or change of path, and returns to the application with
*	the selected path, filename, and exit button.
*/
WORD fs_exinput(LPBYTE pipath, LPBYTE pisel, WORD *pbutton, LPBYTE pname)
{
	REG WORD	touchob, value, fnum;
	WORD		curr, count, sel;
	WORD		mx, my, status;
	REG LPTREE	tree;
	LPBYTE		ad_fpath, ad_fname, ad_ftitle, ad_locstr;
	WORD		fname_len, fpath_len, temp_len; 
	WORD		dclkret, cont, firsttime, newname, elevpos;
	REG BYTE	*pstr, *pspec;
	GRECT		pt;
	BYTE		locstr[64];

/*	debug("-----------------fs_exinput------------------------"); */

					/* get out quick if path is	*/
					/*   nullptr or if pts to null.	*/
	if (pipath    == 0x0L   ||
	    pipath[0] == 0) return(FALSE);

	appl_xbvget(&gl_bvdisk, &gl_bvhard);
/*
	debug("gl_bvdisk=%08lx gl_bvhard=%08lx\n",
			(long)gl_bvdisk, (long)gl_bvhard); */
	rsrc_gaddr(R_TREE, FSELECTR, (LPVOID *)&gl_fstree);
	form_center(gl_fstree, &gl_rfs.g_x, &gl_rfs.g_y, &gl_rfs.g_w, &gl_rfs.g_h);

						/* get memory for 	*/
						/*   the string buffer	*/
	tree = gl_fstree;

	ad_locstr = ADDR(&locstr[0]);
/* Initialise locstr to something. Uninitialised buffers offend me. */
	locstr[0] = -1;
	locstr[1] = 0;
						/* init strings in form	*/
	((LPTEDI)(tree[NFSTITLE].ob_spec))->te_ptext = pname;

						
	ad_ftitle = *(LPBYTE FAR *)(tree[FTITLE].ob_spec);
	LSTCPY(ad_ftitle, ADDR(" *.* "));
	if (LSTCMP(pipath, *(LPBYTE FAR *)(tree[FSDIRECT].ob_spec)))
	  elevpos = gl_fspos;			/* same dir as last time */	
	else					
	  elevpos = 0;
  	fs_sset(tree, FSDIRECT, pipath, &ad_fpath, &temp_len);
	LSTCPY(gl_tmp1, pisel);
	fmt_str(&gl_tmp1[0], &gl_tmp2[0]);
	fs_sset(tree, FSSELECT, gl_tmp2, &ad_fname, &fname_len);
						/* set clip and start	*/
						/*   form fill-in by	*/
						/*   drawing the form	*/
	gsx_sclip(&gl_rfs);	
	form_dial(FMD_START, gl_rfs.g_x, gl_rfs.g_y, gl_rfs.g_w, gl_rfs.g_h,
			     gl_rfs.g_x, gl_rfs.g_y, gl_rfs.g_w, gl_rfs.g_h);
	g_dir[0] = NULL;			

	tidy_tree(tree);


	ob_draw(tree, ROOT, 2);
						/* init for while loop	*/
						/*   by forcing initial	*/
						/*   fs_newdir call	*/
	sel = 0;
	newname = gl_shdrive = FALSE;
	cont = firsttime = TRUE;
	while( cont )
	{
	  touchob = (firsttime) ? 0x0 : form_do(tree, FSSELECT);
	  vq_mouse(vdi_handle, &status, &mx, &my);
	
	  fpath_len = LSTCPY(ad_locstr, ad_fpath);
	  if ( strcmp(&g_dir[0], &locstr[0]) )
	  {
	    fs_sel(sel, NORMAL);
	    if ( (touchob == FSOK) ||
		 (touchob == FSCANCEL) )
	      ob_change(tree, touchob, NORMAL, TRUE);
	    strcpy(g_dir, locstr);	//&locstr[0], &g_dir[0]);
	    pspec = fs_pspec(&g_dir[0], &g_dir[fpath_len]);	    
/*	    LSTCPY(ad_fpath, ADDR(&g_dir[0])); */
  	    fs_sset(tree, FSDIRECT, ADDR(&g_dir[0]), &ad_fpath, &temp_len);
	    pstr = fs_pspec(&locstr[0], &locstr[fpath_len]);	    
	    strcpy(pstr, "*.*"); //strcpy("*.*", pstr);
	    fs_newdir(ad_ftitle, ad_locstr, pspec, tree, &count, elevpos);
	    curr = elevpos;
	    sel = touchob = elevpos = 0;
	    firsttime = FALSE;
	  }

	  value = 0;
	  dclkret = ((touchob & 0x8000) != 0);
	  switch( (touchob &= 0x7fff) )
	  {
	    case FSOK:
	    case FSCANCEL:
		cont = FALSE;
		break;
	    case FUPAROW:
	    case FDNAROW:
		value = 1;
		break;
	    case FSVSLID:
		ob_actxywh(tree, FSVELEV, &pt);
#if APPLE_COMPLIANT
		pt.g_x -= 3;
		pt.g_w += 6;
#endif
		if ( rc_inside(mx, my, &pt) )
		  goto dofelev;
		touchob = (my <= pt.g_y) ? FUPAROW : FDNAROW;
		value = NM_NAMES;
		break;
	    case FSVELEV:
dofelev:	wind_update(3); //fm_own(TRUE);
		ob_relxywh(tree, FSVSLID, &pt);
#if APPLE_COMPLIANT
		pt.g_x += 3;		/* APPLE	*/
		pt.g_w -= 6;
#endif
		tree[FSVSLID].ob_x      = pt.g_x;
		tree[FSVSLID].ob_width  = pt.g_w;
		value = graf_slidebox(tree, FSVSLID, FSVELEV, TRUE);
#if APPLE_COMPLIANT
		pt.g_x -= 3;
		pt.g_w += 6;
#endif
		tree[FSVSLID].ob_x      = pt.g_x;
		tree[FSVSLID].ob_width  = pt.g_w;
		wind_update(2); //fm_own(FALSE);
		value = curr - mul_div(value, count-NM_NAMES, 1000);
		if (value >= 0)
		  touchob = FUPAROW;
		else
		{
		  touchob = FDNAROW;
		  value = -value;
		}
		break;
	    case F1NAME:
	    case F2NAME:
	    case F3NAME:
	    case F4NAME:
	    case F5NAME:
	    case F6NAME:
	    case F7NAME:
	    case F8NAME:
	    case F9NAME:
		fnum = touchob - F1NAME + 1;
		if ( fnum <= count )
		{
		  if ( (sel) && (sel != fnum) )
		  {
		    fs_sel(sel, NORMAL);
		  }
		  if ( sel != fnum)
		  {
		    sel = fnum;
		    fs_sel(sel, SELECTED);
		  }
						/* get string and see	*/
						/*   if file or folder	*/
		  fs_sget(tree, touchob, gl_tmp1);
		  if (gl_tmp1[0] == ' ')
		  {
						/* copy to selection	*/
			newname = TRUE;
			if (dclkret) cont = FALSE;
		  }
		  else
		  {
		    if (gl_shdrive)
		    {
						/* prepend in drive name*/
		      if (locstr[1] == ':')
		        locstr[0] = gl_tmp1[2];
		    }
		    else
		    {
						/* append in folder name*/
		        pstr = fs_pspec(&locstr[0], &locstr[fpath_len]);
/* Get last character? */
			strcpy(gl_tmp2, pstr - 1);  //pstr - 1, &gl_tmp2[0]);
			unfmt_str(&gl_tmp1[1], pstr);
			strcat(pstr, &gl_tmp2[0]);
		    }
		    firsttime = TRUE;
		  }
		  gl_shdrive = FALSE;
		}
		break;
		case FSDRIVES:
			pspec = pstr = fs_back(&locstr[0], &locstr[fpath_len]);
			if (issep(*pstr--))
			{
		  		firsttime = TRUE;
		  		while (*pstr != ':')
		  		{
		  		  pstr = fs_back(&locstr[0], pstr);
		    	  	  if (issep(*pstr)) strcpy(pstr, pspec);
		   		  --pstr;
		  		}
		  		if (gl_bvdisk) gl_shdrive = TRUE;
			}
			break;
		
	    case FCLSBOX:
		pspec = pstr = fs_back(&locstr[0], &locstr[fpath_len]);
		if (issep(*pstr--))
		{
		  firsttime = TRUE;
		  if (*pstr != ':')
		  {
		    pstr = fs_back(&locstr[0], pstr);
		    if (issep(*pstr))
		      strcpy(pstr, pspec);
		  }
		  else
		  {
		    if (gl_bvdisk)
		      gl_shdrive = TRUE;
		  }
		}
		break;
	    case FTITLE:
		firsttime = TRUE;
		break;
	  }
	  if (firsttime)
	  {
	   /* LSTCPY(ad_fpath, ad_locstr); */
  	    fs_sset(tree, FSDIRECT, ad_locstr, &ad_fpath, &temp_len);
	    g_dir[0] = NULL;
	    gl_tmp1[1] = NULL;
	    newname = TRUE;
	  }
	  if (newname)
	  {
	    LSTCPY(ad_fname, gl_tmp1 + 1);
	    ob_draw(tree, FSSELECT, MAX_DEPTH);
	    if (!cont)
	      ob_change(tree, FSOK, SELECTED, TRUE);
	    newname = FALSE;
	  }
	  if (value)
	    curr = fs_nscroll(tree, &sel, curr, count, touchob, value);
	}
						/* return path and	*/
						/*   file name to app	*/
	LSTCPY(pipath, ad_fpath);
	LSTCPY(gl_tmp1, ad_fname);
	unfmt_str(&gl_tmp1[0], &gl_tmp2[0]);
	LSTCPY(pisel, gl_tmp2);
						/* start the redraw	*/
	form_dial(FMD_FINISH, gl_rfs.g_x, gl_rfs.g_y, gl_rfs.g_w, gl_rfs.g_h,
			     gl_rfs.g_x, gl_rfs.g_y, gl_rfs.g_w, gl_rfs.g_h);
						/* return exit button	*/
	*pbutton = inf_what(tree, FSOK, FSCANCEL);
	return( TRUE );
}



/************************************************************************/
/************************************************************************/
/****								     ****/
/****		    Item Selector event loop			     ****/
/****								     ****/
/************************************************************************/
/************************************************************************/
/*
MLOCAL char ipath[256];
MLOCAL char isel[256];
MLOCAL char *ititle = "Test Item Selector";
*/

typedef struct msg_100
{
	WORD	code;
	WORD	sender;
	LPBYTE	pipath;
	LPBYTE	pisel;
	LPBYTE	pititle;
} MSG_100;


WORD hndl_mesag()
{
	WORD w, b;
	MSG_100 *msg100;
	WORD result[8];

	if (!gl_working) return;

	switch(gl_rmsg[0])
	{
		case AC_OPEN:
/*			strcpy(ipath, "D:\\*.*");
			strcpy(isel, "");
			w = fs_exinput(ipath, isel, &b, ititle); */
/*			debug("[1][w=%d b=%d|ipath=%s|isel=%s][ OK ]",
				w, b, ipath, isel); */
			break;

		case 100:
			msg100 = (MSG_100*)gl_rmsg;
			w = fs_exinput(msg100->pipath,
				       msg100->pisel,
				       &b,
				       msg100->pititle);
			result[0] = 101;
			result[1] = gl_apid;
			result[2] = w;
			result[3] = b;
			result[4] = result[5] = 
			result[6] = result[7] = 0;
			appl_write(gl_rmsg[1], 16, ADDR(result));
			break;

	}
	return 1;
}


itemsel()
{
        BOOLEAN done;

        /**/                                    /* loop handling user   */
        /**/                                    /*   input until done   */
        done = FALSE;                           /*   -or- if DESKACC    */
        while( !done )                          /*   then forever       */
        {
                ev_which = evnt_mesag(ad_rmsg); /* wait for message     */
                hndl_mesag();  		        /* handle event message */
        }
}



/*

Page*/
/************************************************************************/
/************************************************************************/
/****								     ****/
/****			    Initialization			     ****/
/****								     ****/
/************************************************************************/
/************************************************************************/

/*------------------------------*/
/*	acc1_init		*/
/*------------------------------*/
WORD itemsel_init()
{
	WORD	i;
	WORD	work_in[11];

	gl_apid = appl_init(NULL);			/* initialize libraries	*/
	wind_update(BEG_UPDATE);
	for (i=0; i<10; i++)
	{
		work_in[i]=1;
	}
	work_in[10]=2;
	if (!rsrc_load(ADDR("GEMFSLIB.RSC")))
        {
                form_alert(1, "[3][Fatal Error|GEMFSLIB.RSC not found][ Abort ]");
		return FALSE;
        }
	gem_handle = graf_handle(&gl_wchar,&gl_hchar,&gl_wbox1,&gl_hbox1);
	vdi_handle = gem_handle;
	v_opnvwk(work_in,&vdi_handle,work_out);	/* open virtual work stn*/

	/* Testing code, allowing the selector to be invoked manually
	gl_itemacc1 = menu_register(gl_apid, ADDR("  Item Sel") );
	*/
						/* init. message address*/
	ad_rmsg = ADDR((BYTE *) &gl_rmsg[0]);

	gl_working = 1;
	return(TRUE);
}

/*

Page*/
/************************************************************************/
/************************************************************************/
/****								     ****/
/****			    Main Program			     ****/
/****								     ****/
/************************************************************************/
/************************************************************************/


/*------------------------------*/
/*	GEMAIN			*/
/*------------------------------*/
WORD GEMAIN(WORD argc, BYTE *ARGV[])
{
	if (itemsel_init())			/* initialization	*/
	{
		wind_update(END_UPDATE);
		itemsel();
	}
	return 0;
}



