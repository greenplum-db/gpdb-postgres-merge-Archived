/*-------------------------------------------------------------------------
 *
 * tqueue.c
 *	  Use shm_mq to send & receive tuples between parallel backends
 *
 * A DestReceiver of type DestTupleQueue, which is a TQueueDestReceiver
 * under the hood, writes tuples from the executor to a shm_mq.
 *
 * A TupleQueueReader reads tuples from a shm_mq and returns the tuples.
 *
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * IDENTIFICATION
 *	  src/backend/executor/tqueue.c
 *
 *-------------------------------------------------------------------------
 */

#include "postgres.h"

#include "access/htup_details.h"
#include "executor/tqueue.h"

/*
 * DestReceiver object's private contents
 *
 * queue is a pointer to data supplied by DestReceiver's caller.
 */
typedef struct TQueueDestReceiver
{
	DestReceiver pub;			/* public fields */
	shm_mq_handle *queue;		/* shm_mq to send to */
} TQueueDestReceiver;

/*
 * TupleQueueReader object's private contents
 *
 * queue is a pointer to data supplied by reader's caller.
 *
 * "typedef struct TupleQueueReader TupleQueueReader" is in tqueue.h
 */
struct TupleQueueReader
{
	shm_mq_handle *queue;		/* shm_mq to receive from */
};

/*
 * Receive a tuple from a query, and send it to the designated shm_mq.
 *
 * Returns true if successful, false if shm_mq has been detached.
 */
static bool
tqueueReceiveSlot(TupleTableSlot *slot, DestReceiver *self)
{
	TQueueDestReceiver *tqueue = (TQueueDestReceiver *) self;
	HeapTuple	tuple;
	shm_mq_result result;
<<<<<<< HEAD

	/*
	 * If first time through, compute remapping info for the top-level fields.
	 * On later calls, if the tupledesc has changed, set up for the new
	 * tupledesc.  (This is a strange test both because the executor really
	 * shouldn't change the tupledesc, and also because it would be unsafe if
	 * the old tupledesc could be freed and a new one allocated at the same
	 * address.  But since some very old code in printtup.c uses a similar
	 * approach, we adopt it here as well.)
	 *
	 * Here and elsewhere in this module, when replacing remapping info we
	 * pfree the top-level object because that's easy, but we don't bother to
	 * recursively free any substructure.  This would lead to query-lifespan
	 * memory leaks if the mapping info actually changed frequently, but since
	 * we don't expect that to happen, it doesn't seem worth expending code to
	 * prevent it.
	 */
	if (tqueue->tupledesc != tupledesc)
	{
		/* Is it worth trying to free substructure of the remap tree? */
		if (tqueue->field_remapinfo != NULL)
			pfree(tqueue->field_remapinfo);
		tqueue->field_remapinfo = BuildFieldRemapInfo(tupledesc,
													  tqueue->mycontext);
		tqueue->tupledesc = tupledesc;
	}

	/*
	 * When, because of the types being transmitted, no record typmod mapping
	 * can be needed, we can skip a good deal of work.
	 */
	if (tqueue->field_remapinfo != NULL)
	{
		TupleRemapInfo **remapinfo = tqueue->field_remapinfo;
		int			i;
		MemoryContext oldcontext = NULL;
		Datum	   *slot_values;
		bool	   *slot_isnull;

		/* Deform the tuple so we can examine fields, if not done already. */
		slot_getallattrs(slot);
		slot_values = slot_get_values(slot);
		slot_isnull = slot_get_isnull(slot);

		/* Iterate over each attribute and search it for transient typmods. */
		for (i = 0; i < tupledesc->natts; i++)
		{
			/* Ignore nulls and types that don't need special handling. */
			if (slot_isnull[i] || remapinfo[i] == NULL)
				continue;

			/* Switch to temporary memory context to avoid leaking. */
			if (oldcontext == NULL)
			{
				if (tqueue->tmpcontext == NULL)
					tqueue->tmpcontext =
						AllocSetContextCreate(tqueue->mycontext,
											  "tqueue sender temp context",
											  ALLOCSET_DEFAULT_MINSIZE,
											  ALLOCSET_DEFAULT_INITSIZE,
											  ALLOCSET_DEFAULT_MAXSIZE);
				oldcontext = MemoryContextSwitchTo(tqueue->tmpcontext);
			}

			/* Examine the value. */
			TQExamine(tqueue, remapinfo[i], slot_values[i]);
		}

		/* If we used the temp context, reset it and restore prior context. */
		if (oldcontext != NULL)
		{
			MemoryContextSwitchTo(oldcontext);
			MemoryContextReset(tqueue->tmpcontext);
		}

		/* If we entered control mode, switch back to data mode. */
		if (tqueue->mode != TUPLE_QUEUE_MODE_DATA)
		{
			tqueue->mode = TUPLE_QUEUE_MODE_DATA;
			shm_mq_send(tqueue->queue, sizeof(char), &tqueue->mode, false);
		}
	}
=======
	bool		should_free;
>>>>>>> 9e1c9f959422192bbe1b842a2a1ffaf76b080196

	/* Send the tuple itself. */
	tuple = ExecFetchSlotHeapTuple(slot, true, &should_free);
	result = shm_mq_send(tqueue->queue, tuple->t_len, tuple->t_data, false);

	if (should_free)
		heap_freetuple(tuple);

	/* Check for failure. */
	if (result == SHM_MQ_DETACHED)
		return false;
	else if (result != SHM_MQ_SUCCESS)
		ereport(ERROR,
				(errcode(ERRCODE_OBJECT_NOT_IN_PREREQUISITE_STATE),
				 errmsg("could not send tuple to shared-memory queue")));

	return true;
}

