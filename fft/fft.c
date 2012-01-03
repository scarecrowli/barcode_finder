/* fft.c
 *
 * Routines for Fast Fourier Transforms
 *
 * Taken from Numerical Recipes, 2nd edition
 *
 * Note: all floats changed to doubles
 *
 * 11-12-98
 * 12-10-98
 *
 */

//#include "nrutil.h"
#include <math.h>
#include "fft.h"

/* Replaces data[1..2*nn] by its discrete Fourier transform, if isign is
 * input as 1; or replaces data[1..2*nn] by nn times its inverse
 * discrete Fourier transform, if isign is input as -1.  data is a
 * complex array of length nn or, equivalently, a real array of length
 * 2*nn.  nn MUST be an integer power of 2  (this is not checked for!)
 */
void four1(double data[], unsigned long nn, int isign)
{
   unsigned long n,mmax,m,j,istep,i;
   double wtemp,wr,wpr,wpi,wi,theta;
   double tempr,tempi;

   n=nn << 1;
   j=1;
   for (i=1;i<n;i+=2) {
      if (j > i) {
	 SWAP(data[j],data[i]);
	 SWAP(data[j+1],data[i+1]);
      }
      m=n >> 1;
      while (m >= 2 && j > m) {
	 j -= m;
	 m >>= 1;
      }
      j += m;
   }
   /* Here begins the Danielson-Lanczos section of the routine */
   mmax=2;
   while (n > mmax) {
      istep=mmax << 1;
      theta=isign*(6.28318530717959/mmax);
      wtemp=sin(0.5*theta);
      wpr = -2.0*wtemp*wtemp;
      wpi=sin(theta);
      wr=1.0;
      wi=0.0;
      for (m=1;m<mmax;m+=2) {
	 for (i=m;i<=n;i+=istep) {
	    j=i+mmax;
	    tempr=wr*data[j]-wi*data[j+1];
	    tempi=wr*data[j+1]+wi*data[j];
	    data[j]=data[i]-tempr;
	    data[j+1]=data[i+1]-tempi;
	    data[i] += tempr;
	    data[i+1] += tempi;
	 }
	 wr=(wtemp=wr)*wpr-wi*wpi+wr;
	 wi=wi*wpr+wtemp*wpi+wi;
      }
      mmax=istep;
   }
}

/* Calculates the Fourier transform of a set of n real-valued data
 * points.  Replaces this data (which is store in array data[1..n]) by
 * the positive frequency half of its complex Fourier transform.  The
 * real-valued first and last components of the complex transform are
 * returned as elements data[1] and data[2], respectively.  n must be a
 * power of 2.  This routine also calculates the inverse transform of a
 * complex data array if it is the transform of real data. (Result in
 * this case must be multiplied by 2/n.) */
void realft(double data[], unsigned long n, int isign)
{
   void four1(double data[], unsigned long nn, int isign);
   unsigned long i,i1,i2,i3,i4,np3;
   double c1=0.5,c2,h1r,h1i,h2r,h2i;
   double wr,wi,wpr,wpi,wtemp,theta;

   theta=M_PI/(double) (n>>1);
   if (isign == 1) {
      c2 = -0.5;
      four1(data,n>>1,1);
   } else {
      c2=0.5;
      theta = -theta;
   }
   wtemp=sin(0.5*theta);
   wpr = -2.0*wtemp*wtemp;
   wpi=sin(theta);
   wr=1.0+wpr;
   wi=wpi;
   np3=n+3;
   for (i=2;i<=(n>>2);i++) {
      i4=1+(i3=np3-(i2=1+(i1=i+i-1)));
      h1r=c1*(data[i1]+data[i3]);
      h1i=c1*(data[i2]-data[i4]);
      h2r = -c2*(data[i2]+data[i4]);
      h2i=c2*(data[i1]-data[i3]);
      data[i1] = h1r+wr*h2r-wi*h2i;
      data[i2] = h1i+wr*h2i+wi*h2r;
      data[i3] = h1r-wr*h2r+wi*h2i;
      data[i4] = -h1i+wr*h2i+wi*h2r;
      wr=(wtemp=wr)*wpr-wi*wpi+wr;
      wi=wi*wpr+wtemp*wpi+wi;
   }
   if (isign == 1) {
      data[1] = (h1r=data[1])+data[2];
      data[2] = h1r-data[2];
   } else {
      data[1]=c1*((h1r=data[1])+data[2]);
      data[2]=c1*(h1r-data[2]);
      four1(data,n>>1,-1);
   }
}

