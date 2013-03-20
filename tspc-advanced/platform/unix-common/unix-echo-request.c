/*
---------------------------------------------------------------------------
 $Id: unix-echo-request.c,v 1.2 2007/05/02 13:32:28 cnepveu Exp $
---------------------------------------------------------------------------
  Copyright (c) 2007 Hexago Inc. All rights reserved.

  LICENSE NOTICE: You may use and modify this source code only if you
  have executed a valid license agreement with Hexago Inc. granting
  you the right to do so, the said license agreement governing such
  use and modifications.   Copyright or other intellectual property
  notices are not to be removed from the source code.
---------------------------------------------------------------------------
*/

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>

#define _USES_SYS_SOCKET_H_
#define _USES_PTHREAD_H_

#include "platform.h"

#include "log.h"
#include "tsp_redirect.h"
#include "tsp_client.h"
#include "xml_tun.h"
#include "hex_strings.h"
#include "net_echo_request.h"


/* Thread routine to calculate the distance for a given broker */
void *tspGetBrokerDistance(void *threadarg) {
  tBrokerTimingThreadArg *arguments = NULL;
  tBrokerList *broker = NULL;
  tConf *conf = NULL;
  tRedirectStatus *status_ptr = NULL;
  tRedirectStatus status = TSP_REDIRECT_OK;
  unsigned int distance = 0;

  /* The thread needs a broker argument to work with */
  if (threadarg == NULL) {
    return ((void *)NULL);
  }

  /* Unwrap the arguments */
  arguments = (tBrokerTimingThreadArg *)threadarg;
  broker = (tBrokerList *)arguments->broker;
  conf = (tConf *)arguments->conf;

  /* Allocate memory for the thread status returned to pthread_join */
  if ((status_ptr = (tRedirectStatus *)malloc(sizeof(tRedirectStatus))) == NULL) {
    Display(LOG_LEVEL_3, ELError, "tspGetBrokerDistance", HEX_STR_RDR_DISTANCE_THREAD_MALLOC_FAIL, broker->address);
    broker->distance = ECHO_REQUEST_ERROR_ADJUST;
    return ((void *)NULL);
  }

  /* Perform the echo request, calculating the distance */
  status = tspDoEchoRequest(broker->address, broker->address_type, conf, &distance);

  /* Set the calculated distance in the broker list element */
  broker->distance = distance;

  /* Make the status available to pthread_join */
  *status_ptr = status;

  return (void *)status_ptr;
}

