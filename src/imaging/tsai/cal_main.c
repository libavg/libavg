/*******************************************************************************\
 *                                                                               *
 * This file contains routines for calibrating Tsai's 11 parameter camera model. *
 * The camera model is based on the pin hole model of 3D-2D perspective          *
 * projection with 1st order radial lens distortion.  The model consists of      *
 * 5 internal (also called intrinsic or interior) camera parameters:             *
 *                                                                               *
 *       f       - effective focal length of the pin hole camera                 *
 *       kappa1  - 1st order radial lens distortion coefficient                  *
 *       Cx, Cy  - coordinates of center of radial lens distortion               *
 *         (also used as the piercing point of the camera coordinate *
 *          frame's Z axis with the camera's sensor plane)               *
 *       sx      - uncertainty factor for scale of horizontal scanline           *
 *                                                                               *
 * and 6 external (also called extrinsic or exterior) camera parameters:         *
 *                                                                               *
 *       Rx, Ry, Rz, Tx, Ty, Tz  - rotational and translational components of    *
 *                                 the transform between the world's coordinate  *
 *                                 frame and the camera's coordinate frame.      *
 *                                                                               *
 * Data for model calibration consists of the (x,y,z) world coordinates of a     *
 * feature point (in mm) and the corresponding coordinates (Xf,Yf) (in pixels)   *
 * of the feature point in the image.  Two types of calibration are available:   *
 *                                                                               *
 *       coplanar     - all of the calibration points lie in a single plane      *
 *                                                                               *
 * This file contains routines for two levels of calibration.  The first level   *
 * of calibration is a direct implementation of Tsai's algorithm in which only   *
 * the f, Tz and kappa1 parameters are optimized for.  The routines are:         *
 *                                                                               *
 *       coplanar_calibration ()                                                 *
 *                                                                               *
 * The second level of calibration optimizes for everything.  This level is      *
 * very slow but provides the most accurate calibration.  The routines are:      *
 *                                                                               *
 *       coplanar_calibration_with_full_optimization ()                          *
 *                                                                               *
 * Extra notes                                                                   *
 * -----------                                                                   *
 *                                                                               *
 * An explanation of the basic algorithms and description of the variables       *
 * can be found in several publications, including:                              *
 *                                                                               *
 * "An Efficient and Accurate Camera Calibration Technique for 3D Machine        *
 *  Vision", Roger Y. Tsai, Proceedings of IEEE Conference on Computer Vision    *
*  and Pattern Recognition, Miami Beach, FL, 1986, pages 364-374.               *
*                                                                               *
*  and                                                                          *
*                                                                               *
* "A versatile Camera Calibration Technique for High-Accuracy 3D Machine        *
*  Vision Metrology Using Off-the-Shelf TV Cameras and Lenses", Roger Y. Tsai,  *
*  IEEE Journal of Robotics and Automation, Vol. RA-3, No. 4, August 1987,      *
*  pages 323-344.                                                               *
*                                                                               *
*                                                                               *
* Notation                                                                      *
* --------                                                                      *
*                                                                               *
* The camera's X axis runs along increasing column coordinates in the           *
* image/frame.  The Y axis runs along increasing row coordinates.               *
*                                                                               *
* pix == image/frame grabber picture element                                    *
* sel == camera sensor element                                                  *
*                                                                               *
* Internal routines starting with "cc_" are for coplanar calibration.           *
* Internal routines starting with "ncc_" are for noncoplanar calibration.       *
\*******************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <errno.h>

#include "matrix.h"
#include "cal_main.h"

#include "../../lmfit/lmmin.h"

//#define DEBUG

/* Variables used by the subroutines for I/O (perhaps not the best way of doing this) */
struct camera_parameters cp;
struct calibration_data cd;
struct calibration_constants cc;

/* Local working storage */
double    Xd[MAX_POINTS],
          Yd[MAX_POINTS],
          r_squared[MAX_POINTS],
          U[7];

double safe_sqrt(double d) 
{
    if (d < 0 && d > -0.00000001) {
        return 0;
    } else {
        return sqrt(d);
    }
}

void lm_print(int n_par, double* par, int m_dat, double* fvec, 
        void *data, int iflag, int iter, int nfev)
{
}

/***********************************************************************\
 * This routine solves for the roll, pitch and yaw angles (in radians)   *
 * for a given orthonormal rotation matrix (from Richard P. Paul,        *
 * Robot Manipulators: Mathematics, Programming and Control, p70).       *
 * Note 1, should the rotation matrix not be orthonormal these will not  *
 * be the "best fit" roll, pitch and yaw angles.                         *
 * Note 2, there are actually two possible solutions for the matrix.     *
 * The second solution can be found by adding 180 degrees to Rz before   *
 * Ry and Rx are calculated.                                             *
 \***********************************************************************/
void      solve_RPY_transform ()
{
    double    sg,
              cg;

    cc.Rz = atan2 (cc.r4, cc.r1);

    SINCOS (cc.Rz, sg, cg);

    cc.Ry = atan2 (-cc.r7, cc.r1 * cg + cc.r4 * sg);

    cc.Rx = atan2 (cc.r3 * sg - cc.r6 * cg, cc.r5 * cg - cc.r2 * sg);
}


/***********************************************************************\
 * This routine simply takes the roll, pitch and yaw angles and fills in *
 * the rotation matrix elements r1-r9.                   *
 \***********************************************************************/
