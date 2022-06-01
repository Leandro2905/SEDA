/***************************************************************************
 *                                                                         *
 *    EMSLIB - Support functions for GEM pursuant to putting GEM's         *
 *            internal state into EMS memory                               *
 *    Copyright (C) 2004  John Elliott <jce@seasip.demon.co.uk>            *
 *                                                                         *
 *    This library is free software; you can redistribute it and/or        *
 *    modify it under the terms of the GNU Library General Public          *
 *    License as published by the Free Software Foundation; either         *
 *    version 2 of the License, or (at your option) any later version.     *
 *                                                                         *
 *    This library is distributed in the hope that it will be useful,      *
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of       *
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU    *
 *    Library General Public License for more details.                     *
 *                                                                         *
 *    You should have received a copy of the GNU Library General Public    *
 *    License along with this library; if not, write to the Free           *
 *    Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,      *
 *    MA 02111-1307, USA                                                   *
 *                                                                         *
 ***************************************************************************/

#include "aes.h"



void ems_get_window(int wh, WINDOW *win)
{
	int w;

	if (wh >= NUM_WIN)
	{
		bfill(sizeof(*win), 0, win);
#if DEBUG
		TRACE("Error: Accessing out-of-range window handle\r\n$");
#endif
		return;
	}
	EMS_PageMap(0, NULL, D.g_emsmap); 
	for (w = 0; w < gl_emm_size; w++) 
	{
              EMS_Map(D.g_emshandle, w, w); 
	}
	LBCOPY(ADDR(win), &D.w_emmwin[wh], sizeof(*win));
	EMS_PageMap(1, D.g_emsmap, NULL); 
}



void ems_put_window(int wh, WINDOW *win)
{
	int w;

	if (wh >= NUM_WIN)
	{
#if DEBUG
		TRACE("Error: Accessing out-of-range window handle\r\n$");
#endif
		return;
	}
/*	if (wh == 1) 
	{
		TRACE("Writing window 1: work rect=($");
		ophex(win->w_xwork); TRACE(",");
		ophex(win->w_ywork); TRACE(",");
		ophex(win->w_wwork); TRACE(",");
		ophex(win->w_hwork); TRACE(")\r\n$");
	} */
	EMS_PageMap(0, NULL, D.g_emsmap); 
	for (w = 0; w < gl_emm_size; w++) 
	{
              EMS_Map(D.g_emshandle, w, w); 
	}
	LBCOPY(&D.w_emmwin[wh], ADDR(win), sizeof(*win));
	EMS_PageMap(1, D.g_emsmap, NULL); 
}



/* ORECTs use a 1-based numbering system, so that '0' can mean 
 * 'null pointer' */ 
void ems_get_orect(int orect, ORECT *or)
{
	int w;

	if (orect == 0 || orect > (NUM_WIN * 10))
	{
		bfill(sizeof(*or), 0, or);
#if DEBUG
		TRACE("Error: Reading out-of-range ORECT $");
		ophex(orect);
		TRACE("\r\n$");
#endif
		return;
	}
	EMS_PageMap(0, NULL, D.g_emsmap); 
	for (w = 0; w < gl_emm_size; w++) 
	{
              EMS_Map(D.g_emshandle, w, w); 
	}
	LBCOPY(ADDR(or), &D.g_emmolist[orect-1], sizeof(*or));
	EMS_PageMap(1, D.g_emsmap, NULL); 
}




void ems_put_orect(int orect, ORECT *or)
{
	int w;

	if (orect == 0 || orect > (NUM_WIN * 10))
	{
#if DEBUG
		TRACE("Error: Writing out-of-range ORECT $");
		ophex(orect);
		TRACE("\r\n$");
#endif
		return;
	}
	--orect;
	EMS_PageMap(0, NULL, D.g_emsmap); 
	for (w = 0; w < gl_emm_size; w++) 
	{
              EMS_Map(D.g_emshandle, w, w); 
	}
	LBCOPY(&D.g_emmolist[orect], ADDR(or), sizeof(ORECT));
	EMS_PageMap(1, D.g_emsmap, NULL); 
}





void ems_get_wtree(int wh, OBJECT *ob)
{
	int w;

	if (wh >= NUM_WIN)
	{
		bfill(sizeof(*ob), 0, ob);
#if DEBUG
		TRACE("Error: Accessing out-of-range window handle\r\n$");
#endif
		return;
	}
	EMS_PageMap(0, NULL, D.g_emsmap); 
	for (w = 0; w < gl_emm_size; w++) 
	{
              EMS_Map(D.g_emshandle, w, w); 
	}
	LBCOPY(ADDR(ob), &D.g_emmtree[wh], sizeof(*ob));
	EMS_PageMap(1, D.g_emsmap, NULL); 
}



void ems_put_wtree(int wh, OBJECT *ob)
{
	int w;

	if (wh >= NUM_WIN)
	{
#if DEBUG
		TRACE("Error: Accessing out-of-range window handle\r\n$");
#endif
		return;
	}
	EMS_PageMap(0, NULL, D.g_emsmap); 
	for (w = 0; w < gl_emm_size; w++) 
	{
              EMS_Map(D.g_emshandle, w, w); 
	}
	LBCOPY(&D.g_emmtree[wh], ADDR(ob), sizeof(*ob));
	EMS_PageMap(1, D.g_emsmap, NULL); 
}



