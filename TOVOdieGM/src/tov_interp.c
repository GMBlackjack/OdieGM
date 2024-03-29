// This C header file reads a TOV solution from data file and performs
//    1D interpolation of the solution to a desired radius.

// Author: Zachariah B. Etienne
//         zachetie **at** gmail **dot* com
// Stolen from nrpytutorial for use in Odie. 
// Additions to that purpose made by Gabriel M Steward. 

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include <cctk.h>
#include <cctk_Arguments.h>
#include <cctk_Parameters.h>

#define REAL double

#define velx (&vel[0*cctk_lsh[0]*cctk_lsh[1]*cctk_lsh[2]])
#define vely (&vel[1*cctk_lsh[0]*cctk_lsh[1]*cctk_lsh[2]])
#define velz (&vel[2*cctk_lsh[0]*cctk_lsh[1]*cctk_lsh[2]])
#define velx_p (&vel_p[0*cctk_lsh[0]*cctk_lsh[1]*cctk_lsh[2]])
#define vely_p (&vel_p[1*cctk_lsh[0]*cctk_lsh[1]*cctk_lsh[2]])
#define velz_p (&vel_p[2*cctk_lsh[0]*cctk_lsh[1]*cctk_lsh[2]])
#define velx_p_p (&vel_p_p[0*cctk_lsh[0]*cctk_lsh[1]*cctk_lsh[2]])
#define vely_p_p (&vel_p_p[1*cctk_lsh[0]*cctk_lsh[1]*cctk_lsh[2]])
#define velz_p_p (&vel_p_p[2*cctk_lsh[0]*cctk_lsh[1]*cctk_lsh[2]])

#define DIFF_X(a) (((i==0)?(a[CCTK_GFINDEX3D(cctkGH, i+1, j, k)] - \
                            a[CCTK_GFINDEX3D(cctkGH, i  , j, k)]): \
                    (i==(cctk_lsh[0]-1))?                          \
                           (a[CCTK_GFINDEX3D(cctkGH, i  , j, k)] - \
                            a[CCTK_GFINDEX3D(cctkGH, i-1, j, k)]): \
                       0.5*(a[CCTK_GFINDEX3D(cctkGH, i+1, j, k)] - \
                            a[CCTK_GFINDEX3D(cctkGH, i-1, j, k)]))/\
                   CCTK_DELTA_SPACE(0))
#define DIFF_Y(a) (((j==0)?(a[CCTK_GFINDEX3D(cctkGH, i, j+1, k)] - \
                            a[CCTK_GFINDEX3D(cctkGH, i, j  , k)]): \
                    (j==(cctk_lsh[1]-1))?                          \
                           (a[CCTK_GFINDEX3D(cctkGH, i, j  , k)] - \
                            a[CCTK_GFINDEX3D(cctkGH, i, j-1, k)]): \
                       0.5*(a[CCTK_GFINDEX3D(cctkGH, i, j+1, k)] - \
                            a[CCTK_GFINDEX3D(cctkGH, i, j-1, k)]))/\
                    CCTK_DELTA_SPACE(1))
#define DIFF_Z(a) (((k==0)?(a[CCTK_GFINDEX3D(cctkGH, i, j, k+1)] - \
                            a[CCTK_GFINDEX3D(cctkGH, i, j, k  )]): \
                    (k==(cctk_lsh[2]-1))?                          \
                           (a[CCTK_GFINDEX3D(cctkGH, i, j, k  )] - \
                            a[CCTK_GFINDEX3D(cctkGH, i, j, k-1)]): \
                       0.5*(a[CCTK_GFINDEX3D(cctkGH, i, j, k+1)] - \
                            a[CCTK_GFINDEX3D(cctkGH, i, j, k-1)]))/\
                    CCTK_DELTA_SPACE(2))

//#define STANDALONE_UNIT_TEST

int count_num_lines_in_file(FILE *in1Dpolytrope) {
  int numlines_in_file = 0;
  char * line = NULL;

  size_t len = 0;
  ssize_t read;
  while ((read = getline(&line, &len, in1Dpolytrope)) != -1) {
    numlines_in_file++;
  }
  rewind(in1Dpolytrope);

  free(line);
  return numlines_in_file;
}