/*
<<<<<<< HEAD
 * Examine the given datum and send any necessary control messages for
 * transient record types contained in it.
 *
 * remapinfo is previously-computed remapping info about the datum's type.
 *
 * This function just dispatches based on the remap class.
 */
static void
TQExamine(TQueueDestReceiver *tqueue, TupleRemapInfo *remapinfo, Datum value)
{
	/* This is recursive, so it could be driven to stack overflow. */
	check_stack_depth();

	switch (remapinfo->remapclass)
	{
		case TQUEUE_REMAP_ARRAY:
			TQExamineArray(tqueue, &remapinfo->u.arr, value);
			break;
		case TQUEUE_REMAP_RANGE:
			TQExamineRange(tqueue, &remapinfo->u.rng, value);
			break;
		case TQUEUE_REMAP_RECORD:
			TQExamineRecord(tqueue, &remapinfo->u.rec, value);
			break;
	}
}

/*
 * Examine a record datum and send any necessary control messages for
 * transient record types contained in it.
 */
static void
TQExamineRecord(TQueueDestReceiver *tqueue, RecordRemapInfo *remapinfo,
				Datum value)
{
	HeapTupleHeader tup;
	Oid			typid;
	int32		typmod;
	TupleDesc	tupledesc;

	/* Extract type OID and typmod from tuple. */
	tup = DatumGetHeapTupleHeader(value);
	typid = HeapTupleHeaderGetTypeId(tup);
	typmod = HeapTupleHeaderGetTypMod(tup);

	/*
	 * If first time through, or if this isn't the same composite type as last
	 * time, consider sending a control message, and then look up the
	 * necessary information for examining the fields.
	 */
	if (typid != remapinfo->rectypid || typmod != remapinfo->rectypmod)
	{
		/* Free any old data. */
		if (remapinfo->tupledesc != NULL)
			FreeTupleDesc(remapinfo->tupledesc);
		/* Is it worth trying to free substructure of the remap tree? */
		if (remapinfo->field_remap != NULL)
			pfree(remapinfo->field_remap);

		/* Look up tuple descriptor in typcache. */
		tupledesc = lookup_rowtype_tupdesc(typid, typmod);

		/*
		 * If this is a transient record type, send the tupledesc in a control
		 * message.  (TQSendRecordInfo is smart enough to do this only once
		 * per typmod.)
		 */
		if (typid == RECORDOID)
			TQSendRecordInfo(tqueue, typmod, tupledesc);

		/* Figure out whether fields need recursive processing. */
		remapinfo->field_remap = BuildFieldRemapInfo(tupledesc,
													 tqueue->mycontext);
		if (remapinfo->field_remap != NULL)
		{
			/*
			 * We need to inspect the record contents, so save a copy of the
			 * tupdesc.  (We could possibly just reference the typcache's
			 * copy, but then it's problematic when to release the refcount.)
			 */
			MemoryContext oldcontext = MemoryContextSwitchTo(tqueue->mycontext);

			remapinfo->tupledesc = CreateTupleDescCopy(tupledesc);
			MemoryContextSwitchTo(oldcontext);
		}
		else
		{
			/* No fields of the record require remapping. */
			remapinfo->tupledesc = NULL;
		}
		remapinfo->rectypid = typid;
		remapinfo->rectypmod = typmod;

		/* Release reference count acquired by lookup_rowtype_tupdesc. */
		DecrTupleDescRefCount(tupledesc);
	}

	/*
	 * If field remapping is required, deform the tuple and examine each
	 * field.
	 */
	if (remapinfo->field_remap != NULL)
	{
		Datum	   *values;
		bool	   *isnull;
		HeapTupleData tdata;
		int			i;

		/* Deform the tuple so we can check each column within. */
		tupledesc = remapinfo->tupledesc;
		values = (Datum *) palloc(tupledesc->natts * sizeof(Datum));
		isnull = (bool *) palloc(tupledesc->natts * sizeof(bool));
		tdata.t_len = HeapTupleHeaderGetDatumLength(tup);
		ItemPointerSetInvalid(&(tdata.t_self));
#if 0
		tdata.t_tableOid = InvalidOid;
#endif
		tdata.t_data = tup;
		heap_deform_tuple(&tdata, tupledesc, values, isnull);

		/* Recursively check each interesting non-NULL attribute. */
		for (i = 0; i < tupledesc->natts; i++)
		{
			if (!isnull[i] && remapinfo->field_remap[i])
				TQExamine(tqueue, remapinfo->field_remap[i], values[i]);
		}

		/* Need not clean up, since we're in a short-lived context. */
	}
}

