/**************************************************************************************************
*                                                                                                 *
* This file is part of HPIPM.                                                                     *
*                                                                                                 *
* HPIPM -- High Performance Interior Point Method.                                                *
* Copyright (C) 2017 by Gianluca Frison.                                                          *
* Developed at IMTEK (University of Freiburg) under the supervision of Moritz Diehl.              *
* All rights reserved.                                                                            *
*                                                                                                 *
* HPIPM is free software; you can redistribute it and/or                                          *
* modify it under the terms of the GNU Lesser General Public                                      *
* License as published by the Free Software Foundation; either                                    *
* version 2.1 of the License, or (at your option) any later version.                              *
*                                                                                                 *
* HPIPM is distributed in the hope that it will be useful,                                        *
* but WITHOUT ANY WARRANTY; without even the implied warranty of                                  *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                                            *
* See the GNU Lesser General Public License for more details.                                     *
*                                                                                                 *
* You should have received a copy of the GNU Lesser General Public                                *
* License along with HPIPM; if not, write to the Free Software                                    *
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA                  *
*                                                                                                 *
* Author: Gianluca Frison, gianluca.frison (at) imtek.uni-freiburg.de                             *
*                                                                                                 *
**************************************************************************************************/



int MEMSIZE_OCP_QP(struct OCP_QP_DIM *dim)
	{

	// extract dim
	int N = dim->N;
	int *nx = dim->nx;
	int *nu = dim->nu;
	int *nb = dim->nb;
	int *ng = dim->ng;
	int *ns = dim->ns;

	// loop index
	int ii;

	int nbt = 0;
	int ngt = 0;
	for(ii=0; ii<=N; ii++)
		{
		nbt += nb[ii];
		ngt += ng[ii];
		}

	int size = 0;

	size += 5*(N+1)*sizeof(int); // nx nu nb ng ns
	size += 2*(N+1)*sizeof(int *); // idxb idxs
	size += 2*(N+1)*sizeof(struct STRMAT); // RSqrq DCt
	size += 1*N*sizeof(struct STRMAT); // BAbt
	size += 4*(N+1)*sizeof(struct STRVEC); // rq d Z z
	size += 1*N*sizeof(struct STRVEC); // b

	for(ii=0; ii<N; ii++)
		{
		size += nb[ii]*sizeof(int); // idxb
		size += ns[ii]*sizeof(int); // idxs
		size += SIZE_STRMAT(nu[ii]+nx[ii]+1, nx[ii+1]); // BAbt
		size += SIZE_STRVEC(nx[ii+1]); // b
		size += SIZE_STRMAT(nu[ii]+nx[ii]+1, nu[ii]+nx[ii]); // RSQrq
		size += SIZE_STRVEC(nu[ii]+nx[ii]); // rq
		size += SIZE_STRMAT(nu[ii]+nx[ii], ng[ii]); // DCt
		size += 2*SIZE_STRVEC(2*ns[ii]); // Z z
		}
	ii = N;
	size += nb[ii]*sizeof(int); // idxb
	size += ns[ii]*sizeof(int); // idxs
	size += SIZE_STRMAT(nu[ii]+nx[ii]+1, nu[ii]+nx[ii]); // RSQrq
	size += SIZE_STRVEC(nu[ii]+nx[ii]); // rq
	size += SIZE_STRMAT(nu[ii]+nx[ii], ng[ii]); // DCt
	size += 2*SIZE_STRVEC(2*ns[ii]); // Z z

	size += 1*SIZE_STRVEC(2*nbt+2*ngt); // d

	size = (size+63)/64*64; // make multiple of typical cache line size
	size += 64; // align to typical cache line size
	
	return size;

	}



