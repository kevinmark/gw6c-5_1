/*
---------------------------------------------------------------------------
 $Id: xmlparse.h,v 1.4 2007/05/02 13:32:23 cnepveu Exp $
---------------------------------------------------------------------------
  Copyright (c) 2007 Hexago Inc. All rights reserved.

  LICENSE NOTICE: You may use and modify this source code only if you
  have executed a valid license agreement with Hexago Inc. granting
  you the right to do so, the said license agreement governing such
  use and modifications.   Copyright or other intellectual property
  notices are not to be removed from the source code.
---------------------------------------------------------------------------
*/

#ifndef XMLPARSE_H
#define XMLPARSE_H

#ifdef XMLPARSE
# define ACCESS
#else
# define ACCESS extern
#endif

/*
 * In the node structure, the following identify how many attributes a node can
 * have max. It can be adjusted at compile time.
 */

#define MAX_ATTRIBUTES 5

/*
 * The following identify the format of a node processing function that must be
 * supplied in the node structure to take care of the information supplied in the
 * structure.
 */

struct stNode;

typedef int processNode(struct stNode *n, char *content);

/*
 * The node structure contains the relevant information of each node to be
 * parsed by each invocation of the parser.
 */

typedef struct stAttribute {
  char *name;
  char *value;
} tAttribute;

typedef struct stNode {
  char *name;
  processNode *p;
  tAttribute attributes[MAX_ATTRIBUTES + 1];
} tNode;

/*
 * The XMLParser function is responsible of parsing the contain of a string,
 * calling the appropriate processNode function when a node token is recognized.
 *
 * Parameters:
 *
 *
 * Return value:
 *
 *    0 - Success
 *   -1 - Memory allocation error
 *    n - Parsing error at position n in the string
 */

ACCESS int XMLParse(char *str, tNode nodes[]);

/*
 * The following macros are usefull to declare the node structures
 *
 * They assume that the event functions will be named the same as the
 * XML tags, prefixed with p_
 *
 */

#define NODE(n) { #n, p_##n, {
#define ENDNODE { "", 0 } } },
#define ATTR(a) { #a, 0 },
#define ENDLIST { "", 0, { { "", 0 } } } };
#define STARTLIST {

#define PROC(pr) int p_##pr (tNode *n, char *content)

#undef ACCESS

#endif
