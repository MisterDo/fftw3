/*
 * Copyright (c) 2003 Matteo Frigo
 * Copyright (c) 2003 Massachusetts Institute of Technology
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

/* $Id: dftw-direct.c,v 1.2 2004-10-24 05:18:14 stevenj Exp $ */

#include "ct.h"

typedef struct {
     ct_solver super;
     const ct_desc *desc;
     kdftw k;
} S;

typedef struct {
     plan_dftw super;
     kdftw k;
     int vl;
     int s, vs;
     int mcount;
     stride ios;
     const R *tdW;
     int mstart, m, r;
     twid *td;
     const S *slv;
} P;

static void apply(const plan *ego_, R *rio, R *iio)
{
     const P *ego = (const P *) ego_;
     int i, vl = ego->vl, s = ego->s, vs = ego->vs, mcount = ego->mcount;
     const R *W = ego->tdW;
     ASSERT_ALIGNED_DOUBLE;
     for (i = 0; i < vl; ++i)
	  ego->k(rio + i * vs, iio + i * vs, W, ego->ios, mcount, s);
}

static void awake(plan *ego_, int flg)
{
     P *ego = (P *) ego_;

     X(twiddle_awake)(flg, &ego->td, ego->slv->desc->tw, 
		      ego->r * ego->m, ego->r, ego->m);
     ego->tdW = X(twiddle_shift)(ego->td, ego->mstart);
}

static void destroy(plan *ego_)
{
     P *ego = (P *) ego_;
     X(stride_destroy)(ego->ios);
}

static void print(const plan *ego_, printer *p)
{
     const P *ego = (const P *) ego_;
     const S *slv = ego->slv;
     const ct_desc *e = slv->desc;

     p->print(p, "(dftw-direct-%d/%d%v \"%s\")",
              ego->r, X(twiddle_length)(ego->r, e->tw), ego->vl, e->nam);
}

static int applicable0(const S *ego, 
		       int dec, int r, int m, int s, int vl, int vs, 
		       R *rio, R *iio,
		       const planner *plnr)
{
     const ct_desc *e = ego->desc;
     UNUSED(vl);

     return (
	  1

	  && dec == ego->super.dec
	  && r == e->radix

	  /* check for alignment/vector length restrictions */
	  && (e->genus->okp(e, rio, iio, m * s, 0, m, s, plnr))
	  && (e->genus->okp(e, rio + vs, iio + vs, m * s, 0, m, s, plnr))
				 
	  );
}

static int applicable(const S *ego, 
		      int dec, int r, int m, int s, int vl, int vs, 
		      R *rio, R *iio,
		      const planner *plnr)
{
     if (!applicable0(ego, dec, r, m, s, vl, vs, rio, iio, plnr))
          return 0;

     if (NO_UGLYP(plnr) && X(ct_uglyp)(16, m * r, r))
	  return 0;

     return 1;
}

static plan *mkcldw(const ct_solver *ego_, 
		    int dec, int r, int m, int s, int vl, int vs, 
		    int mstart, int mcount,
		    R *rio, R *iio,
		    planner *plnr)
{
     const S *ego = (const S *) ego_;
     P *pln;
     const ct_desc *e = ego->desc;

     static const plan_adt padt = {
	  0, awake, print, destroy
     };

     A(mstart >= 0 && mstart + mcount <= m);
     if (!applicable(ego, dec, r, m, s, vl, vs, rio, iio, plnr))
          return (plan *)0;

     pln = MKPLAN_DFTW(P, &padt, apply);

     pln->k = ego->k;
     pln->ios = X(mkstride)(r, m * s);
     pln->td = 0;
     pln->tdW = 0;
     pln->r = r;
     pln->m = m;
     pln->s = s;
     pln->vl = vl;
     pln->vs = vs;
     pln->mstart = mstart;
     pln->mcount = mcount;
     pln->slv = ego;

     X(ops_zero)(&pln->super.super.ops);
     X(ops_madd2)(vl * (mcount/e->genus->vl), &e->ops, &pln->super.super.ops);

     return &(pln->super.super);
}

void X(regsolver_ct_directw)(planner *plnr,
				 kdftw codelet, const ct_desc *desc, int dec)
{
     S *slv = (S *)X(mksolver_ct)(sizeof(S), desc->radix, dec, mkcldw);
     slv->k = codelet;
     slv->desc = desc;
     REGISTER_SOLVER(plnr, &(slv->super.super));
     if (X(mksolver_ct_hook)) {
	  slv = (S *)X(mksolver_ct_hook)(sizeof(S), desc->radix, 
					     dec, mkcldw);
	  slv->k = codelet;
	  slv->desc = desc;
	  REGISTER_SOLVER(plnr, &(slv->super.super));
     }
}
