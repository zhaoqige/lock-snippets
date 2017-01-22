/*
 * xml2.c
 *
 *  Created on: May 17, 2016
 *      Author: qige
 */

#include <stdio.h>
#include <string.h>

#include "xml2.h"


//+ need xml2FreeDoc(pDoc);
XDOC xml2ParseMemory(const char *pXml, const unsigned int uXmlLength)
{
    return xmlParseMemory(pXml, uXmlLength);
}

XNODE xml2GetRootNode(XDOC pDoc)
{
    if (NULL != pDoc) {
        XNODE pRootNode;
        pRootNode = xmlDocGetRootElement(pDoc);
        return pRootNode;
    } else {
        return NULL;
    }
}


char *xml2GetNodeName(XNODE pNode)
{
    if (NULL != pNode) {
        return (char *) pNode->name;
    } else {
        return NULL;
    }
}

//+ need xml2FreeDoc(pAttr);
XCHAR *xml2GetNodeAttrValue(XNODE pNode, XCHAR *pAttr)
{
    if (NULL != pNode) {
        return xmlGetProp(pNode, BAD_CAST pAttr);
    } else {
        return NULL;
    }
}

XNODE xml2GetChildNode(XNODE pNode)
{
    if (NULL != pNode) {
        return pNode->xmlChildrenNode;
    } else {
        return NULL;
    }
}


XNODE xml2GetNodeNext(XNODE pNode)
{
    if (NULL != pNode) {
        return pNode->next;
    } else {
        return NULL;
    }
}

void xml2FreeDoc(XDOC pDoc)
{
    if (NULL != pDoc) {
        xmlFreeDoc(pDoc);
    }
}

void xml2FreeAttr(XCHAR *pAttr)
{
    if (NULL != pAttr) {
        xmlFree(pAttr);
    }
}
