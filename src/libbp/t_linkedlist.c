#include "linkedlist.h"
#include "new.h"

#include "unity_fixture.h"

TEST_GROUP(LinkedListGroup);

TEST_SETUP(LinkedListGroup) {}

TEST_TEAR_DOWN(LinkedListGroup) {}

/* What is the set of tests which, when passes, demonstrates that the code
 * works correctly? */

/* When writing a test, imagine the perfect interface for that function */
/* 	What function do we wish existed */

/* Write a test list */

TEST(LinkedListGroup, CorrectDefaultsTest)
{
	/* The next pointer must be initialised with NULL */

	t_LinkedList *list = new (LinkedList);
	TEST_ASSERT_NOT_NULL(list);
	TEST_ASSERT_NULL(getNext(list));
}

TEST(LinkedListGroup, AddRemoveTest)
{
	/* Adding and removing items should change count */
	/* Removing last item should return null pointer */
	/* Removing from a null pointer should return null */
	/* Removing a middle item should join remining items */

	t_LinkedList *list01 = new (LinkedList);
	TEST_ASSERT_EQUAL(1, getCount(list01));
	TEST_ASSERT_EQUAL_PTR(list01, getLast(list01));

	/* Add items */
	t_LinkedList *list02 = addChild(list01);
	TEST_ASSERT_EQUAL(2, getCount(list01));
	TEST_ASSERT_EQUAL_PTR(list02, getNext(list01));
	TEST_ASSERT_EQUAL_PTR(list02, getLast(list01));
	TEST_ASSERT_NULL(getNext(list02));

	t_LinkedList *list03 = addChild(list01);
	TEST_ASSERT_EQUAL_PTR(list02, getNext(list03));
	TEST_ASSERT_EQUAL(3, getCount(list01));

	/* Remove items */
	t_LinkedList *result = removeEntry(list01, list03);
	TEST_ASSERT_EQUAL(2, getCount(list01));
	TEST_ASSERT_EQUAL_PTR(list02, getNext(list01));
	TEST_ASSERT_EQUAL_PTR(list02, getLast(list01));

	/* Attempt to remove a nonexistent item */
	result = removeEntry(list01, list03);
	TEST_ASSERT_EQUAL(2, getCount(list01));

	result = removeEntry(list01, list02);
	TEST_ASSERT_EQUAL(1, getCount(list01));
	TEST_ASSERT_EQUAL_PTR(list01, getLast(list01));
	TEST_ASSERT_EQUAL_PTR(list01, result);

	result = removeEntry(list01, list01);
	TEST_ASSERT_NULL(result);
}

TEST_GROUP_RUNNER(LinkedListGroup)
{ /* Add a line below for each unit test */

	RUN_TEST_CASE(LinkedListGroup, CorrectDefaultsTest);
	RUN_TEST_CASE(LinkedListGroup, AddRemoveTest);
}

static void
runAllTests()
{
	RUN_TEST_GROUP(LinkedListGroup);
}

int
main(int argc, const char *argv[])
{
	printf("\nRunning %s...\n", argv[0]);
	return UnityMain(argc, argv, runAllTests);
}
