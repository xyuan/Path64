/*
 * Copyright 2005, 2006 PathScale, Inc.  All Rights Reserved.
 */

/*

  Copyright (C) 2000, 2001 Silicon Graphics, Inc.  All Rights Reserved.

   Path64 is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   Path64 is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with Path64; see the file COPYING.  If not, write to the Free
   Software Foundation, 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.

   Special thanks goes to SGI for their continued support to open source

*/


// isa_lits_gen.cxx
/////////////////////////////////////
//
//  Generate a list of lit classes and their values.
//
/////////////////////////////////////
//
//  $Revision: 1.4 $
//  $Date: 05/05/06 08:20:23-07:00 $
//  $Author: bos@eng-25.pathscale.com $
//  $Source: common/targ_info/generate/SCCS/s.isa_lits_gen.cxx $

#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <assert.h>
#include <limits.h>
#include "gen_util.h"
#include "isa_lits_gen.h"

struct lit_range {
  const char *name;
  long long min;
  long long max;
};

static const char * const interface[] = {
  "/* ====================================================================",
  " * ====================================================================",
  " *",
  " * Description:",
  " *",
  " *   A list of all the lit classes used in an ISA.",
  " *   It exports the following:",
  " *",
  " *   typedef (enum) ISA_LIT_CLASS",
  " *       An enumeration of the lit classes.",
  " *",
  " *   typedef (struct) ISA_LIT_CLASS_INFO",
  " *       Contains info about first and last ECV in the EC.",
  " *       The contents are private.",
  " *",
  " *   typedef (struct) ISA_LIT_CLASS_VALUE_INFO",
  " *       Contains info about name and min/max of the LC.",
  " *       The contents are private.",
  " *",
  " *   const char * ISA_LC_Name (ISA_LIT_CLASS lc)",
  " *       Returns name of <lc>.",
  " *",
  " *   INT64 ISA_LC_Min (ISA_LIT_CLASS lc)",
  " *       Returns the minimum value for the specified <lc>. For classes",
  " *       that have multiple sub-ranges, ISA_LC_Min returns the smallest",
  " *       minimum of all the sub-ranges.",
  " *",
  " *   INT64 ISA_LC_Max (ISA_LIT_CLASS lc)",
  " *       Returns the maximum value for the specified <lc>. For classes",
  " *       that have multiple sub-ranges, ISA_LC_Max returns the largest",
  " *       maximum of all the sub-ranges.",
  " *",
  " *   BOOL ISA_LC_Is_Signed (ISA_LIT_CLASS lc)",
  " *       Returns whether the lit-class <lc> is signed.",
  " *",
  " *   BOOL ISA_LC_Value_In_Class (INT64 val, ISA_LIT_CLASS lc)",
  " *       Returns whether <val> is a value that belongs to <lc>.",
  " *",
  " * ====================================================================",
  " * ====================================================================",
  " */",
  NULL
};

static FILE *hfile, *cfile, *efile;
static struct lit_range signed_range[65];
static struct lit_range unsigned_range[65];
static int max_ranges = 0;

/////////////////////////////////////
void ISA_Lits_Begin (void)
/////////////////////////////////////
//  See interface description.
/////////////////////////////////////
{
#define FNAME "targ_isa_lits"
  char buf[1000];
  sprintf (buf, "%s.h", FNAME);
  hfile = fopen(buf, "w");
  sprintf (buf, "%s.c", FNAME);
  cfile = fopen(buf, "w");
  sprintf (buf, "%s.Exported", FNAME);
  efile = fopen(buf, "w");

  fprintf(cfile,"#include \"%s.h\"\n\n", FNAME);

  sprintf (buf, "%s", FNAME);
  Emit_Header (hfile, buf, interface);

  fprintf(hfile, "\ntypedef enum {\n");
  // start with undefined value
  fprintf(hfile, "\tLC_UNDEFINED,\n");

  fprintf(cfile, "const ISA_LIT_CLASS_INFO ISA_LIT_CLASS_info[] = {\n");
  fprintf(cfile, "  { { { 0x0000000000000000LL, 0x0000000000000000LL } }, 0, 0, \"LC_UNDEFINED\" },\n");

  for (int i = 1; i <= 64; ++i) {
    unsigned_range[i].min = 0;
    unsigned_range[i].max = (i == 64) ? -1ULL : (1ULL << i) - 1;

    signed_range[i].min = -1LL << (i - 1);
    signed_range[i].max = (1LL << (i - 1)) - 1;
  }
}


/////////////////////////////////////
LIT_RANGE SignedBitRange(unsigned int bit_size)
/////////////////////////////////////
//  See interface description.
/////////////////////////////////////
{
  if (bit_size == 0 || bit_size > 64) {
    fprintf(stderr, "### Error: invalid signed bit range: %d\n", bit_size);
    exit(EXIT_FAILURE);
  }
  return &signed_range[bit_size];
}


/////////////////////////////////////
LIT_RANGE UnsignedBitRange(unsigned int bit_size)
/////////////////////////////////////
//  See interface description.
/////////////////////////////////////
{
  if (bit_size == 0 || bit_size > 64) {
    fprintf(stderr, "### Error: invalid unsigned bit range: %d\n", bit_size);
    exit(EXIT_FAILURE);
  }
  return &unsigned_range[bit_size];
}


