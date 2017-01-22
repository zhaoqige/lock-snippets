/*
 * xml2.h
 *
 *  Created on: May 17, 2016
 *      Author: qige
 */

#ifndef XML2_H_
#define XML2_H_

#ifdef	__cplusplus
extern "C" {
#endif

#include <libxml/parser.h>

    //+ requires "libxml2", use "-lxml2" to compile
    typedef xmlChar      XCHAR;
    typedef xmlNodePtr   XNODE;
    typedef xmlDocPtr    XDOC;

    //+ need xml2FreeDoc(pDoc);
    XDOC  xml2ParseMemory(const char *xml, const unsigned int xmlLength);

    XNODE xml2GetRootNode(XDOC pDoc);
    XNODE xml2GetChildNode(XNODE pNode);
    XNODE xml2GetNodeNext(XNODE pNode);

    char *xml2GetNodeName(XNODE pNode);

    //+ need xml2Free(pAttr);
    XCHAR *xml2GetNodeAttrValue(XNODE pNode, XCHAR *pAttr);

    void  xml2FreeDoc(XDOC pDoc);
    void  xml2FreeAttr(XCHAR *pAttr);

#ifdef	__cplusplus
}
#endif

#endif /* XML2_H_ */
