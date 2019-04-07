/* Copyright (c) 2002-2005 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* This is an entirely rewritten version of Joseph A. Sarlo's code.
   The most important changes are listed in "pd-lib-notes.txt" file.  */

#include "m_pd.h"
#include <common/api.h>

#define DECODE_C74MAXOUTS  512  // CHECKED (max goes up higher but freezes) */
#define DECODE_DEFOUTS     1

typedef struct _Decode
{
    t_object    x_ob;
    int         x_numouts;
    int         x_onout;
    int         x_allon;   /* submaster switch */
    int         x_alloff;  /* master switch */
    t_outlet  **x_outs;
    t_outlet   *x_outbuf[DECODE_C74MAXOUTS];
} t_Decode;

static t_class *Decode_class;

/* CHECKED: all outlets deliver after any action */
/* CHECKED: outlets output in right-to-left order */


static void Decode_deliver(t_Decode *x)
{
    int i = x->x_numouts;
    if (x->x_alloff)
	while (i--) outlet_float(x->x_outs[i], 0);
    else if (x->x_allon)
	while (i--) outlet_float(x->x_outs[i], 1);
    else
	while (i--) outlet_float(x->x_outs[i], (i == x->x_onout ? 1 : 0));
}

static void Decode_float(t_Decode *x, t_floatarg f)
{
    int val = (int)f;
    /* CHECKED: out-of-range input is clipped, not ignored */
    if (val < 0)
	val = 0;
    else if (val >= x->x_numouts)
	val = x->x_numouts - 1;
    /* CHECKED: while in all-off mode, input is stored, not ignored */
    x->x_onout = val;
    Decode_deliver(x);
}


static void Decode_bang(t_Decode *x)
{
    Decode_deliver(x);
}

static void Decode_allon(t_Decode *x, t_floatarg f)
{
    x->x_allon = (f != 0);
    Decode_deliver(x);
}

static void Decode_alloff(t_Decode *x, t_floatarg f)
{
    x->x_alloff = (f != 0);
    Decode_deliver(x);
}

static void Decode_free(t_Decode *x)
{
    if (x->x_outs != x->x_outbuf)
	freebytes(x->x_outs, x->x_numouts * sizeof(*x->x_outs));
}

static void *Decode_new(t_floatarg val){
    t_Decode *x;
    int i, nouts = (int)val;
    t_outlet **outs;
    if (nouts < 1)
	nouts = DECODE_DEFOUTS;
    if (nouts > DECODE_C74MAXOUTS){
        nouts = DECODE_C74MAXOUTS;
        if (!(outs = (t_outlet **)getbytes(nouts * sizeof(*outs))))
            return (0);
    }
    else outs = 0;
    x = (t_Decode *)pd_new(Decode_class);
    x->x_numouts = nouts;
    x->x_outs = (outs ? outs : x->x_outbuf);
    x->x_onout = 0;
    x->x_allon = 0;
    x->x_alloff = 0;
    inlet_new((t_object *)x, (t_pd *)x, &s_float, gensym("ft1"));
    inlet_new((t_object *)x, (t_pd *)x, &s_float, gensym("ft2"));
    for (i = 0; i < nouts; i++)
	x->x_outs[i] = outlet_new((t_object *)x, &s_float);
    return (x);
}

CYCLONE_OBJ_API void decode_setup(void)
{
    Decode_class = class_new(gensym("decode"),
			     (t_newmethod)Decode_new,
			     (t_method)Decode_free,
			     sizeof(t_Decode), 0, A_DEFFLOAT, 0);
    class_addcreator((t_newmethod)Decode_new, gensym("Decode"), A_DEFFLOAT, 0);
    class_addcreator((t_newmethod)Decode_new, gensym("cyclone/Decode"), A_DEFFLOAT, 0);
    class_addfloat(Decode_class, Decode_float);
    class_addbang(Decode_class, Decode_bang);
    class_addmethod(Decode_class, (t_method)Decode_allon,
		    gensym("ft1"), A_FLOAT, 0);
    class_addmethod(Decode_class, (t_method)Decode_alloff,
		    gensym("ft2"), A_FLOAT, 0);
}

void Decode_setup(void)
{
    decode_setup();
}