int read_datafile__set_arrays(FILE *in1Dpolytrope, REAL *restrict r_Schw_arr,REAL *restrict rho_arr,REAL *restrict rho_baryon_arr,REAL *restrict P_arr,
                              REAL *restrict M_arr,REAL *restrict expnu_arr,REAL *restrict exp4phi_arr,REAL *restrict rbar_arr) {
  DECLARE_CCTK_PARAMETERS
  
  char * line = NULL;

  size_t len = 0;
  ssize_t read;

  int which_line = 0;
  while ((read = getline(&line, &len, in1Dpolytrope)) != -1) {
    // Define the line delimiters (i.e., the stuff that goes between the data on a given
    //     line of data.  Here, we define both spaces " " and tabs "\t" as data delimiters.
    const char delimiters[] = " \t";

    //Now we define "token", a pointer to the first column of data
    char *token;

    //Each successive time we call strtok(NULL,blah), we read in a new column of data from
    //     the originally defined character array, as pointed to by token.
    token=strtok(line, delimiters); if(token==NULL) { printf("BADDDD\n"); return 1; }
    r_Schw_arr[which_line]     = strtod(token, NULL); token = strtok( NULL, delimiters );
    rho_arr[which_line]        = strtod(token, NULL); token = strtok( NULL, delimiters );
    rho_baryon_arr[which_line] = strtod(token, NULL); token = strtok( NULL, delimiters );
    P_arr[which_line]          = strtod(token, NULL); token = strtok( NULL, delimiters );
    M_arr[which_line]          = strtod(token, NULL); token = strtok( NULL, delimiters );
    expnu_arr[which_line]      = strtod(token, NULL); token = strtok( NULL, delimiters );
    rbar_arr[which_line]       = strtod(token, NULL);	
    which_line++;
  }
  
  // In our case, the rbar read in is not normalized. Let's normalize it! (Also the lapse and the conformal factor need adjusting) 
  // (Gabriel M Steward) 
  
  // First, find the edge of the star (stolen from code below)
  double R_Schw = -100;
  int Rbar_idx = -100;
  for(int i=1;i<which_line;i++) {
    if(rho_arr[i-1]>0 && rho_arr[i]==0) { R_Schw = r_Schw_arr[i-1]; Rbar_idx = i-1; }
  }
  
  FILE *fp3;
  fp3 = fopen(TOVOdieGM_file_name_adjusted,"w"); 
  printf("Printing adjusted data to file '%s'.\n",TOVOdieGM_file_name_adjusted);
  // The values produced by Odie are not normalized, but people
  // might still need the normalized values and not want to 
  // manage them manually. Thus we print the values we adjusted. 
  
  double M_Total = M_arr[Rbar_idx];
  double Rbar_Edge = rbar_arr[Rbar_idx];
  double NU_Edge = expnu_arr[Rbar_idx]; // this is read in as nu first, rather than e^nu. 
  double normalizer = 0.5 * (sqrt(R_Schw * (R_Schw - 2*M_Total)) + R_Schw - M_Total) / (Rbar_Edge);
  // Normalize and adjust values to what they really are in post-processing, and print it to a new file for the user. 
    	rbar_arr[0] = 0.0;
  	expnu_arr[0] = exp(expnu_arr[0] - NU_Edge + log(1 -2.0*M_Total/R_Schw));
  	rbar_arr[1] = rbar_arr[1] * normalizer;
  	expnu_arr[1] = exp(expnu_arr[1] - NU_Edge + log(1 -2.0*M_Total/R_Schw));
  	exp4phi_arr[1] = (r_Schw_arr[1]/rbar_arr[1])*(r_Schw_arr[1]/rbar_arr[1]);
  	exp4phi_arr[0] = exp4phi_arr[1];
	// Assume conformal factor is constant near core of star, to avoid divide by zero error. 
	// Placed here since it relies on values calculated in iteration 1. 
	fprintf(fp3, "%15.14e %15.14e %15.14e %15.14e\n",rbar_arr[0]*sqrt(exp4phi_arr[0]),rbar_arr[0],expnu_arr[0],exp4phi_arr[0]);
	fprintf(fp3, "%15.14e %15.14e %15.14e %15.14e\n",rbar_arr[1]*sqrt(exp4phi_arr[1]),rbar_arr[1],expnu_arr[1],exp4phi_arr[1]);
  for (int i = 2; i < which_line; i++) { // Set zero values separately. 
  	rbar_arr[i] = rbar_arr[i] * normalizer;
  	expnu_arr[i] = exp(expnu_arr[i] - NU_Edge + log(1 -2.0*M_Total/R_Schw));
  	exp4phi_arr[i] = (r_Schw_arr[i]/rbar_arr[i])*(r_Schw_arr[i]/rbar_arr[i]);
  	fprintf(fp3, "%15.14e %15.14e %15.14e %15.14e\n",rbar_arr[i]*sqrt(exp4phi_arr[i]),rbar_arr[i],expnu_arr[i],exp4phi_arr[i]);
  }
  printf("Post processing data reported as radius, normalized isotropic radius, scaled square lapse, scaled conformal factor^4\n");  
  
  fclose(fp3);
  
  free(line);
  return 0;
}