void      apply_RPY_transform ()
{
    double    sa,
              ca,
              sb,
              cb,
              sg,
              cg;

    SINCOS (cc.Rx, sa, ca);
    SINCOS (cc.Ry, sb, cb);
    SINCOS (cc.Rz, sg, cg);

    cc.r1 = cb * cg;
    cc.r2 = cg * sa * sb - ca * sg;
    cc.r3 = sa * sg + ca * cg * sb;
    cc.r4 = cb * sg;
    cc.r5 = sa * sb * sg + ca * cg;
    cc.r6 = ca * sb * sg - cg * sa;
    cc.r7 = -sb;
    cc.r8 = cb * sa;
    cc.r9 = ca * cb;
}


/***********************************************************************\
 * Routines for coplanar camera calibration              *
 \***********************************************************************/
void      cc_compute_Xd_Yd_and_r_squared ()
{
    int       i;

    double    Xd_,
              Yd_;

    for (i = 0; i < cd.point_count; i++) {
        Xd[i] = Xd_ = cp.dpx * (cd.Xf[i] - cp.Cx) / cp.sx;  /* [mm] */
        Yd[i] = Yd_ = cp.dpy * (cd.Yf[i] - cp.Cy);          /* [mm] */
        r_squared[i] = SQR (Xd_) + SQR (Yd_);                   /* [mm^2] */
    }
}


void      cc_compute_U ()
{
    int       i;

    dmat      M,
              a,
              b;

    M = newdmat (0, (cd.point_count - 1), 0, 4, &errno);
    if (errno) {
        fprintf (stderr, "cc compute U: unable to allocate matrix M\n");
        exit (-1);
    }

    a = newdmat (0, 4, 0, 0, &errno);
    if (errno) {
        fprintf (stderr, "cc compute U: unable to allocate vector a\n");
        exit (-1);
    }

    b = newdmat (0, (cd.point_count - 1), 0, 0, &errno);
    if (errno) {
        fprintf (stderr, "cc compute U: unable to allocate vector b\n");
        exit (-1);
    }

    for (i = 0; i < cd.point_count; i++) {
        M.el[i][0] = Yd[i] * cd.xw[i];
        M.el[i][1] = Yd[i] * cd.yw[i];
        M.el[i][2] = Yd[i];
        M.el[i][3] = -Xd[i] * cd.xw[i];
        M.el[i][4] = -Xd[i] * cd.yw[i];
        b.el[i][0] = Xd[i];
    }

    if (solve_system (M, a, b)) {
        fprintf (stderr, "cc compute U: unable to solve system  Ma=b\n");
        exit (-1);
    }

    U[0] = a.el[0][0];
    U[1] = a.el[1][0];
    U[2] = a.el[2][0];
    U[3] = a.el[3][0];
    U[4] = a.el[4][0];

    freemat (M);
    freemat (a);
    freemat (b);
}


void      cc_compute_Tx_and_Ty ()
{
    int       i,
              far_point;

    double    Tx,
              Ty,
              Ty_squared,
              x,
              y,
              Sr,
              r1p,
              r2p,
              r4p,
              r5p,
              r1,
              r2,
              r4,
              r5,
              distance,
              far_distance;

    r1p = U[0];
    r2p = U[1];
    r4p = U[3];
    r5p = U[4];

    /* first find the square of the magnitude of Ty */
    if ((fabs (r1p) < EPSILON) && (fabs (r2p) < EPSILON))
        Ty_squared = 1 / (SQR (r4p) + SQR (r5p));
    else if ((fabs (r4p) < EPSILON) && (fabs (r5p) < EPSILON))
        Ty_squared = 1 / (SQR (r1p) + SQR (r2p));
    else if ((fabs (r1p) < EPSILON) && (fabs (r4p) < EPSILON))
        Ty_squared = 1 / (SQR (r2p) + SQR (r5p));
    else if ((fabs (r2p) < EPSILON) && (fabs (r5p) < EPSILON))
        Ty_squared = 1 / (SQR (r1p) + SQR (r4p));
    else {
        Sr = SQR (r1p) + SQR (r2p) + SQR (r4p) + SQR (r5p);
        Ty_squared = (Sr - safe_sqrt (SQR (Sr) - 4 * SQR (r1p * r5p - r4p * r2p))) /
            (2 * SQR (r1p * r5p - r4p * r2p));
    }

    /* find a point that is far from the image center */
    far_distance = 0;
    far_point = 0;
    for (i = 0; i < cd.point_count; i++)
        if ((distance = r_squared[i]) > far_distance) {
            far_point = i;
            far_distance = distance;
        }

    /* now find the sign for Ty */
    /* start by assuming Ty > 0 */
    Ty = safe_sqrt (Ty_squared);
    r1 = U[0] * Ty;
    r2 = U[1] * Ty;
    Tx = U[2] * Ty;
    r4 = U[3] * Ty;
    r5 = U[4] * Ty;
    x = r1 * cd.xw[far_point] + r2 * cd.yw[far_point] + Tx;
    y = r4 * cd.xw[far_point] + r5 * cd.yw[far_point] + Ty;

    /* flip Ty if we guessed wrong */
    if ((SIGNBIT (x) != SIGNBIT (Xd[far_point])) ||
            (SIGNBIT (y) != SIGNBIT (Yd[far_point])))
        Ty = -Ty;

    /* update the calibration constants */
    cc.Tx = U[2] * Ty;
    cc.Ty = Ty;
}


