/***************************************************************************
                          gnomexml.cpp  -  XML Helper
                             -------------------
    begin                : sam mai 17 2003
    copyright            : (C) 2003 by Michael CATANZARITI
    email                : mcatan@free.fr
 ***************************************************************************/

/***************************************************************************
* Copyright (C) The Apache Software Foundation. All rights reserved.      *
*                                                                         *
* This software is published under the terms of the Apache Software       *
* License version 1.1, a copy of which has been included with this        *
* distribution in the LICENSE.txt file.                                   *
***************************************************************************/

#include <log4cxx/config.h>

#ifdef HAVE_LIBXML2

#include <log4cxx/helpers/gnomexml.h>
#include <log4cxx/helpers/loglog.h>

using namespace log4cxx;
using namespace log4cxx::helpers;

IMPLEMENT_LOG4CXX_OBJECT(GnomeXMLDOMDocument)
IMPLEMENT_LOG4CXX_OBJECT(GnomeXMLDOMNodeList)
IMPLEMENT_LOG4CXX_OBJECT(GnomeXMLDOMNode)
IMPLEMENT_LOG4CXX_OBJECT(GnomeXMLDOMElement)

// GnomeXMLDOMNode

GnomeXMLDOMNode::GnomeXMLDOMNode(xmlNodePtr node)
: node(node)
{
}

XMLDOMNodeListPtr GnomeXMLDOMNode::getChildNodes()
{
	if (node == 0) throw DOMException();
	return new GnomeXMLDOMNodeList(node->children);
}

XMLDOMDocumentPtr GnomeXMLDOMNode::getOwnerDocument()
{
	if (node == 0) throw DOMException();
	return new GnomeXMLDOMDocument(node->doc);
}

// GnomeXMLDOMDocument

GnomeXMLDOMDocument::GnomeXMLDOMDocument(xmlDocPtr document)
: document(document), ownDocument(false)
{
}

GnomeXMLDOMDocument::GnomeXMLDOMDocument()
: document(0), ownDocument(false)
{
}

GnomeXMLDOMDocument::~GnomeXMLDOMDocument()
{
	if (ownDocument)
	{
		::xmlFreeDoc(document);
	}
}

XMLDOMNodeListPtr GnomeXMLDOMDocument::getChildNodes()
{
	if (document == 0) throw DOMException();
	return new GnomeXMLDOMNodeList(::xmlDocGetRootElement(document));
}

XMLDOMDocumentPtr GnomeXMLDOMDocument::getOwnerDocument()
{
	return this;
}

void GnomeXMLDOMDocument::load(const tstring& fileName)
{
	if (document != 0)
	{
		if (ownDocument)
		{
			::xmlFreeDoc(document);
		}
		document = 0;
	}

	USES_CONVERSION;
	document = ::xmlParseFile(T2A(fileName.c_str()));

	if (document == 0)
	{
		LogLog::error(_T("Could not open [")+fileName+_T("]."));
	}
	else
	{
		ownDocument = true;
	}
}

XMLDOMElementPtr GnomeXMLDOMDocument::getDocumentElement()
{
	if (document == 0) throw DOMException();
	xmlNodePtr element = ::xmlDocGetRootElement(document);
	return new GnomeXMLDOMElement(element);
}

XMLDOMElementPtr GnomeXMLDOMDocument::getElementById(const tstring& tagName, const tstring& elementId)
{
	if (document == 0) throw DOMException();
	USES_CONVERSION;
	xmlNodePtr node = ::xmlDocGetRootElement(document);

	while (node != 0)
	{
		if (node->type == XML_ELEMENT_NODE
			&& tagName == A2T((char *)node->name))
		{
			char * attributeValue = (char *)xmlGetProp(
				node, (const xmlChar *)"name");
			if (attributeValue != 0
				&& elementId == A2T(attributeValue))
			{
				return new GnomeXMLDOMElement(node);
			}
		}

		if (node->children != 0)
		{
			node = node->children;
		}
		else if (node->next != 0)
		{
			node = node->next;
		}
		else
		{
			node = node->parent->next;
		}
	}

	return 0;
}

// GnomeXMLDOMElement
GnomeXMLDOMElement::GnomeXMLDOMElement(xmlNodePtr element)
: element(element)
{
}

XMLDOMNodeListPtr GnomeXMLDOMElement::getChildNodes()
{
	if (element == 0) throw DOMException();
	return new GnomeXMLDOMNodeList(element->children);
}

XMLDOMDocumentPtr GnomeXMLDOMElement::getOwnerDocument()
{
	if (element == 0) throw DOMException();
	return new GnomeXMLDOMDocument(element->doc);
}

tstring GnomeXMLDOMElement::getTagName()
{
	if (element == 0) throw DOMException();
	USES_CONVERSION;
	return A2T((char *)element->name);
}

tstring GnomeXMLDOMElement::getAttribute(const tstring& name)
{
	if (element == 0) throw DOMException();
	USES_CONVERSION;
	char * attributeValue = (char *)xmlGetProp(
		element, (const xmlChar *)T2A(name.c_str()));
	return (attributeValue == 0) ? tstring() : A2T(attributeValue);
}

// GnomeXMLDOMNodeList	
GnomeXMLDOMNodeList::GnomeXMLDOMNodeList(xmlNodePtr firstChild)
: firstChild(firstChild), currentChild(firstChild), currentIndex(0)
{
}

int GnomeXMLDOMNodeList::getLength()
{
	xmlNodePtr child = firstChild;
	int length = 0;
	while (child != 0)
	{
		child = child->next;
		length++;
	}

	return length;
}

XMLDOMNodePtr GnomeXMLDOMNodeList::item(int index)
{
	xmlNodePtr child = 0;

	if (index == currentIndex)
	{
		child = currentChild;
	}
	else
	{
		child = firstChild;
		int n = 0;
		while (child != 0 && n < index)
		{
			child = child->next;
			n++;
		}
	}

	currentIndex = index + 1;
	currentChild = child ? child->next : 0;

	if (child != 0)
	{
		if (child->type == XML_ELEMENT_NODE)
		{
			return new GnomeXMLDOMElement(child);
		}
		else
		{
			return new GnomeXMLDOMNode(child);
		}
	}
	else
	{
		return 0;
	}
}

#endif // HAVE_LIBXML2
