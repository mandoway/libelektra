/**
 * @file
 *
 * @brief
 *
 * @copyright BSD License (see LICENSE.md or https://www.libelektra.org)
 */

#include <../../src/libs/elektra/backend.c>
#include <../../src/libs/elektra/trie.c>
#include <tests_internal.h>


Trie * test_insert (Trie * trie, char * name, char * value)
{
	Backend * backend = elektraCalloc (sizeof (Backend));

	if (strlen (name) == 0)
	{
		backend->mountpoint = NULL;
	}
	else
	{
		backend->mountpoint = keyNew (name, KEY_VALUE, value, KEY_END);
		keyIncRef (backend->mountpoint);
	}
	backend->refcounter = 1;
	return trieInsert (trie, name, backend);
}


static void test_minimaltrie (void)
{
	printf ("Test minimal trie\n");

	Trie * trie = test_insert (0, "", "");

	succeed_if (trieLookup (trie, "/"), "trie should not be null");
	succeed_if (trieLookup (trie, "/")->mountpoint == NULL, "default backend should have NULL mountpoint");
	succeed_if (trieLookup (trie, "user:/")->mountpoint == NULL, "default backend should have NULL mountpoint");
	succeed_if (trieLookup (trie, "system:/")->mountpoint == NULL, "default backend should have NULL mountpoint");
	succeed_if (trieLookup (trie, "user:/below")->mountpoint == NULL, "default backend should have NULL mountpoint");
	succeed_if (trieLookup (trie, "system:/below")->mountpoint == NULL, "default backend should have NULL mountpoint");

	trieClose (trie, 0);
}

KeySet * simple_config (void)
{
	return ksNew (5, keyNew ("system:/elektra/mountpoints", KEY_END), keyNew ("system:/elektra/mountpoints/simple", KEY_END),
		      keyNew ("system:/elektra/mountpoints/simple/mountpoint", KEY_VALUE, "user:/tests/simple", KEY_END), KS_END);
}

static void test_simple (void)
{
	printf ("Test simple trie\n");

	Trie * trie = test_insert (0, "user:/tests/simple", "simple");

	exit_if_fail (trie, "trie was not build up successfully");

	Backend * backend = trieLookup (trie, "user:/");
	succeed_if (!backend, "there should be no backend");


	Key * mp = keyNew ("user:/tests/simple", KEY_VALUE, "simple", KEY_END);
	backend = trieLookup (trie, "user:/tests/simple");
	succeed_if (backend, "there should be a backend");
	if (backend) compare_key (backend->mountpoint, mp);

	Backend * b2 = trieLookup (trie, "user:/tests/simple/below");
	succeed_if (b2, "there should be a backend");
	succeed_if (backend == b2, "should be same backend");
	if (b2) compare_key (b2->mountpoint, mp);


	b2 = trieLookup (trie, "user:/tests/simple/deep/below");
	succeed_if (b2, "there should be a backend");
	succeed_if (backend == b2, "should be same backend");
	if (b2) compare_key (b2->mountpoint, mp);

	trieClose (trie, 0);
	keyDel (mp);
}

static void collect_mountpoints (Trie * trie, KeySet * mountpoints)
{
	int i;
	for (i = 0; i < KDB_MAX_UCHAR; ++i)
	{
		if (trie->value[i]) ksAppendKey (mountpoints, ((Backend *) trie->value[i])->mountpoint);
		if (trie->children[i]) collect_mountpoints (trie->children[i], mountpoints);
	}
	if (trie->empty_value)
	{
		ksAppendKey (mountpoints, ((Backend *) trie->empty_value)->mountpoint);
	}
}

