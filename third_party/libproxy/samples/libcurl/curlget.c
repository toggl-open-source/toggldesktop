/*******************************************************************************
 * libproxy - A library for proxy configuration
 * Copyright (C) 2006 Nathaniel McCallum <nathaniel@natemccallum.com>
 * Copyright (C) 2008 Dominique Leuenberger <dominique-libproxy@leuenberger.net>
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 ******************************************************************************/

#include "proxy.h"
#include "curl/curl.h"
#include <string.h>
#include <stdlib.h>

int main(int argc, char * argv[]) {
  pxProxyFactory *pf = NULL;
  CURL *curl         = NULL;
  CURLcode result    = 1;

  /* Check if we have a parameter passed, otherwise bail out... I need one */
  if (argc < 2)
  {
    printf("libcurl / libproxy example implementation\n");
    printf("=========================================\n");
    printf("The program has to be started with one parameter\n");
    printf("Defining the URL that should be downloaded\n\n");
    printf("Example: %s http://code.google.com/p/libproxy/\n", argv[0]);
    return -1;
  }

  /* Initializing curl... has to happen exactly once in the program */
  if (curl_global_init( CURL_GLOBAL_ALL )) return -4;

  /* Create pxProxyFactory object */
  if (!(pf = px_proxy_factory_new()))
  {
    printf("Unable to create pxProxyFactory object!\n");
    curl_global_cleanup();
    return -2;
  }

  /* Create curl handle */
  if (!(curl = curl_easy_init()))
  {
    printf("Unable to get libcurl handle!\n");
    px_proxy_factory_free(pf);
    curl_global_cleanup();
    return -3;
  }

  /* Get the array of proxies to try */
  char **proxies = px_proxy_factory_get_proxies(pf, argv[1]);

  /* Set the URL into the curl handle */
  curl_easy_setopt(curl, CURLOPT_URL, argv[1]);

  /* Try to fetch our url using each proxy */
  for (int i=0 ; proxies[i] ; i++)
  {
    if (result != 0)
    {
      /* Setup our proxy's config into curl */
      curl_easy_setopt(curl, CURLOPT_PROXY, proxies[i]);

      /* HTTP Proxy */
      if (!strncmp("http", proxies[i], 4))
        curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_HTTP);

      /* SOCKS Proxy, is this correct??? */
      /* What about SOCKS 4A, 5 and 5_HOSTNAME??? */
      else if (!strncmp("socks", proxies[i], 4))
        curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS5);

      /* Attempt to fetch the page */
      result = curl_easy_perform(curl);
    }

    /* Free the proxy */
    free(proxies[i]);

    /* If we reached the end of the proxies array and still did not
    succeed to conntect, let's inform the user that we failed. */
    if (proxies[i+1] == NULL && result != 0)
      printf ("The requested URL (%s) could not be retrieved using the current setup\n", argv[1]);

  }

  /* Free the (now empty) proxy array */
  free(proxies);

  /* Free the curl and libproxy handles */
  px_proxy_factory_free(pf);
  curl_easy_cleanup(curl);

  /* Cleanup the libcurl library */
  curl_global_cleanup();
  return 0;
  
}

