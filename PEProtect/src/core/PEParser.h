#pragma once

#include<vector>
#include<Windows.h>

#include"BasicPE.h"
#include"Utils.h"

#include <iostream>
#include <map>

class PEParser : public BasicPE {
private:

    std::vector<DLLtoImport> originalIAT;
    
    const std::vector<SectionInfo> parseSectionList(bool logSection = 0) const;

    void parseBlocksToVirt();

    DWORD tagVmStartRVA = 0, tagVmEndRVA = 0;
    
public:

    const std::vector<CodeBlockToVirt>& getBlocksToVirt();

    DWORD getVaFromExportTable(const char* funcName);
    
    SectionInfo& getSectionFromRVA(DWORD RVA);

    int getSectionIndexFromRVA(DWORD RVA);
    
	void read(const std::string& path, bool logSection = 0);

    ImportFunctionInfo getImportFuncFromRVA(DWORD RVA);

    DWORD getRvaImportFunc(const std::string& dllName, const std::string& funcName);

    const std::vector<DLLtoImport> getOriginalIAT() const;
};
