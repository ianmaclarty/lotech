#include "ltrandom.h"

// This code is based on Knuth's program available at:
// http://www-cs-staff.stanford.edu/~uno/programs/rng.c.

ct_assert(sizeof(int) == 4);

#define KK 100                     /* the int lag */
#define LL  37                     /* the short lag */
#define MM (1<<30)                 /* the modulus */
#define mod_diff(x,y) (((x)-(y))&(MM-1)) /* subtraction mod MM */
#define QUALITY 1009 /* recommended quality level for high-res use */

#define ran_arr_next() (*ran_arr_ptr>=0? *ran_arr_ptr++: ran_arr_cycle())
int ran_arr_started = -1;

LTRandomGenerator::LTRandomGenerator(int seed) : LTObject(LT_TYPE_RANDOMGENERATOR) {
    ran_x = new int[KK + QUALITY];
    ran_arr_buf = &ran_x[KK];
    ran_start(seed);
}

LTRandomGenerator::~LTRandomGenerator() {
    delete[] ran_x;
}

int LTRandomGenerator::nextInt(int n) {
    // I don't want to convert to floats, because I want to get the same
    // results on different architectures.
    //
    // This seems to work ok, although requires more testing.
    return ran_arr_next() % n;
    /*
     * This might be better (?)
    unsigned int r = ran_arr_next();
    unsigned char *c0 = (unsigned char*)&r;
    unsigned char c[4];
    // Swap bytes, because most significant bits should be "more random".
    c[0] = c0[3];
    c[1] = c0[2];
    c[2] = c0[1];
    c[3] = c0[0];
    return (*((unsigned int*)c)) % n;
    */
}

bool LTRandomGenerator::nextBool() {
    static const int t = 1 << 27;
    return ran_arr_next() & t;
}

LTdouble LTRandomGenerator::nextDouble() {
    return (LTdouble)ran_arr_next() / (LTdouble)MM;
}

LTfloat LTRandomGenerator::nextFloat() {
    return (LTfloat)ran_arr_next() / (LTfloat)MM;
}

void LTRandomGenerator::ran_array(int *aa, int n) {
  register int i,j;
  for (j=0;j<KK;j++) aa[j]=ran_x[j];
  for (;j<n;j++) aa[j]=mod_diff(aa[j-KK],aa[j-LL]);
  for (i=0;i<LL;i++,j++) ran_x[i]=mod_diff(aa[j-KK],aa[j-LL]);
  for (;i<KK;i++,j++) ran_x[i]=mod_diff(aa[j-KK],ran_x[i-LL]);
}

#define TT  70   /* guaranteed separation between streams */
#define is_odd(x)  ((x)&1)          /* units bit of x */

void LTRandomGenerator::ran_start(int seed) {
  register int t,j;
  int x[KK+KK-1];              /* the preparation buffer */
  register int ss=(seed+2)&(MM-2);
  for (j=0;j<KK;j++) {
    x[j]=ss;                      /* bootstrap the buffer */
    ss<<=1; if (ss>=MM) ss-=MM-2; /* cyclic shift 29 bits */
  }
  x[1]++;              /* make x[1] (and only x[1]) odd */
  for (ss=seed&(MM-1),t=TT-1; t; ) {       
    for (j=KK-1;j>0;j--) x[j+j]=x[j], x[j+j-1]=0; /* "square" */
    for (j=KK+KK-2;j>=KK;j--)
      x[j-(KK-LL)]=mod_diff(x[j-(KK-LL)],x[j]),
      x[j-KK]=mod_diff(x[j-KK],x[j]);
    if (is_odd(ss)) {              /* "multiply by z" */
      for (j=KK;j>0;j--)  x[j]=x[j-1];
      x[0]=x[KK];            /* shift the buffer cyclically */
      x[LL]=mod_diff(x[LL],x[KK]);
    }
    if (ss) ss>>=1; else t--;
  }
  for (j=0;j<LL;j++) ran_x[j+KK-LL]=x[j];
  for (;j<KK;j++) ran_x[j-LL]=x[j];
  for (j=0;j<10;j++) ran_array(x,KK+KK-1); /* warm things up */
  ran_arr_ptr=&ran_arr_started;
}

int LTRandomGenerator::ran_arr_cycle() {
  ran_array(ran_arr_buf,QUALITY);
  ran_arr_buf[KK]=-1;
  ran_arr_ptr=ran_arr_buf+1;
  return ran_arr_buf[0];
}
