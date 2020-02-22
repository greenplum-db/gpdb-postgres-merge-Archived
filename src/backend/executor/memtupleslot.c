// GPDB_12_MERGE_FIXME: Here we should implement the TupleTableSlotOps functions for
// dealing with MemTuples. For now, this just contains a couple of functions that used
// to be in execTuples.c.

MemTuple
ExecFetchSlotMemTuple(TupleTableSlot *slot, bool *shouldFree)
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

