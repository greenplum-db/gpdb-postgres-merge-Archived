#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include "cmockery.h"

#include "catquery_mock.c"

/*
 * Tests
 */
void
test__is_builtin_object__oid(void **state)
{
	HeapTuple ht = build_pg_class_tuple();

	HeapTupleSetOid(ht, 1000);
	assert_true(is_builtin_object(NULL, ht));

	HeapTupleSetOid(ht, 17000);
	assert_false(is_builtin_object(NULL, ht));
}

void
test__is_builtin_object__non_oid(void **state)
{
	cqContext ctx;
	ctx.cq_relationId = AttributeRelationId;
	HeapTuple ht = build_pg_attribute_tuple(1255);

	assert_true(is_builtin_object(&ctx, ht));

	ht = build_pg_attribute_tuple(17000);
	assert_false(is_builtin_object(&ctx, ht));
}

int
main(int argc, char* argv[])
{
	cmockery_parse_arguments(argc, argv);
	const UnitTest tests[] =
	{
		unit_test(test__is_builtin_object__oid),
		unit_test(test__is_builtin_object__non_oid),
	};

	MemoryContextInit();

	return run_tests(tests);
}

