/*	GEMPD.C		1/27/84 - 03/20/85	Lee Jay Lorenzen	*/
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

/* ---------- added for metaware compiler ---------- */
						/* in OPTIMOPT.C 	*/
EXTERN VOID 	movs();
EXTERN WORD 	strcmp();
EXTERN VOID	bfill();

EXTERN VOID 	setdsss();			/* in DOSIF.A86		*/
EXTERN VOID 	psetup();

/* -------------------------------------------------- */

EXTERN THEGLO	D;				/* in GLOBAL.C		*/

EXTERN WORD	totpds;
EXTERN VOID show_ems(char c, UDA *uda);

	PD
*pd_index(i)
	WORD		i;
{
	return( (i<2) ? &D.g_intpd[i] : &D.g_extpd[i-2] );
}


	PD
*fpdnm(pname, pid)
	BYTE		*pname;
	UWORD		pid;
{
	WORD		i;
	BYTE		temp[9];
	PD		*p;

	temp[8] = NULL;
	for(i=0; i<totpds; i++)
	{
	  p = pd_index(i);
	  if (pname != NULLPTR)
	  {
	    movs(8, p->p_name, &temp[0]);
	    if (strcmp(pname, &temp[0]) )
	      return(p);
	  }
	  else
	    if (p->p_pid == pid)
	      return(p);
	}
	return(0);
}


PD *getpd()
{
	REG PD		*p;
						/* we got all our 	*/
						/*   memory so link it	*/
	p = pd_index(curpid);
	p->p_pid = curpid++;
						/* initialize his DS&SS	*/
						/*   registers so	*/
						/*   stproc works	*/
	setdsss(p->p_uda);
						/* return the pd we got	*/
	return(p);
}


	VOID
p_nameit(p, pname)
	PD		*p;
	BYTE		*pname;
{
	bfill(8, ' ',  &p->p_name[0]);
	strscn(pname, &p->p_name[0], '.');
}


PD *pstart(BYTE *pcode, BYTE *pfilespec, LPBYTE ldaddr, BOOLEAN in_ems, WORD ems_handle)
{
	REG PD		*px;

						/* create process to	*/
						/*   execute it		*/
	px = getpd();
	px->p_ldaddr = ldaddr;
						/* copy in name of file	*/
	p_nameit(px, pfilespec);
						/* cs, ip, use 0 flags	*/
	psetup(px, pcode);
						/* link him up		*/
	px->p_stat &= ~WAITIN;
	px->p_link = drl;
	drl = px;
/* If ACC loaded in EMS, save the current EMS paging state so that when
 * the ACC runs it will be restored. */
#if DEBUG
	{
		int n;
		TRACE("Launching $");
		for (n = 0; n < 8; n++) TRCHAR(px->p_name[n]);
		TRACE(" uda=$");
		ophex((WORD)(px->p_uda));
		TRACE("\r\n$");
	}
#endif
/* Allocate a chunk of the EMS state save array. We allocate one for 
 * each process and two for temporary. */
	px->p_uda->u_ems = D.g_emsmap + (gl_emm_mapsize * (2 + px->p_pid));
/* Whether or not the process is running in EMS, save EMS state. Otherwise
 * the dispatcher will try to restore a state which was never saved, with
 * hilarious consequences */
	EMS_PageMap(0, NULL, px->p_uda->u_ems);
	if (in_ems)
	{
		px->p_uda->u_emshandle = ems_handle;
	}
	show_ems('!', px->p_uda);
	return(px);
}

						/* put pd pi into list 	*/
						/*   *root at the end	*/
	VOID
insert_process(pi, root)
	PD		*pi, **root;
{
	REG PD		*p, *q;
						/* find the end		*/
	for ( p = (q = (PD *) root) -> p_link ; p ; p = (q = p) -> p_link); 
						/* link him in		*/
	pi->p_link = p;
	q->p_link = pi;
}



