#ifndef SEGMENTHELPER_H
#define SEGMENTHELPER_H
#pragma once

#include "Card.h"
#include "tinyxml/headers/tinyxml.h"

class CSegmentHelper
{
public:
	CSegmentHelper(void);
	CSegmentHelper(struct XmlProgramS *, struct CardOps *cardOps);
	CSegmentHelper(struct CardOps *cardOps);
	~CSegmentHelper(void);

	void setXmlProgramS(struct XmlProgramS *);
	void setCardOps(struct CardOps *);

	struct XmlColumnS  *CloneColmn(struct XmlColumnS *ColmnElement, int mode);

	struct XmlSegmentS *CloneSegment(struct XmlSegmentS *SegmentElement, int mode);

	int FindColumIDByColumName(struct XmlSegmentS *list, const char *name);

	int FindSegIDByColumName(struct XmlSegmentS *list ,const char *name);

	struct XmlSegmentS *FindSegmentByID(struct XmlSegmentS *listHead, int ID);

	struct XmlSegmentS *FindSegmentByColumName(struct XmlSegmentS *list ,const char *name);

	struct XmlSegmentS * getSegmentByColumName(struct XmlSegmentS *list ,const char *name);

	struct XmlColumnS* 	FindColumByColumName(struct XmlSegmentS *list, const char *name);

	struct XmlColumnS* getColumByColumName(struct XmlSegmentS *list, const char *name);

	struct XmlColumnS *FindColumnByID(struct XmlColumnS *listHead, int ID);

	struct XmlSegmentS * GetXmlSegmentByFlag(int flag);

private:
	struct XmlProgramS * m_XmlListHead;
	struct CardOps *	 m_CardOps;
};
#endif