static void test_iterate (void)
{
	printf ("Test iterate trie\n");

	Trie * trie = test_insert (0, "user:/tests/hosts", "hosts");
	trie = test_insert (trie, "user:/tests/hosts/below", "below");

	exit_if_fail (trie, "trie was not build up successfully");

	Backend * backend = trieLookup (trie, "user:/");
	succeed_if (!backend, "there should be no backend");


	Key * mp = keyNew ("user:/tests/hosts", KEY_VALUE, "hosts", KEY_END);
	backend = trieLookup (trie, "user:/tests/hosts");
	succeed_if (backend, "there should be a backend");
	if (backend) compare_key (backend->mountpoint, mp);
	// printf ("backend: %p\n", (void*)backend);


	Backend * b2 = trieLookup (trie, "user:/tests/hosts/other/below");
	succeed_if (b2, "there should be a backend");
	succeed_if (backend == b2, "should be same backend");
	if (b2) compare_key (b2->mountpoint, mp);
	// printf ("b2: %p\n", (void*)b2);


	b2 = trieLookup (trie, "user:/tests/hosts/other/deep/below");
	succeed_if (b2, "there should be a backend");
	succeed_if (backend == b2, "should be same backend");
	if (b2) compare_key (b2->mountpoint, mp);


	Key * mp2 = keyNew ("user:/tests/hosts/below", KEY_VALUE, "below", KEY_END);
	Backend * b3 = trieLookup (trie, "user:/tests/hosts/below");
	succeed_if (b3, "there should be a backend");
	succeed_if (backend != b3, "should be different backend");
	if (b3) compare_key (b3->mountpoint, mp2);
	backend = b3;
	// printf ("b3: %p\n", (void*)b3);


	b3 = trieLookup (trie, "user:/tests/hosts/below/other/deep/below");
	succeed_if (b3, "there should be a backend");
	succeed_if (backend == b3, "should be same backend");
	if (b3) compare_key (b3->mountpoint, mp2);

	KeySet * mps = ksNew (0, KS_END);
	collect_mountpoints (trie, mps);
	// output_keyset(mps);
	// output_trie(trie);
	succeed_if (ksGetSize (mps) == 2, "not both mountpoints collected");
	compare_key (ksAtCursor (mps, 0), mp);
	compare_key (ksAtCursor (mps, ksGetSize (mps) - 1), mp2);
	ksDel (mps);

	trieClose (trie, 0);

	keyDel (mp);
	keyDel (mp2);
}

static void test_reviterate (void)
{
	printf ("Test reviterate trie\n");

	Trie * trie = test_insert (0, "user:/tests/hosts/below", "below");
	trie = test_insert (trie, "user:/tests/hosts", "hosts");

	exit_if_fail (trie, "trie was not build up successfully");

	Backend * backend = trieLookup (trie, "user:/");
	succeed_if (!backend, "there should be no backend");


	Key * mp = keyNew ("user:/tests/hosts", KEY_VALUE, "hosts", KEY_END);
	backend = trieLookup (trie, "user:/tests/hosts");
	succeed_if (backend, "there should be a backend");
	if (backend) compare_key (backend->mountpoint, mp);
	// printf ("backend: %p\n", (void*)backend);


	Backend * b2 = trieLookup (trie, "user:/tests/hosts/other/below");
	succeed_if (b2, "there should be a backend");
	succeed_if (backend == b2, "should be same backend");
	if (b2) compare_key (b2->mountpoint, mp);
	// printf ("b2: %p\n", (void*)b2);


	b2 = trieLookup (trie, "user:/tests/hosts/other/deep/below");
	succeed_if (b2, "there should be a backend");
	succeed_if (backend == b2, "should be same backend");
	if (b2) compare_key (b2->mountpoint, mp);


	Key * mp2 = keyNew ("user:/tests/hosts/below", KEY_VALUE, "below", KEY_END);
	Backend * b3 = trieLookup (trie, "user:/tests/hosts/below");
	succeed_if (b3, "there should be a backend");
	succeed_if (backend != b3, "should be different backend");
	if (b3) compare_key (b3->mountpoint, mp2);
	backend = b3;
	// printf ("b3: %p\n", (void*)b3);


	b2 = trieLookup (trie, "user:/tests/hosts/below/other/deep/below");
	succeed_if (b2, "there should be a backend");
	succeed_if (backend == b2, "should be same backend");
	if (b2) compare_key (b2->mountpoint, mp2);

	KeySet * mps = ksNew (0, KS_END);
	collect_mountpoints (trie, mps);
	succeed_if (ksGetSize (mps) == 2, "not both mountpoints collected");
	compare_key (ksAtCursor (mps, 0), mp);
	compare_key (ksAtCursor (mps, ksGetSize (mps) - 1), mp2);
	ksDel (mps);

	trieClose (trie, 0);

	keyDel (mp);
	keyDel (mp2);
}

