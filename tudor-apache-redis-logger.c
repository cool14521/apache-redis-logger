/*
	tudor-apache-redis-logger.c
	Copyright 2016 Patrick Tudor MIT License
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h> // getopt

#include <hiredis.h>

int debugflag = 0; // debug files
int syslogflag = 0; // syslog files

/*
 *
 * Read log information from Apache, insert into Redis
 *
 */
int
main(int argc, char *argv[])
{
    extern char *optarg;
	char *str1, *str2, *token, *subtoken;
	char *saveptr1, *saveptr2;
	int j;
	const char *hostname = "127.0.0.1";
	int port = 6379;
	struct timeval timeout = { 0, 250000 }; // 0.25 seconds

	int hflag = 0; // help
	int vflag = 0; // version
	int ch;

	while ((ch = getopt (argc, argv, "hvdls:p:")) != -1) {
		switch (ch) {
		case 'h':
			printf("Options: -h(elp) -v(ersion) -d(ebugoutput) sys(-l)og -s Server -p Port\n");
			exit(EXIT_SUCCESS);
			break;
		case 'v':
			printf("Version 0.0 (c) ptudor@ptudor.net https://github.com/ptudor\n");
			exit(EXIT_SUCCESS);
			break;
		case 'l':
			syslogflag = 1;
			break;
		case 's':
			hostname = optarg;
			//printf ("Server: \"%s\"\n", hostname);
			break;
		case 'p':
			port = atoi(optarg);
			//printf ("Port: \"%d\"\n", port);
			//exit(EXIT_SUCCESS);
			break;
		case 'd':
			debugflag = 1;
			break;
		default:
			//usage();
			abort();
		}
	}

	// 
	if (syslogflag) {
		openlog("tudor-apache-redis", LOG_PID | LOG_NDELAY, LOG_LOCAL1);
		syslog(LOG_NOTICE, "starting...");
	}

	// redis objects
	redisContext *rc;
	redisReply *reply;

	rc = redisConnectWithTimeout(hostname, port, timeout);
	if (rc == NULL || rc->err) {
		if (rc) {
			printf("Connection error: %s\n", rc->errstr);
			redisFree(rc);
		} else {
			printf("Connection error: can't allocate redis context\n");
		}
		exit(EXIT_FAILURE);
	}

	// what is in our array?
	// "127.0.0.1,www.ptudor.net,200"
	// "172.19.82.63,www.example.com,405"
	char *array[3];
	char buffer[255];
	while( fgets(buffer, 255, stdin) !=NULL ){
		// comma delim.
		buffer[strlen(buffer) - 1] = '\0';
		int i = 0;
		char *p = strtok_r(buffer, ",", &saveptr1);
		while (p != '\0') 
		{
			array[i++] = p;
			p = strtok_r(NULL, ",", &saveptr1);
		}

		int redisDb = 2;
		redisAppendCommand(rc,"SELECT %d", redisDb);
		redisAppendCommand(rc,"EXPIRE %s 86400", array[0]);
		redisAppendCommand(rc,"EXPIRE %s 86400", array[1]);
		redisAppendCommand(rc,"EXPIRE %s 86400", array[2]);
		redisAppendCommand(rc,"HINCRBY %s %s 1", array[0], array[1]); // node, vhost
		redisAppendCommand(rc,"HINCRBY %s %s 1", array[0], array[2]); // node, statuscode
		redisAppendCommand(rc,"HINCRBY %s %s 1", array[1], array[0]); // vhost, node
		redisAppendCommand(rc,"HINCRBY %s %s 1", array[1], array[2]); // vhost, statuscode
		redisAppendCommand(rc,"HINCRBY %s %s 1", array[2], array[1]); // statuscode, vhost

		// counter here, 8, is number of items in the AppendCommand (from zero)
		long long finalReply = 0;
		for(int m = 0; m < 8; m++) {
			int r = redisGetReply(rc, (void **) &reply );
			if ( r == REDIS_ERR ) { printf("Generic Redis Reply Error\n"); exit(-1); }
			finalReply = reply->integer;
		  	freeReplyObject(reply);
		}

		if ((syslogflag) && (debugflag)) {
         		syslog(LOG_DEBUG, "HINCRBY: %s,%s,%s %lld\n", array[0], array[1], array[2],  finalReply);
		}

		if (debugflag) {
			printf("HINCRBY %s %s 1\n", array[0], array[1]);
			printf("HINCRBY %s %s 1\n", array[0], array[2]);
			printf("HINCRBY %s %s 1\n", array[1], array[0]);
			printf("HINCRBY %s %s 1\n", array[1], array[2]);
			printf("HINCRBY %s %s 1\n", array[2], array[1]);
		}
	}
	closelog ();
	redisFree(rc);
	exit(EXIT_SUCCESS);
}
