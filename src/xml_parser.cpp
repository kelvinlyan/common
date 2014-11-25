#include "xml_parser.h"

xml_parser::~xml_parser()
{
	if(_pDoc != NULL)
		delete _pDoc;
}

bool xml_parser::check_pDoc() const
{
	if(_pDoc == NULL)
	{
		_pDoc = new TiXmlDocument(_name.c_str());
		if(_pDoc == NULL)
			return false;
		_pDoc->LoadFile();
	}
	return true;
}

bool xml_parser::get_declaration(string& strVersion, string& strStandalone, string& strEncoding)
{
	if(!check_pDoc())
		return false;

	TiXmlNode* pNode = _pDoc->FirstChild();
	if(pNode != NULL)
	{
		TiXmlDeclaration* pDec = pNode->ToDeclaration();
		if(pDec != NULL)
		{
			strVersion = pDec->Version();
			strStandalone = pDec->Standalone();
			strEncoding = pDec->Encoding();
		}
	}
	return true;
}

bool xml_parser::get_first_node(const string& strNodeName, TiXmlElement*& pNode)
{
	if(!check_pDoc())
		return false;

	TiXmlElement* pRoot = _pDoc->RootElement();
	return _get_first_node(pRoot, strNodeName, pNode);
}

bool xml_parser::_get_first_node(TiXmlElement* pRoot, const string& strNodeName, TiXmlElement*& pNode)
{
	if(strNodeName == pRoot->Value())
	{
		pNode = pRoot;
		return true;
	}

	for(TiXmlElement* pEle = pRoot->FirstChildElement(); pEle; pEle = pEle->NextSiblingElement())
	{
		if(_get_first_node(pEle, strNodeName, pNode))
			return true;
	}
	return false;
}

bool xml_parser::print() const
{
	if(!check_pDoc())
		return false;
	
	_pDoc->Print();
	return true;
}

bool xml_parser::del_first_node(const string& strNodeName)
{
	if(!check_pDoc())
		return false;

	TiXmlElement* pNode = NULL;
	get_first_node(strNodeName, pNode);

	if(pNode == _pDoc->RootElement())
	{
		if(_pDoc->RemoveChild(pNode))
		{
			_pDoc->SaveFile();
			return true;
		}
		else
			return false;
	}

	if(pNode != NULL)
	{
		if(pNode->Parent() == NULL)
			return false;

		TiXmlElement* pParentEle = pNode->Parent()->ToElement();
		if(pParentEle != NULL)
		{
			if(pParentEle->RemoveChild(pNode))
			{
				_pDoc->SaveFile();
				return true;
			}
			else
				return false;
		}
	}
	return false;
}

bool xml_parser::get_all_node(const string& strNodeName, vector<TiXmlElement*>& pNodeVec)
{
	if(!check_pDoc())
		return false;

	TiXmlElement* pRoot = _pDoc->RootElement();
	_get_all_node(pRoot, strNodeName, pNodeVec);
	return !pNodeVec.empty();
}

void xml_parser::_get_all_node(TiXmlElement* pRoot, const string& strNodeName, vector<TiXmlElement*>& pNodeVec)
{
	if(strNodeName == pRoot->Value())
	{
		pNodeVec.push_back(pRoot);
	}

	for(TiXmlElement* pEle = pRoot->FirstChildElement(); pEle; pEle = pEle->NextSiblingElement())
	{
		_get_all_node(pEle, strNodeName, pNodeVec);
	}
}

bool xml_parser::modify_first_attribute(const string& strNodeName, map<string, string>& attrs)
{
	if(!check_pDoc())
		return false;
	
	TiXmlElement* pRoot = _pDoc->RootElement();
	TiXmlElement* pNode = NULL;
	if(!_get_first_node(pRoot, strNodeName, pNode))
		return false;

	for(TiXmlAttribute* pAttr = pNode->FirstAttribute(); pAttr; pAttr = pAttr->Next())
	{
		map<string, string>::iterator iter = attrs.find(pAttr->Name());
		if(iter != attrs.end())
			pAttr->SetValue(iter->second.c_str());
		else
			pNode->SetAttribute(iter->first.c_str(), iter->second.c_str());
	}

	_pDoc->SaveFile();

	return true;	
}

bool xml_parser::modify_all_attribute(const string& strNodeName, map<string, string>& attrs)
{
	if(!check_pDoc())
		return false;
	
	TiXmlElement* pRoot = _pDoc->RootElement();
	vector<TiXmlElement*> pNodeVec;
	_get_all_node(pRoot, strNodeName, pNodeVec);

	if(pNodeVec.empty())
		return false;

	for(vector<TiXmlElement*>::iterator iter = pNodeVec.begin(); iter != pNodeVec.end(); ++iter)
	{
		TiXmlElement* pNode = *iter;
		for(TiXmlAttribute* pAttr = pNode->FirstAttribute(); pAttr; pAttr = pAttr->Next())
		{
			map<string, string>::iterator it = attrs.find(pAttr->Name());
			if(it != attrs.end())
				pAttr->SetValue(it->second.c_str());
			else
				pNode->SetAttribute(it->first.c_str(), it->second.c_str());
		}
	}
	_pDoc->SaveFile();

	return true;	
}