/////////////////////////////////////
LIT_RANGE ISA_Create_Lit_Range(const char *name, long long min, long long max)
/////////////////////////////////////
//  See interface description.
/////////////////////////////////////
{
  LIT_RANGE range = new lit_range;
  range->name = name;
  range->min = min;
  range->max = max;
  return range;
}


/////////////////////////////////////
void ISA_Create_Lit_Class(const char* name, LIT_CLASS_TYPE type, ...)
/////////////////////////////////////
//  See interface description.
/////////////////////////////////////
{
  va_list ap;
  LIT_RANGE range;
  bool is_signed = type == SIGNED;
  long long min = is_signed ? LONG_LONG_MAX : ULONG_LONG_MAX;
  long long max = is_signed ? LONG_LONG_MIN : 0;
  int num_ranges = 0;

  // Find the smallest min and largest max for all ranges, and
  // count the number of ranges.
  va_start(ap,type);
  while ((range = va_arg(ap,LIT_RANGE)) != LIT_RANGE_END) {
    ++num_ranges;
    if (is_signed) {
      if (range->min < min) min = range->min;
      if (range->max > max) max = range->max;
    } else {
      if ((unsigned long long)range->min < (unsigned long long)min) {
	min = range->min;
      }
      if ((unsigned long long)range->max > (unsigned long long)max) {
	max = range->max;
      }
    }
  }
  va_end(ap);
  if (num_ranges > max_ranges) max_ranges = num_ranges;

  // Initialize ISA_LIT_CLASS_info for this class. Note that .range[0]
  // holds the smallest min/largest max; .range[1] is the first sub-range.
  fprintf(hfile, "\tLC_%s,\n", name);
  fprintf(cfile, "  { { { 0x%016llxLL, 0x%016llxLL }", min, max);
  va_start(ap,type);
  while ((range = va_arg(ap,LIT_RANGE)) != LIT_RANGE_END) {
    fprintf(cfile, ",\n      { 0x%016llxLL, 0x%016llxLL }", 
		   range->min, range->max);
  }
  va_end(ap);
  fprintf(cfile, " }, %d, %d, \"LC_%s\" },\n",
		 num_ranges, is_signed, name);
}


/////////////////////////////////////
void ISA_Lits_End(void)
/////////////////////////////////////
//  See interface description.
/////////////////////////////////////
{
  fprintf(hfile, "\tLC_MAX\n");
  fprintf(hfile, "} ISA_LIT_CLASS;\n");

  fprintf(cfile, "};\n");

  fprintf(hfile, "\ntypedef struct {\n"
		"  struct { INT64 min; INT64 max; } range[%d];\n"
		"  mUINT8 num_ranges;\n"
		"  mBOOL is_signed;\n"
		"  const char *name;\n"
		"} ISA_LIT_CLASS_INFO;\n",
		max_ranges + 1);
  fprintf(efile, "ISA_LIT_CLASS_info\n");

  fprintf(hfile, "\ninline const char * ISA_LC_Name (ISA_LIT_CLASS lc)\n"
		 "{\n"
		 "  extern const ISA_LIT_CLASS_INFO ISA_LIT_CLASS_info[];\n"
		 "  return ISA_LIT_CLASS_info[lc].name;\n"
		 "}\n");

  fprintf(hfile, "\ninline INT64 ISA_LC_Min (ISA_LIT_CLASS lc)\n"
		 "{\n"
		 "  extern const ISA_LIT_CLASS_INFO ISA_LIT_CLASS_info[];\n"
		 "  return ISA_LIT_CLASS_info[lc].range[0].min;\n"
		 "}\n");

  fprintf(hfile, "\ninline INT64 ISA_LC_Max (ISA_LIT_CLASS lc)\n"
		 "{\n"
		 "  extern const ISA_LIT_CLASS_INFO ISA_LIT_CLASS_info[];\n"
		 "  return ISA_LIT_CLASS_info[lc].range[0].max;\n"
		 "}\n");

  fprintf(hfile, "\ninline BOOL ISA_LC_Is_Signed (ISA_LIT_CLASS lc)\n"
		 "{\n"
		 "  extern const ISA_LIT_CLASS_INFO ISA_LIT_CLASS_info[];\n"
		 "  return ISA_LIT_CLASS_info[lc].is_signed;\n"
		 "}\n");

  fprintf(hfile, "\ninline BOOL ISA_LC_Value_In_Class (INT64 val, ISA_LIT_CLASS lc)\n"
		 "{\n"
		 "  extern const ISA_LIT_CLASS_INFO ISA_LIT_CLASS_info[];\n"
		 "  const ISA_LIT_CLASS_INFO *plc = ISA_LIT_CLASS_info + lc;\n"
		 "  INT i;\n"
		 "  for (i = 1; i <= plc->num_ranges; ++i) {\n"
		 "    INT64 min = plc->range[i].min;\n"
		 "    INT64 max = plc->range[i].max;\n"
		 "    if ( plc->is_signed ) {\n"
		 "      if (val >= min && val <= max) return TRUE;\n"
		 "    } else {\n"
		 "      if ((UINT64)val >= (UINT64)min && (UINT64)val <= (UINT64)max) return TRUE;\n"
		 "    }\n"
		 "  }\n"
		 "  return FALSE;\n"
		 "}\n");

  Emit_Footer (hfile);
}