/*
 * Examine an array datum and send any necessary control messages for
 * transient record types contained in it.
 */
static void
TQExamineArray(TQueueDestReceiver *tqueue, ArrayRemapInfo *remapinfo,
			   Datum value)
{
	ArrayType  *arr = DatumGetArrayTypeP(value);
	Oid			typid = ARR_ELEMTYPE(arr);
	Datum	   *elem_values;
	bool	   *elem_nulls;
	int			num_elems;
	int			i;

	/* Deconstruct the array. */
	deconstruct_array(arr, typid, remapinfo->typlen,
					  remapinfo->typbyval, remapinfo->typalign,
					  &elem_values, &elem_nulls, &num_elems);

	/* Examine each element. */
	for (i = 0; i < num_elems; i++)
	{
		if (!elem_nulls[i])
			TQExamine(tqueue, remapinfo->element_remap, elem_values[i]);
	}
}

/*
 * Examine a range datum and send any necessary control messages for
 * transient record types contained in it.
 */
static void
TQExamineRange(TQueueDestReceiver *tqueue, RangeRemapInfo *remapinfo,
			   Datum value)
{
	RangeType  *range = DatumGetRangeType(value);
	RangeBound	lower;
	RangeBound	upper;
	bool		empty;

	/* Extract the lower and upper bounds. */
	range_deserialize(remapinfo->typcache, range, &lower, &upper, &empty);

	/* Nothing to do for an empty range. */
	if (empty)
		return;

	/* Examine each bound, if present. */
	if (!upper.infinite)
		TQExamine(tqueue, remapinfo->bound_remap, upper.val);
	if (!lower.infinite)
		TQExamine(tqueue, remapinfo->bound_remap, lower.val);
}

/*
 * Send tuple descriptor information for a transient typmod, unless we've
 * already done so previously.
 */
static void
TQSendRecordInfo(TQueueDestReceiver *tqueue, int32 typmod, TupleDesc tupledesc)
{
	StringInfoData buf;
	bool		found;
	int			i;

	/* Initialize hash table if not done yet. */
	if (tqueue->recordhtab == NULL)
	{
		HASHCTL		ctl;

		MemSet(&ctl, 0, sizeof(ctl));
		/* Hash table entries are just typmods */
		ctl.keysize = sizeof(int32);
		ctl.entrysize = sizeof(int32);
		ctl.hcxt = tqueue->mycontext;
		tqueue->recordhtab = hash_create("tqueue sender record type hashtable",
										 100, &ctl,
									  HASH_ELEM | HASH_BLOBS | HASH_CONTEXT);
	}

	/* Have we already seen this record type?  If not, must report it. */
	hash_search(tqueue->recordhtab, &typmod, HASH_ENTER, &found);
	if (found)
		return;

	elog(DEBUG3, "sending tqueue control message for record typmod %d", typmod);

	/* If message queue is in data mode, switch to control mode. */
	if (tqueue->mode != TUPLE_QUEUE_MODE_CONTROL)
	{
		tqueue->mode = TUPLE_QUEUE_MODE_CONTROL;
		shm_mq_send(tqueue->queue, sizeof(char), &tqueue->mode, false);
	}

	/* Assemble a control message. */
	initStringInfo(&buf);
	appendBinaryStringInfo(&buf, (char *) &typmod, sizeof(int32));
	appendBinaryStringInfo(&buf, (char *) &tupledesc->natts, sizeof(int));
	appendBinaryStringInfo(&buf, (char *) &tupledesc->tdhasoid, sizeof(bool));
	for (i = 0; i < tupledesc->natts; i++)
	{
		appendBinaryStringInfo(&buf, (char *) tupledesc->attrs[i],
							   sizeof(FormData_pg_attribute));
	}

	/* Send control message. */
	shm_mq_send(tqueue->queue, buf.len, buf.data, false);

	/* We assume it's OK to leak buf because we're in a short-lived context. */
}

