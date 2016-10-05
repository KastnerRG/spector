// ----------------------------------------------------------------------
// Copyright 2011-2015 by Virginia Polytechnic Institute and State University
// All rights reserved.
// From the OpenDwarfs benchmark suite (https://github.com/vtsynergy/OpenDwarfs),
// released under the LGPL v2.1 license provided in the LICENSE file accompanying this software.
// ----------------------------------------------------------------------

float r4_exp ( unsigned long int *jsr, int ke[256], float fe[256], 
  float we[256] );
void r4_exp_setup ( int ke[256], float fe[256], float we[256] );
float r4_nor ( unsigned long int *jsr, int kn[128], float fn[128], 
  float wn[128] );
void r4_nor_setup ( int kn[128], float fn[128], float wn[128] );
float r4_uni ( unsigned long int *jsr );
unsigned long int shr3 ( unsigned long int *jsr );
void timestamp ( void );