void CREATE_OCP_QP(struct OCP_QP_DIM *dim, struct OCP_QP *qp, void *mem)
	{

	// extract dim
	int N = dim->N;
	int *nx = dim->nx;
	int *nu = dim->nu;
	int *nb = dim->nb;
	int *ng = dim->ng;
	int *ns = dim->ns;

	// loop index
	int ii;

	int nbt = 0;
	int ngt = 0;
	for(ii=0; ii<=N; ii++)
		{
		nbt += nb[ii];
		ngt += ng[ii];
		}


	// int pointer stuff
	int **ip_ptr;
	ip_ptr = (int **) mem;

	// idxb
	qp->idxb = ip_ptr;
	ip_ptr += N+1;

	// idxs
	qp->idxs = ip_ptr;
	ip_ptr += N+1;


	// matrix struct stuff
	struct STRMAT *sm_ptr = (struct STRMAT *) ip_ptr;

	// BAbt
	qp->BAbt = sm_ptr;
	sm_ptr += N;

	// RSQrq
	qp->RSQrq = sm_ptr;
	sm_ptr += N+1;

	// DCt
	qp->DCt = sm_ptr;
	sm_ptr += N+1;


	// vector struct stuff
	struct STRVEC *sv_ptr = (struct STRVEC *) sm_ptr;

	// b
	qp->b = sv_ptr;
	sv_ptr += N;

	// rq
	qp->rq = sv_ptr;
	sv_ptr += N+1;

	// d
	qp->d = sv_ptr;
	sv_ptr += N+1;

	// Z
	qp->Z = sv_ptr;
	sv_ptr += N+1;

	// z
	qp->z = sv_ptr;
	sv_ptr += N+1;


	// integer stuff
	int *i_ptr;
	i_ptr = (int *) sv_ptr;

	// idxb
	for(ii=0; ii<=N; ii++)
		{
		(qp->idxb)[ii] = i_ptr;
		i_ptr += nb[ii];
		}

	// idxs
	for(ii=0; ii<=N; ii++)
		{
		(qp->idxs)[ii] = i_ptr;
		i_ptr += ns[ii];
		}


	// align to typical cache line size
	long long l_ptr = (long long) i_ptr;
	l_ptr = (l_ptr+63)/64*64;


	// floating point stuff
	char *c_ptr;
	c_ptr = (char *) l_ptr;

	char *tmp_ptr;

	// BAbt
	for(ii=0; ii<N; ii++)
		{
		CREATE_STRMAT(nu[ii]+nx[ii]+1, nx[ii+1], qp->BAbt+ii, c_ptr);
		c_ptr += (qp->BAbt+ii)->memsize;
		}

	// RSQrq
	for(ii=0; ii<=N; ii++)
		{
		CREATE_STRMAT(nu[ii]+nx[ii]+1, nu[ii]+nx[ii], qp->RSQrq+ii, c_ptr);
		c_ptr += (qp->RSQrq+ii)->memsize;
		}

	// DCt
	for(ii=0; ii<=N; ii++)
		{
		CREATE_STRMAT(nu[ii]+nx[ii], ng[ii], qp->DCt+ii, c_ptr);
		c_ptr += (qp->DCt+ii)->memsize;
		}

	// b
	for(ii=0; ii<N; ii++)
		{
		CREATE_STRVEC(nx[ii+1], qp->b+ii, c_ptr);
		c_ptr += (qp->b+ii)->memsize;
		}

	// rq
	for(ii=0; ii<=N; ii++)
		{
		CREATE_STRVEC(nu[ii]+nx[ii], qp->rq+ii, c_ptr);
		c_ptr += (qp->rq+ii)->memsize;
		}

	// Z
	for(ii=0; ii<=N; ii++)
		{
		CREATE_STRVEC(2*ns[ii], qp->Z+ii, c_ptr);
		c_ptr += (qp->Z+ii)->memsize;
		}

	// z
	for(ii=0; ii<=N; ii++)
		{
		CREATE_STRVEC(2*ns[ii], qp->z+ii, c_ptr);
		c_ptr += (qp->z+ii)->memsize;
		}

	// d
	tmp_ptr = c_ptr;
	c_ptr += SIZE_STRVEC(2*nbt+2*ngt);
	for(ii=0; ii<=N; ii++)
		{
		CREATE_STRVEC(2*nb[ii]+2*ng[ii], qp->d+ii, tmp_ptr);
		tmp_ptr += nb[ii]*sizeof(REAL);
		tmp_ptr += ng[ii]*sizeof(REAL);
		tmp_ptr += nb[ii]*sizeof(REAL);
		tmp_ptr += ng[ii]*sizeof(REAL);
		}

	qp->dim = dim;

	qp->memsize = MEMSIZE_OCP_QP(dim);


#if defined(RUNTIME_CHECKS)
	if(c_ptr > ((char *) mem) + qp->memsize)
		{
		printf("\nCreate_ocp_qp: outsize memory bounds!\n\n");
		exit(1);
		}
#endif


	return;

	}