/*
=======
>>>>>>> 9e1c9f959422192bbe1b842a2a1ffaf76b080196
 * Prepare to receive tuples from executor.
 */
static void
tqueueStartupReceiver(DestReceiver *self, int operation, TupleDesc typeinfo)
{
	/* do nothing */
}

/*
 * Clean up at end of an executor run
 */
static void
tqueueShutdownReceiver(DestReceiver *self)
{
	TQueueDestReceiver *tqueue = (TQueueDestReceiver *) self;

	if (tqueue->queue != NULL)
		shm_mq_detach(tqueue->queue);
	tqueue->queue = NULL;
}

/*
 * Destroy receiver when done with it
 */
static void
tqueueDestroyReceiver(DestReceiver *self)
{
	TQueueDestReceiver *tqueue = (TQueueDestReceiver *) self;

	/* We probably already detached from queue, but let's be sure */
	if (tqueue->queue != NULL)
		shm_mq_detach(tqueue->queue);
	pfree(self);
}

/*
 * Create a DestReceiver that writes tuples to a tuple queue.
 */
DestReceiver *
CreateTupleQueueDestReceiver(shm_mq_handle *handle)
{
	TQueueDestReceiver *self;

	self = (TQueueDestReceiver *) palloc0(sizeof(TQueueDestReceiver));

	self->pub.receiveSlot = tqueueReceiveSlot;
	self->pub.rStartup = tqueueStartupReceiver;
	self->pub.rShutdown = tqueueShutdownReceiver;
	self->pub.rDestroy = tqueueDestroyReceiver;
	self->pub.mydest = DestTupleQueue;
	self->queue = handle;

	return (DestReceiver *) self;
}

/*
 * Create a tuple queue reader.
 */
TupleQueueReader *
CreateTupleQueueReader(shm_mq_handle *handle)
{
	TupleQueueReader *reader = palloc0(sizeof(TupleQueueReader));

	reader->queue = handle;

	return reader;
}

/*
 * Destroy a tuple queue reader.
 *
 * Note: cleaning up the underlying shm_mq is the caller's responsibility.
 * We won't access it here, as it may be detached already.
 */
void
DestroyTupleQueueReader(TupleQueueReader *reader)
{
	pfree(reader);
}

/*
 * Fetch a tuple from a tuple queue reader.
 *
 * The return value is NULL if there are no remaining tuples or if
 * nowait = true and no tuple is ready to return.  *done, if not NULL,
 * is set to true when there are no remaining tuples and otherwise to false.
 *
 * The returned tuple, if any, is allocated in CurrentMemoryContext.
 * Note that this routine must not leak memory!  (We used to allow that,
 * but not any more.)
 *
 * Even when shm_mq_receive() returns SHM_MQ_WOULD_BLOCK, this can still
 * accumulate bytes from a partially-read message, so it's useful to call
 * this with nowait = true even if nothing is returned.
 */
HeapTuple
TupleQueueReaderNext(TupleQueueReader *reader, bool nowait, bool *done)
{
	HeapTupleData htup;
	shm_mq_result result;
	Size		nbytes;
	void	   *data;

	if (done != NULL)
		*done = false;

	/* Attempt to read a message. */
	result = shm_mq_receive(reader->queue, &nbytes, &data, nowait);

	/* If queue is detached, set *done and return NULL. */
	if (result == SHM_MQ_DETACHED)
	{
		if (done != NULL)
			*done = true;
		return NULL;
	}

	/* In non-blocking mode, bail out if no message ready yet. */
	if (result == SHM_MQ_WOULD_BLOCK)
		return NULL;
	Assert(result == SHM_MQ_SUCCESS);

	/*
	 * Set up a dummy HeapTupleData pointing to the data from the shm_mq
	 * (which had better be sufficiently aligned).
	 */
	ItemPointerSetInvalid(&htup.t_self);
#if 0
	htup.t_tableOid = InvalidOid;
#endif
	htup.t_len = nbytes;
	htup.t_data = data;

<<<<<<< HEAD
	/*
	 * Either just copy the data into a regular palloc'd tuple, or remap it,
	 * as required.
	 */
	return TQRemapTuple(reader,
						reader->tupledesc,
						reader->field_remapinfo,
						&htup);
}