void      cc_compute_R ()
{
    double    r1,
              r2,
              r3,
              r4,
              r5,
              r6,
              r7,
              r8,
              r9;

    r1 = U[0] * cc.Ty;
    r2 = U[1] * cc.Ty;
    r3 = safe_sqrt (1 - SQR (r1) - SQR (r2));

    r4 = U[3] * cc.Ty;
    r5 = U[4] * cc.Ty;
    r6 = safe_sqrt (1 - SQR (r4) - SQR (r5));
    if (!SIGNBIT (r1 * r4 + r2 * r5))
        r6 = -r6;

    /* use the outer product of the first two rows to get the last row */
    r7 = r2 * r6 - r3 * r5;
    r8 = r3 * r4 - r1 * r6;
    r9 = r1 * r5 - r2 * r4;

    /* update the calibration constants */
    cc.r1 = r1;
    cc.r2 = r2;
    cc.r3 = r3;
    cc.r4 = r4;
    cc.r5 = r5;
    cc.r6 = r6;
    cc.r7 = r7;
    cc.r8 = r8;
    cc.r9 = r9;

    /* fill in cc.Rx, cc.Ry and cc.Rz */
    solve_RPY_transform ();
}


void      cc_compute_approximate_f_and_Tz ()
{
    int       i;

    dmat      M,
              a,
              b;

    M = newdmat (0, (cd.point_count - 1), 0, 1, &errno);
    if (errno) {
        fprintf (stderr, "cc compute apx: unable to allocate matrix M\n");
        exit (-1);
    }

    a = newdmat (0, 1, 0, 0, &errno);
    if (errno) {
        fprintf (stderr, "cc compute apx: unable to allocate vector a\n");
        exit (-1);
    }

    b = newdmat (0, (cd.point_count - 1), 0, 0, &errno);
    if (errno) {
        fprintf (stderr, "cc compute apx: unable to allocate vector b\n");
        exit (-1);
    }

    for (i = 0; i < cd.point_count; i++) {
        M.el[i][0] = cc.r4 * cd.xw[i] + cc.r5 * cd.yw[i] + cc.Ty;
        M.el[i][1] = -Yd[i];
        b.el[i][0] = (cc.r7 * cd.xw[i] + cc.r8 * cd.yw[i]) * Yd[i];
    }

    if (solve_system (M, a, b)) {
        fprintf (stderr, "cc compute apx: unable to solve system  Ma=b\n");
        // This means that the image plane is perpendicular to the camera axis.
        // f and Tz are linearly dependent, so we just choose one and calculate
        // the other.
        cc.f = 1;
        cc.Tz =(b.el[0][0]-M.el[0][0]*cc.f) / M.el[0][1];
//        print_mat(a);
//        print_mat(b);
//        exit (-1);
    }

    /* update the calibration constants */
    cc.f = a.el[0][0];
    cc.Tz = a.el[1][0];
    cc.kappa1 = 0.0;        /* this is the assumption that our calculation was made under */

    freemat (M);
    freemat (a);
    freemat (b);
}


/************************************************************************/
void      cc_compute_exact_f_and_Tz_error (double* params, int m_dat, double *err,
        void * pData, int * info)
{
    int       i;

    double    f,
              Tz,
              kappa1,
              xc,
              yc,
              zc,
              Xu_1,
              Yu_1,
              Xu_2,
              Yu_2,
              distortion_factor;

    f = params[0];
    Tz = params[1];
    kappa1 = params[2];

    for (i = 0; i < cd.point_count; i++) {
        /* convert from world coordinates to camera coordinates */
        /* Note: zw is implicitly assumed to be zero for these (coplanar) calculations */
        xc = cc.r1 * cd.xw[i] + cc.r2 * cd.yw[i] + cc.Tx;
        yc = cc.r4 * cd.xw[i] + cc.r5 * cd.yw[i] + cc.Ty;
        zc = cc.r7 * cd.xw[i] + cc.r8 * cd.yw[i] + Tz;

        /* convert from camera coordinates to undistorted sensor coordinates */
        Xu_1 = f * xc / zc;
        Yu_1 = f * yc / zc;

        /* convert from distorted sensor coordinates to undistorted sensor coordinates */
        distortion_factor = 1 + kappa1 * (SQR (Xd[i]) + SQR (Yd[i]));
        Xu_2 = Xd[i] * distortion_factor;
        Yu_2 = Yd[i] * distortion_factor;

        /* record the error in the undistorted sensor coordinates */
        err[i] = hypot (Xu_1 - Xu_2, Yu_1 - Yu_2);
    }
    *info = 0;
}