KeySet * moreiterate_config (void)
{
	return ksNew (50, keyNew ("system:/elektra/mountpoints", KEY_END), keyNew ("system:/elektra/mountpoints/user", KEY_END),
		      keyNew ("system:/elektra/mountpoints/user/mountpoint", KEY_VALUE, "user", KEY_END),
		      keyNew ("system:/elektra/mountpoints/tests", KEY_END),
		      keyNew ("system:/elektra/mountpoints/tests/mountpoint", KEY_VALUE, "user:/tests", KEY_END),
		      keyNew ("system:/elektra/mountpoints/hosts", KEY_END),
		      keyNew ("system:/elektra/mountpoints/hosts/mountpoint", KEY_VALUE, "user:/tests/hosts", KEY_END),
		      keyNew ("system:/elektra/mountpoints/below", KEY_END),
		      keyNew ("system:/elektra/mountpoints/below/mountpoint", KEY_VALUE, "user:/tests/hosts/below", KEY_END),
		      keyNew ("system:/elektra/mountpoints/system", KEY_END),
		      keyNew ("system:/elektra/mountpoints/system/mountpoint", KEY_VALUE, "system", KEY_END),
		      keyNew ("system:/elektra/mountpoints/systests", KEY_END),
		      keyNew ("system:/elektra/mountpoints/systests/mountpoint", KEY_VALUE, "system:/tests", KEY_END),
		      keyNew ("system:/elektra/mountpoints/syshosts", KEY_END),
		      keyNew ("system:/elektra/mountpoints/syshosts/mountpoint", KEY_VALUE, "system:/tests/hosts", KEY_END),
		      keyNew ("system:/elektra/mountpoints/sysbelow", KEY_END),
		      keyNew ("system:/elektra/mountpoints/sysbelow/mountpoint", KEY_VALUE, "system:/tests/hosts/below", KEY_END), KS_END);
}

KeySet * set_mountpoints (void)
{
	return ksNew (10, keyNew ("user:/", KEY_VALUE, "user", KEY_END), keyNew ("user:/tests", KEY_VALUE, "tests", KEY_END),
		      keyNew ("user:/tests/hosts", KEY_VALUE, "hosts", KEY_END),
		      keyNew ("user:/tests/hosts/below", KEY_VALUE, "below", KEY_END), keyNew ("system:/", KEY_VALUE, "system", KEY_END),
		      keyNew ("system:/tests", KEY_VALUE, "systests", KEY_END),
		      keyNew ("system:/tests/hosts", KEY_VALUE, "syshosts", KEY_END),
		      keyNew ("system:/tests/hosts/below", KEY_VALUE, "sysbelow", KEY_END), KS_END);
}