/*
 * Copy the given tuple, remapping any transient typmods contained in it.
 */
static HeapTuple
TQRemapTuple(TupleQueueReader *reader,
			 TupleDesc tupledesc,
			 TupleRemapInfo **field_remapinfo,
			 HeapTuple tuple)
{
	Datum	   *values;
	bool	   *isnull;
	bool		changed = false;
	int			i;

	/*
	 * If no remapping is necessary, just copy the tuple into a single
	 * palloc'd chunk, as caller will expect.
	 */
	if (field_remapinfo == NULL)
		return heap_copytuple(tuple);

	/* Deform tuple so we can remap record typmods for individual attrs. */
	values = (Datum *) palloc(tupledesc->natts * sizeof(Datum));
	isnull = (bool *) palloc(tupledesc->natts * sizeof(bool));
	heap_deform_tuple(tuple, tupledesc, values, isnull);

	/* Recursively process each interesting non-NULL attribute. */
	for (i = 0; i < tupledesc->natts; i++)
	{
		if (isnull[i] || field_remapinfo[i] == NULL)
			continue;
		values[i] = TQRemap(reader, field_remapinfo[i], values[i], &changed);
	}

	/* Reconstruct the modified tuple, if anything was modified. */
	if (changed)
		return heap_form_tuple(tupledesc, values, isnull);
	else
		return heap_copytuple(tuple);
}

/*
 * Process the given datum and replace any transient record typmods
 * contained in it.  Set *changed to TRUE if we actually changed the datum.
 *
 * remapinfo is previously-computed remapping info about the datum's type.
 *
 * This function just dispatches based on the remap class.
 */
static Datum
TQRemap(TupleQueueReader *reader, TupleRemapInfo *remapinfo,
		Datum value, bool *changed)
{
	/* This is recursive, so it could be driven to stack overflow. */
	check_stack_depth();

	switch (remapinfo->remapclass)
	{
		case TQUEUE_REMAP_ARRAY:
			return TQRemapArray(reader, &remapinfo->u.arr, value, changed);

		case TQUEUE_REMAP_RANGE:
			return TQRemapRange(reader, &remapinfo->u.rng, value, changed);

		case TQUEUE_REMAP_RECORD:
			return TQRemapRecord(reader, &remapinfo->u.rec, value, changed);
	}

	elog(ERROR, "unrecognized tqueue remap class: %d",
		 (int) remapinfo->remapclass);
	return (Datum) 0;
}

/*
 * Process the given array datum and replace any transient record typmods
 * contained in it.  Set *changed to TRUE if we actually changed the datum.
 */
static Datum
TQRemapArray(TupleQueueReader *reader, ArrayRemapInfo *remapinfo,
			 Datum value, bool *changed)
{
	ArrayType  *arr = DatumGetArrayTypeP(value);
	Oid			typid = ARR_ELEMTYPE(arr);
	bool		element_changed = false;
	Datum	   *elem_values;
	bool	   *elem_nulls;
	int			num_elems;
	int			i;

	/* Deconstruct the array. */
	deconstruct_array(arr, typid, remapinfo->typlen,
					  remapinfo->typbyval, remapinfo->typalign,
					  &elem_values, &elem_nulls, &num_elems);

	/* Remap each element. */
	for (i = 0; i < num_elems; i++)
	{
		if (!elem_nulls[i])
			elem_values[i] = TQRemap(reader,
									 remapinfo->element_remap,
									 elem_values[i],
									 &element_changed);
	}

	if (element_changed)
	{
		/* Reconstruct and return the array.  */
		*changed = true;
		arr = construct_md_array(elem_values, elem_nulls,
							   ARR_NDIM(arr), ARR_DIMS(arr), ARR_LBOUND(arr),
								 typid, remapinfo->typlen,
								 remapinfo->typbyval, remapinfo->typalign);
		return PointerGetDatum(arr);
	}

	/* Else just return the value as-is. */
	return value;
}

/*
 * Process the given range datum and replace any transient record typmods
 * contained in it.  Set *changed to TRUE if we actually changed the datum.
 */
