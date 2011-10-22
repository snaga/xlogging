/*
 * contrib/logging/logging.c
 *
 *
 * logging
 *
 * Copyright (c) 2011 Satoshi Nagayasu <satoshi.nagayasu@gmail.com>
 *
 * Permission to use, copy, modify, and distribute this software and
 * its documentation for any purpose, without fee, and without a
 * written agreement is hereby granted, provided that the above
 * copyright notice and this paragraph and the following two
 * paragraphs appear in all copies.
 *
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE TO ANY PARTY FOR DIRECT,
 * INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING
 * LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS
 * DOCUMENTATION, EVEN IF THE UNIVERSITY OF CALIFORNIA HAS BEEN ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * THE AUTHOR SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS ON AN "AS
 * IS" BASIS, AND THE AUTHOR HAS NO OBLIGATIONS TO PROVIDE MAINTENANCE,
 * SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 */

#include "postgres.h"

#include "access/heapam.h"
#include "catalog/indexing.h"
#include "catalog/pg_class.h"
#include "funcapi.h"
#include "miscadmin.h"
#include "utils/builtins.h"
#include "utils/rel.h"
#include "utils/syscache.h"

PG_MODULE_MAGIC;

extern Datum enable_logging(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(enable_logging);

static bool
update_catalog(Oid relid, bool enable_logging)
{
	Relation r;

	Relation relrel;
	HeapTuple reltup;
	Form_pg_class relform;
	
	r = relation_open(relid, AccessExclusiveLock);

	/* ------------------------------------
	 * Update the system catalog
	 * ------------------------------------
	 */
	relrel = heap_open(RelationRelationId, RowExclusiveLock);
	
	reltup = SearchSysCacheCopy1(RELOID, ObjectIdGetDatum(relid));
	if (!HeapTupleIsValid(reltup))		/* shouldn't happen */
		elog(ERROR, "cache lookup failed for relation %u", relid);
	relform = (Form_pg_class) GETSTRUCT(reltup);
		
	relform->relpersistence = (enable_logging ? RELPERSISTENCE_PERMANENT : RELPERSISTENCE_UNLOGGED);

	simple_heap_update(relrel, &reltup->t_self, reltup);
	
	CatalogUpdateIndexes(relrel, reltup);
	
	heap_freetuple(reltup);
	heap_close(relrel, RowExclusiveLock);

	relation_close(r, NoLock);

	return true;
}


/* ------------------------------------------------------
 * enable_logging()
 *
 * Usage: SELECT enable_logging(oid);
 * ------------------------------------------------------
 */
Datum
enable_logging(PG_FUNCTION_ARGS)
{
	Oid relid = PG_GETARG_OID(0);
	bool mode = PG_GETARG_BOOL(1);
	bool rc = false;

	Relation rel;
	char relpersistence;
	List *indexoidlist;
	ListCell *indexoid;

	if (!superuser())
		ereport(ERROR,
				(errcode(ERRCODE_INSUFFICIENT_PRIVILEGE),
				 (errmsg("must be superuser to use enable_logging functions"))));

	rel = relation_open(relid, AccessExclusiveLock);
	relpersistence = rel->rd_rel->relpersistence;
	
	if ( (relpersistence == RELPERSISTENCE_PERMANENT && mode == false) ||
	     (relpersistence == RELPERSISTENCE_UNLOGGED && mode == true) )
	{
		/* toggle wal logging. */
		update_catalog(relid, mode);

		indexoidlist = RelationGetIndexList(rel);

		foreach(indexoid, indexoidlist)
		{
			Oid indoid = lfirst_oid(indexoid);

			update_catalog(indoid, mode);
		}

		list_free(indexoidlist);

		rc = true;
	}

	relation_close(rel, NoLock);

	PG_RETURN_BOOL(rc);
}
