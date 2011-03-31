/*             M I N I M A L D A T A B A S E . C X X
 * BRL-CAD
 *
 * Copyright (c) 2011 United States Government as represented by
 * the U.S. Army Research Laboratory.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this file; see the file named COPYING for more
 * information.
 */
/** @file MinimalDatabase.cxx
 * MinimalDatabase.cxx
 *
 *  Created on: Mar 31, 2011
 *      Author: dloman
 */

#include "MinimalDatabase.h"
#include "raytrace.h"

// TODO should this defined here AND in ConstDatabase.cpp, or should they be combined in a header somewhere?
#if defined (_DEBUG)
#   define LOOKUP_NOISE LOOKUP_NOISY
#else
#   define LOOKUP_NOISE LOOKUP_QUIET
#endif

using namespace BRLCAD;

MinimalDatabase::MinimalDatabase() throw(bad_alloc) : MemoryDatabase() {

}

MinimalDatabase::~MinimalDatabase(void) throw() {
}

MinimalObject*
MinimalDatabase::getObjectByName(std::string name) {
	bu_external* ext = this->GetExternal(name.c_str());

	/* Check to see if the lookup succeeded or not. */
	if (ext == NULL) {
		return NULL;
	}

	return NULL;
}

std::list<MinimalObject*>*
MinimalDatabase::getAllObjectsBelow(std::string name) {
	return NULL;
}

std::list<MinimalObject*>*
MinimalDatabase::getAllObjects() {
	return NULL;
}

std::list<MinimalObject*>*
MinimalDatabase::getAllTopObjects() {
	return NULL;
}

bu_external*
MinimalDatabase::GetExternal
(
    const char*     objectName
) const {
	bu_external* ext = NULL;

    if (m_rtip != 0) {
        if (!BU_SETJUMP) {
            if ((objectName != 0) && (strlen(objectName) > 0)) {
                directory* pDir = db_lookup(m_rtip->rti_dbip, objectName, LOOKUP_NOISE);

                if (pDir != RT_DIR_NULL) {
                    //rt_db_internal intern;
                    //int id = rt_db_get_internal(&intern, pDir, m_rtip->rti_dbip, 0, m_resp);

                    /* Check to see if passed in ext was malloced */
                    if (ext == NULL)
                    	ext = (bu_external*)bu_calloc(sizeof(bu_external),1,"GetExternal bu_external calloc");

                	int rVal = db_get_external(ext, pDir, this->m_rtip->rti_dbip);

                	if (rVal < 0) {
                        BU_UNSETJUMP;
                		bu_free(ext, "Freeing bu_external due to error.");
                		return ext;
                	}
                   // rt_db_free_internal(&intern);
                }
            }
        }
        BU_UNSETJUMP;
    }
    return ext;
}


// Local Variables:
// tab-width: 8
// mode: C++
// c-basic-offset: 4
// indent-tabs-mode: t
// c-file-style: "stroustrup"
// End:
// ex: shiftwidth=4 tabstop=8
