#pragma once
#ifndef _SHAPE_H_
#define _SHAPE_H_

#include <string>
#include <vector>
#include <memory>

class Program;

class Shape
{
public:
	Shape();
	virtual ~Shape();
	void loadMesh(const std::string &meshName);
	void init();
	void draw(const std::shared_ptr<Program> prog) const;
    
    std::vector<float> getPosBuf();
    std::vector<float> getNorBuf();
    
    void setPosBuf(std::vector<float> pos);
    void setNorBuf(std::vector<float> nor);
    
	
private:
	std::vector<unsigned int> eleBuf;
	std::vector<float> posBuf;
	std::vector<float> norBuf;
	std::vector<float> texBuf;
	unsigned eleBufID;
	unsigned posBufID;
	unsigned norBufID;
	unsigned texBufID;
};

#endif
