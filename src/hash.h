/* $Id: hash.h,v 1.4 2005/12/03 18:29:20 rockyb Exp $
   hash.h -- decls for hash table
   Copyright (C) 1995, 1999, 2002, 2004, 2005 Free Software Foundation, Inc.
   Written by Greg McGary <gkm@gnu.org> <greg@mcgary.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA. */

#ifndef _HASH_H__
#define _HASH_H__

#include <stdio.h>
#include <ctype.h>

typedef unsigned long (*hash_func_t) (void const *key);
typedef int (*hash_cmp_func_t) (void const *x, void const *y);
typedef void (*hash_map_func_t) (void const *item);
typedef void (*hash_map_arg_func_t) (void const *item, void *arg);

typedef struct {
  void **ht_vec;
  unsigned long   ht_size;	 /* total number of slots (power of 2) */
  unsigned long   ht_capacity;	 /* usable slots, limited by loading-factor */
  unsigned long   ht_fill;	 /* items in table */
  unsigned long   ht_empty_slots;/* empty slots not including deleted slots */
  unsigned long   ht_collisions; /* # of failed calls to comparison function */
  unsigned long   ht_lookups;	 /* # of queries */
  unsigned int    ht_rehashes;	 /* # of times we've expanded table */
  hash_func_t     ht_hash_1;	 /* primary hash function */
  hash_func_t     ht_hash_2;	 /* secondary hash function */
  hash_cmp_func_t ht_compare;	 /* comparison function */
} hash_table_t;

hash_table_t files;

typedef int (*qsort_cmp_t) (void const *, void const *);

void hash_init(hash_table_t *ht, unsigned long size,
	       hash_func_t hash_1, hash_func_t hash_2, 
	       hash_cmp_func_t hash_cmp);
void hash_load (hash_table_t *ht, void *item_table, unsigned long cardinality, 
		unsigned long size);
void **hash_find_slot (hash_table_t *ht, void const *key);
void *hash_find_item (hash_table_t *ht, void const *key);
void *hash_insert (hash_table_t *ht, void *item);
void *hash_insert_at (hash_table_t *ht, void *item, void const *slot);
void *hash_delete (hash_table_t *ht, void const *item);
void *hash_delete_at (hash_table_t *ht, void const *slot);
void hash_delete_items (hash_table_t *ht);
void hash_free_items (hash_table_t *ht);
void hash_free (hash_table_t *ht, int free_items);
void hash_map (hash_table_t *ht, hash_map_func_t map);
void hash_map_arg (hash_table_t *ht, hash_map_arg_func_t map, 
		       void *arg);
void hash_print_stats (hash_table_t *ht, FILE *out_FILE);
void **hash_dump (hash_table_t *ht, void **vector_0, qsort_cmp_t compare);

extern void *hash_deleted_item;
#define HASH_VACANT(item) ((item) == 0 || (void *) (item) == hash_deleted_item)


/* hash and comparison macros for case-sensitive string keys. */

#define STRING_HASH_1(KEY, RESULT) do { \
  unsigned char const *_key_ = (unsigned char const *) (KEY) - 1; \
  while (*++_key_) \
    (RESULT) += (*_key_ << (_key_[1] & 0xf)); \
} while (0)
#define return_STRING_HASH_1(KEY) do { \
  unsigned long _result_ = 0; \
  STRING_HASH_1 ((KEY), _result_); \
  return _result_; \
} while (0)

#define STRING_HASH_2(KEY, RESULT) do { \
  unsigned char const *_key_ = (unsigned char const *) (KEY) - 1; \
  while (*++_key_) \
    (RESULT) += (*_key_ << (_key_[1] & 0x7)); \
} while (0)
#define return_STRING_HASH_2(KEY) do { \
  unsigned long _result_ = 0; \
  STRING_HASH_2 ((KEY), _result_); \
  return _result_; \
} while (0)

#define STRING_COMPARE(X, Y, RESULT) do { \
  RESULT = strcmp ((X), (Y)); \
} while (0)
#define return_STRING_COMPARE(X, Y) do { \
  return strcmp ((X), (Y)); \
} while (0)


#define STRING_N_HASH_1(KEY, N, RESULT) do { \
  unsigned char const *_key_ = (unsigned char const *) (KEY) - 1; \
  int _n_ = (N); \
  if (_n_) \
    while (--_n_ && *++_key_) \
      (RESULT) += (*_key_ << (_key_[1] & 0xf)); \
  (RESULT) += *++_key_; \
} while (0)
#define return_STRING_N_HASH_1(KEY, N) do { \
  unsigned long _result_ = 0; \
  STRING_N_HASH_1 ((KEY), (N), _result_); \
  return _result_; \
} while (0)