// Find interpolation index using Bisection root-finding algorithm:
static inline int bisection_idx_finder(const REAL rrbar, const int numlines_in_file, const REAL *restrict rbar_arr) {
  int x1 = 0;
  int x2 = numlines_in_file-2;
  REAL y1 = rrbar-rbar_arr[x1];
  REAL y2 = rrbar-rbar_arr[x2];
	if (rrbar == 0) {
		return 0; // Look, why would you want to interpolate to the center? It's the center! But if you want it you can have it, it's the 0 index. 
	}
  if(y1*y2 >= 0) {
    fprintf(stderr,"INTERPOLATION BRACKETING ERROR %e | %e %e\n",rrbar,y1,y2);
    exit(1);
  }
  for(int i=0;i<numlines_in_file;i++) {
    int x_midpoint = (x1+x2)/2;
    REAL y_midpoint = rrbar-rbar_arr[x_midpoint];
    if(y_midpoint*y1 < 0) {
      x2 = x_midpoint;
      y2 = y_midpoint;
    } else {
      x1 = x_midpoint;
      y1 = y_midpoint;
    }
    if( abs(x2-x1) == 1 ) {
      // If rbar_arr[x1] is closer to rrbar than rbar_arr[x2] then return x1:
      if(fabs(rrbar-rbar_arr[x1]) < fabs(rrbar-rbar_arr[x2])) return x1;
      // Otherwise return x2:
      return x2;
    }
  }
  fprintf(stderr,"INTERPOLATION BRACKETING ERROR: DID NOT CONVERGE.\n");
  exit(1);
}

void TOV_interpolate_1D(REAL rrbar,const REAL Rbar,const int Rbar_idx,const int interp_stencil_size,
                        const int numlines_in_file,const REAL *restrict r_Schw_arr,const REAL *restrict rho_arr,const REAL *restrict rho_baryon_arr,const REAL *restrict P_arr,
                        const REAL *restrict M_arr,const REAL *restrict expnu_arr,const REAL *restrict exp4phi_arr,const REAL *restrict rbar_arr,
                        REAL *restrict rho,REAL *restrict rho_baryon,REAL *restrict P,REAL *restrict M,REAL *restrict expnu,REAL *restrict exp4phi) {

  // For this case, we know that for all functions, f(r) = f(-r)
  if(rrbar < 0) rrbar = -rrbar;

  // First find the central interpolation stencil index:
  int idx = bisection_idx_finder(rrbar,numlines_in_file,rbar_arr);


#ifdef MAX
#undef MAX
#endif
#define MAX(A, B) ( ((A) > (B)) ? (A) : (B) )

  int idxmin = MAX(0,idx-interp_stencil_size/2-1);

#ifdef MIN
#undef MIN
#endif
#define MIN(A, B) ( ((A) < (B)) ? (A) : (B) )

  // -= Do not allow the interpolation stencil to cross the star's surface =-
  // max index is when idxmin + (interp_stencil_size-1) = Rbar_idx
  //  -> idxmin at most can be Rbar_idx - interp_stencil_size + 1
  if(rrbar < Rbar) {
    idxmin = MIN(idxmin,Rbar_idx - interp_stencil_size + 1);
  } else {
    idxmin = MAX(idxmin,Rbar_idx+1);
    idxmin = MIN(idxmin,numlines_in_file - interp_stencil_size + 1);
  }
  // Now perform the Lagrange polynomial interpolation:

  // First set the interpolation coefficients:
  REAL rbar_sample[interp_stencil_size];
  for(int i=idxmin;i<idxmin+interp_stencil_size;i++) {
    rbar_sample[i-idxmin] = rbar_arr[i];
  }
  REAL l_i_of_r[interp_stencil_size];
  for(int i=0;i<interp_stencil_size;i++) {
    REAL numer = 1.0;
    REAL denom = 1.0;
    for(int j=0;j<i;j++) {
      numer *= rrbar - rbar_sample[j];
      denom *= rbar_sample[i] - rbar_sample[j];
    }
    for(int j=i+1;j<interp_stencil_size;j++) {
      numer *= rrbar - rbar_sample[j];
      denom *= rbar_sample[i] - rbar_sample[j];
    }
    l_i_of_r[i] = numer/denom;
  }

  // Then perform the interpolation:
  *rho = 0.0;
  *rho_baryon = 0.0;
  *P = 0.0;
  *M = 0.0;
  *expnu = 0.0;
  *exp4phi = 0.0;

  REAL r_Schw = 0.0;
  for(int i=idxmin;i<idxmin+interp_stencil_size;i++) {
    r_Schw      += l_i_of_r[i-idxmin] * r_Schw_arr[i];
    *rho        += l_i_of_r[i-idxmin] * rho_arr[i];
    *rho_baryon += l_i_of_r[i-idxmin] * rho_baryon_arr[i];
    *P          += l_i_of_r[i-idxmin] * P_arr[i];
    *M          += l_i_of_r[i-idxmin] * M_arr[i];
    *expnu      += l_i_of_r[i-idxmin] * expnu_arr[i];
    *exp4phi    += l_i_of_r[i-idxmin] * exp4phi_arr[i];
  }

  if(rrbar > Rbar) {
    *rho        = 0;
    *rho_baryon = 0;
    *P          = 0;
    *M          = M_arr[Rbar_idx+1];
    *expnu      = 1. - 2.*(*M) / r_Schw;
    *exp4phi    = pow(r_Schw / rrbar,2.0);
  }
}