void      cc_compute_exact_f_and_Tz ()
{
#define NPARAMS 3

    int       i;

    /* Parameters needed by MINPACK's lmdif() */

    int     m = cd.point_count;
    int     n = NPARAMS;
    double  x[NPARAMS];
    double *fvec;
    double  ftol = REL_SENSOR_TOLERANCE_ftol;
    double  xtol = REL_PARAM_TOLERANCE_xtol;
    double  gtol = ORTHO_TOLERANCE_gtol;
    int     maxfev = MAXFEV;
    double  epsfcn = EPSFCN;
    double  diag[NPARAMS];
    int     mode = MODE;
    double  factor = FACTOR;
    int     info;
    int     nfev;
    double *fjac;
    int     ipvt[NPARAMS];
    double  qtf[NPARAMS];
    double  wa1[NPARAMS];
    double  wa2[NPARAMS];
    double  wa3[NPARAMS];
    double *wa4;

    /* allocate some workspace */
    if (( fvec = (double *) calloc ((unsigned int) m, (unsigned int) sizeof(double))) == NULL ) {
        fprintf(stderr,"calloc: Cannot allocate workspace fvec\n");
        exit(-1);
    }

    if (( fjac = (double *) calloc ((unsigned int) m*n, (unsigned int) sizeof(double))) == NULL ) {
        fprintf(stderr,"calloc: Cannot allocate workspace fjac\n");
        exit(-1);
    }

    if (( wa4 = (double *) calloc ((unsigned int) m, (unsigned int) sizeof(double))) == NULL ) {
        fprintf(stderr,"calloc: Cannot allocate workspace wa4\n");
        exit(-1);
    }

    /* use the current calibration constants as an initial guess */
    x[0] = cc.f;
    x[1] = cc.Tz;
    x[2] = cc.kappa1;

    /* define optional scale factors for the parameters */
    if ( mode == 2 ) {
        for (i = 0; i < NPARAMS; i++)
            diag[i] = 1.0;             /* some user-defined values */
    }

    /* perform the optimization */ 
    lm_lmdif(m, n, x, fvec, ftol, xtol,
            gtol, maxfev, epsfcn, diag, mode,
            factor, &info, &nfev,
            fjac, ipvt, qtf,
            wa1, wa2, wa3, wa4,
            cc_compute_exact_f_and_Tz_error,
            lm_print, 0);
    
    /* update the calibration constants */
    cc.f = x[0];
    cc.Tz = x[1];
    cc.kappa1 = x[2];

    /* release allocated workspace */
    free (fvec);
    free (fjac);
    free (wa4);

#ifdef DEBUG
    /* print the number of function calls during iteration */
    fprintf(stderr,"info: %d nfev: %d\n\n",info,nfev);
#endif

#undef NPARAMS
}


/************************************************************************/
void      cc_three_parm_optimization ()
{
    int       i;

    for (i = 0; i < cd.point_count; i++)
        if (cd.zw[i]) {
            fprintf (stderr, "error - coplanar calibration tried with data outside of Z plane\n\n");
            exit (-1);
        }

    cc_compute_Xd_Yd_and_r_squared ();

    cc_compute_U ();

    cc_compute_Tx_and_Ty ();

    cc_compute_R ();

    cc_compute_approximate_f_and_Tz ();

    if (cc.f < 0) {
        /* try the other solution for the orthonormal matrix */
        cc.r3 = -cc.r3;
        cc.r6 = -cc.r6;
        cc.r7 = -cc.r7;
        cc.r8 = -cc.r8;
        solve_RPY_transform ();

        cc_compute_approximate_f_and_Tz ();

        if (cc.f < 0) {
            fprintf (stderr, "error - possible handedness problem with data\n");
            exit (-1);
        }
    }

    cc_compute_exact_f_and_Tz ();
}


/************************************************************************/
void      cc_remove_sensor_plane_distortion_from_Xd_and_Yd ()
{
    int       i;
    double    Xu,
              Yu;

    for (i = 0; i < cd.point_count; i++) {
        distorted_to_undistorted_sensor_coord (Xd[i], Yd[i], &Xu, &Yu);
        Xd[i] = Xu;
        Yd[i] = Yu;
        r_squared[i] = SQR (Xu) + SQR (Yu);
    }
}


/************************************************************************/
void      cc_five_parm_optimization_with_late_distortion_removal_error (double* params, 
        int m_dat, double *err, void * pData, int * info)
{
    int       i;

    double    f,
              Tz,
              kappa1,
              xc,
              yc,
              zc,
              Xu_1,
              Yu_1,
              Xu_2,
              Yu_2,
              distortion_factor;

    /* in this routine radial lens distortion is only taken into account */
    /* after the rotation and translation constants have been determined */

    f = params[0];
    Tz = params[1];
    kappa1 = params[2];

    cp.Cx = params[3];
    cp.Cy = params[4];

    cc_compute_Xd_Yd_and_r_squared ();

    cc_compute_U ();

    cc_compute_Tx_and_Ty ();

    cc_compute_R ();

    cc_compute_approximate_f_and_Tz ();

    if (cc.f < 0) {
        /* try the other solution for the orthonormal matrix */
        cc.r3 = -cc.r3;
        cc.r6 = -cc.r6;
        cc.r7 = -cc.r7;
        cc.r8 = -cc.r8;
        solve_RPY_transform ();

        cc_compute_approximate_f_and_Tz ();

        if (cc.f < 0) {
            fprintf (stderr, "error - possible handedness problem with data\n");
            exit (-1);
        }
    }

    for (i = 0; i < cd.point_count; i++) {
        /* convert from world coordinates to camera coordinates */
        /* Note: zw is implicitly assumed to be zero for these (coplanar) calculations */
        xc = cc.r1 * cd.xw[i] + cc.r2 * cd.yw[i] + cc.Tx;
        yc = cc.r4 * cd.xw[i] + cc.r5 * cd.yw[i] + cc.Ty;
        zc = cc.r7 * cd.xw[i] + cc.r8 * cd.yw[i] + Tz;

        /* convert from camera coordinates to undistorted sensor coordinates */
        Xu_1 = f * xc / zc;
        Yu_1 = f * yc / zc;

        /* convert from distorted sensor coordinates to undistorted sensor coordinates */
        distortion_factor = 1 + kappa1 * r_squared[i];
        Xu_2 = Xd[i] * distortion_factor;
        Yu_2 = Yd[i] * distortion_factor;

        /* record the error in the undistorted sensor coordinates */
        err[i] = hypot (Xu_1 - Xu_2, Yu_1 - Yu_2);
    }
    *info=0;
}


