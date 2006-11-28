/* This file is generated from strigidaemonconfiguration.xsd */
#include <iostream>
#include <fstream>
#include <sstream>
#include "strigidaemonconfiguration.h"
std::string
read(const std::string& file) {
	std::stringbuf buf;
	std::ifstream f(file.c_str(), std::ios::binary);
	f.get(buf, '\0');
	f.close();
	return buf.str();
}
int
main() {
	std::string xml;
	int n = 1;
	std::ofstream f;

	std::ostringstream filename;

	filename << n++ << ".xml";
	xml = read(filename.str());
	Path path(xml);
	f.open(filename.str().c_str(), std::ios::binary);
	f << path;
	f.close();
	filename.str("");

	filename << n++ << ".xml";
	xml = read(filename.str());
	Repository repository(xml);
	f.open(filename.str().c_str(), std::ios::binary);
	f << repository;
	f.close();
	filename.str("");

	filename << n++ << ".xml";
	xml = read(filename.str());
	Pathfilter pathfilter(xml);
	f.open(filename.str().c_str(), std::ios::binary);
	f << pathfilter;
	f.close();
	filename.str("");

	filename << n++ << ".xml";
	xml = read(filename.str());
	Patternfilter patternfilter(xml);
	f.open(filename.str().c_str(), std::ios::binary);
	f << patternfilter;
	f.close();
	filename.str("");

	filename << n++ << ".xml";
	xml = read(filename.str());
	Filteringrules filteringrules(xml);
	f.open(filename.str().c_str(), std::ios::binary);
	f << filteringrules;
	f.close();
	filename.str("");

	filename << n++ << ".xml";
	xml = read(filename.str());
	Fylter fylter(xml);
	f.open(filename.str().c_str(), std::ios::binary);
	f << fylter;
	f.close();
	filename.str("");

	filename << n++ << ".xml";
	xml = read(filename.str());
	Filters filters(xml);
	f.open(filename.str().c_str(), std::ios::binary);
	f << filters;
	f.close();
	filename.str("");

	filename << n++ << ".xml";
	xml = read(filename.str());
	StrigiDaemonConfiguration strigidaemonconfiguration(xml);
	f.open(filename.str().c_str(), std::ios::binary);
	f << strigidaemonconfiguration;
	f.close();
	filename.str("");


	return 0;
}