void CVT_COLMAJ_TO_OCP_QP(REAL **A, REAL **B, REAL **b, REAL **Q, REAL **S, REAL **R, REAL **q, REAL **r, int **idxb, REAL **d_lb, REAL **d_ub, REAL **C, REAL **D, REAL **d_lg, REAL **d_ug, REAL **Zl, REAL **Zu, REAL **zl, REAL **zu, int **idxs, struct OCP_QP *qp)
	{

	// extract dim
	int N = qp->dim->N;
	int *nx = qp->dim->nx;
	int *nu = qp->dim->nu;
	int *nb = qp->dim->nb;
	int *ng = qp->dim->ng;
	int *ns = qp->dim->ns;

	int ii, jj;

	for(ii=0; ii<N; ii++)
		{
		CVT_TRAN_MAT2STRMAT(nx[ii+1], nu[ii], B[ii], nx[ii+1], qp->BAbt+ii, 0, 0);
		CVT_TRAN_MAT2STRMAT(nx[ii+1], nx[ii], A[ii], nx[ii+1], qp->BAbt+ii, nu[ii], 0);
		CVT_TRAN_MAT2STRMAT(nx[ii+1], 1, b[ii], nx[ii+1], qp->BAbt+ii, nu[ii]+nx[ii], 0);
		CVT_VEC2STRVEC(nx[ii+1], b[ii], qp->b+ii, 0);
		}
	
	for(ii=0; ii<=N; ii++)
		{
		CVT_MAT2STRMAT(nu[ii], nu[ii], R[ii], nu[ii], qp->RSQrq+ii, 0, 0);
		CVT_TRAN_MAT2STRMAT(nu[ii], nx[ii], S[ii], nu[ii], qp->RSQrq+ii, nu[ii], 0);
		CVT_MAT2STRMAT(nx[ii], nx[ii], Q[ii], nx[ii], qp->RSQrq+ii, nu[ii], nu[ii]);
		CVT_TRAN_MAT2STRMAT(nu[ii], 1, r[ii], nu[ii], qp->RSQrq+ii, nu[ii]+nx[ii], 0);
		CVT_TRAN_MAT2STRMAT(nx[ii], 1, q[ii], nx[ii], qp->RSQrq+ii, nu[ii]+nx[ii], nu[ii]);
		CVT_VEC2STRVEC(nu[ii], r[ii], qp->rq+ii, 0);
		CVT_VEC2STRVEC(nx[ii], q[ii], qp->rq+ii, nu[ii]);
		}
	
	for(ii=0; ii<=N; ii++)
		{
		if(nb[ii]>0)
			{
			for(jj=0; jj<nb[ii]; jj++)
				qp->idxb[ii][jj] = idxb[ii][jj];
			CVT_VEC2STRVEC(nb[ii], d_lb[ii], qp->d+ii, 0);
			CVT_VEC2STRVEC(nb[ii], d_ub[ii], qp->d+ii, nb[ii]+ng[ii]);
			VECSC_LIBSTR(nb[ii], -1.0, qp->d+ii, nb[ii]+ng[ii]);
			}
		}
	
	for(ii=0; ii<=N; ii++)
		{
		if(ng[ii]>0)
			{
			CVT_TRAN_MAT2STRMAT(ng[ii], nu[ii], D[ii], ng[ii], qp->DCt+ii, 0, 0);
			CVT_TRAN_MAT2STRMAT(ng[ii], nx[ii], C[ii], ng[ii], qp->DCt+ii, nu[ii], 0);
			CVT_VEC2STRVEC(ng[ii], d_lg[ii], qp->d+ii, nb[ii]);
			CVT_VEC2STRVEC(ng[ii], d_ug[ii], qp->d+ii, 2*nb[ii]+ng[ii]);
			VECSC_LIBSTR(ng[ii], -1.0, qp->d+ii, 2*nb[ii]+ng[ii]);
			}
		}

	for(ii=0; ii<=N; ii++)
		{
		if(ns[ii]>0)
			{
			for(jj=0; jj<ns[ii]; jj++)
				qp->idxs[ii][jj] = idxs[ii][jj];
			CVT_VEC2STRVEC(ns[ii], Zl[ii], qp->Z+ii, 0);
			CVT_VEC2STRVEC(ns[ii], Zu[ii], qp->Z+ii, ns[ii]);
			CVT_VEC2STRVEC(ns[ii], zl[ii], qp->z+ii, 0);
			CVT_VEC2STRVEC(ns[ii], zu[ii], qp->z+ii, ns[ii]);
			}
		}

	return;

	}



