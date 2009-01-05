///////////////////////////////////////////////////////////
//  AbstractPortal.h
//  Implementation of the Class AbstractPortal
//  Created on:      04-Dec-2008 8:26:36 AM
//  Original author: Dave Loman
///////////////////////////////////////////////////////////

#if !defined(__ABSTRACTPORTAL_H__)
#define __ABSTRACTPORTAL_H__

#include <iostream>
#include <list>

using namespace std;

class AbstractPortal
{

public:
	AbstractPortal();
	virtual ~AbstractPortal();

private:
	int connectionType;
	string host;
	list <string> proxy_hosts;

};
#endif // !defined(__ABSTRACTPORTAL_H__)


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8