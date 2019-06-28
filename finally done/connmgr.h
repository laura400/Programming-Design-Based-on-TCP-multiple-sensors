#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "config.h"
#include "lib/tcpsock.h"
#include <netinet/in.h> 
#include <arpa/inet.h>


void connmgr_listen(int port_number,sbuffer_t * buffer);

void connmgr_free();