static Datum
TQRemapRange(TupleQueueReader *reader, RangeRemapInfo *remapinfo,
			 Datum value, bool *changed)
{
	RangeType  *range = DatumGetRangeType(value);
	bool		bound_changed = false;
	RangeBound	lower;
	RangeBound	upper;
	bool		empty;

	/* Extract the lower and upper bounds. */
	range_deserialize(remapinfo->typcache, range, &lower, &upper, &empty);

	/* Nothing to do for an empty range. */
	if (empty)
		return value;

	/* Remap each bound, if present. */
	if (!upper.infinite)
		upper.val = TQRemap(reader, remapinfo->bound_remap,
							upper.val, &bound_changed);
	if (!lower.infinite)
		lower.val = TQRemap(reader, remapinfo->bound_remap,
							lower.val, &bound_changed);

	if (bound_changed)
	{
		/* Reserialize.  */
		*changed = true;
		range = range_serialize(remapinfo->typcache, &lower, &upper, empty);
		return RangeTypeGetDatum(range);
	}

	/* Else just return the value as-is. */
	return value;
}

/*
 * Process the given record datum and replace any transient record typmods
 * contained in it.  Set *changed to TRUE if we actually changed the datum.
 */
static Datum
TQRemapRecord(TupleQueueReader *reader, RecordRemapInfo *remapinfo,
			  Datum value, bool *changed)
{
	HeapTupleHeader tup;
	Oid			typid;
	int32		typmod;
	bool		changed_typmod;
	TupleDesc	tupledesc;

	/* Extract type OID and typmod from tuple. */
	tup = DatumGetHeapTupleHeader(value);
	typid = HeapTupleHeaderGetTypeId(tup);
	typmod = HeapTupleHeaderGetTypMod(tup);

	/*
	 * If first time through, or if this isn't the same composite type as last
	 * time, identify the required typmod mapping, and then look up the
	 * necessary information for processing the fields.
	 */
	if (typid != remapinfo->rectypid || typmod != remapinfo->rectypmod)
	{
		/* Free any old data. */
		if (remapinfo->tupledesc != NULL)
			FreeTupleDesc(remapinfo->tupledesc);
		/* Is it worth trying to free substructure of the remap tree? */
		if (remapinfo->field_remap != NULL)
			pfree(remapinfo->field_remap);

		/* If transient record type, look up matching local typmod. */
		if (typid == RECORDOID)
		{
			RecordTypmodMap *mapent;

			Assert(reader->typmodmap != NULL);
			mapent = hash_search(reader->typmodmap, &typmod,
								 HASH_FIND, NULL);
			if (mapent == NULL)
				elog(ERROR, "tqueue received unrecognized remote typmod %d",
					 typmod);
			remapinfo->localtypmod = mapent->localtypmod;
		}
		else
			remapinfo->localtypmod = -1;

		/* Look up tuple descriptor in typcache. */
		tupledesc = lookup_rowtype_tupdesc(typid, remapinfo->localtypmod);

		/* Figure out whether fields need recursive processing. */
		remapinfo->field_remap = BuildFieldRemapInfo(tupledesc,
													 reader->mycontext);
		if (remapinfo->field_remap != NULL)
		{
			/*
			 * We need to inspect the record contents, so save a copy of the
			 * tupdesc.  (We could possibly just reference the typcache's
			 * copy, but then it's problematic when to release the refcount.)
			 */
			MemoryContext oldcontext = MemoryContextSwitchTo(reader->mycontext);

			remapinfo->tupledesc = CreateTupleDescCopy(tupledesc);
			MemoryContextSwitchTo(oldcontext);
		}
		else
		{
			/* No fields of the record require remapping. */
			remapinfo->tupledesc = NULL;
		}
		remapinfo->rectypid = typid;
		remapinfo->rectypmod = typmod;

		/* Release reference count acquired by lookup_rowtype_tupdesc. */
		DecrTupleDescRefCount(tupledesc);
	}

	/* If transient record, replace remote typmod with local typmod. */
	if (typid == RECORDOID && typmod != remapinfo->localtypmod)
	{
		typmod = remapinfo->localtypmod;
		changed_typmod = true;
	}
	else
		changed_typmod = false;

	/*
	 * If we need to change the typmod, or if there are any potentially
	 * remappable fields, replace the tuple.
	 */
	if (changed_typmod || remapinfo->field_remap != NULL)
	{
		HeapTupleData htup;
		HeapTuple	atup;

		/* For now, assume we always need to change the tuple in this case. */
		*changed = true;

		/* Copy tuple, possibly remapping contained fields. */
		ItemPointerSetInvalid(&htup.t_self);
#if 0
		htup.t_tableOid = InvalidOid;
#endif
		htup.t_len = HeapTupleHeaderGetDatumLength(tup);
		htup.t_data = tup;
		atup = TQRemapTuple(reader,
							remapinfo->tupledesc,
							remapinfo->field_remap,
							&htup);

		/* Apply the correct labeling for a local Datum. */
		HeapTupleHeaderSetTypeId(atup->t_data, typid);
		HeapTupleHeaderSetTypMod(atup->t_data, typmod);
		HeapTupleHeaderSetDatumLength(atup->t_data, htup.t_len);

		/* And return the results. */
		return HeapTupleHeaderGetDatum(atup->t_data);
	}

	/* Else just return the value as-is. */
	return value;
}