bool xml_parser::modify_first_attribute_by_condition(const string& strNodeName, map<string, string>& condition, map<string, string>& attrs)
{
	if(!check_pDoc())
		return false;
	
	TiXmlElement* pRoot = _pDoc->RootElement();
	TiXmlElement* pNode = NULL;
	if(!_get_first_node_by_condition(pRoot, strNodeName, condition, pNode))
		return false;

	for(map<string, string>::iterator iter = attrs.begin(); iter != attrs.end(); ++iter)
	{
		pNode->SetAttribute(iter->first.c_str(), iter->second.c_str());
		pNode->SetAttribute(iter->first.c_str(), iter->second.c_str());
	}
	/*for(TiXmlAttribute* pAttr = pNode->FirstAttribute(); pAttr; pAttr = pAttr->Next())
	{
		map<string, string>::iterator iter = attrs.find(pAttr->Name());
		if(iter != attrs.end())
			pAttr->SetValue(iter->second.c_str());
	}*/
	

	_pDoc->SaveFile();

	return true;	
}

bool xml_parser::modify_all_attribute_by_condition(const string& strNodeName, map<string, string>& condition, map<string, string>& attrs)
{
	if(!check_pDoc())
		return false;
	
	TiXmlElement* pRoot = _pDoc->RootElement();
	vector<TiXmlElement*> pNodeVec;
	_get_all_node_by_condition(pRoot, strNodeName, condition, pNodeVec);

	if(pNodeVec.empty())
		return false;

	for(vector<TiXmlElement*>::iterator iter = pNodeVec.begin(); iter != pNodeVec.end(); ++iter)
	{
		TiXmlElement* pNode = *iter;
		for(TiXmlAttribute* pAttr = pNode->FirstAttribute(); pAttr; pAttr = pAttr->Next())
		{
			map<string, string>::iterator it = attrs.find(pAttr->Name());
			if(it != attrs.end())
				pAttr->SetValue(it->second.c_str());
			else
				pNode->SetAttribute(it->first.c_str(), it->second.c_str());
		}
	}
	_pDoc->SaveFile();

	return true;	
}


bool xml_parser::get_first_node_by_condition(const string& strNodeName, map<string, string>& condition, TiXmlElement*& pNode)
{
	if(!check_pDoc())
		return false;

	TiXmlElement* pRoot = _pDoc->RootElement();
	return _get_first_node_by_condition(pRoot, strNodeName, condition, pNode);
}

bool xml_parser::_get_first_node_by_condition(TiXmlElement* pRoot, const string& strNodeName, map<string, string>& condition, TiXmlElement*& pNode)
{
	if(strNodeName == pRoot->Value() && check_condition(pRoot, condition))
	{
		pNode = pRoot;
		return true;
	}

	for(TiXmlElement* pEle = pRoot->FirstChildElement(); pEle; pEle = pEle->NextSiblingElement())
	{
		if(_get_first_node_by_condition(pEle, strNodeName, condition, pNode))
			return true;
	}
	return false;
}

bool xml_parser::get_all_node_by_condition(const string& strNodeName, map<string, string>& condition, vector<TiXmlElement*>& pNodeVec)
{
	if(!check_pDoc())
		return false;

	TiXmlElement* pRoot = _pDoc->RootElement();
	_get_all_node_by_condition(pRoot, strNodeName, condition, pNodeVec);

	return !pNodeVec.empty();
}

void xml_parser::_get_all_node_by_condition(TiXmlElement* pRoot, const string& strNodeName, map<string, string>& condition, vector<TiXmlElement*>& pNodeVec)
{
	if(strNodeName == pRoot->Value() && check_condition(pRoot, condition))
	{
		pNodeVec.push_back(pRoot);
	}

	for(TiXmlElement* pEle = pRoot->FirstChildElement(); pEle; pEle = pEle->NextSiblingElement())
	{
		_get_all_node_by_condition(pEle, strNodeName, condition, pNodeVec);
	}
}

bool xml_parser::check_condition(TiXmlElement* pNode, map<string, string>& condition)
{
	for(map<string, string>::iterator iter = condition.begin(); iter != condition.end(); ++iter)
	{
		bool find = false;
		for(TiXmlAttribute* pAttr = pNode->FirstAttribute(); pAttr; pAttr = pAttr->Next())
		{
			if(pAttr->Name() == iter->first)
			{
				if(pAttr->Value() == iter->second)
				{
					find = true;
					break;
				}
				else
					return false;
			}
		}
		if(!find)
			return false;
	}
	return true;
}