void      cc_five_parm_optimization_with_late_distortion_removal ()
{
#define NPARAMS 5

    int       i;

    /* Parameters needed by MINPACK's lmdif() */

    int     m = cd.point_count;
    int     n = NPARAMS;
    double  x[NPARAMS];
    double *fvec;
    double  ftol = REL_SENSOR_TOLERANCE_ftol;
    double  xtol = REL_PARAM_TOLERANCE_xtol;
    double  gtol = ORTHO_TOLERANCE_gtol;
    int     maxfev = MAXFEV;
    double  epsfcn = EPSFCN;
    double  diag[NPARAMS];
    int     mode = MODE;
    double  factor = FACTOR;
    int     info;
    int     nfev;
    double *fjac;
    int     ipvt[NPARAMS];
    double  qtf[NPARAMS];
    double  wa1[NPARAMS];
    double  wa2[NPARAMS];
    double  wa3[NPARAMS];
    double *wa4;

    /* allocate some workspace */
    if (( fvec = (double *) calloc ((unsigned int) m, (unsigned int) sizeof(double))) == NULL ) {
        fprintf(stderr,"calloc: Cannot allocate workspace fvec\n");
        exit(-1);
    }

    if (( fjac = (double *) calloc ((unsigned int) m*n, (unsigned int) sizeof(double))) == NULL ) {
        fprintf(stderr,"calloc: Cannot allocate workspace fjac\n");
        exit(-1);
    }

    if (( wa4 = (double *) calloc ((unsigned int) m, (unsigned int) sizeof(double))) == NULL ) {
        fprintf(stderr,"calloc: Cannot allocate workspace wa4\n");
        exit(-1);
    }

    /* use the current calibration constants as an initial guess */
    x[0] = cc.f;
    x[1] = cc.Tz;
    x[2] = cc.kappa1;
    x[3] = cp.Cx;
    x[4] = cp.Cy;

    /* define optional scale factors for the parameters */
    if ( mode == 2 ) {
        for (i = 0; i < NPARAMS; i++)
            diag[i] = 1.0;             /* some user-defined values */
    }

    /* perform the optimization */
    lm_lmdif(m, n, x, fvec, ftol, xtol,
            gtol, maxfev, epsfcn, diag, mode,
            factor, &info, &nfev,
            fjac, ipvt, qtf,
            wa1, wa2, wa3, wa4,
            cc_five_parm_optimization_with_late_distortion_removal_error,
            lm_print, 0);
/*    
    lm_lmdif (cc_five_parm_optimization_with_late_distortion_removal_error,
            &m, &n, x, fvec, &ftol, &xtol, &gtol, &maxfev, &epsfcn,
            diag, &mode, &factor, &nprint, &info, &nfev, fjac, &ldfjac,
            ipvt, qtf, wa1, wa2, wa3, wa4);
*/
    /* update the calibration and camera constants */
    cc.f = x[0];
    cc.Tz = x[1];
    cc.kappa1 = x[2];
    cp.Cx = x[3];
    cp.Cy = x[4];

    /* release allocated workspace */
    free (fvec);
    free (fjac);
    free (wa4);

#ifdef DEBUG
    /* print the number of function calls during iteration */
    fprintf(stderr,"info: %d nfev: %d\n\n",info,nfev);
#endif

#undef NPARAMS
}


/************************************************************************/
void      cc_five_parm_optimization_with_early_distortion_removal_error (double* params, 
        int m_dat, double *err, void * pData, int * info)
{
    int       i;

    double    f,
              Tz,
              xc,
              yc,
              zc,
              Xu_1,
              Yu_1,
              Xu_2,
              Yu_2;

    /* in this routine radial lens distortion is taken into account */
    /* before the rotation and translation constants are determined */
    /* (this assumes we have the distortion reasonably modelled)    */

    f = params[0];
    Tz = params[1];

    cc.kappa1 = params[2];
    cp.Cx = params[3];
    cp.Cy = params[4];

    cc_compute_Xd_Yd_and_r_squared ();

    /* remove the sensor distortion before computing the translation and rotation stuff */
    cc_remove_sensor_plane_distortion_from_Xd_and_Yd ();

    cc_compute_U ();

    cc_compute_Tx_and_Ty ();

    cc_compute_R ();

    /* we need to do this just to see if we have to flip the rotation matrix */
    cc_compute_approximate_f_and_Tz ();

    if (cc.f < 0) {
        /* try the other solution for the orthonormal matrix */
        cc.r3 = -cc.r3;
        cc.r6 = -cc.r6;
        cc.r7 = -cc.r7;
        cc.r8 = -cc.r8;
        solve_RPY_transform ();

        cc_compute_approximate_f_and_Tz ();

        if (cc.f < 0) {
            fprintf (stderr, "error - possible handedness problem with data\n");
            exit (-1);
        }
    }

    /* now calculate the squared error assuming zero distortion */
    for (i = 0; i < cd.point_count; i++) {
        /* convert from world coordinates to camera coordinates */
        /* Note: zw is implicitly assumed to be zero for these (coplanar) calculations */
        xc = cc.r1 * cd.xw[i] + cc.r2 * cd.yw[i] + cc.Tx;
        yc = cc.r4 * cd.xw[i] + cc.r5 * cd.yw[i] + cc.Ty;
        zc = cc.r7 * cd.xw[i] + cc.r8 * cd.yw[i] + Tz;

        /* convert from camera coordinates to undistorted sensor coordinates */
        Xu_1 = f * xc / zc;
        Yu_1 = f * yc / zc;

        /* convert from distorted sensor coordinates to undistorted sensor coordinates  */
        /* (already done, actually)                          */
        Xu_2 = Xd[i];
        Yu_2 = Yd[i];

        /* record the error in the undistorted sensor coordinates */
        err[i] = hypot (Xu_1 - Xu_2, Yu_1 - Yu_2);
    }
    *info = 0;
}


