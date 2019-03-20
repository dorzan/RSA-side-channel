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
#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <pthread.h>



#include <util.h>
#include <l3.h>

#define SETS_IN_SLICE 2048 
#define NUM_OF_SLICES 4
#define SAMPLES 1000000
#define SET 9
l3pp_t l3;

void pinToCore(int coreId);


void menu(int nsets);


int sets_into_file(int nsets,int slot);


__always_inline uint64_t rdtscp64() {
	uint32_t low, high;
	asm volatile ("rdtscp": "=a" (low), "=d" (high) :: "ecx");
	return (((uint64_t)high) << 32) | low;
}



int main(int ac, char **av) {
	//system("pwd");
	pinToCore(0);
	int sets[]={924};
	delayloop(3000000000U);

	l3 = l3_prepare(NULL);
	int nsets = l3_getSets(l3);
	printf("prepare amos finish, n sets = %d\n", nsets);
	//system("sudo sync; sudo sh -c \"echo 3 > /proc/sys/vm/drop_caches\"");
	//system("sudo sync && echo 3 | sudo tee /proc/sys/vm/drop_caches");

	menu(nsets);
	//sets_into_files(l3,sets,1);
	// int nmonitored = nsets/64;
	l3_release(l3);
}


void menu(int nsets){
	char command[90];
	int i , num_of_sets = 0;
	int graphs; // 0 for set, 1 for sets filtered
	int choice, setToProbe, slot,slice;
	float filter;
	FILE *f = fopen("options.txt", "r+");
	fread(&setToProbe,sizeof(int),1,f);
	fread(&slot,sizeof(int),1,f);
	fclose(f);
	while(1){
		FILE *f = fopen("options.txt", "r+");
		printf(    "1) choose set and probe         slot time: %d      set to probe is : %d\n"
				"2) probe chosen set\n"
				"3) probe all sets with filter\n"
				"4) print graphs\n"
				"5) prepare l3 again\n"
				"6) probe loop chosen set\n"
				"7) set slot time\n"
				"8) probe specific set and histogram\n"
				"9) score square and multiply\n"
				"10) exit\n", slot, setToProbe);
		scanf("%d", &choice);
		if(choice < 0 || choice > 10){
			printf("choose legit number\n\n");
			continue;
		}
		switch (choice){
		case 1:
			graphs = 0;
			printf("choose set\n");
			scanf("%d", &setToProbe);
			fseek(f, 0, SEEK_SET);
			fwrite(&setToProbe,sizeof(int),1,f);
			fclose(f);
			set_into_file2(setToProbe, slot) ;
			break;
		case 2:
			graphs = 0;
			set_into_file2(setToProbe, slot) ;
			break;
		case 3:
			graphs = 1;
			//printf("choose filter\n");
			//scanf("%f", &filter);
			num_of_sets = sets_into_file(nsets, slot);
			break;
		case 4:
			Activate_Python( graphs, num_of_sets);
			break;
		case 5:
			l3_release(l3);
			l3 = l3_prepare(NULL);
			break;
		case 6:
			for(i=0; i < 12 ;i++)
				set_into_file(setToProbe, slot) ;
			break;
		case 7:
			printf("enter slot time ( ticks)\n");
			scanf("%d", &slot);
			fseek(f, 4, SEEK_SET);
			fwrite(&slot,sizeof(int),1,f);
			fclose(f);
			break;
		case 8:
			graphs = 2;
			printf("choose set\n");
			scanf("%d", &setToProbe);
			printf("choose slice (0-3)\n");
			scanf("%d", &slice);
			fseek(f, 0, SEEK_SET);
			fwrite(&setToProbe,sizeof(int),1,f);
			fclose(f);
			num_of_sets = 5;
			set_into_file3(setToProbe + slice * SETS_IN_SLICE, slot) ;
			break;
		case 9:
			sets_into_file2(nsets, slot);
			break;
		case 10:
			fclose(f);
			l3_release(l3);
			exit(1);
		}
	}
}


void Activate_Python(int graphs, int num_of_sets){
	char command[90];

	sprintf(command,"python ./Python/setToGraph.py %d %d %d %d", graphs, SAMPLES,num_of_sets, NUM_OF_SLICES);
	system(command); //from probe all sets
}

void set_into_file(int set, int slot) {
	uint64_t start;
	char str[200];
	start = rdtscp64();
	for(int i=0;i<NUM_OF_SLICES;i++){
		uint16_t *res = calloc(SAMPLES, sizeof(uint16_t));
		l3_unmonitorall(l3);
		l3_monitor(l3, set+i*SETS_IN_SLICE);
		l3_repeatedprobecount(l3, SAMPLES, res, slot);
		sprintf(str,"datashit%d.txt", i);
		FILE *f = fopen(str, "wb");
		fwrite(res,sizeof(uint16_t),SAMPLES,f);
		fclose(f);
		free(res);

	}
	printf("%ld\n",start);
}