static void test_moreiterate (void)
{
	printf ("Test moreiterate trie\n");

	Trie * trie = test_insert (0, "user:/", "user");
	trie = test_insert (trie, "user:/tests", "tests");
	trie = test_insert (trie, "user:/tests/hosts", "hosts");
	trie = test_insert (trie, "user:/tests/hosts/below", "below");
	trie = test_insert (trie, "system:/", "system");
	trie = test_insert (trie, "system:/tests", "systests");
	trie = test_insert (trie, "system:/tests/hosts", "syshosts");
	trie = test_insert (trie, "system:/tests/hosts/below", "sysbelow");

	KeySet * mps = set_mountpoints ();

	exit_if_fail (trie, "trie was not build up successfully");

	Backend * backend = trieLookup (trie, "user:/");
	succeed_if (backend, "there should be a backend");
	compare_key (backend->mountpoint, ksLookupByName (mps, "user:/", 0));
	// printf ("backend: %p\n", (void*)backend);


	Backend * b2 = trieLookup (trie, "user:/tests/hosts/other/below");
	succeed_if (b2, "there should be a backend");
	compare_key (b2->mountpoint, ksLookupByName (mps, "user:/tests/hosts", 0));
	// printf ("b2: %p\n", (void*)b2);


	b2 = trieLookup (trie, "user:/tests/hosts/other/deep/below");
	succeed_if (b2, "there should be a backend");
	compare_key (b2->mountpoint, ksLookupByName (mps, "user:/tests/hosts", 0));


	Backend * b3 = trieLookup (trie, "user:/tests/hosts/below");
	succeed_if (b3, "there should be a backend");
	if (b3) compare_key (b3->mountpoint, ksLookupByName (mps, "user:/tests/hosts/below", 0));
	// printf ("b3: %p\n", (void*)b3);


	backend = trieLookup (trie, "user:/tests/hosts/below/other/deep/below");
	succeed_if (backend, "there should be a backend");
	if (backend) compare_key (backend->mountpoint, ksLookupByName (mps, "user:/tests/hosts/below", 0));

	backend = trieLookup (trie, "system:/");
	succeed_if (backend, "there should be a backend");
	if (backend) compare_key (backend->mountpoint, ksLookupByName (mps, "system:/", 0));
	// printf ("backend: %p\n", (void*)backend);


	b2 = trieLookup (trie, "system:/tests/hosts/other/below");
	succeed_if (b2, "there should be a backend");
	if (b2) compare_key (b2->mountpoint, ksLookupByName (mps, "system:/tests/hosts", 0));
	// printf ("b2: %p\n", (void*)b2);


	b2 = trieLookup (trie, "system:/tests/hosts/other/deep/below");
	succeed_if (b2, "there should be a backend");
	if (b2) compare_key (b2->mountpoint, ksLookupByName (mps, "system:/tests/hosts", 0));


	b3 = trieLookup (trie, "system:/tests/hosts/below");
	succeed_if (b3, "there should be a backend");
	if (b3) compare_key (b3->mountpoint, ksLookupByName (mps, "system:/tests/hosts/below", 0));
	// printf ("b3: %p\n", (void*)b3);


	b2 = trieLookup (trie, "system:/tests/hosts/below/other/deep/below");
	succeed_if (b2, "there should be a backend");
	if (b2) compare_key (b2->mountpoint, ksLookupByName (mps, "system:/tests/hosts/below", 0));

	KeySet * mps_cmp = ksNew (0, KS_END);
	collect_mountpoints (trie, mps_cmp);
	succeed_if (ksGetSize (mps_cmp) == 8, "size should be 8");
	compare_keyset (mps, mps_cmp);

	ksDel (mps_cmp);
	ksDel (mps);

	trieClose (trie, 0);
}

