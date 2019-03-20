/*
 * Copyright 2016 CSIRO
 *
 * This file is part of Mastik.
 *
 * Mastik is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Mastik is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Mastik.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <util.h>
#include <l3.h>

#define SAMPLES 200


int main(int ac, char **av) {
  delayloop(3000000000U);
  printf("CLOCKS_PER_SEC = %d\n", CLOCKS_PER_SEC);

  l3pp_t l3 = l3_prepare(NULL);

  int nsets = l3_getSets(l3);

  int *res = calloc(SAMPLES, sizeof(int));
  int *resFiveWindow = calloc(SAMPLES, sizeof(int));
  int *resTenWindow = calloc(SAMPLES, sizeof(int));

  for (int i = 0; i < SAMPLES; i+= 4096/sizeof(int))
    res[i] = 1;
  
  for (int i = 0; i < nsets; i++) {
	printf("set num %d\n", i);
    l3_unmonitorall(l3);
    l3_monitor(l3, i);

    l3_repeatedprobecount(l3, SAMPLES, res, 2000);

    for (int j = 0; j < SAMPLES; j++) {
    	int sam = ((res[j] < 0) | res[j]>20 ? 0 : res[j]);
    	int sam2 = ((res[j-4] <0)| res[j-4]>20 ? 0 : res[j-4]);
    	int sam3 = ((res[j-9] <0)| res[j-9]>20 ? 0 : res[j-9]);
    	if(j>=5){
    		resFiveWindow[j] = (resFiveWindow[j-1]*5-sam2+sam)/5;
    	}
    	if(j>=10){
    		resTenWindow[j] = (resTenWindow[j-1]*10-sam3+sam)/10;

    	}
      printf("%4d, %4d, %4d \n", (int)res[j],(int)resFiveWindow[j],(int)resTenWindow[j]);
    }
  }

  free(res);
  l3_release(l3);
}
