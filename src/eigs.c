// LAPACK stegr

#include "dpca.h"


void zMatVec(double _Complex* x, double _Complex* y, double _Complex* mat, int dim) {

  int i, j;

  for (i = 0; i < dim; ++i) {
    double _Complex accum = 0.0 + 0.0 * _Complex_I;
    for (j = 0; j < dim; j++) {
      accum += x[j] * mat[i * dim + j];
    }
    y[i] = accum;
  }

}

void arnoldi_eigs(Rcomplex *mat, int dim, int q,
  Rcomplex *eval, Rcomplex *evecs) {


  // znaupd parameters
  a_int ido = 0;                // reverse communication flag, (handled internally) must be zero at start
  char bmat[] = "I";            // B matrix ( I -> standard EV problem )
  a_int N = (a_int) dim;        // number of rows (dimension of the eigenproblem)
  char which[] = "LM";          // LM -> largest eigenvalues are of interest
  a_int nev = (a_int) q;        // Number of eigenvalues
  double tol = 0.000001;        // small tol => more stable checks after EV computation.
  double _Complex resid[N];     // residual vector
  a_int ncv = 2 * nev + 1; //
  double _Complex V[ncv * N];
  a_int ldv = N;
  a_int iparam[11];
  a_int ipntr[14];
  double _Complex workd[3 * N];
  a_int rvec = 1;
  char howmny[] = "A";
  double _Complex* d =
      (double _Complex*)malloc((nev + 1) * sizeof(double _Complex));
  a_int select[ncv];
  for (int i = 0; i < ncv; i++) select[i] = 1;
  //double _Complex z[(N + 1) * (nev + 1)];
  double _Complex z[N  * nev];
  a_int ldz = N + 1;
  double _Complex sigma = 0. + I * 0.;
  int k;
  for (k = 0; k < 3 * N; ++k) workd[k] = 0;
  double _Complex workl[3 * (ncv * ncv) + 6 * ncv];
  for (k = 0; k < 3 * (ncv * ncv) + 6 * ncv; ++k) workl[k] = 0;
  a_int lworkl = 3 * (ncv * ncv) + 6 * ncv;
  double rwork[ncv];
  double _Complex workev[2 * ncv];
  a_int info = 0;

  iparam[0] = 1;
  iparam[2] = 10 * N;
  iparam[3] = 1;
  iparam[4] = 0;  // number of ev found by arpack.
  iparam[6] = 1;

  // we still copy the array
  R_ShowMessage("test1");
  double _Complex cmplx_mat[dim * dim];
  for (int i= 0; i < dim * dim; i++) cmplx_mat[i] = mat[i].r + _Complex_I * mat[i].i;

  R_ShowMessage("test2");
  while (ido != 99) {
    /* call arpack like you would have, but, use znaupd_c instead of znaupd_ */
    znaupd_c(&ido, bmat, N, which, nev, tol, resid, ncv, V, ldv, iparam, ipntr,
             workd, workl, lworkl, rwork, &info);

    zMatVec(&(workd[ipntr[0] - 1]), &(workd[ipntr[1] - 1]), cmplx_mat, dim);
  }
  R_ShowMessage("test3");
  if (iparam[4] != nev) {
    printf("Error: iparam[4] %d, nev %d\n", iparam[4], nev); // check number of ev found by arpack.
  }
  R_ShowMessage("test4");
  /* call arpack like you would have, but, use zneupd_c instead of zneupd_ */
  zneupd_c(rvec, howmny, select, d, z, ldz, sigma, workev, bmat, N, which, nev,
           tol, resid, ncv, V, ldv, iparam, ipntr, workd, workl, lworkl, rwork,
           &info);

  R_ShowMessage("test5");
  // copy results
  for (int i = 0; i < q; i++) {
    eval[i].r = creal(d[i]);
    eval[i].i = cimag(d[i]);
    for (int j = 0; j < dim; j++){
      evecs[i*dim + j].r = creal(z[i*dim + j]);
      evecs[i*dim + j].i = cimag(z[i*dim + j]);
    }
  }

}


