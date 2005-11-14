/*
 * File:          sidl_PostViolation_Impl.h
 * Symbol:        sidl.PostViolation-v0.9.3
 * Symbol Type:   class
 * Babel Version: 0.10.12
 * Release:       $Name$
 * Revision:      @(#) $Id$
 * Description:   Server-side implementation for sidl.PostViolation
 * 
 * Copyright (c) 2000-2002, The Regents of the University of California.
 * Produced at the Lawrence Livermore National Laboratory.
 * Written by the Components Team <components@llnl.gov>
 * All rights reserved.
 * 
 * This file is part of Babel. For more information, see
 * http://www.llnl.gov/CASC/components/. Please read the COPYRIGHT file
 * for Our Notice and the LICENSE file for the GNU Lesser General Public
 * License.
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License (as published by
 * the Free Software Foundation) version 2.1 dated February 1999.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the IMPLIED WARRANTY OF
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the terms and
 * conditions of the GNU Lesser General Public License for more details.
 * 
 * You should have recieved a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 * 
 * WARNING: Automatically generated; only changes within splicers preserved
 * 
 * babel-version = 0.10.12
 */

#ifndef included_sidl_PostViolation_Impl_h
#define included_sidl_PostViolation_Impl_h

#ifndef included_sidl_header_h
#include "sidl_header.h"
#endif
#ifndef included_sidl_SIDLException_h
#include "sidl_SIDLException.h"
#endif
#ifndef included_sidl_PostViolation_h
#include "sidl_PostViolation.h"
#endif
#ifndef included_sidl_ClassInfo_h
#include "sidl_ClassInfo.h"
#endif
#ifndef included_sidl_BaseInterface_h
#include "sidl_BaseInterface.h"
#endif
#ifndef included_sidl_BaseException_h
#include "sidl_BaseException.h"
#endif
#ifndef included_sidl_BaseClass_h
#include "sidl_BaseClass.h"
#endif

#line 63 "../../../babel/runtime/sidl/sidl_PostViolation_Impl.h"
/* DO-NOT-DELETE splicer.begin(sidl.PostViolation._includes) */
/* Put additional include files here... */
/* DO-NOT-DELETE splicer.end(sidl.PostViolation._includes) */
#line 67 "sidl_PostViolation_Impl.h"

/*
 * Private data for class sidl.PostViolation
 */

struct sidl_PostViolation__data {
#line 72 "../../../babel/runtime/sidl/sidl_PostViolation_Impl.h"
  /* DO-NOT-DELETE splicer.begin(sidl.PostViolation._data) */
  /* Put private data members here... */
  int ignore; /* dummy to force non-empty struct; remove if you add data */
  /* DO-NOT-DELETE splicer.end(sidl.PostViolation._data) */
#line 79 "sidl_PostViolation_Impl.h"
};

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Access functions for class private data and built-in methods
 */

extern struct sidl_PostViolation__data*
sidl_PostViolation__get_data(
  sidl_PostViolation);

extern void
sidl_PostViolation__set_data(
  sidl_PostViolation,
  struct sidl_PostViolation__data*);

extern
void
impl_sidl_PostViolation__load(
  void);

extern
void
impl_sidl_PostViolation__ctor(
  /* in */ sidl_PostViolation self);

extern
void
impl_sidl_PostViolation__dtor(
  /* in */ sidl_PostViolation self);

/*
 * User-defined object methods
 */

extern struct sidl_SIDLException__object* 
  impl_sidl_PostViolation_fconnect_sidl_SIDLException(char* url,
  sidl_BaseInterface *_ex);
extern char* impl_sidl_PostViolation_fgetURL_sidl_SIDLException(struct 
  sidl_SIDLException__object* obj);
extern struct sidl_PostViolation__object* 
  impl_sidl_PostViolation_fconnect_sidl_PostViolation(char* url,
  sidl_BaseInterface *_ex);
extern char* impl_sidl_PostViolation_fgetURL_sidl_PostViolation(struct 
  sidl_PostViolation__object* obj);
extern struct sidl_ClassInfo__object* 
  impl_sidl_PostViolation_fconnect_sidl_ClassInfo(char* url,
  sidl_BaseInterface *_ex);
extern char* impl_sidl_PostViolation_fgetURL_sidl_ClassInfo(struct 
  sidl_ClassInfo__object* obj);
extern struct sidl_BaseInterface__object* 
  impl_sidl_PostViolation_fconnect_sidl_BaseInterface(char* url,
  sidl_BaseInterface *_ex);
extern char* impl_sidl_PostViolation_fgetURL_sidl_BaseInterface(struct 
  sidl_BaseInterface__object* obj);
extern struct sidl_BaseException__object* 
  impl_sidl_PostViolation_fconnect_sidl_BaseException(char* url,
  sidl_BaseInterface *_ex);
extern char* impl_sidl_PostViolation_fgetURL_sidl_BaseException(struct 
  sidl_BaseException__object* obj);
extern struct sidl_BaseClass__object* 
  impl_sidl_PostViolation_fconnect_sidl_BaseClass(char* url,
  sidl_BaseInterface *_ex);
extern char* impl_sidl_PostViolation_fgetURL_sidl_BaseClass(struct 
  sidl_BaseClass__object* obj);
extern struct sidl_SIDLException__object* 
  impl_sidl_PostViolation_fconnect_sidl_SIDLException(char* url,
  sidl_BaseInterface *_ex);
extern char* impl_sidl_PostViolation_fgetURL_sidl_SIDLException(struct 
  sidl_SIDLException__object* obj);
extern struct sidl_PostViolation__object* 
  impl_sidl_PostViolation_fconnect_sidl_PostViolation(char* url,
  sidl_BaseInterface *_ex);
extern char* impl_sidl_PostViolation_fgetURL_sidl_PostViolation(struct 
  sidl_PostViolation__object* obj);
extern struct sidl_ClassInfo__object* 
  impl_sidl_PostViolation_fconnect_sidl_ClassInfo(char* url,
  sidl_BaseInterface *_ex);
extern char* impl_sidl_PostViolation_fgetURL_sidl_ClassInfo(struct 
  sidl_ClassInfo__object* obj);
extern struct sidl_BaseInterface__object* 
  impl_sidl_PostViolation_fconnect_sidl_BaseInterface(char* url,
  sidl_BaseInterface *_ex);
extern char* impl_sidl_PostViolation_fgetURL_sidl_BaseInterface(struct 
  sidl_BaseInterface__object* obj);
extern struct sidl_BaseException__object* 
  impl_sidl_PostViolation_fconnect_sidl_BaseException(char* url,
  sidl_BaseInterface *_ex);
extern char* impl_sidl_PostViolation_fgetURL_sidl_BaseException(struct 
  sidl_BaseException__object* obj);
extern struct sidl_BaseClass__object* 
  impl_sidl_PostViolation_fconnect_sidl_BaseClass(char* url,
  sidl_BaseInterface *_ex);
extern char* impl_sidl_PostViolation_fgetURL_sidl_BaseClass(struct 
  sidl_BaseClass__object* obj);
#ifdef __cplusplus
}
#endif
#endif
