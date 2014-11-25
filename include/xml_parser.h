#ifndef _COMMON_XML_PARSER_H
#define _COMMON_XML_PARSER_H

#include "tinyxml/tinyxml.h"
#include "tinyxml/tinystr.h"
#include <string>
#include <vector>
#include <map>
#include <iostream>

using namespace std;

class xml_parser
{
	public:
		xml_parser(const string& name): _name(name), _pDoc(NULL){}
		~xml_parser();
		bool get_declaration(string& strVersion, string& strStandalone, string& strEncoding);
		bool get_first_node(const string& strNodeName, TiXmlElement*& pNode);
		bool get_all_node(const string& strNodeName, vector<TiXmlElement*>& pNodeVec);
		bool get_first_node_by_condition(const string& strNodeName, map<string, string>& condition, TiXmlElement*& pNode);
		bool get_all_node_by_condition(const string& strNodeName, map<string, string>& condition, vector<TiXmlElement*>& pNodeVec);
		bool del_first_node(const string& strNodeName);
		bool modify_first_attribute(const string& strNodeName, map<string, string>& attrs);
		bool modify_first_attribute_by_condition(const string& strNodeName, map<string, string>& condition, map<string, string>& attrs);
		bool modify_all_attribute(const string& strNodeName, map<string, string>& attrs);
		bool modify_all_attribute_by_condition(const string& strNodeName, map<string, string>& condition, map<string, string>& attrs);


		bool print() const;

	private:
		bool _get_first_node(TiXmlElement* pRoot, const string& strNodeName, TiXmlElement*& pNode);
		bool _get_first_node_by_condition(TiXmlElement* pRoot, const string& strNodeName, map<string, string>& condition, TiXmlElement*& pNode);
		void _get_all_node(TiXmlElement* pRoot, const string& strNodeName, vector<TiXmlElement*>& pNodeVec);
		void _get_all_node_by_condition(TiXmlElement* pRoot, const string& strNodeName, map<string, string>& condition, vector<TiXmlElement*>& pNodeVec);
		bool check_pDoc() const;
		bool check_condition(TiXmlElement* pNode, map<string, string>& condition);

	private:
		string _name;
		mutable TiXmlDocument* _pDoc;
};

#endif