/* Fill the distance values for the brokers in a list */
tRedirectStatus tspGetBrokerDistances(tBrokerList *broker_list, int broker_count, tConf *conf) {
  int rc = 0;
  int t = 0;
  tRedirectStatus status = TSP_REDIRECT_OK;
  void *status_ptr = NULL;
  pthread_t *threads = NULL;
  pthread_attr_t attr;
  tBrokerTimingThreadArg *thread_arguments = NULL;
  tBrokerList *broker_list_index = NULL;

  /* Initialize thread array */
  if ((threads = (pthread_t *)malloc(broker_count * sizeof(pthread_t))) == NULL) {
    Display(LOG_LEVEL_3, ELError, "tspGetBrokerDistances", HEX_STR_RDR_CANT_MALLOC_THREAD_ARRAY);
    return TSP_REDIRECT_CANT_MALLOC_THREAD_ARRAY;
  }

  /* Initialize thread argument array */
  if ((thread_arguments = (tBrokerTimingThreadArg *)malloc(broker_count * sizeof(tBrokerTimingThreadArg))) == NULL) {
    free(threads);
    Display(LOG_LEVEL_3, ELError, "tspGetBrokerDistances", HEX_STR_RDR_CANT_MALLOC_THREAD_ARGS);
    return TSP_REDIRECT_CANT_MALLOC_THREAD_ARGS;
  }

  /* Initialize thread creation argument */
  if (pthread_attr_init(&attr) != 0) {
    free(threads);
    free(thread_arguments);
    Display(LOG_LEVEL_3, ELError, "tspGetBrokerDistances", HEX_STR_RDR_CANT_INIT_THREAD_ARG);
    return TSP_REDIRECT_CANT_INIT_THREAD_ARG;
  }

  /* The thread must be joinable */
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

  /* We start at the begining of the broker list */
  broker_list_index = broker_list;

  /* Loop through the broker list */
  for (t = 0; ((t < broker_count) && (broker_list_index != NULL)); t++) {
    Display(LOG_LEVEL_3, ELInfo, "tspGetBrokerDistances", HEX_STR_RDR_CREATING_DISTANCE_THREAD, broker_list_index->address);

    /* Set the thread arguments */
    thread_arguments[t].broker = broker_list_index;
    thread_arguments[t].conf = conf;

    /* Start the distance calculation thread for that broker in the list */
    rc = pthread_create(&threads[t], &attr, tspGetBrokerDistance, (void *)&thread_arguments[t]);

    /* If we can't create the thread, return an error */
    if (rc != 0) {
      free(threads);
      free(thread_arguments);
      Display(LOG_LEVEL_3, ELError, "tspGetBrokerDistances", HEX_STR_RDR_CANT_CREATE_DISTANCE_THREAD, broker_list_index->address);
      pthread_attr_destroy(&attr);
      return TSP_REDIRECT_CANT_CREATE_THREAD;
    }

    /* Move to the next broker in the list */
    broker_list_index = broker_list_index->next;
  }

  /* We don't need the thread creation argument anymore */
  pthread_attr_destroy(&attr);

  /* We start from the begining of the list again to join the threads */
  broker_list_index = broker_list;

  /* Loop through the broker list */
  for (t = 0; ((t < broker_count) && (broker_list_index != NULL)); t++) {
    Display(LOG_LEVEL_3, ELInfo, "tspGetBrokerDistances", HEX_STR_RDR_WAITING_FOR_THREAD, broker_list_index->address);

    status_ptr = NULL;

    /* Try to join the thread corresponding to the broker in the list */
    rc = pthread_join(threads[t], (void **)&status_ptr);

    /* If we can't join the thread, return an error */
    if (rc != 0) {
      Display(LOG_LEVEL_3, ELError, "tspGetBrokerDistances", HEX_STR_RDR_ERR_WAITING_FOR_THREAD, broker_list_index->address);
      free(threads);
      free(thread_arguments);
      if (status_ptr != NULL) {
        free(status_ptr);
      }
      return TSP_REDIRECT_CANT_WAIT_FOR_THREAD;
    }
    /* If we're able to join the thread, check the status and log accordingly */
    else {
      if (status_ptr != NULL) {
        status = *((tRedirectStatus *)status_ptr);

        /* The distance was calculated correctly */
        if (status == TSP_REDIRECT_OK) {
          Display(LOG_LEVEL_3, ELInfo, "tspGetBrokerDistances", HEX_STR_RDR_DISTANCE_CALCULATION_OK, broker_list_index->address, broker_list_index->distance);
        }
        /* Echo requests timed out */
        else if (status == TSP_REDIRECT_ECHO_REQUEST_TIMEOUT) {
          Display(LOG_LEVEL_3, ELInfo, "tspGetBrokerDistances", HEX_STR_RDR_DISTANCE_CALCULATION_TIMEOUT, broker_list_index->address);
        }
        /* There was an error somewhere */
        else {
          Display(LOG_LEVEL_3, ELError, "tspGetBrokerDistances", HEX_STR_RDR_DISTANCE_CALCULATION_ERR, broker_list_index->address);
        }

        free(status_ptr);
      }
      /* No thread status means there was an error */
      else {
        Display(LOG_LEVEL_3, ELError, "tspGetBrokerDistances", HEX_STR_RDR_DISTANCE_CALCULATION_ERR, broker_list_index->address);
      }
    }

    /* We move to the next broker in the list */
    broker_list_index = broker_list_index->next;

  }

  free(threads);
  free(thread_arguments);

  return TSP_REDIRECT_OK;
}