void CVT_ROWMAJ_TO_OCP_QP(REAL **A, REAL **B, REAL **b, REAL **Q, REAL **S, REAL **R, REAL **q, REAL **r, int **idxb, REAL **d_lb, REAL **d_ub, REAL **C, REAL **D, REAL **d_lg, REAL **d_ug, REAL **Zl, REAL **Zu, REAL **zl, REAL **zu, int **idxs, struct OCP_QP *qp)
	{

	// extract dim
	int N = qp->dim->N;
	int *nx = qp->dim->nx;
	int *nu = qp->dim->nu;
	int *nb = qp->dim->nb;
	int *ng = qp->dim->ng;
	int *ns = qp->dim->ns;

	int ii, jj;

	for(ii=0; ii<N; ii++)
		{
		CVT_MAT2STRMAT(nu[ii], nx[ii+1], B[ii], nu[ii], qp->BAbt+ii, 0, 0);
		CVT_MAT2STRMAT(nx[ii], nx[ii+1], A[ii], nx[ii], qp->BAbt+ii, nu[ii], 0);
		CVT_TRAN_MAT2STRMAT(nx[ii+1], 1, b[ii], nx[ii+1], qp->BAbt+ii, nu[ii]+nx[ii], 0);
		CVT_VEC2STRVEC(nx[ii+1], b[ii], qp->b+ii, 0);
		}
	
	for(ii=0; ii<=N; ii++)
		{
		CVT_TRAN_MAT2STRMAT(nu[ii], nu[ii], R[ii], nu[ii], qp->RSQrq+ii, 0, 0);
		CVT_MAT2STRMAT(nx[ii], nu[ii], S[ii], nx[ii], qp->RSQrq+ii, nu[ii], 0);
		CVT_TRAN_MAT2STRMAT(nx[ii], nx[ii], Q[ii], nx[ii], qp->RSQrq+ii, nu[ii], nu[ii]);
		CVT_TRAN_MAT2STRMAT(nu[ii], 1, r[ii], nu[ii], qp->RSQrq+ii, nu[ii]+nx[ii], 0);
		CVT_TRAN_MAT2STRMAT(nx[ii], 1, q[ii], nx[ii], qp->RSQrq+ii, nu[ii]+nx[ii], nu[ii]);
		CVT_VEC2STRVEC(nu[ii], r[ii], qp->rq+ii, 0);
		CVT_VEC2STRVEC(nx[ii], q[ii], qp->rq+ii, nu[ii]);
		}
	
	for(ii=0; ii<=N; ii++)
		{
		if(nb[ii]>0)
			{
			for(jj=0; jj<nb[ii]; jj++)
				qp->idxb[ii][jj] = idxb[ii][jj];
			CVT_VEC2STRVEC(nb[ii], d_lb[ii], qp->d+ii, 0);
			CVT_VEC2STRVEC(nb[ii], d_ub[ii], qp->d+ii, nb[ii]+ng[ii]);
			VECSC_LIBSTR(nb[ii], -1.0, qp->d+ii, nb[ii]+ng[ii]);
			}
		}
	
	for(ii=0; ii<=N; ii++)
		{
		if(ng[ii]>0)
			{
			CVT_MAT2STRMAT(nu[ii], ng[ii], D[ii], nu[ii], qp->DCt+ii, 0, 0);
			CVT_MAT2STRMAT(nx[ii], ng[ii], C[ii], nx[ii], qp->DCt+ii, nu[ii], 0);
			CVT_VEC2STRVEC(ng[ii], d_lg[ii], qp->d+ii, nb[ii]);
			CVT_VEC2STRVEC(ng[ii], d_ug[ii], qp->d+ii, 2*nb[ii]+ng[ii]);
			VECSC_LIBSTR(ng[ii], -1.0, qp->d+ii, 2*nb[ii]+ng[ii]);
			}
		}

	for(ii=0; ii<=N; ii++)
		{
		if(ns[ii]>0)
			{
			for(jj=0; jj<ns[ii]; jj++)
				qp->idxs[ii][jj] = idxs[ii][jj];
			CVT_VEC2STRVEC(ns[ii], Zl[ii], qp->Z+ii, 0);
			CVT_VEC2STRVEC(ns[ii], Zu[ii], qp->Z+ii, ns[ii]);
			CVT_VEC2STRVEC(ns[ii], zl[ii], qp->z+ii, 0);
			CVT_VEC2STRVEC(ns[ii], zu[ii], qp->z+ii, ns[ii]);
			}
		}

	return;

	}