/*
 * Handle a control message from the tuple queue reader.
 *
 * Control messages are sent when the remote side is sending tuples that
 * contain transient record types.  We need to arrange to bless those
 * record types locally and translate between remote and local typmods.
 */
static void
TupleQueueHandleControlMessage(TupleQueueReader *reader, Size nbytes,
							   char *data)
{
	int32		remotetypmod;
	int			natts;
	bool		hasoid;
	Size		offset = 0;
	Form_pg_attribute *attrs;
	TupleDesc	tupledesc;
	RecordTypmodMap *mapent;
	bool		found;
	int			i;

	/* Extract remote typmod. */
	memcpy(&remotetypmod, &data[offset], sizeof(int32));
	offset += sizeof(int32);

	/* Extract attribute count. */
	memcpy(&natts, &data[offset], sizeof(int));
	offset += sizeof(int);

	/* Extract hasoid flag. */
	memcpy(&hasoid, &data[offset], sizeof(bool));
	offset += sizeof(bool);

	/* Extract attribute details. The tupledesc made here is just transient. */
	attrs = palloc(natts * sizeof(Form_pg_attribute));
	for (i = 0; i < natts; i++)
	{
		attrs[i] = palloc(sizeof(FormData_pg_attribute));
		memcpy(attrs[i], &data[offset], sizeof(FormData_pg_attribute));
		offset += sizeof(FormData_pg_attribute);
	}

	/* We should have read the whole message. */
	Assert(offset == nbytes);

	/* Construct TupleDesc, and assign a local typmod. */
	tupledesc = CreateTupleDesc(natts, hasoid, attrs);
	tupledesc = BlessTupleDesc(tupledesc);

	/* Create mapping hashtable if it doesn't exist already. */
	if (reader->typmodmap == NULL)
	{
		HASHCTL		ctl;

		MemSet(&ctl, 0, sizeof(ctl));
		ctl.keysize = sizeof(int32);
		ctl.entrysize = sizeof(RecordTypmodMap);
		ctl.hcxt = reader->mycontext;
		reader->typmodmap = hash_create("tqueue receiver record type hashtable",
										100, &ctl,
									  HASH_ELEM | HASH_BLOBS | HASH_CONTEXT);
	}

	/* Create map entry. */
	mapent = hash_search(reader->typmodmap, &remotetypmod, HASH_ENTER,
						 &found);
	if (found)
		elog(ERROR, "duplicate tqueue control message for typmod %d",
			 remotetypmod);
	mapent->localtypmod = tupledesc->tdtypmod;

	elog(DEBUG3, "tqueue mapping remote typmod %d to local typmod %d",
		 remotetypmod, mapent->localtypmod);
}

/*
 * Build remap info for the specified data type, storing it in mycontext.
 * Returns NULL if neither the type nor any subtype could require remapping.
 */
static TupleRemapInfo *
BuildTupleRemapInfo(Oid typid, MemoryContext mycontext)
{
	HeapTuple	tup;
	Form_pg_type typ;

	/* This is recursive, so it could be driven to stack overflow. */
	check_stack_depth();

restart:
	tup = SearchSysCache1(TYPEOID, ObjectIdGetDatum(typid));
	if (!HeapTupleIsValid(tup))
		elog(ERROR, "cache lookup failed for type %u", typid);
	typ = (Form_pg_type) GETSTRUCT(tup);

	/* Look through domains to underlying base type. */
	if (typ->typtype == TYPTYPE_DOMAIN)
	{
		typid = typ->typbasetype;
		ReleaseSysCache(tup);
		goto restart;
	}

	/* If it's a true array type, deal with it that way. */
	if (OidIsValid(typ->typelem) && typ->typlen == -1)
	{
		typid = typ->typelem;
		ReleaseSysCache(tup);
		return BuildArrayRemapInfo(typid, mycontext);
	}

	/* Similarly, deal with ranges appropriately. */
	if (typ->typtype == TYPTYPE_RANGE)
	{
		ReleaseSysCache(tup);
		return BuildRangeRemapInfo(typid, mycontext);
	}

	/*
	 * If it's a composite type (including RECORD), set up for remapping.  We
	 * don't attempt to determine the status of subfields here, since we do
	 * not have enough information yet; just mark everything invalid.
	 */
	if (typ->typtype == TYPTYPE_COMPOSITE || typid == RECORDOID)
	{
		TupleRemapInfo *remapinfo;

		remapinfo = (TupleRemapInfo *)
			MemoryContextAlloc(mycontext, sizeof(TupleRemapInfo));
		remapinfo->remapclass = TQUEUE_REMAP_RECORD;
		remapinfo->u.rec.rectypid = InvalidOid;
		remapinfo->u.rec.rectypmod = -1;
		remapinfo->u.rec.localtypmod = -1;
		remapinfo->u.rec.tupledesc = NULL;
		remapinfo->u.rec.field_remap = NULL;
		ReleaseSysCache(tup);
		return remapinfo;
	}

	/* Nothing else can possibly need remapping attention. */
	ReleaseSysCache(tup);
	return NULL;
}