void set_into_file2(int set, int slot) {
	uint64_t start;
	char str[200];
	start = rdtscp64();
	for(int i=0;i<NUM_OF_SLICES;i++){
		uint16_t *res = calloc(SAMPLES, sizeof(uint16_t));
		uint64_t *Times_Array = calloc(SAMPLES, sizeof(uint64_t));

		l3_unmonitorall(l3);
		l3_monitor(l3, set+i*SETS_IN_SLICE);
		l3_repeatedprobecount_with_times(l3, SAMPLES, res,Times_Array, slot);
		sprintf(str,"datashit%d.txt", i);
		FILE *f = fopen(str, "wb");
		fwrite(res,sizeof(uint16_t),SAMPLES,f);
		fclose(f);
		sprintf(str,"datashitTimes%d.txt", i);
		FILE *f2 = fopen(str, "wb");
		fwrite(Times_Array,sizeof(uint64_t),SAMPLES,f2);
		fclose(f2);
		free(res);
		free(Times_Array);
	}
	printf("%ld\n",start);

}


void set_into_file3(int set, int slot) {
	uint64_t start;
	char str[200];
	printf("set = %d\n",set);
	uint16_t *res = calloc(SAMPLES, sizeof(uint16_t));

	uint64_t *Times_Array = calloc(SAMPLES, sizeof(uint64_t));

	l3_unmonitorall(l3);
	l3_monitor(l3, set);

	l3_repeatedprobecount_with_times(l3, SAMPLES, res,Times_Array, slot);
	int count=0;
	for(int i=0;i<SAMPLES;i++){
		count+= (res[i]>0);
	}
	printf("count/SAMPLES = %.2f\n", (float) count/(SAMPLES));
	sprintf(str,"speSet.txt");
	FILE *f = fopen(str, "wb");
	fwrite(res,sizeof(uint16_t),SAMPLES,f);
	fclose(f);
	sprintf(str,"speSetTimes.txt");
	FILE *f2 = fopen(str, "wb");
	fwrite(Times_Array,sizeof(uint64_t),SAMPLES,f2);
	fclose(f2);
	free(res);
	free(Times_Array);
}


int sets_into_file(int nsets,int slot) {
	int i,n,j,count=0,size=0,progress = 0;
	float low_filter = 0.05, high_filter = 0.3;
	uint16_t *res;
	uint16_t **res2 = calloc(0, sizeof(uint16_t**));
	int samp = 20000;
	FILE *f = fopen("datashit.txt", "wb");

	for( n=0; n<nsets ;n++)
	{
		if ( n%(nsets/100) == 0)
		{
			printf("%d% \n ",progress);
			progress++;
		}		
		l3_unmonitorall(l3);
		l3_monitor(l3, n);
		res = calloc(samp, sizeof(uint16_t));
		l3_repeatedprobecount(l3, samp, res, slot);

		count=0;
		for(j=0; j<samp; j++)
		{
			if(res[j]>13)
				res[j]=0;
			if(res[j]>0)
				count++;

		}
		if( (((float)count/samp) < high_filter) && (((float)count/samp) > low_filter) )
		{
			//printf("sdfgsdfgsdfgsdfgsd\n");
			size++;
			res2=(uint16_t**)realloc(res2,sizeof(uint16_t*)*size);
			res2[size-1]=res;
			printf("set = %d , count/samp = %.2f \n",n, (float)count/samp );
		}
		else
			free(res);

		l3_unmonitorall(l3);
	}
	//	for (int j = 0; j < nsets; j++) {
	//	if(res2[j]>1500)
	//		printf("set #%d got %u misses \n",j, res2[j]);
	//}
	printf("size is %d\n",size);
	for(i=0;i<size;i++)
		fwrite(res2[i],sizeof(uint16_t),samp,f);
	fclose(f);
	for(i=0;i<size;i++)
		free(res2[i]);
	free(res2);
	return size;
}

int sets_into_file2(int nsets,int slot) {
	int i,n,j,count=0,size=0,progress = 0;
	float low_filter = 0.05, high_filter = 0.3;
	uint16_t *res;
	char str[200];
	int samp = 20000;
	FILE *f = fopen("mean.txt", "wb");
	fprintf(f, "%d", 0);
	fclose(f);
	for( n=0; n<nsets ;n++)
	{
		if ( n%(nsets/100) == 0)
		{
			printf("%d% \n",progress);
			progress++;
		}
		l3_unmonitorall(l3);
		l3_monitor(l3, n);
		res = calloc(samp, sizeof(uint16_t));
		l3_repeatedprobecount(l3, samp, res, slot);
		l3_unmonitorall(l3);

		count=0;
		for(j=0; j<samp; j++)
		{
			if(res[j]>13)
				res[j]=0;
			if(res[j]>0)
				count++;

		}
		if( (((float)count/samp) < high_filter) && (((float)count/samp) > low_filter) )
		{
			size++;
			free(res);
			set_into_file3(n, slot);
			Activate_Python(2, 4);


		}



	}
	int sum = 0;
	printf("size is %d\n",size);
	FILE *f3 = fopen("mean.txt", "r");
	fscanf (f3, "%d", &sum);
	printf("maen is %.2f\n",(float) sum/size);
	fclose(f3);
	return size;
}







void pinToCore(int coreId){
	int rs;
	cpu_set_t cpuset;
	CPU_ZERO(&cpuset);
	CPU_SET(coreId, &cpuset);
	rs = pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
	if (rs) {
		perror("pthread_setaffinity_np");
		exit(EXIT_FAILURE);
	}
}