void UPDATE_Q(int stage, REAL *mat, struct OCP_QP *qp) {
	int num_rows = num_rows_Q(stage, qp->dim), num_cols = num_cols_Q(stage, qp->dim);
    int row_offset = qp->dim->nu[stage], col_offset = qp->dim->nu[stage];
    blasfeo_pack_dmat(num_rows, num_cols, mat, num_rows, &(qp->RSQrq[stage]), row_offset, col_offset);
}

void UPDATE_S(int stage, REAL *mat, struct OCP_QP *qp) {
    int num_rows = num_rows_S(stage, qp->dim), num_cols = num_cols_S(stage, qp->dim);
    int row_offset = qp->dim->nu[stage], col_offset = 0;
    blasfeo_pack_tran_dmat(num_rows, num_cols, mat, num_rows, &(qp->RSQrq[stage]), row_offset, col_offset);
}

void UPDATE_R(int stage, REAL *mat, struct OCP_QP *qp) {
    int num_rows = num_rows_R(stage, qp->dim), num_cols = num_cols_R(stage, qp->dim);
    int row_offset = 0, col_offset = 0;
    blasfeo_pack_dmat(num_rows, num_cols, mat, num_rows, &(qp->RSQrq[stage]), row_offset, col_offset);
}

void UPDATE_QVEC(int stage, REAL *vec, struct OCP_QP *qp) {
    int num_rows = num_elems_q(stage, qp->dim), num_cols = 1;
    int row_offset = qp->dim->nu[stage] + qp->dim->nx[stage], col_offset = qp->dim->nu[stage];
    blasfeo_pack_tran_dmat(num_rows, num_cols, vec, num_rows, &(qp->RSQrq[stage]), row_offset, col_offset);
    blasfeo_pack_dvec(num_rows, vec, &(qp->rq[stage]), col_offset);
}

void UPDATE_RVEC(int stage, REAL *vec, struct OCP_QP *qp) {
    int num_rows = num_elems_r(stage, qp->dim), num_cols = 1;
    int row_offset = qp->dim->nu[stage] + qp->dim->nx[stage], col_offset = 0;
    blasfeo_pack_tran_dmat(num_rows, num_cols, vec, num_rows, &(qp->RSQrq[stage]), row_offset, col_offset);
    blasfeo_pack_dvec(num_rows, vec, &(qp->rq[stage]), 0);
}

void UPDATE_A(int stage, REAL *mat, struct OCP_QP *qp) {
    int num_rows = num_rows_A(stage, qp->dim), num_cols = num_cols_A(stage, qp->dim);
    int row_offset = qp->dim->nu[stage], col_offset = 0;
    blasfeo_pack_tran_dmat(num_rows, num_cols, mat, num_rows, &(qp->BAbt[stage]), row_offset, col_offset);
}