#define STRING_N_HASH_2(KEY, N, RESULT) do { \
  unsigned char const *_key_ = (unsigned char const *) (KEY) - 1; \
  int _n_ = (N); \
  if (_n_) \
    while (--_n_ && *++_key_) \
      (RESULT) += (*_key_ << (_key_[1] & 0x7)); \
  (RESULT) += *++_key_; \
} while (0)
#define return_STRING_N_HASH_2(KEY, N) do { \
  unsigned long _result_ = 0; \
  STRING_N_HASH_2 ((KEY), (N), _result_); \
  return _result_; \
} while (0)

#define STRING_N_COMPARE(X, Y, N, RESULT) do { \
  RESULT = strncmp ((X), (Y), (N)); \
} while (0)
#define return_STRING_N_COMPARE(X, Y, N) do { \
  return strncmp ((X), (Y), (N)); \
} while (0)

#ifdef HAVE_CASE_INSENSITIVE_FS

/* hash and comparison macros for case-insensitive string _key_s. */

#define ISTRING_HASH_1(KEY, RESULT) do { \
  unsigned char const *_key_ = (unsigned char const *) (KEY) - 1; \
  while (*++_key_) \
    (RESULT) += ((isupper (*_key_) ? tolower (*_key_) : *_key_) << (_key_[1] & 0xf)); \
} while (0)
#define return_ISTRING_HASH_1(KEY) do { \
  unsigned long _result_ = 0; \
  ISTRING_HASH_1 ((KEY), _result_); \
  return _result_; \
} while (0)

#define ISTRING_HASH_2(KEY, RESULT) do { \
  unsigned char const *_key_ = (unsigned char const *) (KEY) - 1; \
  while (*++_key_) \
    (RESULT) += ((isupper (*_key_) ? tolower (*_key_) : *_key_) << (_key_[1] & 0x7)); \
} while (0)
#define return_ISTRING_HASH_2(KEY) do { \
  unsigned long _result_ = 0; \
  ISTRING_HASH_2 ((KEY), _result_); \
  return _result_; \
} while (0)

#define ISTRING_COMPARE(X, Y, RESULT) do { \
  RESULT = strcmpi ((X), (Y)); \
} while (0)
#define return_ISTRING_COMPARE(X, Y) do { \
  return strcmpi ((X), (Y)); \
} while (0)

#else

#define ISTRING_HASH_1(KEY, RESULT) STRING_HASH_1 ((KEY), (RESULT))
#define return_ISTRING_HASH_1(KEY) return_STRING_HASH_1 (KEY)

#define ISTRING_HASH_2(KEY, RESULT) STRING_HASH_2 ((KEY), (RESULT))
#define return_ISTRING_HASH_2(KEY) return_STRING_HASH_2 (KEY)

#define ISTRING_COMPARE(X, Y, RESULT) STRING_COMPARE ((X), (Y), (RESULT))
#define return_ISTRING_COMPARE(X, Y) return_STRING_COMPARE ((X), (Y))

#endif

/* hash and comparison macros for integer _key_s. */

#define INTEGER_HASH_1(KEY, RESULT) do { \
  (RESULT) += ((unsigned long)(KEY)); \
} while (0)
#define return_INTEGER_HASH_1(KEY) do { \
  unsigned long _result_ = 0; \
  INTEGER_HASH_1 ((KEY), _result_); \
  return _result_; \
} while (0)

#define INTEGER_HASH_2(KEY, RESULT) do { \
  (RESULT) += ~((unsigned long)(KEY)); \
} while (0)
#define return_INTEGER_HASH_2(KEY) do { \
  unsigned long _result_ = 0; \
  INTEGER_HASH_2 ((KEY), _result_); \
  return _result_; \
} while (0)

#define INTEGER_COMPARE(X, Y, RESULT) do { \
  (RESULT) = X - Y; \
} while (0)
#define return_INTEGER_COMPARE(X, Y) do { \
  int _result_; \
  INTEGER_COMPARE (X, Y, _result_); \
  return _result_; \
} while (0)

/* hash and comparison macros for address keys. */

#define ADDRESS_HASH_1(KEY, RESULT) INTEGER_HASH_1 (((unsigned long)(KEY)) >> 3, (RESULT))
#define ADDRESS_HASH_2(KEY, RESULT) INTEGER_HASH_2 (((unsigned long)(KEY)) >> 3, (RESULT))
#define ADDRESS_COMPARE(X, Y, RESULT) INTEGER_COMPARE ((X), (Y), (RESULT))
#define return_ADDRESS_HASH_1(KEY) return_INTEGER_HASH_1 (((unsigned long)(KEY)) >> 3)
#define return_ADDRESS_HASH_2(KEY) return_INTEGER_HASH_2 (((unsigned long)(KEY)) >> 3)
#define return_ADDRESS_COMPARE(X, Y) return_INTEGER_COMPARE ((X), (Y))

#endif /* _HASH_H__ */
