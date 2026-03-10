/*-------------------------------------------------------------------------
 * Copyright 2025 IvorySQL Global Development Team
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * ora_aggregate_functions.c
 *
 * Oracle-compatible aggregate function support.
 *
 * Currently implements:
 *   sys.listagg_check(text) -> text
 *     Enforces Oracle's VARCHAR2 maximum length (4000 bytes) on the result
 *     of a LISTAGG aggregate.  Raises ORA-01489-equivalent error on overflow.
 *
 * Portions Copyright (c) 2023-2026, IvorySQL Global Development Team
 *
 * contrib/ivorysql_ora/src/builtin_functions/ora_aggregate_functions.c
 *
 *-------------------------------------------------------------------------
 */

#include "postgres.h"
#include "fmgr.h"
#include "utils/builtins.h"
#include "varatt.h"

/* Maximum result length for Oracle's VARCHAR2 data type (bytes) */
#define ORA_VARCHAR2_MAX_LEN	4000

PG_FUNCTION_INFO_V1(ora_listagg_check);

/*
 * ora_listagg_check(text) -> text
 *
 * Validates that the LISTAGG result does not exceed Oracle's VARCHAR2 maximum
 * length of 4000 bytes.  If the result is longer, raises an error matching
 * Oracle's ORA-01489 "result of string concatenation is too long".
 *
 * This function is injected as a wrapper around string_agg() by the Oracle
 * parser when it processes the LISTAGG(...) WITHIN GROUP (ORDER BY ...) syntax.
 */
Datum
ora_listagg_check(PG_FUNCTION_ARGS)
{
	text	   *result;
	int			len;

	if (PG_ARGISNULL(0))
		PG_RETURN_NULL();

	result = PG_GETARG_TEXT_PP(0);
	len = VARSIZE_ANY_EXHDR(result);

	if (len > ORA_VARCHAR2_MAX_LEN)
		ereport(ERROR,
				(errcode(ERRCODE_STRING_DATA_RIGHT_TRUNCATION),
				 errmsg("result of string concatenation is too long"),
				 errdetail("The LISTAGG result is %d bytes, which exceeds the "
						   "maximum length for VARCHAR2 (%d bytes).",
						   len, ORA_VARCHAR2_MAX_LEN)));

	PG_RETURN_TEXT_P(result);
}
