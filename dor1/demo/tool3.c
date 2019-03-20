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

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <signal.h>
#include <string.h>
#include <util.h>
#include <l3.h>
#include <low.h>

#include <time.h>
#include <sched.h>
#include "vlist.h"

#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>

#ifdef MAP_HUGETLB
#define HUGEPAGES MAP_HUGETLB
#endif

//Parameters
#define NSETS 8192
#define NUM_OF_ACTIVE_SETS 300
#define LINES_PER_SET 12

//Sampling Parameter
#define SAMPLES_FOR_RANKING 5000
#define SAMPLES_FOR_ATTACKER 5000
#define ATTACKER_PHASES 20
#define RANKING_PHASES 10
#define INTERVAL_DEF 3333
#define SPECIFICSET_SAMPLES 1000000

#define INPUTSIZE 				40
#define RUMBLE_CORE_0 	0
#define RUMBLE_CORE_1 	1
#define SIGNAL_CORE 	2
#define ATTACKER_CORE 	3

#define SIGNAL_ACTIVE_INTERVAL_DEF 10000
#define SIGNAL_SLEEP_INTERVAL_DEF 20000

#define EXPERIMENT_RUNS 300
#define STRIDE_BUFFSIZE 300000
#define STRIDE_LINES 500

sig_atomic_t doNoise = 0, doSignal = 0;

l3pp_t l3;
char * buffer;
bool *ActiveSets = NULL;
int NumOfActiveSets = 0;
int nsets = 0;
int setsPerSlice = 0;
int probensity, probethresh, rumbleOpt;
int setForProbe;
int INTERVAL = INTERVAL_DEF;
int SIGNAL_ACTIVE_INTERVAL = SIGNAL_ACTIVE_INTERVAL_DEF;
int SIGNAL_SLEEP_INTERVAL = SIGNAL_SLEEP_INTERVAL_DEF;
int nslices = 0;
int automated = false;
char input[INPUTSIZE];
char *token;
uint16_t *specific_res;
char ** lines;
char test;

