#include "message.h"
#include "new.h"
#include "unity_fixture.h"

TEST_GROUP(MessageGroup);

TEST_SETUP(MessageGroup) {}

TEST_TEAR_DOWN(MessageGroup) {}

/* Interface: new(Message, size, data)
 *
 * Allocates <size> bytes of memory and copies <size> bytes of data into it.
 *
 * Member Variables
 * ----------------
 *
 * void * message; // Address of allocated memory
 *
 * Methods
 * -------
 *
 * int getSize(): Returns size of allocated memory
 *
 */

TEST(MessageGroup, AllocateMemoryTest)
{ /* Ensure that we get a valid pointer to memory */

	t_Message* message = new (Message, 10, "0123456789");
	TEST_ASSERT_NOT_NULL(message);

	/* TEST_ASSERT_NULL(getMessageData(message)); */
	TEST_ASSERT_EQUAL(10, getMessageSize(message));

	delete (message);
}

TEST(MessageGroup, SizeTest)
{ /* Ensure that the reported size is correct */

	t_Message* message = new (Message, 10, "ABCDEFGHIJ");
	TEST_ASSERT_EQUAL(10, getMessageSize(message));

	delete (message);
}

TEST(MessageGroup, ComparisonTest)
{ /* Test the messageEquals() method */

	/* The message should not overflow */
	t_Message* message = new (Message, 5, "A1B2C3");
	*getMessageData(message) = 'Z';
	TEST_ASSERT_EQUAL_MEMORY("Z1B2C", getMessageData(message), 5);
	TEST_ASSERT_EQUAL(0, isMessageEqual(message, "Z1B2C"));

	delete (message);
}

TEST_GROUP_RUNNER(MessageGroup)
{ /* Add a line below for each unit test */

	RUN_TEST_CASE(MessageGroup, AllocateMemoryTest);
	RUN_TEST_CASE(MessageGroup, SizeTest);
	RUN_TEST_CASE(MessageGroup, ComparisonTest);
}

static void
runAllTests()
{
	RUN_TEST_GROUP(MessageGroup);
}

int
main(int argc, const char* argv[])
{
	printf("\nRunning %s...\n", argv[0]);
	return UnityMain(argc, argv, runAllTests);
}