static void test_revmoreiterate (void)
{
	printf ("Test revmoreiterate trie\n");

	for (int i = 0; i < 5; ++i)
	{

		Trie * trie = 0;
		switch (i)
		{
		case 0:
			trie = test_insert (trie, "user:/tests", "tests");
			trie = test_insert (trie, "user:/tests/hosts", "hosts");
			trie = test_insert (trie, "user:/tests/hosts/below", "below");
			trie = test_insert (trie, "system:/tests", "systests");
			trie = test_insert (trie, "system:/tests/hosts", "syshosts");
			trie = test_insert (trie, "system:/tests/hosts/below", "sysbelow");
			trie = test_insert (trie, "system:/", "system");
			trie = test_insert (trie, "user:/", "user");
			break;
		case 1:
			trie = test_insert (trie, "system:/tests/hosts", "syshosts");
			trie = test_insert (trie, "system:/", "system");
			trie = test_insert (trie, "user:/tests", "tests");
			trie = test_insert (trie, "user:/tests/hosts", "hosts");
			trie = test_insert (trie, "user:/tests/hosts/below", "below");
			trie = test_insert (trie, "system:/tests", "systests");
			trie = test_insert (trie, "user:/", "user");
			trie = test_insert (trie, "system:/tests/hosts/below", "sysbelow");
			break;
		case 2:
			trie = test_insert (trie, "system:/tests/hosts/below", "sysbelow");
			trie = test_insert (trie, "system:/tests/hosts", "syshosts");
			trie = test_insert (trie, "user:/tests/hosts/below", "below");
			trie = test_insert (trie, "user:/tests/hosts", "hosts");
			trie = test_insert (trie, "user:/tests", "tests");
			trie = test_insert (trie, "user:/", "user");
			trie = test_insert (trie, "system:/tests", "systests");
			trie = test_insert (trie, "system:/", "system");
			break;
		case 3:
			trie = test_insert (trie, "user:/tests/hosts/below", "below");
			trie = test_insert (trie, "user:/tests/hosts", "hosts");
			trie = test_insert (trie, "user:/tests", "tests");
			trie = test_insert (trie, "user:/", "user");
			trie = test_insert (trie, "system:/tests/hosts/below", "sysbelow");
			trie = test_insert (trie, "system:/tests/hosts", "syshosts");
			trie = test_insert (trie, "system:/tests", "systests");
			trie = test_insert (trie, "system:/", "system");
			break;
		case 4:
			trie = test_insert (trie, "system:/tests/hosts/below", "sysbelow");
			trie = test_insert (trie, "system:/tests/hosts", "syshosts");
			trie = test_insert (trie, "system:/tests", "systests");
			trie = test_insert (trie, "system:/", "system");
			trie = test_insert (trie, "user:/tests/hosts/below", "below");
			trie = test_insert (trie, "user:/tests/hosts", "hosts");
			trie = test_insert (trie, "user:/tests", "tests");
			trie = test_insert (trie, "user:/", "user");
			break;
		}

		KeySet * mps = set_mountpoints ();

		exit_if_fail (trie, "trie was not build up successfully");

		Backend * backend = trieLookup (trie, "user:/");
		succeed_if (backend, "there should be a backend");
		if (backend) compare_key (backend->mountpoint, ksLookupByName (mps, "user:/", 0));
		// printf ("backend: %p\n", (void*)backend);


		Backend * b2 = trieLookup (trie, "user:/tests/hosts/other/below");
		succeed_if (b2, "there should be a backend");
		if (b2) compare_key (b2->mountpoint, ksLookupByName (mps, "user:/tests/hosts", 0));
		// printf ("b2: %p\n", (void*)b2);


		b2 = trieLookup (trie, "user:/tests/hosts/other/deep/below");
		succeed_if (b2, "there should be a backend");
		if (b2) compare_key (b2->mountpoint, ksLookupByName (mps, "user:/tests/hosts", 0));


		Backend * b3 = trieLookup (trie, "user:/tests/hosts/below");
		succeed_if (b3, "there should be a backend");
		if (b3) compare_key (b3->mountpoint, ksLookupByName (mps, "user:/tests/hosts/below", 0));
		// printf ("b3: %p\n", (void*)b3);


		backend = trieLookup (trie, "user:/tests/hosts/below/other/deep/below");
		succeed_if (backend, "there should be a backend");
		if (backend) compare_key (backend->mountpoint, ksLookupByName (mps, "user:/tests/hosts/below", 0));

		backend = trieLookup (trie, "system:/");
		succeed_if (backend, "there should be a backend");
		if (backend) compare_key (backend->mountpoint, ksLookupByName (mps, "system:/", 0));
		// printf ("backend: %p\n", (void*)backend);


		b2 = trieLookup (trie, "system:/tests/hosts/other/below");
		succeed_if (b2, "there should be a backend");
		if (b2) compare_key (b2->mountpoint, ksLookupByName (mps, "system:/tests/hosts", 0));
		// printf ("b2: %p\n", (void*)b2);


		b2 = trieLookup (trie, "system:/tests/hosts/other/deep/below");
		succeed_if (b2, "there should be a backend");
		if (b2) compare_key (b2->mountpoint, ksLookupByName (mps, "system:/tests/hosts", 0));


		b3 = trieLookup (trie, "system:/tests/hosts/below");
		succeed_if (b3, "there should be a backend");
		if (b3) compare_key (b3->mountpoint, ksLookupByName (mps, "system:/tests/hosts/below", 0));
		// printf ("b3: %p\n", (void*)b3);


		b2 = trieLookup (trie, "system:/tests/hosts/below/other/deep/below");
		succeed_if (b2, "there should be a backend");
		if (b2) compare_key (b2->mountpoint, ksLookupByName (mps, "system:/tests/hosts/below", 0));

		/*
		printf ("---------\n");
		output_trie(trie);
		*/

		KeySet * mps_cmp = ksNew (0, KS_END);
		collect_mountpoints (trie, mps_cmp);
		succeed_if (ksGetSize (mps_cmp) == 8, "size should be 8");
		compare_keyset (mps, mps_cmp);

		ksDel (mps_cmp);
		ksDel (mps);

		trieClose (trie, 0);

	} // end for
}