// To compile, copy this file to tov_interp.c, and then run:
//  gcc -Ofast tov_interp.c -o tov_interp -DSTANDALONE_UNIT_TEST
// #ifdef STANDALONE_UNIT_TEST
void interp_main(CCTK_ARGUMENTS) {
  // ETK interface
  DECLARE_CCTK_ARGUMENTS
  DECLARE_CCTK_PARAMETERS

  // Open the data file:
  char filename[100];
  sprintf(filename,TOVOdieGM_file_name);

  FILE *in1Dpolytrope = fopen(filename, "r");
  if (in1Dpolytrope == NULL) {
    fprintf(stderr,"ERROR: could not open file %s\n",filename);
    exit(1);
  }

  // Count the number of lines in the data file:
  int numlines_in_file = count_num_lines_in_file(in1Dpolytrope);

  // Allocate space for all data arrays:
  REAL *r_Schw_arr     = (REAL *)malloc(sizeof(REAL)*numlines_in_file);
  REAL *rho_arr        = (REAL *)malloc(sizeof(REAL)*numlines_in_file);
  REAL *rho_baryon_arr = (REAL *)malloc(sizeof(REAL)*numlines_in_file);
  REAL *P_arr          = (REAL *)malloc(sizeof(REAL)*numlines_in_file);
  REAL *M_arr          = (REAL *)malloc(sizeof(REAL)*numlines_in_file);
  REAL *expnu_arr      = (REAL *)malloc(sizeof(REAL)*numlines_in_file);
  REAL *exp4phi_arr    = (REAL *)malloc(sizeof(REAL)*numlines_in_file);
  REAL *rbar_arr       = (REAL *)malloc(sizeof(REAL)*numlines_in_file);

  // Read from the data file, filling in arrays
  if(read_datafile__set_arrays(in1Dpolytrope, r_Schw_arr,rho_arr,rho_baryon_arr,P_arr,M_arr,expnu_arr,exp4phi_arr,rbar_arr) == 1) {
    fprintf(stderr,"ERROR WHEN READING FILE %s!\n",filename);
    exit(1);
  }
  fclose(in1Dpolytrope);

  REAL Rbar = -100;
  int Rbar_idx = -100;
  for(int i=1;i<numlines_in_file;i++) {
    if(rho_arr[i-1]>0 && rho_arr[i]==0) { Rbar = rbar_arr[i-1]; Rbar_idx = i-1; }
  }
  
  printf("Star's Final Isotropic Radius, Radius, and Mass: %15.14e %15.14e %15.14e \n", Rbar*1477.974, r_Schw_arr[Rbar_idx]*1477.974, M_arr[Rbar_idx]);
  
  if(Rbar<0) {
    fprintf(stderr,"Error: could not find r=R from data file.\n");
    exit(1);
  }

	// Here's the part where we actually place values on the
	// ETK grid. 
  for(int i=0; i<cctk_lsh[0]; i++) {
	for(int j=0; j<cctk_lsh[1]; j++)  {
	    for(int k=0; k<cctk_lsh[2]; k++) {
	    	int i3D = CCTK_GFINDEX3D(cctkGH, i, j, k);
	    	
		double radiusETK = sqrt( x[i3D]*x[i3D] + y[i3D]*y[i3D] + z[i3D]*z[i3D] );
		REAL rho_e,rho_baryon,P,M,expnu,exp4phi;
		TOV_interpolate_1D(radiusETK,Rbar,Rbar_idx,4,  numlines_in_file,r_Schw_arr,rho_arr,rho_baryon_arr,P_arr,M_arr,expnu_arr,exp4phi_arr,rbar_arr,  &rho_e,&rho_baryon,&P,&M,&expnu,&exp4phi);

		rho[i3D] = rho_baryon;
		press[i3D] = P;
		betax[i3D] = 0.0;
		betay[i3D] = 0.0;
		betaz[i3D] = 0.0;
		alp[i3D] = pow(expnu,0.5); 
		// Validated that this is the correct assignment, 
		// as TOVSolver's values are comparable.
		eps[i3D] = (rho_e / (rho_baryon+1e-30)) - 1.0; 
		//tiny number prevents 0/0. 
		gxx[i3D] = 1.0/(exp4phi+1e-30);
		gyy[i3D] = gxx[i3D];
		gzz[i3D] = gxx[i3D];
		gxy[i3D] = 0.0;
		gxz[i3D] = 0.0;
		gyz[i3D] = 0.0;
		velx[i3D] = 0.0;
		vely[i3D] = 0.0;
		velz[i3D] = 0.0;
	        w_lorentz[i3D] = 1/sqrt(1.0-(
                          gxx[i3D] * velx[i3D] * velx[i3D]+
                          gyy[i3D] * vely[i3D] * vely[i3D]+
                          gzz[i3D] * velz[i3D] * velz[i3D]+
                        2*gxy[i3D] * velx[i3D] * vely[i3D]+
                        2*gxz[i3D] * velx[i3D] * velz[i3D]+
                        2*gyz[i3D] * vely[i3D] * velz[i3D])*
                        exp4phi);
                // As the velocities are zero this reduces to 1.
	    }	
	}
  }
  int i3D = cctk_lsh[2]*cctk_lsh[1]*cctk_lsh[0];
  switch(TOV_Populate_Timelevels)
  {
    case 3:
        TOV_Copy(i3D, gxx_p_p,  gxx);
        TOV_Copy(i3D, gyy_p_p,  gyy);
        TOV_Copy(i3D, gzz_p_p,  gzz);
        TOV_Copy(i3D, gxy_p_p,  gxy);
        TOV_Copy(i3D, gxz_p_p,  gxz);
        TOV_Copy(i3D, gyz_p_p,  gyz);
        TOV_Copy(i3D, rho_p_p,  rho);
        TOV_Copy(i3D, eps_p_p,  eps);
        TOV_Copy(i3D, velx_p_p, velx);
        TOV_Copy(i3D, vely_p_p, vely);
        TOV_Copy(i3D, velz_p_p, velz);
        TOV_Copy(i3D, w_lorentz_p_p, w_lorentz);
        // fall through
    case 2:
        TOV_Copy(i3D, gxx_p,  gxx);
        TOV_Copy(i3D, gyy_p,  gyy);
        TOV_Copy(i3D, gzz_p,  gzz);
        TOV_Copy(i3D, gxy_p,  gxy);
        TOV_Copy(i3D, gxz_p,  gxz);
        TOV_Copy(i3D, gyz_p,  gyz);
        TOV_Copy(i3D, rho_p,  rho);
        TOV_Copy(i3D, eps_p,  eps);
        TOV_Copy(i3D, velx_p, velx);
        TOV_Copy(i3D, vely_p, vely);
        TOV_Copy(i3D, velz_p, velz);
        TOV_Copy(i3D, w_lorentz_p, w_lorentz);
        // fall through
    case 1:
        break;
    default:
        CCTK_VWarn(CCTK_WARN_ABORT, __LINE__, __FILE__, CCTK_THORNSTRING,
                   "Unsupported number of TOV_Populate_TimelevelsL: %d",
                   (int)TOV_Populate_Timelevels);
        break;
  }

  printf("Interpolation Successful!\n");

  // Free the malloc()'s!
  free(r_Schw_arr);
  free(rho_arr);
  free(rho_baryon_arr);
  free(P_arr);
  free(M_arr);
  free(expnu_arr);
  free(exp4phi_arr);
  free(rbar_arr);
  
  printf("Freeing Successful!\n");
  // This is here to make sure we have actually reached the absolute
  // end of the file. 
  
  // return 0;
}
// #endif