SEXP R_arnoldi_eigs(SEXP r_mat, SEXP r_dim, SEXP r_q) {

  Rcomplex *mat = COMPLEX(r_mat);
  int dim = *INTEGER(r_dim);
  int q = *INTEGER(r_q);

  // result objects
  SEXP res = PROTECT(allocVector(VECSXP, 2));;
  SEXP evecs = PROTECT(allocMatrix(CPLXSXP, dim, q));
  SEXP evals = PROTECT(allocVector(CPLXSXP, q));

  arnoldi_eigs(mat, dim, q, COMPLEX(evals), COMPLEX(evecs));

  SET_VECTOR_ELT(res, 0, evals);
  SET_VECTOR_ELT(res, 1, evecs);

  SEXP nms = PROTECT(allocVector(STRSXP, 2));
  SET_STRING_ELT(nms, 0, mkChar("values"));
  SET_STRING_ELT(nms, 1, mkChar("vectors"));

  setAttrib(res, R_NamesSymbol, nms);

  UNPROTECT(4);
  return res;
}





/***************************************************************************************************************************************/
/* int zn() {                                                                                                                          */
/*   a_int ido = 0;        // reverse communication flag, (handled internally) must be zero at start                                   */
/*   char bmat[] = "I";    // B matrix ( I -> standard EV problem )                                                                    */
/*   a_int N = 1000;       // number of rows (dimension of the eigenproblem)                                                           */
/*   char which[] = "LM";  // LM -> largest eigenvalues are of interest                                                                */
/*   a_int nev = 9;        // Number of eigenvalues                                                                                    */
/*   double tol = 0.000001; // small tol => more stable checks after EV computation.                                                   */
/*   double _Complex resid[N]; // residual vector                                                                                      */
/*   a_int ncv = 2 * nev + 1; //                                                                                                       */
/*   double _Complex V[ncv * N];                                                                                                       */
/*   a_int ldv = N;                                                                                                                    */
/*   a_int iparam[11];                                                                                                                 */
/*   a_int ipntr[14];                                                                                                                  */
/*   double _Complex workd[3 * N];                                                                                                     */
/*   a_int rvec = 0;                                                                                                                   */
/*   char howmny[] = "A";                                                                                                              */
/*   double _Complex* d =                                                                                                              */
/*       (double _Complex*)malloc((nev + 1) * sizeof(double _Complex));                                                                */
/*   a_int select[ncv];                                                                                                                */
/*   for (int i = 0; i < ncv; i++) select[i] = 1;                                                                                      */
/*   double _Complex z[(N + 1) * (nev + 1)];                                                                                           */
/*   a_int ldz = N + 1;                                                                                                                */
/*   double _Complex sigma = 0. + I * 0.;                                                                                              */
/*   int k;                                                                                                                            */
/*   for (k = 0; k < 3 * N; ++k) workd[k] = 0;                                                                                         */
/*   double _Complex workl[3 * (ncv * ncv) + 6 * ncv];                                                                                 */
/*   for (k = 0; k < 3 * (ncv * ncv) + 6 * ncv; ++k) workl[k] = 0;                                                                     */
/*   a_int lworkl = 3 * (ncv * ncv) + 6 * ncv;                                                                                         */
/*   double rwork[ncv];                                                                                                                */
/*   double _Complex workev[2 * ncv];                                                                                                  */
/*   a_int info = 0;                                                                                                                   */
/*                                                                                                                                     */
/*   iparam[0] = 1;                                                                                                                    */
/*   iparam[2] = 10 * N;                                                                                                               */
/*   iparam[3] = 1;                                                                                                                    */
/*   iparam[4] = 0;  // number of ev found by arpack.                                                                                  */
/*   iparam[6] = 1;                                                                                                                    */
/*                                                                                                                                     */
/*   while (ido != 99) {                                                                                                               */
/*     /\* call arpack like you would have, but, use znaupd_c instead of znaupd_ *\/                                                   */
/*     znaupd_c(&ido, bmat, N, which, nev, tol, resid, ncv, V, ldv, iparam, ipntr,                                                     */
/*              workd, workl, lworkl, rwork, &info);                                                                                   */
/*                                                                                                                                     */
/*   }                                                                                                                                 */
/*   if (iparam[4] != nev) {printf("Error: iparam[4] %d, nev %d\n", iparam[4], nev); return 1;} // check number of ev found by arpack. */
/*                                                                                                                                     */
/*   /\* call arpack like you would have, but, use zneupd_c instead of zneupd_ *\/                                                     */
/*   zneupd_c(rvec, howmny, select, d, z, ldz, sigma, workev, bmat, N, which, nev,                                                     */
/*            tol, resid, ncv, V, ldv, iparam, ipntr, workd, workl, lworkl, rwork,                                                     */
/*            &info);                                                                                                                  */
/*   int i;                                                                                                                            */
/*   for (i = 0; i < nev; ++i) {                                                                                                       */
/*     double rval = creal(d[i]);                                                                                                      */
/*     double rref = (N-(nev-1)+i);                                                                                                    */
/*     double reps = fabs(rval - rref);                                                                                                */
/*     double ival = cimag(d[i]);                                                                                                      */
/*     double iref = (N-(nev-1)+i);                                                                                                    */
/*     double ieps = fabs(ival - iref);                                                                                                */
/*     printf("%f %f - %f %f - %f %f\n", rval, ival, rref, iref, reps, ieps);                                                          */
/*                                                                                                                                     */
/*     /\*eigen value order: smallest -> biggest*\/                                                                                    */
/*     if(reps>1.e-05 || ieps>1.e-05){                                                                                                 */
/*       free(d);                                                                                                                      */
/*       return 1;                                                                                                                     */
/*     }                                                                                                                               */
/*   }                                                                                                                                 */
/*   free(d);                                                                                                                          */
/*   return 0;                                                                                                                         */
/* }                                                                                                                                   */
/*                                                                                                                                     */
/* int main() {                                                                                                                        */
/*   sstats_c();                                                                                                                       */
/*   a_int nopx_c, nbx_c, nrorth_c, nitref_c, nrstrt_c;                                                                                */
/*   float tsaupd_c, tsaup2_c, tsaitr_c, tseigt_c, tsgets_c, tsapps_c, tsconv_c;                                                       */
/*   float tnaupd_c, tnaup2_c, tnaitr_c, tneigt_c, tngets_c, tnapps_c, tnconv_c;                                                       */
/*   float tcaupd_c, tcaup2_c, tcaitr_c, tceigt_c, tcgets_c, tcapps_c, tcconv_c;                                                       */
/*   float tmvopx_c, tmvbx_c, tgetv0_c, titref_c, trvec_c;                                                                             */
/*   stat_c(&nopx_c, &nbx_c, &nrorth_c, &nitref_c, &nrstrt_c, &tsaupd_c, &tsaup2_c,                                                    */
/*          &tsaitr_c, &tseigt_c, &tsgets_c, &tsapps_c, &tsconv_c, &tnaupd_c,                                                          */
/*          &tnaup2_c, &tnaitr_c, &tneigt_c, &tngets_c, &tnapps_c, &tnconv_c,                                                          */
/*          &tcaupd_c, &tcaup2_c, &tcaitr_c, &tceigt_c, &tcgets_c, &tcapps_c,                                                          */
/*          &tcconv_c, &tmvopx_c, &tmvbx_c, &tgetv0_c, &titref_c, &trvec_c);                                                           */
/*   printf("Timers : nopx %d, tmvopx %f - nbx %d, tmvbx %f\n", nopx_c, tmvopx_c,                                                      */
/*          nbx_c, tmvbx_c);                                                                                                           */
/*                                                                                                                                     */
/*   printf("------\n");                                                                                                               */
/*                                                                                                                                     */
/*   // clang-format off                                                                                                               */
/*   debug_c(6, -6, 1,                                                                                                                 */
/*           1, 1, 1, 1, 1, 1, 1,                                                                                                      */
/*           1, 1, 1, 1, 1, 1, 1,                                                                                                      */
/*           1, 1, 1, 1, 1, 1, 1); // set debug flags.                                                                                 */
/*   // clang-format on                                                                                                                */
/*   int rc = zn();  // arpack with debug.                                                                                             */
/*                                                                                                                                     */
/*   return rc;                                                                                                                        */
/* }                                                                                                                                   */
/***************************************************************************************************************************************/