static void test_umlauts (void)
{
	printf ("Test umlauts trie\n");

	Trie * trie = test_insert (0, "user:/umlauts/test", "slash");
	trie = test_insert (trie, "user:/umlauts#test", "hash");
	trie = test_insert (trie, "user:/umlauts test", "space");
	trie = test_insert (trie, "user:/umlauts\200test", "umlauts");

	exit_if_fail (trie, "trie was not build up successfully");

	Backend * backend = trieLookup (trie, "user:/");
	succeed_if (!backend, "there should be no backend");


	Key * mp = keyNew ("user:/umlauts/test", KEY_VALUE, "slash", KEY_END);
	backend = trieLookup (trie, "user:/umlauts/test");
	succeed_if (backend, "there should be a backend");
	if (backend) compare_key (backend->mountpoint, mp);


	keySetName (mp, "user:/umlauts#test");
	keySetString (mp, "hash");
	Backend * b2 = trieLookup (trie, "user:/umlauts#test");
	succeed_if (b2, "there should be a backend");
	succeed_if (backend != b2, "should be other backend");
	if (b2) compare_key (b2->mountpoint, mp);


	keySetName (mp, "user:/umlauts test");
	keySetString (mp, "space");
	b2 = trieLookup (trie, "user:/umlauts test");
	succeed_if (b2, "there should be a backend");
	succeed_if (backend != b2, "should be other backend");
	if (b2) compare_key (b2->mountpoint, mp);

	keySetName (mp, "user:/umlauts\200test");
	keySetString (mp, "umlauts");
	b2 = trieLookup (trie, "user:/umlauts\200test");
	succeed_if (b2, "there should be a backend");
	succeed_if (backend != b2, "should be other backend");
	if (b2) compare_key (b2->mountpoint, mp);

	// output_trie(trie);

	trieClose (trie, 0);
	keyDel (mp);
}

