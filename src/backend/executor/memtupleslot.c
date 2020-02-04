// GPDB_12_MERGE_FIXME: Here we should implement the TupleTableSlotOps functions for
// dealing with MemTuples. For now, this just contains a couple of functions that used
// to be in execTuples.c.

MemTuple
ExecFetchSlotMemTuple(TupleTableSlot *slot)
{
	MemTuple newTuple;
	uint32 tuplen;

	Assert(!TupIsNull(slot));
	Assert(slot->tts_mt_bind);

	if(slot->PRIVATE_tts_memtuple)
		return slot->PRIVATE_tts_memtuple;

	slot_getallattrs(slot);

	tuplen = slot->PRIVATE_tts_mtup_buf_len;
	newTuple = memtuple_form_to(slot->tts_mt_bind, slot_get_values(slot), slot_get_isnull(slot),
				(MemTuple) slot->PRIVATE_tts_mtup_buf, &tuplen, false);

	if(!newTuple)
	{
		if(slot->PRIVATE_tts_mtup_buf)
			pfree(slot->PRIVATE_tts_mtup_buf);

		slot->PRIVATE_tts_mtup_buf = MemoryContextAlloc(slot->tts_mcxt, tuplen);
		slot->PRIVATE_tts_mtup_buf_len = tuplen;

		newTuple = memtuple_form_to(slot->tts_mt_bind, slot_get_values(slot), slot_get_isnull(slot),
			(MemTuple) slot->PRIVATE_tts_mtup_buf, &tuplen, false);
	}

	Assert(newTuple);
	slot->PRIVATE_tts_memtuple = newTuple;

	return newTuple;
}

/* XXX
 * This function is not very efficient.  We should detech if we can modify
 * the memtuple inline so no deform/form is needed
 */
void ExecModifyMemTuple(TupleTableSlot *slot, Datum *values, bool *isnull, bool *doRepl)
{
	int i;
	MemTuple mtup;
	uint32 tuplen;
	Assert(slot->PRIVATE_tts_memtuple);

	/* First, get all the attrs.  Note we set PRIVATE_tts_nvalid to 0
	 * so we force the attrs are from memtuple
	 */
	slot->PRIVATE_tts_nvalid = 0;
	slot_getallattrs(slot);
	
	/* Next, we construct a new memtuple, on the htup buf to avoid palloc */
	slot->PRIVATE_tts_heaptuple = NULL;
	for(i = 0; i<slot->tts_tupleDescriptor->natts; ++i)
	{
		if(doRepl[i])
		{
			slot->PRIVATE_tts_values[i] = values[i];
			slot->PRIVATE_tts_isnull[i] = isnull[i];
		}
	}

	tuplen = slot->PRIVATE_tts_htup_buf_len;
	mtup = memtuple_form_to(slot->tts_mt_bind, slot->PRIVATE_tts_values, slot->PRIVATE_tts_isnull,
			slot->PRIVATE_tts_htup_buf, &tuplen, false);
	if(!mtup)
	{
		slot->PRIVATE_tts_htup_buf = MemoryContextAlloc(slot->tts_mcxt, tuplen);
		slot->PRIVATE_tts_htup_buf_len = tuplen;

		mtup = memtuple_form_to(slot->tts_mt_bind, slot->PRIVATE_tts_values, slot->PRIVATE_tts_isnull,
			slot->PRIVATE_tts_htup_buf, &tuplen, false);

		Assert(mtup);
	}

	/* Check if we need to free this mem tuple */
	if(TupShouldFree(slot)
			&& slot->PRIVATE_tts_memtuple
			&& slot->PRIVATE_tts_memtuple != slot->PRIVATE_tts_mtup_buf
	  )
		pfree(slot->PRIVATE_tts_memtuple);

	slot->PRIVATE_tts_memtuple = mtup;
	/* swap mtup_buf and htup_buf stuff */

	mtup = (MemTuple) slot->PRIVATE_tts_mtup_buf;
	tuplen = slot->PRIVATE_tts_mtup_buf_len;

	slot->PRIVATE_tts_mtup_buf = slot->PRIVATE_tts_htup_buf;
	slot->PRIVATE_tts_mtup_buf_len = slot->PRIVATE_tts_htup_buf_len;
	slot->PRIVATE_tts_htup_buf = (void *) mtup;
	slot->PRIVATE_tts_htup_buf_len = tuplen;

	/* don't forget to reset PRIVATE_tts_nvalid, because we modified the memtuple */
	slot->PRIVATE_tts_nvalid = 0;
}