void      cc_five_parm_optimization_with_early_distortion_removal ()
{
#define NPARAMS 5

    int       i;

    /* Parameters needed by MINPACK's lmdif() */

    int     m = cd.point_count;
    int     n = NPARAMS;
    double  x[NPARAMS];
    double *fvec;
    double  ftol = REL_SENSOR_TOLERANCE_ftol;
    double  xtol = REL_PARAM_TOLERANCE_xtol;
    double  gtol = ORTHO_TOLERANCE_gtol;
    int     maxfev = MAXFEV;
    double  epsfcn = EPSFCN;
    double  diag[NPARAMS];
    int     mode = MODE;
    double  factor = FACTOR;
    int     info;
    int     nfev;
    double *fjac;
    int     ipvt[NPARAMS];
    double  qtf[NPARAMS];
    double  wa1[NPARAMS];
    double  wa2[NPARAMS];
    double  wa3[NPARAMS];
    double *wa4;

    /* allocate some workspace */
    if (( fvec = (double *) calloc ((unsigned int) m, (unsigned int) sizeof(double))) == NULL ) {
        fprintf(stderr,"calloc: Cannot allocate workspace fvec\n");
        exit(-1);
    }

    if (( fjac = (double *) calloc ((unsigned int) m*n, (unsigned int) sizeof(double))) == NULL ) {
        fprintf(stderr,"calloc: Cannot allocate workspace fjac\n");
        exit(-1);
    }

    if (( wa4 = (double *) calloc ((unsigned int) m, (unsigned int) sizeof(double))) == NULL ) {
        fprintf(stderr,"calloc: Cannot allocate workspace wa4\n");
        exit(-1);
    }

    /* use the current calibration and camera constants as a starting point */
    x[0] = cc.f;
    x[1] = cc.Tz;
    x[2] = cc.kappa1;
    x[3] = cp.Cx;
    x[4] = cp.Cy;

    /* define optional scale factors for the parameters */
    if ( mode == 2 ) {
        for (i = 0; i < NPARAMS; i++)
            diag[i] = 1.0;             /* some user-defined values */
    }

    /* perform the optimization */
    lm_lmdif(m, n, x, fvec, ftol, xtol,
            gtol, maxfev, epsfcn, diag, mode,
            factor, &info, &nfev,
            fjac, ipvt, qtf,
            wa1, wa2, wa3, wa4,
            cc_five_parm_optimization_with_early_distortion_removal_error,
            lm_print, 0);
/*
    lm_lmdif (cc_five_parm_optimization_with_early_distortion_removal_error,
            &m, &n, x, fvec, &ftol, &xtol, &gtol, &maxfev, &epsfcn,
            diag, &mode, &factor, &nprint, &info, &nfev, fjac, &ldfjac,
            ipvt, qtf, wa1, wa2, wa3, wa4);
*/
    /* update the calibration and camera constants */
    cc.f = x[0];
    cc.Tz = x[1];
    cc.kappa1 = x[2];
    cp.Cx = x[3];
    cp.Cy = x[4];

    /* release allocated workspace */
    free (fvec);
    free (fjac);
    free (wa4);

#ifdef DEBUG
    /* print the number of function calls during iteration */
    fprintf(stderr,"info: %d nfev: %d\n\n",info,nfev);
#endif

#undef NPARAMS
}


/************************************************************************/
void      cc_nic_optimization_error (double* params, int m_dat, double *err,
        void * pData, int * info)
{
    int       i;

    double    xc,
              yc,
              zc,
              Xd_,
              Yd_,
              Xu_1,
              Yu_1,
              Xu_2,
              Yu_2,
              distortion_factor,
              Rx,
              Ry,
              Rz,
              Tx,
              Ty,
              Tz,
              kappa1,
              f,
              r1,
              r2,
              r4,
              r5,
              r7,
              r8,
              sa,
              sb,
              sg,
              ca,
              cb,
              cg;

    Rx = params[0];
    Ry = params[1];
    Rz = params[2];
    Tx = params[3];
    Ty = params[4];
    Tz = params[5];
    kappa1 = params[6];
    f = params[7];

    SINCOS (Rx, sa, ca);
    SINCOS (Ry, sb, cb);
    SINCOS (Rz, sg, cg);
    r1 = cb * cg;
    r2 = cg * sa * sb - ca * sg;
    r4 = cb * sg;
    r5 = sa * sb * sg + ca * cg;
    r7 = -sb;
    r8 = cb * sa;

    for (i = 0; i < cd.point_count; i++) {
        /* convert from world coordinates to camera coordinates */
        /* Note: zw is implicitly assumed to be zero for these (coplanar) calculations */
        xc = r1 * cd.xw[i] + r2 * cd.yw[i] + Tx;
        yc = r4 * cd.xw[i] + r5 * cd.yw[i] + Ty;
        zc = r7 * cd.xw[i] + r8 * cd.yw[i] + Tz;

        /* convert from camera coordinates to undistorted sensor plane coordinates */
        Xu_1 = f * xc / zc;
        Yu_1 = f * yc / zc;

        /* convert from 2D image coordinates to distorted sensor coordinates */
        Xd_ = cp.dpx * (cd.Xf[i] - cp.Cx) / cp.sx; 
        Yd_ = cp.dpy * (cd.Yf[i] - cp.Cy);

        /* convert from distorted sensor coordinates to undistorted sensor plane coordinates */
        distortion_factor = 1 + kappa1 * (SQR (Xd_) + SQR (Yd_));
        Xu_2 = Xd_ * distortion_factor;
        Yu_2 = Yd_ * distortion_factor;

        /* record the error in the undistorted sensor coordinates */
        err[i] = hypot (Xu_1 - Xu_2, Yu_1 - Yu_2);
    }
    *info = 0;
}