static void test_endings (void)
{
	printf ("Test endings trie\n");

	for (int i = 0; i < 4; ++i)
	{

		Trie * trie = 0;
		switch (i)
		{
		case 0:
			trie = test_insert (trie, "user:/endings/", "slash");
			trie = test_insert (trie, "user:/endings#", "hash");
			trie = test_insert (trie, "user:/endings ", "space");
			trie = test_insert (trie, "user:/endings\200", "endings");
			break;
		case 1:
			trie = test_insert (trie, "user:/endings#", "hash");
			trie = test_insert (trie, "user:/endings ", "space");
			trie = test_insert (trie, "user:/endings\200", "endings");
			trie = test_insert (trie, "user:/endings/", "slash");
			break;
		case 2:
			trie = test_insert (trie, "user:/endings ", "space");
			trie = test_insert (trie, "user:/endings\200", "endings");
			trie = test_insert (trie, "user:/endings/", "slash");
			trie = test_insert (trie, "user:/endings#", "hash");
			break;
		case 3:
			trie = test_insert (trie, "user:/endings\200", "endings");
			trie = test_insert (trie, "user:/endings ", "space");
			trie = test_insert (trie, "user:/endings#", "hash");
			trie = test_insert (trie, "user:/endings/", "slash");
			break;
		}

		exit_if_fail (trie, "trie was not build up successfully");

		Backend * backend = trieLookup (trie, "user:/");
		succeed_if (!backend, "there should be no backend");


		Key * mp = keyNew ("user:/endings", KEY_VALUE, "slash", KEY_END);
		backend = trieLookup (trie, "user:/endings");
		succeed_if (backend, "there should be a backend");
		if (backend) compare_key (backend->mountpoint, mp);


		keySetName (mp, "user:/endings#");
		keySetString (mp, "hash");
		Backend * b2 = trieLookup (trie, "user:/endings#");
		succeed_if (b2, "there should be a backend");
		succeed_if (backend != b2, "should be other backend");
		if (b2) compare_key (b2->mountpoint, mp);


		keySetName (mp, "user:/endings");
		keySetString (mp, "slash");
		b2 = trieLookup (trie, "user:/endings/_");
		succeed_if (b2, "there should be a backend");
		succeed_if (backend == b2, "should be the same backend");
		if (b2) compare_key (b2->mountpoint, mp);


		keySetName (mp, "user:/endings");
		keySetString (mp, "slash");
		b2 = trieLookup (trie, "user:/endings/X");
		succeed_if (b2, "there should be a backend");
		succeed_if (backend == b2, "should be the same backend");
		if (b2) compare_key (b2->mountpoint, mp);


		b2 = trieLookup (trie, "user:/endings_");
		succeed_if (!b2, "there should be no backend");


		b2 = trieLookup (trie, "user:/endingsX");
		succeed_if (!b2, "there should be no backend");


		b2 = trieLookup (trie, "user:/endings!");
		succeed_if (!b2, "there should be no backend");


		keySetName (mp, "user:/endings ");
		keySetString (mp, "space");
		b2 = trieLookup (trie, "user:/endings ");
		succeed_if (b2, "there should be a backend");
		succeed_if (backend != b2, "should be other backend");
		if (b2) compare_key (b2->mountpoint, mp);

		keySetName (mp, "user:/endings\200");
		keySetString (mp, "endings");
		b2 = trieLookup (trie, "user:/endings\200");
		succeed_if (b2, "there should be a backend");
		succeed_if (backend != b2, "should be other backend");
		if (b2) compare_key (b2->mountpoint, mp);

		// output_trie(trie);

		trieClose (trie, 0);
		keyDel (mp);
	}
}

static void test_root (void)
{
	printf ("Test trie with root\n");

	Trie * trie = 0;
	trie = test_insert (trie, "", "root");
	trie = test_insert (trie, "user:/tests/simple", "simple");

	exit_if_fail (trie, "trie was not build up successfully");

	Key * rmp = keyNew ("/", KEY_VALUE, "root", KEY_END);
	Backend * backend = trieLookup (trie, "user:/");
	succeed_if (backend, "there should be the root backend");
	if (backend) succeed_if (backend->mountpoint == NULL, "default backend mountpoint should be NULL");


	Key * mp = keyNew ("user:/tests/simple", KEY_VALUE, "simple", KEY_END);
	backend = trieLookup (trie, "user:/tests/simple");
	succeed_if (backend, "there should be a backend");
	if (backend) compare_key (backend->mountpoint, mp);


	Backend * b2 = trieLookup (trie, "user:/tests/simple/below");
	succeed_if (b2, "there should be a backend");
	succeed_if (backend == b2, "should be same backend");
	if (b2) compare_key (b2->mountpoint, mp);


	b2 = trieLookup (trie, "user:/tests/simple/deep/below");
	succeed_if (b2, "there should be a backend");
	succeed_if (backend == b2, "should be same backend");
	if (b2) compare_key (b2->mountpoint, mp);

	// output_trie(trie);

	trieClose (trie, 0);
	keyDel (mp);
	keyDel (rmp);
}

