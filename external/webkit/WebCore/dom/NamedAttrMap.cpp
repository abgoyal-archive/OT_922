

#include "config.h"
#include "NamedNodeMap.h"

#include "Attr.h"
#include "Document.h"
#include "Element.h"
#include "ExceptionCode.h"
#include "HTMLNames.h"

namespace WebCore {

using namespace HTMLNames;

static inline bool shouldIgnoreAttributeCase(const Element* e)
{
    return e && e->document()->isHTMLDocument() && e->isHTMLElement();
}

inline void NamedNodeMap::detachAttributesFromElement()
{
    size_t size = m_attributes.size();
    for (size_t i = 0; i < size; i++) {
        if (Attr* attr = m_attributes[i]->attr())
            attr->m_element = 0;
    }
}

NamedNodeMap::~NamedNodeMap()
{
    detachAttributesFromElement();
}

bool NamedNodeMap::isMappedAttributeMap() const
{
    return false;
}

PassRefPtr<Node> NamedNodeMap::getNamedItem(const String& name) const
{
    Attribute* a = getAttributeItem(name, shouldIgnoreAttributeCase(m_element));
    if (!a)
        return 0;
    
    return a->createAttrIfNeeded(m_element);
}

PassRefPtr<Node> NamedNodeMap::getNamedItemNS(const String& namespaceURI, const String& localName) const
{
    return getNamedItem(QualifiedName(nullAtom, localName, namespaceURI));
}

PassRefPtr<Node> NamedNodeMap::removeNamedItem(const String& name, ExceptionCode& ec)
{
    Attribute* a = getAttributeItem(name, shouldIgnoreAttributeCase(m_element));
    if (!a) {
        ec = NOT_FOUND_ERR;
        return 0;
    }
    
    return removeNamedItem(a->name(), ec);
}

PassRefPtr<Node> NamedNodeMap::removeNamedItemNS(const String& namespaceURI, const String& localName, ExceptionCode& ec)
{
    return removeNamedItem(QualifiedName(nullAtom, localName, namespaceURI), ec);
}

PassRefPtr<Node> NamedNodeMap::getNamedItem(const QualifiedName& name) const
{
    Attribute* a = getAttributeItem(name);
    if (!a)
        return 0;

    return a->createAttrIfNeeded(m_element);
}

PassRefPtr<Node> NamedNodeMap::setNamedItem(Node* arg, ExceptionCode& ec)
{
    if (!m_element || !arg) {
        ec = NOT_FOUND_ERR;
        return 0;
    }

    // WRONG_DOCUMENT_ERR: Raised if arg was created from a different document than the one that created this map.
    if (arg->document() != m_element->document()) {
        ec = WRONG_DOCUMENT_ERR;
        return 0;
    }

    // Not mentioned in spec: throw a HIERARCHY_REQUEST_ERROR if the user passes in a non-attribute node
    if (!arg->isAttributeNode()) {
        ec = HIERARCHY_REQUEST_ERR;
        return 0;
    }
    Attr *attr = static_cast<Attr*>(arg);

    Attribute* a = attr->attr();
    Attribute* old = getAttributeItem(a->name());
    if (old == a)
        return RefPtr<Node>(arg); // we know about it already

    // INUSE_ATTRIBUTE_ERR: Raised if arg is an Attr that is already an attribute of another Element object.
    // The DOM user must explicitly clone Attr nodes to re-use them in other elements.
    if (attr->ownerElement()) {
        ec = INUSE_ATTRIBUTE_ERR;
        return 0;
    }

    if (attr->isId())
        m_element->updateId(old ? old->value() : nullAtom, a->value());

    // ### slightly inefficient - resizes attribute array twice.
    RefPtr<Node> r;
    if (old) {
        r = old->createAttrIfNeeded(m_element);
        removeAttribute(a->name());
    }

    addAttribute(a);
    return r.release();
}

// The DOM2 spec doesn't say that removeAttribute[NS] throws NOT_FOUND_ERR
// if the attribute is not found, but at this level we have to throw NOT_FOUND_ERR
// because of removeNamedItem, removeNamedItemNS, and removeAttributeNode.
PassRefPtr<Node> NamedNodeMap::removeNamedItem(const QualifiedName& name, ExceptionCode& ec)
{
    Attribute* a = getAttributeItem(name);
    if (!a) {
        ec = NOT_FOUND_ERR;
        return 0;
    }

    RefPtr<Attr> r = a->createAttrIfNeeded(m_element);

    if (r->isId())
        m_element->updateId(a->value(), nullAtom);

    removeAttribute(name);
    return r.release();
}

PassRefPtr<Node> NamedNodeMap::item(unsigned index) const
{
    if (index >= length())
        return 0;

    return m_attributes[index]->createAttrIfNeeded(m_element);
}

Attribute* NamedNodeMap::getAttributeItemSlowCase(const String& name, bool shouldIgnoreAttributeCase) const
{
    unsigned len = length();

    // Continue to checking case-insensitively and/or full namespaced names if necessary:
    for (unsigned i = 0; i < len; ++i) {
        const QualifiedName& attrName = m_attributes[i]->name();
        if (!attrName.hasPrefix()) {
            if (shouldIgnoreAttributeCase && equalIgnoringCase(name, attrName.localName()))
                return m_attributes[i].get();
        } else {
            // FIXME: Would be faster to do this comparison without calling toString, which
            // generates a temporary string by concatenation. But this branch is only reached
            // if the attribute name has a prefix, which is rare in HTML.
            if (equalPossiblyIgnoringCase(name, attrName.toString(), shouldIgnoreAttributeCase))
                return m_attributes[i].get();
        }
    }
    return 0;
}

void NamedNodeMap::clearAttributes()
{
    detachAttributesFromElement();
    m_attributes.clear();
}

void NamedNodeMap::detachFromElement()
{
    // This can't happen if the holder of the map is JavaScript, because we mark the
    // element if the map is alive. So it has no impact on web page behavior. Because
    // of that, we can simply clear all the attributes to avoid accessing stale
    // pointers to do things like create Attr objects.
    m_element = 0;
    clearAttributes();
}

void NamedNodeMap::setAttributes(const NamedNodeMap& other)
{
    // clone all attributes in the other map, but attach to our element
    if (!m_element)
        return;

    // If assigning the map changes the id attribute, we need to call
    // updateId.
    Attribute* oldId = getAttributeItem(m_element->idAttributeName());
    Attribute* newId = other.getAttributeItem(m_element->idAttributeName());

    if (oldId || newId)
        m_element->updateId(oldId ? oldId->value() : nullAtom, newId ? newId->value() : nullAtom);

    clearAttributes();
    unsigned newLength = other.length();
    m_attributes.resize(newLength);
    for (unsigned i = 0; i < newLength; i++)
        m_attributes[i] = other.m_attributes[i]->clone();

    // FIXME: This is wasteful.  The class list could be preserved on a copy, and we
    // wouldn't have to waste time reparsing the attribute.
    // The derived class, HTMLNamedNodeMap, which manages a parsed class list for the CLASS attribute,
    // will update its member variable when parse attribute is called.
    for (unsigned i = 0; i < newLength; i++)
        m_element->attributeChanged(m_attributes[i].get(), true);
}

void NamedNodeMap::addAttribute(PassRefPtr<Attribute> prpAttribute)
{
    RefPtr<Attribute> attribute = prpAttribute;
    
    // Add the attribute to the list
    m_attributes.append(attribute);

    if (Attr* attr = attribute->attr())
        attr->m_element = m_element;

    // Notify the element that the attribute has been added, and dispatch appropriate mutation events
    // Note that element may be null here if we are called from insertAttribute() during parsing
    if (m_element) {
        m_element->attributeChanged(attribute.get());
        // Because of our updateStyleAttribute() style modification events are never sent at the right time, so don't bother sending them.
        if (attribute->name() != styleAttr) {
            m_element->dispatchAttrAdditionEvent(attribute.get());
            m_element->dispatchSubtreeModifiedEvent();
        }
    }
}

void NamedNodeMap::removeAttribute(const QualifiedName& name)
{
    unsigned len = length();
    unsigned index = len;
    for (unsigned i = 0; i < len; ++i) {
        if (m_attributes[i]->name().matches(name)) {
            index = i;
            break;
        }
    }

    if (index >= len)
        return;

    // Remove the attribute from the list
    RefPtr<Attribute> attr = m_attributes[index].get();
    if (Attr* a = m_attributes[index]->attr())
        a->m_element = 0;

    m_attributes.remove(index);

    // Notify the element that the attribute has been removed
    // dispatch appropriate mutation events
    if (m_element && !attr->m_value.isNull()) {
        AtomicString value = attr->m_value;
        attr->m_value = nullAtom;
        m_element->attributeChanged(attr.get());
        attr->m_value = value;
    }
    if (m_element) {
        m_element->dispatchAttrRemovalEvent(attr.get());
        m_element->dispatchSubtreeModifiedEvent();
    }
}

bool NamedNodeMap::mapsEquivalent(const NamedNodeMap* otherMap) const
{
    if (!otherMap)
        return false;
    
    unsigned len = length();
    if (len != otherMap->length())
        return false;
    
    for (unsigned i = 0; i < len; i++) {
        Attribute *attr = attributeItem(i);
        Attribute *otherAttr = otherMap->getAttributeItem(attr->name());
            
        if (!otherAttr || attr->value() != otherAttr->value())
            return false;
    }
    
    return true;
}

}