void      cc_nic_optimization ()
{
#define NPARAMS 8

    int       i;

    /* Parameters needed by MINPACK's lmdif() */

    int     m = cd.point_count;
    int     n = NPARAMS;
    double  x[NPARAMS];
    double *fvec;
    double  ftol = REL_SENSOR_TOLERANCE_ftol;
    double  xtol = REL_PARAM_TOLERANCE_xtol;
    double  gtol = ORTHO_TOLERANCE_gtol;
    int     maxfev = MAXFEV;
    double  epsfcn = EPSFCN;
    double  diag[NPARAMS];
    int     mode = MODE;
    double  factor = FACTOR;
    int     info;
    int     nfev;
    double *fjac;
    int     ipvt[NPARAMS];
    double  qtf[NPARAMS];
    double  wa1[NPARAMS];
    double  wa2[NPARAMS];
    double  wa3[NPARAMS];
    double *wa4;

    /* allocate some workspace */
    if (( fvec = (double *) calloc ((unsigned int) m, (unsigned int) sizeof(double))) == NULL ) {
        fprintf(stderr,"calloc: Cannot allocate workspace fvec\n");
        exit(-1);
    }

    if (( fjac = (double *) calloc ((unsigned int) m*n, (unsigned int) sizeof(double))) == NULL ) {
        fprintf(stderr,"calloc: Cannot allocate workspace fjac\n");
        exit(-1);
    }

    if (( wa4 = (double *) calloc ((unsigned int) m, (unsigned int) sizeof(double))) == NULL ) {
        fprintf(stderr,"calloc: Cannot allocate workspace wa4\n");
        exit(-1);
    }

    /* use the current calibration and camera constants as a starting point */
    x[0] = cc.Rx;
    x[1] = cc.Ry;
    x[2] = cc.Rz;
    x[3] = cc.Tx;
    x[4] = cc.Ty;
    x[5] = cc.Tz;
    x[6] = cc.kappa1;
    x[7] = cc.f;

    /* define optional scale factors for the parameters */
    if ( mode == 2 ) {
        for (i = 0; i < NPARAMS; i++)
            diag[i] = 1.0;             /* some user-defined values */
    }

    /* perform the optimization */
    lm_lmdif(m, n, x, fvec, ftol, xtol,
            gtol, maxfev, epsfcn, diag, mode,
            factor, &info, &nfev,
            fjac, ipvt, qtf,
            wa1, wa2, wa3, wa4,
            cc_nic_optimization_error,
            lm_print, 0);

    /* update the calibration and camera constants */
    cc.Rx = x[0];
    cc.Ry = x[1];
    cc.Rz = x[2];
    apply_RPY_transform ();

    cc.Tx = x[3];
    cc.Ty = x[4];
    cc.Tz = x[5];
    cc.kappa1 = x[6];
    cc.f = x[7];

    /* release allocated workspace */
    free (fvec);
    free (fjac);
    free (wa4);

#ifdef DEBUG
    /* print the number of function calls during iteration */
    fprintf(stderr,"info: %d nfev: %d\n\n",info,nfev);
#endif

#undef NPARAMS
}


/************************************************************************/
void      cc_full_optimization_error (double* params, int m_dat, double *err,
        void * pData, int * info)
{
    int       i;

    double    xc,
              yc,
              zc,
              Xd_,
              Yd_,
              Xu_1,
              Yu_1,
              Xu_2,
              Yu_2,
              distortion_factor,
              Rx,
              Ry,
              Rz,
              Tx,
              Ty,
              Tz,
              kappa1,
              f,
              Cx,
              Cy,
              r1,
              r2,
              r4,
              r5,
              r7,
              r8,
              sa,
              sb,
              sg,
              ca,
              cb,
              cg;

    Rx = params[0];
    Ry = params[1];
    Rz = params[2];
    Tx = params[3];
    Ty = params[4];
    Tz = params[5];
    kappa1 = params[6];
    f = params[7];
    Cx = params[8];
    Cy = params[9];

    SINCOS (Rx, sa, ca);
    SINCOS (Ry, sb, cb);
    SINCOS (Rz, sg, cg);
    r1 = cb * cg;
    r2 = cg * sa * sb - ca * sg;
    r4 = cb * sg;
    r5 = sa * sb * sg + ca * cg;
    r7 = -sb;
    r8 = cb * sa;

    for (i = 0; i < cd.point_count; i++) {
        /* convert from world coordinates to camera coordinates */
        /* Note: zw is implicitly assumed to be zero for these (coplanar) calculations */
        xc = r1 * cd.xw[i] + r2 * cd.yw[i] + Tx;
        yc = r4 * cd.xw[i] + r5 * cd.yw[i] + Ty;
        zc = r7 * cd.xw[i] + r8 * cd.yw[i] + Tz;

        /* convert from camera coordinates to undistorted sensor plane coordinates */
        Xu_1 = f * xc / zc;
        Yu_1 = f * yc / zc;

        /* convert from 2D image coordinates to distorted sensor coordinates */
        Xd_ = cp.dpx * (cd.Xf[i] - Cx) / cp.sx; 
        Yd_ = cp.dpy * (cd.Yf[i] - Cy);

        /* convert from distorted sensor coordinates to undistorted sensor plane coordinates */
        distortion_factor = 1 + kappa1 * (SQR (Xd_) + SQR (Yd_));
        Xu_2 = Xd_ * distortion_factor;
        Yu_2 = Yd_ * distortion_factor;

        /* record the error in the undistorted sensor coordinates */
        err[i] = hypot (Xu_1 - Xu_2, Yu_1 - Yu_2);
    }
    *info = 0;
}


