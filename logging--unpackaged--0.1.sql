/* contrib/pgstattuple/pgstattuple--unpackaged--1.0.sql */

-- complain if script is sourced in psql, rather than via CREATE EXTENSION
\echo Use "CREATE EXTENSION logging" to load this file. \quit

ALTER EXTENSION logging ADD function enable_logging(oid, bool);