void twofft(double data1[], double data2[], double fft1[], double fft2[], unsigned long n)
/* Given two real input arrays data1[1..n] and data2[1..n], this routine
calls four1 and returns two complex output arrays, fft1[1..2n] and
fft2[1..2n], each of complex length n (i.e., real length 2*n), which
contain the discrete Fourier transforms of the respective data arrays. n
MUST be an integer power of 2. */
{
   void four1(double data[], unsigned long nn, int isign);
   unsigned long nn3,nn2,jj,j;
   double rep,rem,aip,aim;
   nn3=1+(nn2=2+n+n);
   for (j=1,jj=2;j<=n;j++,jj+=2) { 
   /* Pack the two real arrays into one complex array. */
      fft1[jj-1]=data1[j];
      fft1[jj]=data2[j];
   }
   four1(fft1,n,1); /* Transform the complex array. */
   fft2[1]=fft1[2];
   fft1[2]=fft2[2]=0.0;
   for (j=3;j<=n+1;j+=2) {
      /* Use symmetries to separate the two transforms. */
      rep=0.5*(fft1[j]+fft1[nn2-j]); 
      rem=0.5*(fft1[j]-fft1[nn2-j]);
      aip=0.5*(fft1[j+1]+fft1[nn3-j]);
      aim=0.5*(fft1[j+1]-fft1[nn3-j]);
      fft1[j]=rep; /* Ship them out in two complex arrays. */
      fft1[j+1]=aim;
      fft1[nn2-j]=rep;
      fft1[nn3-j] = -aim;
      fft2[j]=aip;
      fft2[j+1] = -rem;
      fft2[nn2-j]=aip;
      fft2[nn3-j]=rem;
   }
}

void convlv(double data[], unsigned long n, double respns[], unsigned long m, int isign, double ans[])
/* Convolves or deconvolves a real data set data[1..n] (including any
user-supplied zero padding) with a response function respns[1..n]. The
response function must be stored in wrap-around order in the first m
elements of respns, where m is an odd integer.  Wrap-around order
means that the first half of the array respns contains the impulse
response function at positive times, while the second half of the array
contains the impulse response function at negative times, counting down
from the highest element respns[m]. On input isign is +1 for
convolution, n. The answer is returned in the first n components of ans.
However, ans must be supplied in the calling program with dimensions
[1..2*n], for consistency with twofft. n MUST be an integer power of
two. */
{
   //void realft(double data[], unsigned long n, int isign);
   //void twofft(double data1[], double data2[], double fft1[], double fft2[], unsigned long n);
   unsigned long i,no2;
   double dum,mag2,*fft;

   fft=dvector(1,n<<1);
   for (i=1;i<=(m-1)/2;i++) /* Put respns in array of length n. */
      respns[n+1-i]=respns[m+1-i];
   for (i=(m+3)/2;i<=n-(m-1)/2;i++) /* Pad with zeros. */
      respns[i]=0.0;
   twofft(data,respns,fft,ans,n); /* FFT both at once. */
   no2=n>>1;
   for (i=2;i<=n+2;i+=2) {
      if (isign == 1) {
	 /* Multiply FFTs to convolve. */
         ans[i-1]=(fft[i-1]*(dum=ans[i-1])-fft[i]*ans[i])/no2; 
         ans[i]=(fft[i]*dum+fft[i-1]*ans[i])/no2;
      } else if (isign == -1) {
         if ((mag2=SQR(ans[i-1])+SQR(ans[i])) == 0.0)
            nrerror("Deconvolving at response zero in convlv");
	 /* Divide FFTs to deconvolve. */ 
         ans[i-1]=(fft[i-1]*(dum=ans[i-1])+fft[i]*ans[i])/mag2/no2; 
	 ans[i]=(fft[i]*dum-fft[i-1]*ans[i])/mag2/no2;
      } else nrerror("No meaning for isign in convlv");
   }
   ans[2]=ans[n+1]; /* Pack last element with first for realft. */
   realft(ans,n,-1); /* Inverse transform back to time domain. */
   free_dvector(fft,1,n<<1);
}
