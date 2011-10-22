/* contrib/logging/logging--packaged--0.1.sql */

-- complain if script is sourced in psql, rather than via CREATE EXTENSION
\echo Use "CREATE EXTENSION logging" to load this file. \quit

CREATE FUNCTION enable_logging(IN relid OID,
    IN mode BOOL,
    OUT result BOOL)
AS 'MODULE_PATHNAME', 'enable_logging'
LANGUAGE C STRICT;