void pinToCore(int coreId) {
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

void *rumble(void* args) {

	int * prms = (int*) args;
	int coreNum = prms[0];
	int lineOffset = prms[1];
	int intensity = prms[2];
	pinToCore(coreNum);
	if (!NumOfActiveSets) {
		printf("Nothing to Rumble!\n");
		fflush(stdout);
		doNoise = 0;
		return (void*) EXIT_SUCCESS;
	}

	int k = 0;
	for (int i = 0; i < intensity; i++) {
		for (int j = 0; j < nsets; j++) {
			if (ActiveSets[j]) {
				lines[k] = l3_getline(l3, j, i + lineOffset);
				k++;
			}
		}
	}

	//	void * p = lines[0];
	//	for (int i = 0; i < NumOfActiveSets * (intensity); i++) {
	//		*(void**)p = lines[i];
	//		p = *(void**)p;
	//	}
	//	*(void**)p = lines[0];

	//	while (doNoise){
	//		if (!(p = *(void**)p)){
	//			printf("err!");
	//		}
	//	}
	//	for (int i = 0; i < NumOfActiveSets * (intensity); i++) {
	//		clflush(lines[i]);
	//	}
//	FILE * log = fopen("log.txt","w+");
	while (doNoise && intensity) {
		int i;
//		uint64_t t;
		int nlines = NumOfActiveSets * (intensity);
//		t = rdtscp64();
		for (i = 0; i <  nlines; i++){
//			clflush((uint64_t)lines[i]);
			__builtin_prefetch(lines[i]);
//			memaccess(lines[i]);
//			__builtin_prefetch((uint64_t)(lines[i]+64));

//			clflush((uint64_t)lines[i]+64);
//			printf("%d,",memaccesstime((uint64_t)lines[i]+64));
		}
//		t = rdtscp64()-t;
//		printf("%d,",t);
//		fflush(log);

	}
//	close(log);
	return (void*) EXIT_SUCCESS;
}

/*
 void* rumble(void* args) {

 pinToCore(RUMBLE_CORE_0);

 printf("intensity = %d, probensity = %d\n", intensity, probensity);
 if (!NumOfActiveSets) {
 printf("Nothing to Rumble!\n");
 fflush(stdout);
 doNoise = 0;
 return (void*) EXIT_SUCCESS;
 }
 if (ActiveSets[setForProbe]){
 printf("Error : setForProbe is in suspected list, press 7 and remove it\n");
 fflush(stdout);
 doNoise = 0;
 return (void*) EXIT_SUCCESS;
 }

 char ** lines = calloc(NumOfActiveSets*intensity, sizeof(char *));

 int k = 0;
 for (int i = 0; i < intensity; i++) {
 for (int j = 0; j < nsets; j++){
 if (ActiveSets[j]){
 lines[k] = l3_getline(l3, j, i);
 k++;
 }
 }
 }
 void * p = sethead(l3,setForProbe);
 int flip = 0;
 int ind = 0;
 if (rumbleOpt == 0){
 while (doNoise) {
 int i, dummy, r;
 flip = !flip;
 r = flip ? probecount(p) : bprobecount(p);

 for (i = 0; i < NumOfActiveSets * (intensity-r); i++) {
 if (1 == lines[i][0])
 dummy++;
 }
 }
 } else if (rumbleOpt == 1){
 while (doNoise) {
 int i, dummy, r, cr = 0;
 for (i = 0; i < probensity; i++){
 r = probecount(p);
 cr+= r;
 }
 if(cr<=probethresh){
 for (i = 0; i < NumOfActiveSets * intensity; i++) {
 if (1 == lines[i][0])
 dummy++;
 }
 }
 }
 } else if (rumbleOpt == 2){
 while (doNoise) {
 int i, dummy, r, cr = 0;
 for (i = 0; i < probensity; i++){
 flip = !flip;
 r = flip ? probecount(p) : bprobecount(p);
 cr+= r;
 }
 if(cr<=probethresh){
 for (i = 0; i < NumOfActiveSets * intensity; i++) {
 if (1 == lines[i][0])
 dummy++;
 }
 }
 }
 }
 free(lines);
 return (void*) EXIT_SUCCESS;
 }
 */

void* signalActiveLines(void* args) {
	int rs;
	//struct timespec sleepValue = {0};
	//sleepValue.tv_nsec = 5000000L;
	int i, j, x;

	pinToCore(SIGNAL_CORE);

	for (i = 0, j = 0; j < nsets; j++) {
		if (ActiveSets[j]) {
			lines[i] = l3_getline(l3, j, 0);
			i++;
		}
	}

	//int dummy;
	uint64_t t1, t2, startTime;
	while (doSignal) {
		//		for (j = 0; j < 1000; j++){

		//Active Time (Make Misses)
		startTime = rdtscp64();
		t1 = 0;
		while (t1 < SIGNAL_ACTIVE_INTERVAL) {
			for (i = 0; i < NumOfActiveSets; i++) {
				memaccess(lines[i]);
			}
			t1 = rdtscp64() - startTime;
		}

		//Sleep Time
		slotwait(startTime + SIGNAL_SLEEP_INTERVAL + SIGNAL_ACTIVE_INTERVAL);

		//			startTime = rdtscp64();
		//			t1 = (t1-SIGNAL_ACTIVE_INTERVAL);
		//			t2 = 0;
		//			x = t1/SIGNAL_SLEEP_INTERVAL;
		//			while(t2 < (int64_t)(SIGNAL_SLEEP_INTERVAL - t1) + x*SIGNAL_SLEEP_INTERVAL){
		//				t2 = rdtscp64() - startTime;
		//			}
		//		}
		//		nanosleep(&sleepValue,NULL);

	}
}
void countMisses(int * dest) {
	uint16_t *res = calloc(SAMPLES_FOR_RANKING, sizeof(uint16_t));

	for (int k = 0; k < RANKING_PHASES; k++) {
		for (int i = 0; i < nsets; i++) {
			l3_unmonitorall(l3);
			l3_monitor(l3, i);

			printf("\rMonitor phase %d [%.2f%]", k,
					(double) k / RANKING_PHASES * 100);
			fflush(stdout);

			l3_repeatedprobecount(l3, SAMPLES_FOR_RANKING / RANKING_PHASES, res,
					INTERVAL);

			for (int j = 0; j < SAMPLES_FOR_RANKING / RANKING_PHASES; j++) {
				int16_t r = (int16_t) res[j];
				dest[i] += (r > 0);
			}
		}
	}
	free(res);
}

void CacheMapping() {
	printf("Mapping the Cache!\n");
	fflush(stdout);

	l3 = l3_prepare(NULL);

	if (!l3)
		return EXIT_FAILURE;

	nsets = l3_getSets(l3);

	printf("Mapping Done! Found %d sets\n", nsets);
	fflush(stdout);

	if (nsets != NSETS)
		printf("mapping failure");

	if (!ActiveSets)
		ActiveSets = (bool*) calloc(nsets, sizeof(bool));
	else
		bzero(ActiveSets, nsets * sizeof(bool));

	nslices = l3_getSlices(l3);
	setsPerSlice = nsets / nslices;
}

float monitorSpecificSet(int setToMonitor, char * filename, bool interactive) {
	int zeroCounter;
	uint64_t AvrMiss;
	FILE * f_specificset;
	if (setToMonitor < 0 || setToMonitor >= nsets) {
		printf("invalid set number!\n");
		return 0;
	}
	//	printf("filename = %s\n", filename);
	CASE5: printf("Monitoring %d...", setToMonitor);
	fflush(stdout);
	f_specificset = fopen(filename, "w+");
	l3_unmonitorall(l3);
	l3_monitor(l3, setToMonitor);
	l3_repeatedprobecount(l3, SPECIFICSET_SAMPLES, specific_res, INTERVAL);
	zeroCounter = 0;
	AvrMiss = 0;
	for (int j = 0; j < SPECIFICSET_SAMPLES; j++) {
		int16_t r = (int16_t) specific_res[j];
		if (r < 13 && r > 0)
			AvrMiss += (uint16_t) r;
		//			printf("%d - %ld, ", (uint16_t) r, AvrMiss);
		if (!r)
			zeroCounter++;
		fprintf(f_specificset, "%d ", r);
	}
	fclose(f_specificset);
	printf("\tdone \t avrMiss %.2f\t (%.2f %)!\n", setToMonitor,
			(float) AvrMiss / SPECIFICSET_SAMPLES,
			(float) (zeroCounter) / SPECIFICSET_SAMPLES * 100);
	if (interactive && !automated) {

		printf(
				"RawSet = %d, %d, %d, %d (a - again, n - next, enter - continue)\n",
				setToMonitor % setsPerSlice,
				setToMonitor % setsPerSlice + setsPerSlice,
				setToMonitor % setsPerSlice + 2 * setsPerSlice,
				setToMonitor % setsPerSlice + 3 * setsPerSlice);
		if (fgets(input, INPUTSIZE, stdin) == NULL) {
			perror("input");
		}
		if (input[0] == 'a') {
			goto CASE5;
		} else if (input[0] == 'n') {
			setToMonitor = (setToMonitor + setsPerSlice) % nsets;
			goto CASE5;
		}

	}
	return (float) AvrMiss / SPECIFICSET_SAMPLES;
}

int getInput(char * buffer) {
	printf("> ");
	fflush(stdout);
	int result;
	if (automated) {
		token = strtok(buffer, " ");
		result = atoi(token);
		printf("%d\n", result);
		fflush(stdout);

	} else {
		if (fgets(input, INPUTSIZE, stdin) != NULL) {
			result = atoi(input);
		} else
			perror("input");
	}
	return result;
}

void checkLRU(){
	void * p = &test;
	void* es[4][12];
	uint64_t t [4];
	void** chosenEs;
	int max = 0;
	int set = -1;
	int result;
	int misses = 0;
	int rawset = ((uint64_t)p >> 6) & 0x7ff;

	printf("Address = %lx, rawset = %d\n",p,rawset);

	for (int i = 0; i < 4; i ++){
		for (int j = 0; j < LINES_PER_SET; j++){
			es[i][j] = l3_getline(l3,rawset+i*2048,j);
		}
	}

	while (misses != 1){
		misses = 0;
		for (int i = 0; i < 4; i ++){
			memaccess(p);
			for (int j = 0; j < LINES_PER_SET; j++)			memaccess(es[i][j]);

			t[i] = memaccesstime(p);

			misses += (t[i] > 140);
			sleep(0.1);
		}
		printf("times = %d %d %d %d\n",t[0],t[1],t[2],t[3]);
	}

	for (int i = 0; i<4; i++){
		if (t[i] > max){
			max = t[i];
			set = rawset + i*2048;
			chosenEs = es[i];
		}
	}

	printf("line belongs to set %d(%d), press any key to continue\n",set,(set - rawset)/2048);
	getInput(NULL);

	for (int k = 0; k < EXPERIMENT_RUNS; k++){
		for (int i = 0; i < LINES_PER_SET; i++) clflush(chosenEs[i]);
		clflush(p);

		for (int i = 0; i < LINES_PER_SET; i++) memaccess(chosenEs[i]);
		memaccess(p);
		for (int i = 0; i < LINES_PER_SET;i++){
			result = memaccesstime(chosenEs[11-i]);
			printf("%d\n",result);
		}

		printf("******************\n");
	}

}

int main(int ac, char **av) {
	delayloop(3000000000U);
	pinToCore(ATTACKER_CORE);
	pthread_t rumble_tid_1, rumble_tid_2, signal_tid;
	char * buff = NULL;
	int rs;
	int zeroCounter;
	int tmp;
	struct stat st = { 0 };
	char str[INPUTSIZE];
	int intensity;
	uint64_t t;
	int option, setToMonitor = 0;
	int rumbleArgs[3] = { 0 };
	//files

	FILE *log, *f_allsetstotalmisses, *f_allsets, *f_specificset,
			*f_EvictionSets, *f_overkill, *f_suspectedsets, *f_twosets;
	f_allsets = fopen("allsets_res.txt", "w+");
	f_allsetstotalmisses = fopen("allsets_total_misses.txt", "w+");
	f_specificset = fopen("specificset_res.txt", "w+");
	fclose(f_specificset);
	fclose(f_allsetstotalmisses);
	fclose(f_allsets);

	int setIdx = 0;
	int result, *result1, *result2;
	void** lines_array;
	void** linesToRead;
	int sliceNumA = 0, sliceNumB = 0;
	int ActivityCounter = 0;
	int *res_rank1 = NULL, *res_rank2 = NULL;
	uint16_t *res = calloc(SAMPLES_FOR_ATTACKER, sizeof(uint16_t));
	specific_res = calloc(SPECIFICSET_SAMPLES, sizeof(uint16_t));
	uint16_t *twosets_res = calloc(SPECIFICSET_SAMPLES * 2, sizeof(uint16_t));
	lines = calloc(NSETS * LINES_PER_SET, sizeof(char *));
	uint setToMonitor_1, setToMonitor_2;
	size_t len = 0;
	uint8_t *probeDirection = calloc(NSETS, sizeof(uint8_t));
	pid_t pid;
	int rumbleCores = 1;
	void * lineAddr1, *lineAddr2;
	int distance;
	/* get the process id */
	if ((pid = getpid()) < 0)
		perror("unable to get pid");
	else
		printf("The process id is %d\n", pid);
	void * p;

	//socket
	int clientSocket;
	char buffer[1024];
	struct sockaddr_in serverAddr;
	socklen_t addr_size;
	int port;
	int settmp;

	res_rank1 = NULL;
	res_rank2 = NULL;
	ActiveSets = NULL;

	while (true) {
		printf("--------------------------\n");
		printf("NumOfActiveSets: %d, Sample Interval: %d,%d,%d, Mode:%s\n",
				NumOfActiveSets, INTERVAL, SIGNAL_ACTIVE_INTERVAL,
				SIGNAL_SLEEP_INTERVAL, automated ? "Automate" : "Manual");
		printf("Please select an option \n");
		printf("1) Map the Cache\n");
		//		printf("2) Ranking I\n");
		//		printf("3) Ranking II\n");
		//		printf("4) Find suspects as attacker\n");
		printf("5) Monitor specific set as attacker\n");
		printf("6) Noise I\n");
		printf("7) Turn off Noise I\n");
		printf("8) Save EvictionSet Mapping to file\n");
		printf("9) Dummy Ranking\n");
		printf("10) Monitor Overkill\n");
		printf("11) Add set to suspect list from file\n");
		printf("12) Add set to suspect list\n");
		printf("13) Remove set from suspect list\n");
		printf("14) Monitor Overkill - on suspect only\n");
		printf("15) Monitor parallely two sets \n");
		printf("16) Print active sets list\n");
		printf("17) Swap Between two slices\n");
		//		printf("18) Change sample interval\n");
		//		printf("19) Probe set\n");
		//		printf("20) Experiment\n");
		printf("21) Signal Active Lines\n");
		printf("22) Cancel Signal Active Lines\n");
		printf("23) Connect & Turn on automatic mode\n");
		printf("24) Turn on automated mode\n");
		printf("25) Turn off automated mode\n");
		//		printf("26) Experiment - Stride\n");
		printf("--------------------------\n");
		fflush(stdout);

		if (automated) {
			bzero(buffer, sizeof(char) * 1024);
			printf("> ");
			fflush(stdout);
			rs = recv(clientSocket, buffer, 1024, 0);
			if (rs <= 0) {
				perror("recv");
				automated = false;
				printf("changed to manual mode\n");
			} else
				printf("Data received: %s\n", buffer);
		}
		option = getInput(buffer);

		switch (option) {
		case 1:
			CacheMapping();
			break;
		case 2:
			if (!res_rank1)
				res_rank1 = (int*) calloc(nsets, sizeof(int));
			else
				bzero(res_rank1, nsets * sizeof(int));
			printf("Ranking I!\n");
			fflush(stdout);

			countMisses(res_rank1);

			putchar('\n');
			printf("Ranking I Done! Please Start the Victim for Ranking II\n");
			fflush(stdout);
			break;
		case 3:
			if (!res_rank2)
				res_rank2 = (int*) calloc(nsets, sizeof(int));
			else
				bzero(res_rank2, nsets * sizeof(int));
			printf("Ranking II!\n");
			fflush(stdout);
			bzero(ActiveSets, nsets * sizeof(uint8_t));
			NumOfActiveSets = 0;
			f_allsetstotalmisses = fopen("allsets_total_misses.txt", "w+");

			countMisses(res_rank2);

			putchar('\n');
			for (int i = 0; i < nsets; i++) {
				int diff = res_rank2[i] - res_rank1[i];
				fprintf(f_allsetstotalmisses, "%d,%d,%d,%d\n", i, res_rank1[i],
						res_rank2[i], diff);
			}

			fclose(f_allsetstotalmisses);
			printf(
					"\nRanking II done! Results file (allsets_total_misses.txt) updated\n");
			fflush(stdout);

			break;
		case 4:
			printf("Find suspects as attacker!\n");
			fflush(stdout);
			f_allsets = fopen("allsets_res.txt", "w+");
			for (int k = 0; k < ATTACKER_PHASES; k++) {
				for (int i = 0; i < nsets; i++) {
					l3_unmonitorall(l3);
					l3_monitor(l3, i);

					printf("\rMonitor phase %d [%.2f%]", k,
							(double) k / ATTACKER_PHASES * 100);
					fflush(stdout);

					l3_repeatedprobecount(l3,
					SAMPLES_FOR_ATTACKER / ATTACKER_PHASES, res, INTERVAL);

					for (int j = 0; j < SAMPLES_FOR_ATTACKER / ATTACKER_PHASES;
							j++) {
						char c;
						int16_t r = (int16_t) res[j];
						switch (r) {
						case 0:
							c = '0';
							break;
						case -1:
							c = '-';
							break;
						default:
							c = '1';
						}
						fprintf(f_allsets, "%c", c);
					}
					fprintf(f_allsets, "\n");
				}
			}
			fclose(f_allsets);
			printf("\n");
			printf("Find suspects as attacker done!\n");
			fflush(stdout);
			break;
		case 5:
			printf("Monitor specific set!\n");
			printf("Select set num: ");
			fflush(stdout);
			setToMonitor = getInput(NULL);
			if (automated) {
				token = strtok(NULL, " ");
				monitorSpecificSet(setToMonitor, token, false);
			} else {
				monitorSpecificSet(setToMonitor, "specificset_res.txt", true);
			}
			break;
		case 6:
			if (doNoise) {
				printf("doNoise = true, pause the noise and try again\n");
				break;
			}
			printf("Noise I!\n");

			/*
			 puts("Which kind of rumble?\n0 - BruteForce\n1 - Complement to intensity\n2 - probe i.o\n3 - probe r.o");
			 rumbleOpt = getInput(NULL);

			 if (!rumbleOpt){
			 puts("Enter set id for probe");
			 setForProbe = getInput(NULL);
			 rumbleCores = 1;
			 } else {
			 puts("How much cores? (1/2)");
			 rumbleCores = getInput(NULL);
			 }
			 if (rumbleOpt == 2 || rumbleOpt == 3){
			 puts("Enter probensity");
			 probensity = getInput(NULL);
			 puts("Enter probe threshold");
			 probethresh = getInput(NULL);
			 }
			 */
			doNoise = true;
			puts("Enter intensity #1");
			intensity = getInput(NULL);
			rumbleArgs[0] = RUMBLE_CORE_0;
			rumbleArgs[1] = 0;
			rumbleArgs[2] = intensity;
			if (pthread_create(&rumble_tid_1, NULL, &rumble, rumbleArgs)) {
				printf("pthread_create 1 failed\n");
				fflush(stdout);
				break;
			}
			printf("Noise #1 is on\n");

			puts("Enter intensity #2");
			intensity = getInput(NULL);
			rumbleArgs[0] = RUMBLE_CORE_1;
			rumbleArgs[1] = 6;
			rumbleArgs[2] = intensity;
				if (pthread_create(&rumble_tid_2, NULL, &rumble, rumbleArgs)) {
					printf("pthread_create 2 failed\n");
					fflush(stdout);
					break;
				}
				printf("Noise #2 is on\n");
			fflush(stdout);
			break;
		case 7:
			printf("Turn off Noise!\n");
			fflush(stdout);
			doNoise = false;
			if (pthread_join(rumble_tid_1, NULL)) {
				printf("pthread_join 1 failed\n");
				fflush(stdout);
				break;
			}
			printf("Noise1 is off\n");

			if (pthread_join(rumble_tid_2, NULL)) {
				printf("pthread_join 2 failed\n");
				fflush(stdout);
				break;
			}
			printf("Noise2 is off\n");
			fflush(stdout);
			break;
		case 8:
			f_EvictionSets = fopen("EvictionSets.txt", "w+");
			uint64_t line;
			for (int i = 0; i < nsets; i++) {
				fprintf(f_EvictionSets, "Set %d\n", i);
				for (int j = 0; j < 12; j++) {
					line = l3_getline(l3, i, j);
					fprintf(f_EvictionSets, "%lx (%d)\n", line,
							(int) ((line >> 6) & 0x7ff));
				}
			}
			fclose(f_EvictionSets);
			break;
		case 9:
			NumOfActiveSets = 0;
			bzero(ActiveSets, nsets * sizeof(uint8_t));
			for (int i = 0; i < NUM_OF_ACTIVE_SETS; i++) {
				srand(rdtscp());
				tmp = rand() % nsets;
				if (ActiveSets[tmp]) {
					i--;
					continue;
				}
				ActiveSets[tmp] = true;
				NumOfActiveSets++;
			}
			printf("NumOfActiveSets = %d\n", NumOfActiveSets);
			break;
		case 10:
			sprintf(str, "log%d.txt", ActivityCounter++);
			log = fopen(str, "w+");
			if (stat("./overkill_all", &st) == -1) {
				mkdir("./overkill_all", 0700);
			}

			for (int set = 0; set < nsets; set++) {
				sprintf(str, "./overkill_all/set_%d.txt", set);
				fprintf(log, "%d, %.2f\n", set,
						monitorSpecificSet(set, str, false));
				fflush(log);
			}
			printf("\n");
			fclose(log);
			break;
		case 11:
			setIdx = 0;
			printf("Add sets to suspect list from file: \n");
			f_suspectedsets = fopen("suspected_sets_num.txt", "r+");
			bzero(input, INPUTSIZE);
			while ((fscanf(f_suspectedsets, "%d", &setIdx))) {
				if (setIdx == -1)
					break;
				ActiveSets[setIdx] = 1;
				NumOfActiveSets++;
			}
			fclose(f_suspectedsets);
			break;
		case 12:
			setIdx = 0;
			printf("Add set to suspect list:\n");
			fflush(stdout);
			setIdx = getInput(NULL);

			if (setIdx < 0 || setIdx >= nsets) {
				printf("invalid set number!\n");
				break;
			}
			if (ActiveSets[setIdx]) {
				printf("Already a suspect!\n");
				break;
			} else {
				ActiveSets[setIdx] = 1;
				NumOfActiveSets++;
			}
			break;

		case 13:
			setIdx = 0;
			printf("Remove set to suspect list: ");
			fflush(stdout);
			setIdx = getInput(NULL);
			if (setIdx == -1) {
				NumOfActiveSets = 0;
				bzero(ActiveSets, nsets * sizeof(uint8_t));
				printf("Removed all!\n");
				break;
			}
			if (setIdx < 0 || setIdx >= nsets) {
				printf("invalid set number!\n");
				break;
			}
			if (!ActiveSets[setIdx]) {
				printf("Not a suspect!\n");
				break;
			} else {
				ActiveSets[setIdx] = 0;
				NumOfActiveSets--;
			}
			break;
		case 14:
			if (stat("./overkill_selected", &st) == -1) {
				mkdir("./overkill_selected", 0700);
			}
			for (int set = 0; set < nsets; set++) {
				if (ActiveSets[set]) {
					sprintf(str, "./overkill_selected/set_%d.txt", set);
					monitorSpecificSet(set, str, false);
				}
			}
			printf("\n");
			break;
		case 15:
			printf("Monitor two sets!\n");
			printf("Select set 1 num: ");
			fflush(stdout);
			setToMonitor_1 = getInput(NULL);

			if (setToMonitor_1 < 0 || setToMonitor_1 >= nsets) {
				printf("invalid set number!\n");
				break;
			}

			printf("Select set 2 num: ");
			fflush(stdout);
			setToMonitor_2 = getInput(NULL);
			if (setToMonitor_2 < 0 || setToMonitor_2 >= nsets) {
				printf("invalid set number!\n");
				break;
			}

			if (automated) {
				token = strtok(NULL, " ");
				f_twosets = fopen(token, "w+");
			} else {
				f_twosets = fopen("twosets_res.txt", "w+");
			}

			l3_unmonitorall(l3);
			l3_monitor(l3, setToMonitor_1);
			l3_monitor(l3, setToMonitor_2);
			l3_repeatedprobecount(l3, SPECIFICSET_SAMPLES, twosets_res,
					INTERVAL);
			for (int j = 0; j < SPECIFICSET_SAMPLES * 2; j++) {
				int16_t r = (int16_t) twosets_res[j];
				fprintf(f_twosets, "%d ", r);
			}
			fclose(f_twosets);
			l3_unmonitorall(l3);
			break;
		case 16:
			printf("Suspected Sets:\n");
			f_suspectedsets = fopen("suspected_sets_num.txt", "w+");
			for (int i = 0; i < nsets; i++) {
				if (!(i % 2048))
					printf("\n%d-%d:\n", i, i + 2047);
				if (ActiveSets[i]) {
					fprintf(f_suspectedsets, "%d\n", i);
					printf("%d, ", i);
				}

			}
			printf("\n");
			fprintf(f_suspectedsets, "-1");
			fclose(f_suspectedsets);

			fflush(stdout);
			break;
		case 17:
			sliceNumA = 0;
			sliceNumB = 0;
			printf("Swap Slices!\n");
			printf("Select slice A num (0-%d): ", nslices - 1);
			fflush(stdout);
			sliceNumA = getInput(NULL);
			if (sliceNumA < 0 || sliceNumA >= nslices) {
				printf("invalid slice number!\n");
				break;
			}

			printf("Select slice B num (0-%d): ", nslices - 1);
			fflush(stdout);
			sliceNumB = getInput(NULL);

			if (sliceNumB < 0 || sliceNumB >= nslices) {
				printf("invalid slice number!\n");
				break;
			}

			l3_swapslices(l3, sliceNumA, sliceNumB);
			break;
		case 18:
			printf("Enter sample interval: \n");
			fflush(stdout);
			tmp = getInput(NULL);
			if (tmp != -1)
				INTERVAL = tmp;

			printf("Enter active signal interval: \n");
			fflush(stdout);
			tmp = getInput(NULL);
			if (tmp != -1)
				SIGNAL_ACTIVE_INTERVAL = tmp;

			printf("Enter sleep signal interval: \n");
			fflush(stdout);
			tmp = getInput(NULL);
			if (tmp != -1)
				SIGNAL_SLEEP_INTERVAL = tmp;

			if (INTERVAL < 0) {
				printf("invalid number!\n");
				break;
			}
			break;

		case 19:
			setIdx = 0;
			printf("Probe set!\n");
			printf("Select set num: ");
			fflush(stdout);
			setIdx = getInput(NULL);

			if (setIdx < 0 || setIdx >= nsets) {
				printf("invalid set number!\n");
				break;
			}
			p = sethead(l3, setIdx);

			printf("Set:\n", setIdx);
			for (int j = 0; j < 12; j++) {
				line = l3_getline(l3, setIdx, j);
				clflush(line);
				printf("%lx - %li (%d)\n", line, line,
						(int) (((uint64_t) line >> 6) & 0x7ff));
			}

			result = probeDirection[setIdx] ? probecount(p) : bprobecount(p);
			probeDirection[setIdx] = !probeDirection[setIdx];
			printf("first probe = %d Misses\n", result);
			result = probeDirection[setIdx] ? probecount(p) : bprobecount(p);
			probeDirection[setIdx] = !probeDirection[setIdx];
			printf("second probe = %d Misses\n", result);
			break;
		case 20:
			lineAddr1 = 0;
			lineAddr2 = 0;
			printf("Probe Line!\n");
			printf("Type Set Num: ");
			setIdx = getInput(NULL);

			result1 = (int*) calloc(21, sizeof(int));
			result2 = (int*) calloc(21, sizeof(int));
			lineAddr1 = l3_getline(l3, setIdx, rand() % 12);
			for (distance = -10; distance <= 10; distance++) {
				lineAddr2 = (void*) ((uint64_t) ((uint64_t) lineAddr1
						+ distance * 64));
				setToMonitor_1 = (((uint64_t) lineAddr1)) >> 6 & 0x7ff;
				setToMonitor_2 = (((uint64_t) lineAddr2)) >> 6 & 0x7ff;

				for (int i = 0; i < EXPERIMENT_RUNS; i++) {
					clflush(lineAddr2);
					clflush(lineAddr1);
					result = memaccesstime(lineAddr1);
					result1[distance + 10] += result;
					result = memaccesstime(lineAddr2);
					result2[distance + 10] += result;

				}
			}
			for (distance = -10; distance <= 10; distance++) {
				result1[distance + 10] = result1[distance + 10]
						/ (EXPERIMENT_RUNS);
				result2[distance + 10] = result2[distance + 10]
						/ (EXPERIMENT_RUNS);
				printf("distance = %d, result1 = %d, result2 = %d\n", distance,
						result1[distance + 10], result2[distance + 10]);
			}
			free(result1);
			free(result2);
			break;

		case 21:
			if (doSignal) {
				printf("doSignal = true, pause the signal and try again\n");
				break;
			}
			printf("Signal!\n");
			doSignal = true;
			if (pthread_create(&signal_tid, NULL, &signalActiveLines, NULL)) {
				printf("pthread_create failed\n");
				fflush(stdout);
				break;
			}
			printf("Signal is on\n");
			fflush(stdout);

			break;
		case 22:
			printf("Turn off Signal!\n");
			fflush(stdout);
			doSignal = false;
			if (pthread_join(signal_tid, NULL)) {
				printf("pthread_join failed\n");
				fflush(stdout);
				break;
			}
			printf("Signal is off\n");
			fflush(stdout);
			break;
		case 23:

			/*---- Create the socket. The three arguments are: ----*/
			/* 1) Internet domain 2) Stream socket 3) Default protocol (TCP in this case) */
			clientSocket = socket(PF_INET, SOCK_STREAM, 0);
			/*---- Configure settings of the server address struct ----*/
			/* Address family = Internet */
			serverAddr.sin_family = AF_INET;
			/* Set port number, using htons function to use proper byte order */
			printf("Port: ");
			fflush(stdout);
			port = getInput(NULL);
			printf("port = %d\n", port);
			serverAddr.sin_port = htons(port);
			/* Set IP address to localhost */
			serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
			/* Set all bits of the padding field to 0 */
			memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);
			/*---- Connect the socket to the server using the address struct ----*/
			addr_size = sizeof serverAddr;
			for (int i = 0; i < 10 && !automated; i++) {
				rs = connect(clientSocket, (struct sockaddr *) &serverAddr,
						addr_size);
				if (rs < 0) {
					perror("connect");
					sleep(1);
					continue;
				}
				automated = true;
				printf("Connected! \n");
			}
			break;
		case 24:
			automated = true;
			break;
		case 25:
			automated = false;
			break;
		case 26:
			printf("random? 1/0");
			tmp = getInput(NULL);
			buff = mmap(NULL, STRIDE_BUFFSIZE, PROT_READ | PROT_WRITE,
			MAP_ANONYMOUS | MAP_PRIVATE | HUGEPAGES, -1, 0);
			result1 = (int*) calloc(STRIDE_LINES, sizeof(int));
			lines_array = (void**) calloc(STRIDE_LINES, sizeof(void*));
			linesToRead = (void**) calloc(20, sizeof(void*));
			for (int j = 0; j < EXPERIMENT_RUNS; j++) {
				lineAddr1 = buff;
				for (int i = 0; i < STRIDE_BUFFSIZE; i++) {
					clflush(&(buff[i]));
				}
				if (tmp == 1) {
					for (int i = 0; i < STRIDE_LINES; i++) {
						srand(rdtscp());
						for (int k = 0; k < 20; k++) {
							linesToRead[k] = buff
									+ (rand() % rdtscp64()) % (STRIDE_BUFFSIZE);
						}

						t = rdtscp64();
						for (int k = 0; k < 20; k++) {
							memaccess(linesToRead[k]);
						}
						result = rdtscp64() - t;
						result1[i] = result / 20;
					}
				} else {
					for (int i = 0; i < STRIDE_LINES; i++) {
						lines_array[i] = lineAddr1;
						result = memaccesstime(lineAddr1);
						lineAddr1 = (void*) ((uint64_t) lineAddr1
								+ 10 * (8 * sizeof(void*)));
						result1[i] = result;
						if (i > STRIDE_LINES / 2) {
							lineAddr1 = (void*) ((uint64_t) lineAddr1
									+ 100 * (8 * sizeof(void*)));
						}
					}
				}
			}
			for (int i = 0; i < STRIDE_LINES; i++) {
				printf("%d, ", result1[i]);
			}
			//			printf("\n-------------\n");
			//			for (int i = 0; i < STRIDE_LINES; i ++){
			//				printf("%lx, ",lines[i]);
			//			}
			//			printf("\n-------------\n");
			//			for (int i = 1; i < STRIDE_LINES; i ++){
			//				printf("%d, ",(lines[i]-lines[i-1])/64);
			//			}
			printf("\n");
			fflush(stdout);
			free(result1);
			free(lines_array);
			munmap(buff, STRIDE_BUFFSIZE);
			break;
		case 27:
			log = fopen("log.txt", "w+");
			;
			buff = mmap(NULL, 50 * 1024 * 1024, PROT_READ | PROT_WRITE,
			MAP_ANONYMOUS | MAP_PRIVATE | HUGEPAGES, -1, 0);
			if (buff == MAP_FAILED) {
				printf("MAP_FAILED");
				break;
			}
			printf("memory allocated\n");

			result1 = (int*) calloc(STRIDE_LINES, sizeof(int));
			lines_array = (void**) calloc(STRIDE_LINES, sizeof(void*));
			linesToRead = (void**) calloc(20, sizeof(void*));

			buff = (uint64_t) buff | (1023 << 6);
			tmp = 1 << 17;
			while (1) {
				//				printf("%lx\n",buff + tmp);
				//				t = rdtscp64();
				memaccess(buff + tmp);
				tmp = (tmp + 1 << 17) & 0x1ffffff;
				memaccess(buff + tmp);
				tmp = (tmp + 1 << 17) & 0x1ffffff;
				memaccess(buff + tmp);
				tmp = (tmp + 1 << 17) & 0x1ffffff;
				memaccess(buff + tmp);
				tmp = (tmp + 1 << 17) & 0x1ffffff;
				memaccess(buff + tmp);
				tmp = (tmp + 1 << 17) & 0x1ffffff;
				memaccess(buff + tmp);
				tmp = (tmp + 1 << 17) & 0x1ffffff;
				memaccess(buff + tmp);
				tmp = (tmp + 1 << 17) & 0x1ffffff;
				memaccess(buff + tmp);
				tmp = (tmp + 1 << 17) & 0x1ffffff;
				memaccess(buff + tmp);
				tmp = (tmp + 1 << 17) & 0x1ffffff;
				memaccess(buff + tmp);
				tmp = (tmp + 1 << 17) & 0x1ffffff;
				memaccess(buff + tmp);
				tmp = (tmp + 1 << 17) & 0x1ffffff;
				memaccess(buff + tmp);
				tmp = (tmp + 1 << 17) & 0x1ffffff;
				memaccess(buff + tmp);
				tmp = (tmp + 1 << 17) & 0x1ffffff;
				memaccess(buff + tmp);
				tmp = (tmp + 1 << 17) & 0x1ffffff;
				memaccess(buff + tmp);
				tmp = (tmp + 1 << 17) & 0x1ffffff;
				memaccess(buff + tmp);
				tmp = (tmp + 1 << 17) & 0x1ffffff;
				memaccess(buff + tmp);
				tmp = (tmp + 1 << 17) & 0x1ffffff;
				memaccess(buff + tmp);
				tmp = (tmp + 1 << 17) & 0x1ffffff;
				memaccess(buff + tmp);
				tmp = (tmp + 1 << 17) & 0x1ffffff;
				memaccess(buff + tmp);
				tmp = (tmp + 1 << 17) & 0x1ffffff;
				memaccess(buff + tmp);
				tmp = (tmp + 1 << 17) & 0x1ffffff;
				memaccess(buff + tmp);
				tmp = (tmp + 1 << 17) & 0x1ffffff;
				memaccess(buff + tmp);
				tmp = (tmp + 1 << 17) & 0x1ffffff;
				memaccess(buff + tmp);
				tmp = (tmp + 1 << 17) & 0x1ffffff;
				memaccess(buff + tmp);
				tmp = (tmp + 1 << 17) & 0x1ffffff;
				memaccess(buff + tmp);
				tmp = (tmp + 1 << 17) & 0x1ffffff;
				memaccess(buff + tmp);
				tmp = (tmp + 1 << 17) & 0x1ffffff;
				memaccess(buff + tmp);
				tmp = (tmp + 1 << 17) & 0x1ffffff;
				memaccess(buff + tmp);
				tmp = (tmp + 1 << 17) & 0x1ffffff;
				memaccess(buff + tmp);
				tmp = (tmp + 1 << 17) & 0x1ffffff;
				//				result = (rdtscp64() - t)/30;
				//				result = memaccesstime(buff+tmp); tmp = (tmp + 1 << 17) & 0x1ffffff;
				//				fprintf(log,"%d,",result);

			}

			for (int i = 0; i < STRIDE_LINES; i++) {
				fprintf(log, "%d,", result1[i]);
			}

			printf("\n");
			fflush(stdout);
			free(result1);
			free(linesToRead);
			free(lines_array);
			fclose(log);
			munmap(buff, STRIDE_BUFFSIZE);
			break;
		case 28:
			checkLRU();
			break;


		}
		if (automated && option != 23) {
			sprintf(buffer, "ack %d", option);
			rs = send(clientSocket, buffer, strlen(buffer), 0);
			if (rs <= 0) {
				perror("send");
				automated = false;
				printf("changed to manual mode\n");
			}
		}
	}

	CLOSE:

	fclose(f_allsets);
	fclose(f_specificset);
	free(res);
	free(specific_res);
	free(ActiveSets);
	free(lines);
	if (res_rank1)
		free(res_rank1);
	if (res_rank2)
		free(res_rank2);
	l3_release(l3);
}