void      cc_full_optimization ()
{
#define NPARAMS 10

    int       i;

    /* Parameters needed by MINPACK's lmdif() */

    int     m = cd.point_count;
    int     n = NPARAMS;
    double  x[NPARAMS];
    double *fvec;
    double  ftol = REL_SENSOR_TOLERANCE_ftol;
    double  xtol = REL_PARAM_TOLERANCE_xtol;
    double  gtol = ORTHO_TOLERANCE_gtol;
    int     maxfev = MAXFEV;
    double  epsfcn = EPSFCN;
    double  diag[NPARAMS];
    int     mode = MODE;
    double  factor = FACTOR;
    int     info;
    int     nfev;
    double *fjac;
    int     ipvt[NPARAMS];
    double  qtf[NPARAMS];
    double  wa1[NPARAMS];
    double  wa2[NPARAMS];
    double  wa3[NPARAMS];
    double *wa4;

    /* allocate some workspace */
    if (( fvec = (double *) calloc ((unsigned int) m, (unsigned int) sizeof(double))) == NULL ) {
        fprintf(stderr,"calloc: Cannot allocate workspace fvec\n");
        exit(-1);
    }

    if (( fjac = (double *) calloc ((unsigned int) m*n, (unsigned int) sizeof(double))) == NULL ) {
        fprintf(stderr,"calloc: Cannot allocate workspace fjac\n");
        exit(-1);
    }

    if (( wa4 = (double *) calloc ((unsigned int) m, (unsigned int) sizeof(double))) == NULL ) {
        fprintf(stderr,"calloc: Cannot allocate workspace wa4\n");
        exit(-1);
    }

    /* use the current calibration and camera constants as a starting point */
    x[0] = cc.Rx;
    x[1] = cc.Ry;
    x[2] = cc.Rz;
    x[3] = cc.Tx;
    x[4] = cc.Ty;
    x[5] = cc.Tz;
    x[6] = cc.kappa1;
    x[7] = cc.f;
    x[8] = cp.Cx;
    x[9] = cp.Cy;

    /* define optional scale factors for the parameters */
    if ( mode == 2 ) {
        for (i = 0; i < NPARAMS; i++)
            diag[i] = 1.0;             /* some user-defined values */
    }

    /* perform the optimization */
    lm_lmdif(m, n, x, fvec, ftol, xtol,
            gtol, maxfev, epsfcn, diag, mode,
            factor, &info, &nfev,
            fjac, ipvt, qtf,
            wa1, wa2, wa3, wa4,
            cc_full_optimization_error,
            lm_print, 0);
/* 
    lm_lmdif (cc_full_optimization_error,
            &m, &n, x, fvec, &ftol, &xtol, &gtol, &maxfev, &epsfcn,
            diag, &mode, &factor, &nprint, &info, &nfev, fjac, &ldfjac,
            ipvt, qtf, wa1, wa2, wa3, wa4);
*/
    /* update the calibration and camera constants */
    cc.Rx = x[0];
    cc.Ry = x[1];
    cc.Rz = x[2];
    apply_RPY_transform ();

    cc.Tx = x[3];
    cc.Ty = x[4];
    cc.Tz = x[5];
    cc.kappa1 = x[6];
    cc.f = x[7];
    cp.Cx = x[8];
    cp.Cy = x[9];

    /* release allocated workspace */
    free (fvec);
    free (fjac);
    free (wa4);

#ifdef DEBUG
    /* print the number of function calls during iteration */
    fprintf(stderr,"info: %d nfev: %d\n\n",info,nfev);
#endif

#undef NPARAMS
}

/************************************************************************/

void coplanar_calibration (const struct camera_parameters* pcp, 
        const struct calibration_data* pcd, struct calibration_constants* pcc)
{
    cp = *pcp;
    cd = *pcd;

    /* just do the basic 3 parameter (Tz, f, kappa1) optimization */
    cc_three_parm_optimization ();
    
    *pcc = cc;
}


void coplanar_calibration_with_full_optimization (const struct camera_parameters* pcp, 
        const struct calibration_data* pcd, struct calibration_constants* pcc)
{
    cp = *pcp;
    cd = *pcd;

    /* start with a 3 parameter (Tz, f, kappa1) optimization */
    cc_three_parm_optimization ();

    /* do a 5 parameter (Tz, f, kappa1, Cx, Cy) optimization */
    cc_five_parm_optimization_with_late_distortion_removal ();

    /* do a better 5 parameter (Tz, f, kappa1, Cx, Cy) optimization */
    cc_five_parm_optimization_with_early_distortion_removal ();

    /* do a full optimization minus the image center */
    cc_nic_optimization ();

    /* do a full optimization including the image center */
    cc_full_optimization ();
    
    *pcc = cc;
}