void UPDATE_B(int stage, REAL *mat, struct OCP_QP *qp) {
	int num_rows = num_rows_B(stage, qp->dim), num_cols = num_cols_B(stage, qp->dim);
    int row_offset = 0, col_offset = 0;
    blasfeo_pack_tran_dmat(num_rows, num_cols, mat, num_rows, &(qp->BAbt[stage]), row_offset, col_offset);
}

void UPDATE_BVEC(int stage, REAL *vec, struct OCP_QP *qp) {
    int num_rows = num_elems_b(stage, qp->dim), num_cols = 1;
    int row_offset = qp->dim->nx[stage] + qp->dim->nu[stage], col_offset = 0;
    blasfeo_pack_tran_dmat(num_rows, num_cols, vec, num_rows, &(qp->BAbt[stage]), row_offset, col_offset);
    blasfeo_pack_dvec(num_rows, vec, &(qp->b[stage]), 0);
}

void UPDATE_LBX(int stage, REAL *vec, struct OCP_QP *qp) {
    int num_elems = num_elems_lbx(stage, qp->dim);
    int offset = qp->dim->nbu[stage];
    blasfeo_pack_dvec(num_elems, vec, &(qp->d[stage]), offset);
}

void UPDATE_LBU(int stage, REAL *vec, struct OCP_QP *qp) {
    int num_elems = num_elems_lbu(stage, qp->dim);
    int offset = 0;
    blasfeo_pack_dvec(num_elems, vec, &(qp->d[stage]), offset);
}

void UPDATE_UBX(int stage, REAL *vec, struct OCP_QP *qp) {
    int num_elems = num_elems_ubx(stage, qp->dim);
    int offset = qp->dim->nb[stage] + qp->dim->ng[stage] + qp->dim->nbu[stage];
    blasfeo_pack_dvec(num_elems, vec, &(qp->d[stage]), offset);
    blasfeo_dvecsc(num_elems, -1.0, &(qp->d[stage]), offset);
}

void UPDATE_UBU(int stage, REAL *vec, struct OCP_QP *qp) {
    int num_elems = num_elems_ubu(stage, qp->dim);
    int offset = qp->dim->nb[stage] + qp->dim->ng[stage];
    blasfeo_pack_dvec(num_elems, vec, &(qp->d[stage]), offset);
    blasfeo_dvecsc(num_elems, -1.0, &(qp->d[stage]), offset);
}

void UPDATE_C(int stage, REAL *mat, struct OCP_QP *qp) {
    int num_rows = num_rows_C(stage, qp->dim), num_cols = num_cols_C(stage, qp->dim);
    int row_offset = qp->dim->nu[stage], col_offset = 0;
    blasfeo_pack_tran_dmat(num_rows, num_cols, mat, num_rows, &(qp->DCt[stage]), row_offset, col_offset);
}

void UPDATE_D(int stage, REAL *mat, struct OCP_QP *qp) {
    int num_rows = num_rows_D(stage, qp->dim), num_cols = num_cols_D(stage, qp->dim);
    int row_offset = 0, col_offset = 0;
    blasfeo_pack_tran_dmat(num_rows, num_cols, mat, num_rows, &(qp->DCt[stage]), row_offset, col_offset);
}

void UPDATE_LG(int stage, REAL *vec, struct OCP_QP *qp) {
    int num_elems = num_elems_lg(stage, qp->dim);
    int offset = qp->dim->nb[stage];
    blasfeo_pack_dvec(num_elems, vec, &(qp->d[stage]), offset);
}

void UPDATE_UG(int stage, REAL *vec, struct OCP_QP *qp) {
    int num_elems = num_elems_ug(stage, qp->dim);
    int offset = 2*qp->dim->nb[stage] + qp->dim->ng[stage];
    blasfeo_pack_dvec(num_elems, vec, &(qp->d[stage]), offset);
    blasfeo_dvecsc(num_elems, -1.0, &(qp->d[stage]), offset);
}