static TupleRemapInfo *
BuildArrayRemapInfo(Oid elemtypid, MemoryContext mycontext)
{
	TupleRemapInfo *remapinfo;
	TupleRemapInfo *element_remapinfo;

	/* See if element type requires remapping. */
	element_remapinfo = BuildTupleRemapInfo(elemtypid, mycontext);
	/* If not, the array doesn't either. */
	if (element_remapinfo == NULL)
		return NULL;
	/* OK, set up to remap the array. */
	remapinfo = (TupleRemapInfo *)
		MemoryContextAlloc(mycontext, sizeof(TupleRemapInfo));
	remapinfo->remapclass = TQUEUE_REMAP_ARRAY;
	get_typlenbyvalalign(elemtypid,
						 &remapinfo->u.arr.typlen,
						 &remapinfo->u.arr.typbyval,
						 &remapinfo->u.arr.typalign);
	remapinfo->u.arr.element_remap = element_remapinfo;
	return remapinfo;
}

static TupleRemapInfo *
BuildRangeRemapInfo(Oid rngtypid, MemoryContext mycontext)
{
	TupleRemapInfo *remapinfo;
	TupleRemapInfo *bound_remapinfo;
	TypeCacheEntry *typcache;

	/*
	 * Get range info from the typcache.  We assume this pointer will stay
	 * valid for the duration of the query.
	 */
	typcache = lookup_type_cache(rngtypid, TYPECACHE_RANGE_INFO);
	if (typcache->rngelemtype == NULL)
		elog(ERROR, "type %u is not a range type", rngtypid);

	/* See if range bound type requires remapping. */
	bound_remapinfo = BuildTupleRemapInfo(typcache->rngelemtype->type_id,
										  mycontext);
	/* If not, the range doesn't either. */
	if (bound_remapinfo == NULL)
		return NULL;
	/* OK, set up to remap the range. */
	remapinfo = (TupleRemapInfo *)
		MemoryContextAlloc(mycontext, sizeof(TupleRemapInfo));
	remapinfo->remapclass = TQUEUE_REMAP_RANGE;
	remapinfo->u.rng.typcache = typcache;
	remapinfo->u.rng.bound_remap = bound_remapinfo;
	return remapinfo;
}

/*
 * Build remap info for fields of the type described by the given tupdesc.
 * Returns an array of TupleRemapInfo pointers, or NULL if no field
 * requires remapping.  Data is allocated in mycontext.
 */
static TupleRemapInfo **
BuildFieldRemapInfo(TupleDesc tupledesc, MemoryContext mycontext)
{
	TupleRemapInfo **remapinfo;
	bool		noop = true;
	int			i;

	/* Recursively determine the remapping status of each field. */
	remapinfo = (TupleRemapInfo **)
		MemoryContextAlloc(mycontext,
						   tupledesc->natts * sizeof(TupleRemapInfo *));
	for (i = 0; i < tupledesc->natts; i++)
	{
		Form_pg_attribute attr = tupledesc->attrs[i];

		if (attr->attisdropped)
		{
			remapinfo[i] = NULL;
			continue;
		}
		remapinfo[i] = BuildTupleRemapInfo(attr->atttypid, mycontext);
		if (remapinfo[i] != NULL)
			noop = false;
	}

	/* If no fields require remapping, report that by returning NULL. */
	if (noop)
	{
		pfree(remapinfo);
		remapinfo = NULL;
	}

	return remapinfo;
=======
	return heap_copytuple(&htup);
>>>>>>> 9e1c9f959422192bbe1b842a2a1ffaf76b080196
}
