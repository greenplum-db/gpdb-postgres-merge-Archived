#include "postgres.h"

#include "funcapi.h"
#include "commands/trigger.h"

extern Datum trigger_udf_return_new_oid(PG_FUNCTION_ARGS);

/*
 * trigger_udf_return_new_oid
 *   Scalar returning function - create a new tuple copied base on the input tuple, but
 *   set a new oid.
 */
PG_FUNCTION_INFO_V1(trigger_udf_return_new_oid);
Datum
trigger_udf_return_new_oid(PG_FUNCTION_ARGS)
{
	TriggerData		*trigger_data = (TriggerData*)fcinfo->context;
	HeapTuple		input_tuple;
	HeapTuple		ret_tuple;
	Oid				new_oid = PG_GETARG_OID(0);

    if (!CALLED_AS_TRIGGER(fcinfo))
        ereport(ERROR,
            (errcode(ERRCODE_TRIGGERED_ACTION_EXCEPTION),
             errmsg("not called by trigger manager")));

    if (!TRIGGER_FIRED_BY_INSERT(trigger_data->tg_event))
        ereport(ERROR,
            (errcode(ERRCODE_TRIGGERED_ACTION_EXCEPTION),
             errmsg("not called on valid event")));

	input_tuple = trigger_data->tg_trigtuple;
	ret_tuple = heap_copytuple(input_tuple);
	HeapTupleSetOid(ret_tuple, new_oid);
	return PointerGetDatum(ret_tuple);
}
