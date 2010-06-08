/***************************************************************************
          timeofday.c  -  Skeleton of a plugin to be copied
                             -------------------
    begin                : Fri May 21 2010
    copyright            : (C) 2010 by Markus Raab
    email                : elektra@markus-raab.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the BSD License (revised).                      *
 *                                                                         *
 ***************************************************************************/



/***************************************************************************
 *                                                                         *
 *   This is the skeleton of the methods you'll have to implement in order *
 *   to provide libelektra.so a valid plugin.                             *
 *   Simple fill the empty _timeofday functions with your code and you are   *
 *   ready to go.                                                          *
 *                                                                         *
 ***************************************************************************/


#include "timeofday.h"

#ifndef timersub
#define timersub(val1,val2,res) \
	if (1) \
	{ \
		(res)->tv_sec = (val1)->tv_sec - (val2)->tv_sec; \
		if (((res)->tv_usec = (val1)->tv_usec - (val2)->tv_usec) < 0) \
		{ \
			(res)->tv_sec --; \
			(res)->tv_usec += 1000000; \
		} \
	}
#endif

static char* timeofday(char *t, struct timeval *start)
{
	struct timeval now;
	struct timeval tv;

	gettimeofday(&now, 0);

	timersub (&now, start, &tv);

	for (int i=9; i>=4; --i)
	{
		t[i] = tv.tv_usec %10 + '0';
		tv.tv_usec /= 10;
	}
	for (int i=3; i>=0; --i)
	{
		t[i] = tv.tv_sec %10 + '0';
		tv.tv_sec /= 10;
	}
	t[10] = 0;
	return t;
}

int kdbOpen_timeofday(Plugin *handle)
{
	struct timeval *start = malloc(sizeof (struct timeval));
	char t[10];

	gettimeofday(start, 0);
	elektraPluginSetHandle(handle, start);

	fprintf(stderr, "open\t%s\n", timeofday(t, start));

	return 0; /* success */
}

int kdbClose_timeofday(Plugin *handle)
{
	char t[10];
	struct timeval *start = elektraPluginGetHandle(handle);

	fprintf(stderr, "close\t%s\n", timeofday(t, start));

	free(start);

	return 0; /* success */
}

ssize_t kdbGet_timeofday(Plugin *handle, KeySet *returned, const Key *parentKey)
{
	ssize_t nr_keys = 0;
	char t[10];
	struct timeval *start = elektraPluginGetHandle(handle);

	fprintf(stderr, "get\t%s\n", timeofday(t, start));

	if (elektraStrCmp(keyName(parentKey), "system/elektra/modules/timeofday") >= 0)
	{
		Key *cur;
		cur = keyNew ("system/elektra/modules/timeofday", KEY_END);
		keyClearSync (cur); nr_keys++; ksAppendKey(returned, cur);

		cur = keyNew ("system/elektra/modules/timeofday/hello", KEY_VALUE, "Hello World!", KEY_END);
		keyClearSync (cur); nr_keys++; ksAppendKey(returned, cur);
		
		fprintf(stderr, "fin\t%s\n", timeofday(t, start));
	}

	return nr_keys; /* success */
}

ssize_t kdbSet_timeofday(Plugin *handle, KeySet *returned, const Key *parentKey)
{
	ssize_t nr_keys = 0;
	char t[10];
	struct timeval *start = elektraPluginGetHandle(handle);

	fprintf(stderr, "set\t%s\n", timeofday(t, start));

	return nr_keys;
}

Plugin *ELEKTRA_PLUGIN_EXPORT(timeofday)
{
	return elektraPluginExport(BACKENDNAME,
		ELEKTRA_PLUGIN_OPEN,	&kdbOpen_timeofday,
		ELEKTRA_PLUGIN_CLOSE,	&kdbClose_timeofday,
		ELEKTRA_PLUGIN_GET,		&kdbGet_timeofday,
		ELEKTRA_PLUGIN_SET,		&kdbSet_timeofday,
		ELEKTRA_PLUGIN_VERSION,	BACKENDVERSION,
		ELEKTRA_PLUGIN_AUTHOR,	"Full Name <email@libelektra.org>",
		ELEKTRA_PLUGIN_LICENCE,	"BSD",
		ELEKTRA_PLUGIN_DESCRIPTION,	"Add description here",
		ELEKTRA_PLUGIN_NEEDS,	"",
		ELEKTRA_PLUGIN_PROVIDES,	"",
		ELEKTRA_PLUGIN_END);
}