static void test_double (void)
{
	printf ("Test double insertion\n");

	Trie * trie = test_insert (0, "/", "root");
	succeed_if (trie, "could not insert into trie");

	Trie * t1 = test_insert (trie, "user:/tests/simple", "t1");
	succeed_if (t1, "could not insert into trie");
	succeed_if (t1 == trie, "should be the same");

	// output_trie (trie);

	Trie * t2 = test_insert (trie, "user:/tests/simple", "t2");
	succeed_if (t2, "could not insert into trie");
	succeed_if (t2 == trie, "should be not the same");

	// output_trie (trie);

	/* ... gets lost

	Trie *t3 = test_insert (trie, "user:/tests/simple", "t3");
	succeed_if (t3, "could not insert into trie");
	succeed_if (t3 == trie, "should be not the same");

	// output_trie (trie);

	*/

	trieClose (trie, 0);
}

static void test_emptyvalues (void)
{
	printf ("Test empty values in trie\n");

	Trie * trie = 0;
	trie = test_insert (trie, "user:/umlauts/b/", "b");
	trie = test_insert (trie, "user:/umlauts/a/", "a");
	trie = test_insert (trie, "user:/umlauts/", "/");
	trie = test_insert (trie, "user:/umlauts/c/", "c");
	trie = test_insert (trie, "user:/", "u");

	exit_if_fail (trie, "trie was not build up successfully");

	// output_trie(trie);

	trieClose (trie, 0);
}

static void test_userroot (void)
{
	printf ("Test trie with user:/\n");

	Trie * trie = 0;
	trie = test_insert (trie, "user:/", "root");

	exit_if_fail (trie, "trie was not build up successfully");

	Key * mp = keyNew ("user:/", KEY_VALUE, "root", KEY_END);
	Backend * backend = trieLookup (trie, "user:/");
	succeed_if (backend, "there should be a backend");
	if (backend) compare_key (backend->mountpoint, mp);

	backend = trieLookup (trie, "user:/tests");
	succeed_if (backend, "there should be a backend");
	if (backend) compare_key (backend->mountpoint, mp);

	backend = trieLookup (trie, "user:/tests/simple");
	succeed_if (backend, "there should be a backend");
	if (backend) compare_key (backend->mountpoint, mp);


	Backend * b2 = trieLookup (trie, "user:/tests/simple/below");
	succeed_if (b2, "there should be a backend");
	succeed_if (backend == b2, "should be same backend");
	if (b2) compare_key (b2->mountpoint, mp);


	b2 = trieLookup (trie, "user:/tests/simple/deep/below");
	succeed_if (b2, "there should be a backend");
	succeed_if (backend == b2, "should be same backend");
	if (b2) compare_key (b2->mountpoint, mp);

	// output_trie(trie);

	trieClose (trie, 0);
	keyDel (mp);
}


int main (int argc, char ** argv)
{
	printf ("TRIE       TESTS\n");
	printf ("==================\n\n");

	init (argc, argv);

	test_minimaltrie ();
	test_simple ();
	test_iterate ();
	test_reviterate ();
	test_moreiterate ();
	test_revmoreiterate ();
	test_umlauts ();
	test_endings ();
	test_root ();
	test_double ();
	test_emptyvalues ();
	test_userroot ();

	printf ("\ntest_trie RESULTS: %d test(s) done. %d error(s).\n", nbTest, nbError);

	return nbError;
}
