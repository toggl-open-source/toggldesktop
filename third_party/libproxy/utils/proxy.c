/*******************************************************************************
 * libproxy - A library for proxy configuration
 * Copyright (C) 2006 Nathaniel McCallum <nathaniel@natemccallum.com>
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
#define _BSD_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

/* Import libproxy API */
#include <proxy.h>

void *
malloc0(size_t s)
{
	void *tmp = malloc(s);
	if (!tmp) return NULL;
	memset(tmp, '\0', s);
	return tmp;
}

/**
 * Prints an array of proxies. Proxies are space separated.
 * @proxies an array containing the proxies returned by libproxy.
 */
void
print_proxies(char **proxies)
{
	int j;

	if (!proxies) {
		printf("\n");
		return;
	}

	for (j=0; proxies[j] ; j++)
	{
		printf("%s%s", proxies[j], proxies[j+1] ? " " : "\n");
		free(proxies[j]);
	}
	free(proxies);
}

int
main(int argc, char **argv)
{
	int i;
	char url[102400]; // Should be plently long for most URLs

	/* Create the proxy factory object */
	pxProxyFactory *pf = px_proxy_factory_new();
	if (!pf)
	{
		fprintf(stderr, "An unknown error occurred!\n");
		return 1;
	}
	/* User entered some arguments on startup. skip interactive */
	if (argc > 1)
	{
		for (i=1; i < argc ; i++)
		{
			/*
			 * Get an array of proxies to use. These should be used
			 * in the order returned. Only move on to the next proxy
			 * if the first one fails (etc).
			 */
			print_proxies(px_proxy_factory_get_proxies(pf, argv[i]));
		}
	}
	/* Interactive mode */
	else
	{
		/* For each URL we read on STDIN, get the proxies to use */
		for (url[0] = '\0' ; fgets(url, 102400, stdin) != NULL ; )
		{
			if (url[strlen(url)-1] == '\n') url[strlen(url)-1] = '\0';

			/*
			 * Get an array of proxies to use. These should be used
			 * in the order returned. Only move on to the next proxy
			 * if the first one fails (etc).
			 */
			print_proxies(px_proxy_factory_get_proxies(pf, url));
		}
	}
	/* Destroy the proxy factory object */
	px_proxy_factory_free(pf);
	return 0;
}
